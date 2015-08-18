////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_actors.cpp
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \author  Corey Hoffstein, visage@allacrost.org
*** \author  Andy Gardner, chopperdave@allacrost.org
*** \brief   Source file for actors present in battles.
*** ***************************************************************************/

#include "input.h"
#include "script.h"

#include "battle.h"
#include "battle_actions.h"
#include "battle_actors.h"
#include "battle_command.h"
#include "battle_effects.h"
#include "battle_indicators.h"
#include "battle_utils.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_script;

namespace hoa_battle {

namespace private_battle {

////////////////////////////////////////////////////////////////////////////////
// BattleActor class
////////////////////////////////////////////////////////////////////////////////

BattleActor::BattleActor(GlobalActor* actor) :
	GlobalActor(*actor),
	_state(ACTOR_STATE_INVALID),
	_global_actor(actor),
	_action(NULL),
	_x_origin(0.0f),
	_y_origin(0.0f),
	_x_location(0.0f),
	_y_location(0.0f),
	_execution_finished(false),
	_state_paused(false),
	_idle_state_time(0),
	_animation_timer(0),
	_effects_supervisor(new EffectsSupervisor(this)),
	_indicator_supervisor(new IndicatorSupervisor(this))
{
	if (actor == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "constructor received NULL argument" << endl;
		return;
	}

	// TODO: I have concerns about the copy constructor for GlobalActor. Currently it creates a copy
	// of every single attack point, weapon, armor, and skill. I wonder if perhaps we should only
	// create a copy of the attack point
}



BattleActor::~BattleActor() {
	// If the actor did not get a chance to execute their action, delete it
	if (_action != NULL) {
		delete _action;
		_action = NULL;
	}

	delete _effects_supervisor;
	delete _indicator_supervisor;
}



void BattleActor::ResetActor() {
	_effects_supervisor->RemoveAllStatus();

	ResetHitPoints();
	ResetHitPointFatigue();
	ResetSkillPoints();
	ResetSkillPointFatigue();
	ResetStrength();
	ResetVigor();
	ResetFortitude();
	ResetProtection();
	ResetStamina();
	ResetResilience();
	ResetAgility();
	ResetEvade();

	ChangeState(ACTOR_STATE_INVALID);
	ChangeState(ACTOR_STATE_IDLE);
}



void BattleActor::ChangeState(ACTOR_STATE new_state) {
	if (_state == new_state) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "actor was already in new state: " << new_state << endl;
		return;
	}

	_state = new_state;
	_state_timer.Reset();
	switch (_state) {
		case ACTOR_STATE_IDLE:
			if (_action != NULL) {
				delete _action;
				_action = NULL;
			}
			_state_timer.Initialize(_idle_state_time);
			_state_timer.Run();
			break;
		case ACTOR_STATE_WARM_UP:
			if (_action == NULL) {
				IF_PRINT_WARNING(BATTLE_DEBUG) << "no action available during state change: " << _state << endl;
			}
			else {
				_state_timer.Initialize(_action->GetWarmUpTime());
				_state_timer.Run();
			}
			break;
		case ACTOR_STATE_READY:
			if (_action == NULL) {
				IF_PRINT_WARNING(BATTLE_DEBUG) << "no action available during state change: " << _state << endl;
			}
			else {
				BattleMode::CurrentInstance()->NotifyActorReady(this);
			}
			break;
		case ACTOR_STATE_DEAD:
			_effects_supervisor->RemoveAllStatus();
			BattleMode::CurrentInstance()->NotifyActorDeath(this);
			break;
		default:
			break;
	}
}



void BattleActor::RegisterDamage(uint32 amount) {
	RegisterDamage(amount, NULL);
}



void BattleActor::RegisterDamage(uint32 amount, BattleTarget* target) {
	if (amount == 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function called with a zero value argument" << endl;
		RegisterMiss();
		return;
	}
	if (_state == ACTOR_STATE_DEAD) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function called when actor state was dead" << endl;
		RegisterMiss();
		return;
	}

	SubtractHitPoints(amount);
	_indicator_supervisor->AddDamageIndicator(amount);
	uint32 fatigue_damage = amount / GetStamina();
	if (fatigue_damage > 0) {
		AddHitPointFatigue(fatigue_damage); // This call also subtracts the amount from the active max HP
	}

	if (GetHitPoints() == 0) {
		ChangeState(ACTOR_STATE_DEAD);
	}

	// Apply a stun to the actor timer depending on the amount of damage dealt
	float damage_percent = static_cast<float>(amount) / static_cast<float>(GetMaxHitPoints());
	if (damage_percent < 0.10f) {
		_state_timer.StunTimer(250);
	}
	else if (damage_percent < 0.25f) {
		_state_timer.StunTimer(500);
	}
	else if (damage_percent < 0.50f) {
		_state_timer.StunTimer(750);
	}
	else { // (damage_percent >= 0.50f)
		_state_timer.StunTimer(1000);
	}

	// If the damage dealt was to a point target type, check for and apply any status effects triggered by this point hit
	if ((target != NULL) && (IsTargetPoint(target->GetType()) == true)) {
		GlobalAttackPoint* damaged_point = _global_actor->GetAttackPoint(target->GetPoint());
		if (damaged_point == NULL) {
			IF_PRINT_WARNING(BATTLE_DEBUG) << "target argument contained an invalid point index: " << target->GetPoint() << endl;
		}
		else {
			vector<pair<GLOBAL_STATUS, float> > status_effects = damaged_point->GetStatusEffects();
			for (vector<pair<GLOBAL_STATUS, float> >::const_iterator i = status_effects.begin(); i != status_effects.end(); i++) {
				if (RandomFloat(0.0f, 100.0f) <= i->second) {
					RegisterStatusChange(i->first, GLOBAL_INTENSITY_POS_LESSER);
				}
			}
		}
	}
}



void BattleActor::RegisterHealing(uint32 amount) {
	if (amount == 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function called with a zero value argument" << endl;
		RegisterMiss();
		return;
	}
	if (_state == ACTOR_STATE_DEAD) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function called when actor state was dead" << endl;
		RegisterMiss();
		return;
	}

	AddHitPoints(amount);
	_indicator_supervisor->AddHealingIndicator(amount);
}



void BattleActor::RegisterMiss() {
	_indicator_supervisor->AddMissIndicator();
}



void BattleActor::RegisterStatusChange(GLOBAL_STATUS status, GLOBAL_INTENSITY intensity) {
	GLOBAL_STATUS old_status = GLOBAL_STATUS_INVALID;
	GLOBAL_STATUS new_status = GLOBAL_STATUS_INVALID;
	GLOBAL_INTENSITY old_intensity = GLOBAL_INTENSITY_INVALID;
	GLOBAL_INTENSITY new_intensity = GLOBAL_INTENSITY_INVALID;

	bool status_change_occurred = _effects_supervisor->ChangeStatus(status, intensity, old_status, old_intensity, new_status, new_intensity);

	// If a status change indeed occurred, add the appropriate indicator to display the change to the player
	if (status_change_occurred == true) {
		_indicator_supervisor->AddStatusIndicator(old_status, old_intensity, new_status, new_intensity);
	}
}



void BattleActor::RegisterSkillPointsConsumed(uint32 amount) {
	if (amount == 0 || IsAlive() == false) {
		return;
	}

	SubtractSkillPoints(amount);
	// TODO: SP indicator change text needs to be implemented
// 	_indicator_supervisor->AddSkillPointConsumedIndicator(amount);
	uint32 fatigue_damage = amount / GetResilience();
	if (fatigue_damage > 0) {
		AddSkillPointFatigue(fatigue_damage); // This call also subtracts the amount from the active max SP
	}
}



void BattleActor::Update(bool animation_only) {
	if ((_state_paused == false) && (animation_only == false))
		_state_timer.Update();

	_effects_supervisor->Update();
	_indicator_supervisor->Update();

	if (_state_timer.IsFinished() == true) {
		if (_state == ACTOR_STATE_IDLE) {
			// If an action is already set for the actor, skip the command state and immediately begin the warm up state
			if (_action == NULL)
				ChangeState(ACTOR_STATE_COMMAND);
			else
				ChangeState(ACTOR_STATE_WARM_UP);
		}
		else if (_state == ACTOR_STATE_WARM_UP) {
			ChangeState(ACTOR_STATE_READY);
		}
	}
}



void BattleActor::DrawIndicators() const {
	_indicator_supervisor->Draw();
}



void BattleActor::SetAction(BattleAction* action) {
	if (action == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL argument" << endl;
		return;
	}
	if (_action != NULL) {
		// Note: we do not display any warning if we are overwriting a previously set action in idle or command states. This is a valid operation in those states
		if ((_state != ACTOR_STATE_IDLE) && (_state != ACTOR_STATE_COMMAND)) {
			IF_PRINT_WARNING(BATTLE_DEBUG) << "overwriting previously set action while in actor state: " << _state << endl;
		}
		delete _action;
	}

	_action = action;
}



uint32 BattleActor::TotalPhysicalDefense() {
	uint32 phys_defense = 0;

	for (uint32 i = 0; i < _attack_points.size(); i++)
		phys_defense += _attack_points[i]->GetTotalPhysicalDefense();
	phys_defense /= _attack_points.size();

	return phys_defense;
}



uint32 BattleActor::TotalEtherealDefense() {
	uint32 eth_defense = 0;

	for (uint32 i = 0; i < _attack_points.size(); i++)
		eth_defense += _attack_points[i]->GetTotalEtherealDefense();
	eth_defense /= _attack_points.size();

	return eth_defense;
}



float BattleActor::TotalEvadeRating() {
	float evade = 0.0f;

	for (uint32 i = 0; i < _attack_points.size(); i++)
		evade += _attack_points[i]->GetTotalEvadeRating();
	evade /= static_cast<float>(_attack_points.size());

	return evade;
}

////////////////////////////////////////////////////////////////////////////////
// BattleCharacter class
////////////////////////////////////////////////////////////////////////////////

BattleCharacter::BattleCharacter(GlobalCharacter* character) :
	BattleActor(character),
	_global_character(character),
	_last_rendered_hp(0),
	_last_rendered_sp(0),
	_sprite_animation_alias("idle")
{
	string icon_filename = "img/icons/actors/characters/" + character->GetFilename() + ".png";
	if (DoesFileExist(icon_filename) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "character icon image file did not exist: " << icon_filename << endl;
		_stamina_icon.Load("", 45.0f, 45.0f); // Load an empty image file
	}
	else {
		_stamina_icon.Load(icon_filename, 45.0f, 45.0f);
	}

	_last_rendered_hp = GetHitPoints();
	_last_rendered_sp = GetSkillPoints();

	_name_text.SetStyle(TextStyle("title22"));
	_name_text.SetText(GetName());
	_hit_points_text.SetStyle(TextStyle("text24", VIDEO_TEXT_SHADOW_BLACK));
	_hit_points_text.SetText(NumberToString(_last_rendered_hp));
	_skill_points_text.SetStyle(TextStyle("text24", VIDEO_TEXT_SHADOW_BLACK));
	_skill_points_text.SetText(NumberToString(_last_rendered_sp));

	_action_selection_text.SetStyle(TextStyle("text20"));
	_action_selection_text.SetText("");
	_target_selection_text.SetStyle(TextStyle("text20"));
	_target_selection_text.SetText("");
}



BattleCharacter::~BattleCharacter() {
	// If character was about to use an item before being destructed, restore it to inventory
	if ((_action != NULL) && (_action->IsItemAction() == true)) {
		// TODO: not sure if this is necessary/safe to do.
// 		ItemAction* item_action = dynamic_cast<ItemAction*>(_action);
// 		item_action->GetItem()->IncrementCount(1);
	}
}



void BattleCharacter::ResetActor() {
	BattleActor::ResetActor();

	_global_character->RetrieveBattleAnimation("idle")->GetCurrentFrame()->DisableGrayScale();
}



void BattleCharacter::ChangeState(ACTOR_STATE new_state) {
	ACTOR_STATE old_state = _state;
	BattleActor::ChangeState(new_state);

	switch (_state) {
		case ACTOR_STATE_IDLE:
			// Regenerate a small portion of SP so long as we are not entering from the invalid state (which indicates the start of the battle)
			// TODO: check and make sure that actor states are restored to invalid when a battle retries, otherwise
			// a battle retry might start off with granting an initial SP regeneration boost
			if (old_state != ACTOR_STATE_INVALID) {
				// Regeneration amount is based on the active SP max, not the full max. The character will always regenerate at least 1 SP
				uint32 sp_regeneration = GetActiveMaxSkillPoints() / CHARACTER_SP_REGENERATION_RATE;
				if (sp_regeneration == 0)
					sp_regeneration = 1;
				AddSkillPoints(sp_regeneration);
				// TODO: add indicator text indicating the regeneration amount
			}
		case ACTOR_STATE_COMMAND:
			// When the "wait" setting is active in battle mode we want the command menu to be brought up for the character as soon as we can when the character
			// enters this state. This is done within the BattleMode::Update() method
			break;
		case ACTOR_STATE_WARM_UP:
			// BattleActor::Update() changes to the warm up state if the actor has an action set when the idle time is expired. However for characters, we do not
			// want to proceed forward in this case if the player is currently setting a different action for that same character. Instead we place the character
			// in the command state and wait until the player exits the command menu before moving on to the warm up state.
			if (BattleMode::CurrentInstance()->GetCommandSupervisor()->GetCommandCharacter() == this)
				ChangeState(ACTOR_STATE_COMMAND);
			break;
		case ACTOR_STATE_ACTING:
			// TODO: reset state timer?
			break;
		case ACTOR_STATE_DEAD:
			ChangeSpriteAnimation("idle");
			_global_character->RetrieveBattleAnimation("idle")->GetCurrentFrame()->EnableGrayScale();
			break;
		default:
			break;
	}

	// The action/target text for the character is always updated when the character's state changes. Technically we do not need to update
	// this text display for every possible state change, but we do it anyway just to be safe and to not add unnecessary code complexity.
	ChangeActionText();
}



void BattleCharacter::Update(bool animation_only) {
	BattleActor::Update(animation_only);

	if (_state_paused == false) {
		_animation_timer.Update();

		// Update the active sprite animation
		if (IsAlive() == true) {
			_global_character->RetrieveBattleAnimation(_sprite_animation_alias)->Update();
		}

		// Do no further update action if we are only supposed to update animations
		if (animation_only == true) {
			return;
		}

		// If the character is executing their action,
		if (_state == ACTOR_STATE_ACTING) {
			if (_action->Execute() == true) {
				ChangeState(ACTOR_STATE_IDLE);
			}
		}
	}
}



void BattleCharacter::DrawSprite() {
	// Draw the character sprite
	VideoManager->Move(_x_location, _y_location);

	if (_sprite_animation_alias == "idle") {
		// no need to do anything
	}
	else if (_sprite_animation_alias == "run") {
		// no need to do anything
	}
	else if (_animation_timer.IsFinished()) {
		ChangeSpriteAnimation("idle");
	}
	else {
		uint32 dist = _animation_timer.GetDuration() > 0 ? 120 * _animation_timer.GetTimeExpired() / _animation_timer.GetDuration() : 0;
		VideoManager->MoveRelative(dist, 0.0f);
	}

	_global_character->RetrieveBattleAnimation(_sprite_animation_alias)->Draw();
} // void BattleCharacter::DrawSprite()



void BattleCharacter::ChangeSpriteAnimation(const std::string& alias) {
	_sprite_animation_alias = alias;
	_global_character->RetrieveBattleAnimation(_sprite_animation_alias)->ResetAnimation();
	_animation_timer.Reset();
	_animation_timer.SetDuration(_global_character->RetrieveBattleAnimation(_sprite_animation_alias)->GetAnimationLength());
	_animation_timer.Run();
}



void BattleCharacter::ChangeActionText() {
	// If the character has no action selected to be used, clear both action and target text
	if (_action == NULL) {
		// If the character is able to have an action selected, notify the player
		if ((_state == ACTOR_STATE_IDLE) || (_state == ACTOR_STATE_COMMAND)) {
			_action_selection_text.SetText(Translate("[Select Action]"));
		}
		else {
			_action_selection_text.SetText("");
		}
		_target_selection_text.SetText("");
	}

	else {
		_action_selection_text.SetText(_action->GetName());
		_target_selection_text.SetText(_action->GetTarget().GetName());
	}
}



void BattleCharacter::DrawPortrait() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(5.0f, 10.0f);

	vector<StillImage>& portrait_frames = *(_global_character->GetBattlePortraits());
	float hp_percent =  static_cast<float>(GetHitPoints()) / static_cast<float>(GetMaxHitPoints());

	if (GetHitPoints() == GetMaxHitPoints()) {
		portrait_frames[0].Draw();
	}
	else if (GetHitPoints() == 0) {
		portrait_frames[4].Draw();
	}
	else if (hp_percent > 0.75f) {
		portrait_frames[0].Draw();
		float alpha = 1.0f - ((hp_percent - 0.75f) * 4.0f);
		portrait_frames[1].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else if (hp_percent > 0.50f) {
		portrait_frames[1].Draw();
		float alpha = 1.0f - ((hp_percent - 0.50f) * 4.0f);
		portrait_frames[2].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else if (hp_percent > 0.25f) {
		portrait_frames[2].Draw();
		float alpha = 1.0f - ((hp_percent - 0.25f) * 4.0f);
		portrait_frames[3].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else { // (hp_precent > 0.0f)
		portrait_frames[3].Draw();
		float alpha = 1.0f - (hp_percent * 4.0f);
		portrait_frames[4].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
}



void BattleCharacter::DrawStatus(uint32 order) {
	// X and Y position constants that determine where the various elements are drawn
	const float TOP_CHARACTER_YPOS = 105.0f;
	const float NAME_RIGHT_ALIGN_XPOS = 240.0f;
	const float HPSP_BAR_LEFT_XPOS = NAME_RIGHT_ALIGN_XPOS + 26.0f;
	const float HPSP_BAR_OFFSET_YPOS = 7.0f;
	const float HP_TEXT_XPOS = 450.0f;
	const float SP_TEXT_XPOS = HP_TEXT_XPOS + 50.0f;
	const float COMMAND_ICON_XPOS = 545.0f;

	// Used to determine where to draw the character's status
	float y_position = 0.0f;

	// Colors used for the HP/SP bars
	const Color green_hp(0.294f, 0.776f, 0.184f, 1.0f);
	const Color darkgreen_hp(0.110f, 0.388f, 0.090f, 1.0f);
	const Color blue_sp(0.196f, 0.522f, 0.859f, 1.0f);
	const Color darkblue_sp(0.110f, 0.430f, 0.455f, 1.0f);
	const Color yellow_indicator = Color::yellow;

	// Determine what vertical order the character is in and set the y_position accordingly
	switch (order) {
		case 0:
			y_position = TOP_CHARACTER_YPOS;
			break;
		case 1:
			y_position = TOP_CHARACTER_YPOS - 30.0f;
			break;
		case 2:
			y_position = TOP_CHARACTER_YPOS - 60.0f;
			break;
		case 3:
			y_position = TOP_CHARACTER_YPOS - 90.0f;
			break;
		default:
			IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid order argument: " << order << endl;
			y_position = TOP_CHARACTER_YPOS;
	}

	// Draw the character's name
	VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_CENTER, VIDEO_BLEND, 0);
	VideoManager->Move(NAME_RIGHT_ALIGN_XPOS, y_position);
	_name_text.Draw();

	// If the swap key is being held down, draw status icons
	if (InputManager->SwapState()) {
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_BLEND, 0);
		VideoManager->MoveRelative(20.0f, 0.0f);
		_effects_supervisor->Draw();
	}

	// Otherwise, draw the HP and SP bars (The full bars are 90 pixels wide and 6 pixels high)
	else {
		float bar_size;
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_NO_BLEND, 0);

		// Draw the HP bar in green
		bar_size = static_cast<float>(90 * GetHitPoints()) / static_cast<float>(GetMaxHitPoints());
		VideoManager->Move(HPSP_BAR_LEFT_XPOS, y_position + HPSP_BAR_OFFSET_YPOS);

		if (GetHitPoints() > 0) {
			VideoManager->DrawRectangle(bar_size, 6, green_hp);
		}

		// If the current HP is less than the active max HP, draw a dark green bar to display where the active maximum HP value currently is.
		// The area of the bar between the active max HP and the full max HP (caused by HP fatigue) will remain black
		if (GetHitPoints() < GetActiveMaxHitPoints()) {
			VideoManager->Move(HPSP_BAR_LEFT_XPOS + bar_size, y_position + HPSP_BAR_OFFSET_YPOS);
			bar_size = static_cast<float>(90 * (GetActiveMaxHitPoints() - GetHitPoints())) / static_cast<float>(GetMaxHitPoints());
			VideoManager->DrawRectangle(bar_size, 6.0f, darkgreen_hp);

			// Draw a think yellow line to indicate where the active max HP is at
			VideoManager->Move(HPSP_BAR_LEFT_XPOS + static_cast<float>((90 * GetActiveMaxHitPoints()) / static_cast<float>(GetMaxHitPoints())), y_position + HPSP_BAR_OFFSET_YPOS);
			VideoManager->DrawRectangle(1.0f, 6.0f, yellow_indicator);
		}

		// Draw the SP bar in blue
		bar_size = static_cast<float>(90 * GetSkillPoints()) / static_cast<float>(GetMaxSkillPoints());
		VideoManager->Move(HPSP_BAR_LEFT_XPOS, y_position - HPSP_BAR_OFFSET_YPOS);

		if (GetSkillPoints() > 0) {
			VideoManager->DrawRectangle(bar_size, 6.0f, blue_sp);
		}

		// If the current SP is less than at the active max SP, draw a dark blue bar to display where the active maximum SP value currently is.
		// The area of the bar between the active max SP and the full max SP (caused by SP fatigue) will remain black
		if (GetSkillPoints() < GetActiveMaxSkillPoints()) {
			VideoManager->Move(HPSP_BAR_LEFT_XPOS + bar_size, y_position - HPSP_BAR_OFFSET_YPOS);
			bar_size = static_cast<float>(90 * (GetActiveMaxSkillPoints() - GetSkillPoints())) / static_cast<float>(GetMaxSkillPoints());
			VideoManager->DrawRectangle(bar_size, 6.0f, darkblue_sp);

			// Draw a think yellow line to indicate where the active max SP is at
			VideoManager->Move(HPSP_BAR_LEFT_XPOS + static_cast<float>((90 * GetActiveMaxSkillPoints()) / static_cast<float>(GetMaxSkillPoints())), y_position - HPSP_BAR_OFFSET_YPOS);
			VideoManager->DrawRectangle(1.0f, 6.0f, yellow_indicator);
		}

		// Draw the cover image over the top of both the HP and SP bars
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->Move(NAME_RIGHT_ALIGN_XPOS + 8.0f, y_position);
		BattleMode::CurrentInstance()->GetMedia().character_bar_covers.Draw();

		// TODO: The SetText calls below should not be done here. They should be made whenever the character's HP/SP
		// is modified. This re-renders the text every frame regardless of whether or not the HP/SP changed so its
		// not efficient

		// Draw the character's current health and skill points text
		VideoManager->SetDrawFlags(VIDEO_X_RIGHT, 0);
		VideoManager->Move(HP_TEXT_XPOS, y_position);
		_hit_points_text.Draw();
		VideoManager->Move(SP_TEXT_XPOS, y_position);
		_skill_points_text.Draw();

		// Update hit and skill points after drawing to reduce gpu stall
		if (_last_rendered_hp != GetHitPoints()) {
			_last_rendered_hp = GetHitPoints();
			_hit_points_text.SetText(NumberToString(_last_rendered_hp));
		}

		if (_last_rendered_sp != GetSkillPoints()) {
			_last_rendered_sp = GetSkillPoints();
			_skill_points_text.SetText(NumberToString(_last_rendered_sp));
		}
	}

	// Note: if the command menu is visible, it will be drawn over all of the components that follow below. We still perform these draw calls
	// regardless because sometimes even if the battle is in the command state, the command menu may not be drawn if a dialogue is active or if
	// a scripted scene is taking place. Its easier (and not costly) to just always draw this information rather than check for all possible
	// conditions where the command menu is not drawn.
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_CENTER, VIDEO_BLEND, 0);

	// Move to the position where command button icons are drawn
	VideoManager->Move(COMMAND_ICON_XPOS, y_position);

	// If this character can be issued a command, draw the appropriate command button to indicate this. The type of button drawn depends on
	// whether or not the character already has an action set. Characters that can not be issued a command have no button drawn
	if (CanSelectCommand() == true) {
		uint32 button_index = 0;
		if (IsActionSet() == false)
			button_index = 1;
		else
			button_index = 6;
		button_index += order;
		BattleMode::CurrentInstance()->GetMedia().GetCharacterActionButton(button_index)->Draw();
	}

	// Draw the action text
	VideoManager->MoveRelative(40.0f, 0.0f);
	_action_selection_text.Draw();

	// Draw the target text
	VideoManager->MoveRelative(225.0f, 0.0f);
	_target_selection_text.Draw();
} // void BattleCharacter::DrawStatus()

// /////////////////////////////////////////////////////////////////////////////
// BattleEnemy class
// /////////////////////////////////////////////////////////////////////////////

BattleEnemy::BattleEnemy(GlobalEnemy* enemy) :
	BattleActor(enemy),
	_global_enemy(enemy)
{
	string icon_filename = "img/icons/actors/enemies/" + _global_actor->GetFilename() + ".png";
	if (DoesFileExist(icon_filename) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "enemy icon image file did not exist: " << icon_filename << endl;
		_stamina_icon.Load("", 45.0f, 45.0f); // Load an empty image
	}
	else {
		_stamina_icon.Load(icon_filename, 45.0f, 45.0f);
	}

	for (map<uint32, GlobalSkill*>::const_iterator i = (_global_enemy->GetSkills()).begin(); i != (_global_enemy->GetSkills()).end(); i++) {
		_enemy_skills.push_back(i->second);
	}
}



BattleEnemy::~BattleEnemy() {
	delete _global_actor;
}



// Compares the Y-coordinates of the actors, used for sorting the actors up-down when drawing
bool BattleEnemy::operator<(const BattleEnemy & other) const {
	// NOTE: this code is currently not working correctly
	//if ((_y_location - ((*GetActor()).GetHeight())) > (other.GetYLocation() - (*(other.GetActor()).GetHeight())))
	//	return true;
	return false;
}



void BattleEnemy::ResetActor() {
	BattleActor::ResetActor();

	vector<StillImage>& sprite_frames = *(_global_enemy->GetBattleSpriteFrames());
	sprite_frames[3].DisableGrayScale();
}



void BattleEnemy::ChangeState(ACTOR_STATE new_state) {
	BattleActor::ChangeState(new_state);

	vector<StillImage>& sprite_frames = *(_global_enemy->GetBattleSpriteFrames());
	switch (_state) {
		case ACTOR_STATE_IDLE:
			_execution_finished = false;
			break;
		case ACTOR_STATE_COMMAND:
			_DecideAction();
			ChangeState(ACTOR_STATE_WARM_UP);
			break;
		case ACTOR_STATE_ACTING:
			_state_timer.Initialize(400); // TEMP: 400ms is a random time for the enemy sprite to move
			_state_timer.Run();
			break;
		case ACTOR_STATE_DEAD:
			sprite_frames[3].EnableGrayScale();
			break;
		default:
			break;
	}
}



void BattleEnemy::Update(bool animation_only) {
	BattleActor::Update(animation_only);

	// Do nothing in this function if only animations are to be updated
	if (animation_only == true) {
		return;
	}

	if (_state == ACTOR_STATE_ACTING) {
		if (_execution_finished == false)
			_execution_finished = _action->Execute();

		if ((_execution_finished == true) && (_state_timer.IsFinished() == true))
			ChangeState(ACTOR_STATE_IDLE);
	}
}



void BattleEnemy::DrawSprite() {
	vector<StillImage>& sprite_frames = *(_global_enemy->GetBattleSpriteFrames());

	// Draw the sprite's final damage frame, which should have grayscale enabled
	if (_state == ACTOR_STATE_DEAD) {
		VideoManager->Move(_x_location, _y_location);
		sprite_frames[3].Draw();
		return;
	}

	// TEMP: when the actor is acting, change its x draw position to show it move forward and then
	// backward one tile as it completes its execution. In the future this functionality should be
	// replaced by modifying the enemy's draw location members directly
	uint32 enemy_draw_offset = 0;
	if (_state == ACTOR_STATE_ACTING) {
		if (_state_timer.PercentComplete() <= 0.50f)
			enemy_draw_offset = TILE_SIZE * (2.0f * _state_timer.PercentComplete());
		else
			enemy_draw_offset = TILE_SIZE * (2.0f - 2.0f * _state_timer.PercentComplete());
	}

	// Draw the enemy's damage-blended sprite frames
	VideoManager->Move(_x_location - enemy_draw_offset, _y_location);

	float hp_percent = static_cast<float>(GetHitPoints()) / static_cast<float>(GetMaxHitPoints());

	// Alpha will range from 1.0 to 0.0 in the following calculations
	if (GetHitPoints() == GetMaxHitPoints()) {
		sprite_frames[0].Draw();
	}
	else if (GetHitPoints() == 0) {
		sprite_frames[3].Draw();
	}
	else if (hp_percent > 0.666f) {
		sprite_frames[0].Draw();
		float alpha = 1.0f - ((hp_percent - 0.666f) * 3.0f);
		sprite_frames[1].Draw(Color (1.0f, 1.0f, 1.0f, alpha));
	}
	else if (hp_percent >  0.333f) {
		sprite_frames[1].Draw();
		float alpha = 1.0f - ((hp_percent - 0.333f) * 3.0f);
		sprite_frames[2].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else { // (hp_precent > 0.0f)
		sprite_frames[2].Draw();
		float alpha = 1.0f - (hp_percent * 3.0f);
		sprite_frames[3].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
} // void BattleEnemy::DrawSprite()



void BattleEnemy::_DecideAction() {
	if (_global_enemy->GetSkills().empty() == true) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "enemy had no usable skills" << endl;
		ChangeState(ACTOR_STATE_IDLE);
	}

	// TODO: this method is mostly temporary and makes no intelligent decisions about what action to
	// take or on what target to select. Currently this method does the following.
	//
	// (1): select a random skill from the list that the enemy can execute
	// (2): select a random character that is not in the dead state to target
	// (3): select a random attack point on the selected character target
	//
	// Therefore, only skills that target attack points on enemies are valid. No party or actor targets
	// will work. Obviously these needs will be addressed eventually.

	// TEMP: select a random skill to use
	uint32 skill_index = 0;
	if (_enemy_skills.size() > 1) {
		skill_index = RandomBoundedInteger(0, _enemy_skills.size() - 1);
	}
	GlobalSkill* skill = _enemy_skills[skill_index];

	// TEMP: select a random living character in the party for the target
	BattleTarget target;

	deque<BattleCharacter*> alive_characters = BattleMode::CurrentInstance()->GetCharacterActors();
	deque<BattleCharacter*>::iterator character_iterator = alive_characters.begin();
	while (character_iterator != alive_characters.end()) {
		if ((*character_iterator)->IsAlive() == false)
			character_iterator = alive_characters.erase(character_iterator);
		else
			character_iterator++;
	}

	if (alive_characters.empty() == true) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "no characters were alive when enemy was selecting a target" << endl;
		ChangeState(ACTOR_STATE_IDLE);
		return;
	}

	uint32 point_target = 0;
	BattleActor* actor_target = NULL;

	// TEMP: select a random living character
	if (alive_characters.size() == 1)
		actor_target = alive_characters[0];
	else
		actor_target = alive_characters[RandomBoundedInteger(0, alive_characters.size() - 1)];

	// TEMP: select a random attack point on the target character
	uint32 num_points = actor_target->GetAttackPoints().size();
	if (num_points == 1)
		point_target = 0;
	else
		point_target = RandomBoundedInteger(0, num_points - 1);

	// TEMP: Should not statically assign to target a foe point. Examine the selected skill's target type
	target.SetPointTarget(GLOBAL_TARGET_FOE_POINT, point_target, actor_target);

	SetAction(new SkillAction(this, target, skill));
}

} // namespace private_battle

} // namespace hoa_battle
