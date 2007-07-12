////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle.cpp
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \author  Corey Hoffstein, visage@allacrost.org
*** \author  Andy Gardner, chopperdave@allacrost.org
*** \brief   Source file for battle mode interface.
*** ***************************************************************************/

#include <iostream>
#include <sstream>

#include "utils.h"
#include "audio.h"
#include "video.h"
#include "input.h"
#include "mode_manager.h"
#include "system.h"
#include "global.h"
#include "script.h"
#include "battle.h"
#include "battle_actors.h"
#include "boot.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_script;

using namespace hoa_battle::private_battle;
using namespace hoa_boot;

namespace hoa_battle {

bool BATTLE_DEBUG = false;

namespace private_battle {

BattleMode * current_battle = NULL;

////////////////////////////////////////////////////////////////////////////////
// SCRIPTEVENT CLASS
////////////////////////////////////////////////////////////////////////////////

//ScriptEvent::ScriptEvent(hoa_global::GlobalActor * source, std::deque<IBattleActor*> targets, const std::string & script_name) :
/*ScriptEvent::ScriptEvent(BattleActor* source, std::deque<BattleActor*> targets, const std::string & script_name, uint32 warm_up_time) :
	_script_name(script_name),
	_source(source),
	_skill(NULL),
	_item(NULL),
	_targets(targets)
{
	_warm_up_time.SetDuration(warm_up_time);
	_warm_up_time.Reset();
	_warm_up_time.Play();
}*/

//Constructor for a script event that uses an item
ScriptEvent::ScriptEvent(BattleActor* source, BattleActor* target, GlobalSkill* skill, GlobalAttackPoint* attack_point) :
	_source(source),
	_skill(skill),
	_item(NULL),
	_attack_point(attack_point),
	_target(target)
{
	_warm_up_time.Initialize(skill->GetWarmupTime());
	_warm_up_time.Run();
}

// Constructor for a script event that uses an item
ScriptEvent::ScriptEvent(BattleActor* source, BattleActor* target, GlobalItem* item, GlobalAttackPoint* attack_point, uint32 warm_up_time) :
	_source(source),
	_skill(NULL),
	_item(item),
	_attack_point(attack_point),
	_target(target)

{
	_warm_up_time.Initialize(warm_up_time);
	_warm_up_time.Run();
}


// Constructor with multiple targets and a skill
/*ScriptEvent::ScriptEvent(BattleActor* source, std::deque<BattleActor*> targets, hoa_global::GlobalSkill* skill) :
	_source(source),
	_skill(skill),
	_item(NULL),
	_targets(targets),
	_target(NULL)
{
	_warm_up_time.Initialize(skill->GetWarmupTime());
	_warm_up_time.Run();
}*/


ScriptEvent::~ScriptEvent()
{
}


void ScriptEvent::Update()
{
	//_warm_up_time -= SystemManager->GetUpdateTime();
	if (_warm_up_time.IsRunning())
	{
		float offset = SystemManager->GetUpdateTime() * (107.f / _warm_up_time.GetDuration());
	
		_source->SetTimePortraitLocation(_source->GetTimePortraitLocation() + offset);
	}
	//TODO Any warm up animations
}

void ScriptEvent::RunScript() {
	//If _item is non-NULL, then we're using an item
	if (_item)
	{
		if (_item->GetTargetType() == GLOBAL_TARGET_PARTY)
		{
			if (_target->IsEnemy())
			{
				BattleEnemyActor* bea;
				//Loop through enemies and apply the item
				for (uint32 i = 0; i < current_battle->GetNumberOfEnemies(); ++i)
				{
					bea = current_battle->GetEnemyActorAt(i);
					if (bea->IsAlive())
					{
						bea->CalcEvade(_attack_point);
						bea->CalcMetaPhysicalDefense(_attack_point);
						bea->CalcPhysicalDefense(_attack_point);
					}

					_item->BattleUse(bea, _source);
				}
				
				//_item->DecrementCount(1);

				/*if (_item->GetCount() == 0)
				{
					GlobalManager->RemoveFromInventory(_item->GetID());
				}*/
			}
			else
			{
				BattleCharacterActor* bca;
				//Loop through all party members and apply
				for (uint32 i = 0; i < current_battle->GetNumberOfCharacters(); ++i)
				{
					bca = current_battle->GetPlayerCharacterAt(i);
					if (bca->IsAlive())
					{
						bca->CalcEvade(_attack_point);
						bca->CalcMetaPhysicalDefense(_attack_point);
						bca->CalcPhysicalDefense(_attack_point);
					}

					_item->BattleUse(bca, _source);
				}

				//_item->DecrementCount(1);

				/*if (_item->GetCount() == 0)
				{
					GlobalManager->RemoveFromInventory(_item->GetID());
				}*/
			}
		}
		else
		{
			_target->CalcEvade(_attack_point);
			_target->CalcMetaPhysicalDefense(_attack_point);
			_target->CalcPhysicalDefense(_attack_point);

			_item->BattleUse(_target, _source);
			//_item->DecrementCount(1);

			/*if (_item->GetCount() == 0)
			{
				GlobalManager->RemoveFromInventory(_item->GetID());
			}*/
			//FIX ME Reconstruct action list somehow (do via ran_script)
		}

		if (_item->GetCount() == 0)
		{
			GlobalManager->RemoveFromInventory(_item->GetID());
		}
	}
	// TEMP: do basic damage to the actors
	else if (_skill)
	{
		_source->CalcMetaPhysicalAttack();
		_source->CalcPhysicalAttack();

		if (_skill->GetTargetType() == GLOBAL_TARGET_PARTY)
		{
			if (_target->IsEnemy())
			{
				BattleEnemyActor* bea;
				//Loop through enemies and apply the item
				for (uint32 i = 0; i < current_battle->GetNumberOfEnemies(); ++i)
				{
					bea = current_battle->GetEnemyActorAt(i);
					if (bea->IsAlive())
					{
						bea->CalcEvade(_attack_point);
						bea->CalcMetaPhysicalDefense(_attack_point);
						bea->CalcPhysicalDefense(_attack_point);
					}

					_skill->BattleExecute(bea, _source);
				}
			}
			else
			{
				BattleCharacterActor* bca;
				//Loop through all party members and apply
				for (uint32 i = 0; i < current_battle->GetNumberOfCharacters(); ++i)
				{
					bca = current_battle->GetPlayerCharacterAt(i);
					if (bca->IsAlive())
					{
						bca->CalcEvade(_attack_point);
						bca->CalcMetaPhysicalDefense(_attack_point);
						bca->CalcPhysicalDefense(_attack_point);
					}

					_skill->BattleExecute(bca, _source);
				}
			}
		}
		else
		{
			//if (_target)
			//{
			if (_target->IsAlive())
			{
				_target->CalcEvade(_attack_point);
				_target->CalcMetaPhysicalDefense(_attack_point);
				_target->CalcPhysicalDefense(_attack_point);
			}

			_skill->BattleExecute(_target, _source);
			/*}
			else
			{
				_skill->BattleExecute(current_battle->GetPlayerCharacterAt(0), _source);
			}*/
		}

		_source->SetSkillPoints(_source->GetSkillPoints() - _skill->GetSPRequired());
	}

	//FIXE ME Temp code!!!
	if (_source)
	{
		_source->TEMP_ResetAttackTimer();
		//if (_source->IsEnemy())
		//{
		//	dynamic_cast<BattleEnemyActor*>(_source)->ResetAttackTimer();
		//}
	}
}

} // namespace private battle


////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Initialization and Destruction Code
////////////////////////////////////////////////////////////////////////////////

BattleMode::BattleMode() :
	_initialized(false),
	_performing_script(false),
	_active_se(NULL),
	_battle_over(false),
	_victorious_battle(false),
	_victory_xp(0),
	_victory_sp(0),
	_victory_money(0),
	_victory_level(false),
	_victory_skill(false),
	_cursor_state(CURSOR_IDLE),
	_selected_character(NULL),
	_selected_target(NULL),
	_selected_attack_point(0),
	_current_number_swaps(0),
	_swap_countdown_timer(300000), // 5 minutes
	_min_agility(9999),
	_next_monster_location_index(0)
{
	if (BATTLE_DEBUG)
		cout << "BATTLE: BattleMode constructor invoked" << endl;

	mode_type = MODE_MANAGER_BATTLE_MODE;

	std::vector <hoa_video::StillImage> attack_point_indicator;
	StillImage frame;
	frame.SetDimensions(16, 16);
	frame.SetFilename("img/icons/battle/ap_indicator_fr0.png");
	attack_point_indicator.push_back(frame);
	frame.SetFilename("img/icons/battle/ap_indicator_fr1.png");
	attack_point_indicator.push_back(frame);
	frame.SetFilename("img/icons/battle/ap_indicator_fr2.png");
	attack_point_indicator.push_back(frame);
	frame.SetFilename("img/icons/battle/ap_indicator_fr3.png");
	attack_point_indicator.push_back(frame);

	for (uint32 i = 0; i < attack_point_indicator.size(); i++) {
		if (!VideoManager->LoadImage(attack_point_indicator[i]))
			cerr << "BATTLE ERROR: Failed to load attack point indicator." << endl;
	}

	for (uint32 i = 0; i < attack_point_indicator.size(); i++) {
		_attack_point_indicator.AddFrame(attack_point_indicator[i], 10);
	}

	//Load the universal time meter image
	_universal_time_meter.SetDimensions(10, 512);
	_universal_time_meter.SetFilename("img/menus/stamina_bar.png");
	if (!VideoManager->LoadImage(_universal_time_meter))
		cerr << "BATTLE ERROR: Failed to load time meter." << endl;

	_victory_items.clear();

	_actor_selection_image.SetDimensions(109, 78);
	_actor_selection_image.SetFilename("img/icons/battle/character_selector.png");
	if (!VideoManager->LoadImage(_actor_selection_image)) {
		cerr << "BATTLE ERROR: Unable to load player selector image." << endl;
	}

	_action_window = new ActionWindow();
	_finish_window = new FinishWindow();
	_TEMP_LoadTestData();
} // BattleMode::BattleMode()



BattleMode::~BattleMode() {
	// Don't let current_battle keep pointing to this object instance any longer
	if (current_battle == this) {
		current_battle = NULL;
	}

	for (uint32 i = 0; i < _battle_music.size(); i++)
		_battle_music.at(i).FreeMusic();

	// Delete all character and enemy actors
	for (deque<BattleCharacterActor*>::iterator i = _character_actors.begin(); i != _character_actors.end(); i++) {
		delete *i;
	}
	_character_actors.clear();

	for (deque<BattleEnemyActor*>::iterator i = _enemy_actors.begin(); i != _enemy_actors.end(); i++) {
		delete *i;
	}
	_enemy_actors.clear();

	// FIX ME: If item scripts are still there, add the item back to the inventory
	for (std::list<ScriptEvent*>::iterator i = _script_queue.begin(); i != _script_queue.end(); i++) {
		if ((*i)->GetItem())
		{
			(*i)->GetItem()->IncrementCount(1);
		}
		delete *i;
	}
	_script_queue.clear();

	_victory_items.clear();

	// Remove all of the battle images that were loaded
	VideoManager->DeleteImage(_battle_background);
	VideoManager->DeleteImage(_bottom_menu_image);
	VideoManager->DeleteImage(_actor_selection_image);
	VideoManager->DeleteImage(_attack_point_indicator);
	VideoManager->DeleteImage(_swap_icon);
	VideoManager->DeleteImage(_swap_card);
	VideoManager->DeleteImage(_universal_time_meter);

	// Delete all GUI objects that are allocated
	delete(_action_window);
	delete(_finish_window);
} // BattleMode::~BattleMode()



void BattleMode::Reset() {
	current_battle = this;

	VideoManager->SetCoordSys(0.0f, 1024.0f, 0.0f, 768.0f);
	VideoManager->SetFont("battle");

	// Load the default battle music track if no other music has been added
	if (_battle_music.empty()) {

		if (BATTLE_DEBUG) cerr << "BATTLE DEBUG: No music defined. Attempting to load the default theme" << endl;

		MusicDescriptor MD;
		if (MD.LoadMusic("mus/Confrontation.ogg") == false) {
			cerr << "BATTLE ERROR: failed to load default battle theme track: " << MD.GetFilename() << endl;
		}
		else {
			_battle_music.push_back(MD);
		}
	}

	if (_battle_music.empty() == false && _battle_music.back().IsPlaying() == false) {
		_battle_music.back().PlayMusic();
	}

	if (_initialized == false) {
		_Initialize();
	}
} // void BattleMode::Reset()



void BattleMode::AddEnemy(GlobalEnemy new_enemy) {
	// (1): Don't add the enemy if it has an invalid ID or an experience level that is not zero
	if (new_enemy.GetID() == 0) {
		if (BATTLE_DEBUG) {
			cerr << "BATTLE WARNING: attempted to add a new enemy with an invalid id (0). "
				<< "The enemy was not added to the battle." << endl;
		}
		return;
	}
	if (new_enemy.GetExperienceLevel() != 0) {
		if (BATTLE_DEBUG) {
			cerr << "BATTLE WARNING: attempted to add a new enemy that had already been initialized to experience level "
				<< new_enemy.GetExperienceLevel() << ". The enemy was not added to the battle." << endl;
		}
	}

	// (2): Level the enemy up to be within a reasonable range of the party's strength
	new_enemy.Initialize(GlobalManager->AverageActivePartyExperienceLevel());

	// (3): Hold a copy of this enemy in case the battle needs to be restarted
	_original_enemies.push_back(new_enemy);

	// (4): Construct the enemy battle actor to be placed on the battle field
	float x = MONSTER_LOCATIONS[_next_monster_location_index][0];
	float y = MONSTER_LOCATIONS[_next_monster_location_index][1];
	_next_monster_location_index++;

	// TEMP
	// The next line modulus have been changed. Howeever, a better solution is to provide a std::vector
	// instead of an array for MONSTER_LOCATIONS, init in the constructor, and make modulus with the vector length
	// _next_monster_location_index = _next_monster_location_index % (sizeof(MONSTER_LOCATIONS)/2); <-- Before
	_next_monster_location_index = _next_monster_location_index % 8;  // <-- Now

	BattleEnemyActor* enemy_actor= new BattleEnemyActor(new_enemy, x, y);
	enemy_actor->InitBattleActorStats(&new_enemy);
	_enemy_actors.push_back(enemy_actor);
}



void BattleMode::AddMusic(string music_filename) {
	if (music_filename == "") {
		if (BATTLE_DEBUG)
			cerr << "BATTLE DEBUG: Passed in an empty music filename." << endl;
		return;
	}

	MusicDescriptor MD;
	if (MD.LoadMusic(music_filename) == false) {
		cerr << "BATTLE ERROR: BattleMode::AddMusic failed to load this music file: " << music_filename << endl;
	}
	else {
		_battle_music.push_back(MD);
	}
}



void BattleMode::_TEMP_LoadTestData() {
	// Load all background images
	_battle_background.SetFilename("img/backdrops/battle/desert_cave.png");
	_battle_background.SetDimensions(SCREEN_LENGTH * TILE_SIZE, SCREEN_HEIGHT * TILE_SIZE);
	if (!VideoManager->LoadImage(_battle_background)) {
		cerr << "BATTLE ERROR: Failed to load background image: " << endl;
		_ShutDown();
	}

	_bottom_menu_image.SetFilename("img/menus/battle_bottom_menu.png");
	_bottom_menu_image.SetDimensions(1024, 128);
	if (!VideoManager->LoadImage(_bottom_menu_image)) {
		cerr << "BATTLE ERROR: Failed to load bottom menu image: " << endl;
		_ShutDown();
	}

	_swap_icon.SetFilename("img/icons/battle/swap_icon.png");
	_swap_icon.SetDimensions(35, 30);
	if (!VideoManager->LoadImage(_swap_icon)) {
		cerr << "BATTLE ERROR: Failed to load swap icon: " << endl;
		_ShutDown();
	}

	_swap_card.SetFilename("img/icons/battle/swap_card.png");
	_swap_card.SetDimensions(25, 37);
	if (!VideoManager->LoadImage(_swap_card)) {
		cerr << "BATTLE ERROR: Failed to load swap card: " << endl;
		_ShutDown();
	}
}



void BattleMode::_Initialize()
{
	// (1): Construct all character battle actors from the active party
	GlobalParty* active_party = GlobalManager->GetActiveParty();
	if (active_party->GetPartySize() == 0) {
		if (BATTLE_DEBUG) 
			cerr << "BATTLE ERROR: In BattleMode::_Initialize(), the size of the active party was zero" << endl;
		ModeManager->Pop(); // Self-destruct the battle mode
		return;
	}
	// TODO: implement this later
	// COMMENT: Implement what later?
	for (uint32 i = 0; i < active_party->GetPartySize(); i++) {
		GlobalCharacter* new_character = dynamic_cast<GlobalCharacter*>(active_party->GetActorAtIndex(i));
		BattleCharacterActor* new_actor = new BattleCharacterActor(new_character, 256, 320);
		new_actor->InitBattleActorStats(new_character);
		_character_actors.push_back(new_actor);
		_selected_character = new_actor;
	}

	_selected_character_index = GetIndexOfFirstIdleCharacter();

	// (2) Loop through and find the actor with the lowest agility
	for (uint8 i = 0; i < _enemy_actors.size(); i++) {
		if (_enemy_actors[i]->GetAgility() < _min_agility)
			_min_agility = _enemy_actors[i]->GetAgility();
	}

	for (uint8 i = 0; i < _character_actors.size(); i++) {
		if (_character_actors[i]->GetAgility() < _min_agility)
			_min_agility = _character_actors[i]->GetAgility();
	}

	//Now adjust starting wait times based on agility proportions
	//If current actor's agility is twice the lowest agility, then
	//they will have a wait time that is half of the slowest actor
	float proportion;

	for (uint8 i = 0; i < _enemy_actors.size(); i++) {
		proportion = static_cast<float>(_min_agility) / static_cast<float>(_enemy_actors[i]->GetAgility());
		_enemy_actors[i]->GetWaitTime()->Initialize(static_cast<uint32>(MAX_INIT_WAIT_TIME * proportion));

		//Start the timer.  We can do this here because the calculations will be done so quickly
		//that the other chars wont fall far behind.
		_enemy_actors[i]->ResetWaitTime();
	}

	for (uint8 i = 0; i < _character_actors.size(); i++) {
		proportion = static_cast<float>(_min_agility) / static_cast<float>(_character_actors[i]->GetAgility());
		_character_actors[i]->GetWaitTime()->Initialize(static_cast<uint32>(MAX_INIT_WAIT_TIME * proportion));

		//Start the timer.  We can do this here because the calculations will be done so quickly
		//that the other chars wont fall far behind.
		_character_actors[i]->ResetWaitTime();
	}

	// Andy: Once every game loop, the SystemManager's timers are updated
	// However, in between calls, battle mode is constructed. As part
	// of battle mode's construction, each actor is given a wait timer
	// that is triggered on initialization. But the moving of the stamina
	// portrait uses the update time from SystemManager.  Therefore, the
	// amount of time since SystemManager last updated is greater than
	// the amount of time that has expired on the actors' wait timers
	// during the first orund of battle mode.  This gives the portrait an
	// extra boost, so once the wait time expires for an actor, his portrait
	// is past the designated stopping point

	// <--      time       -->
	// A----------X-----------B
	// If the SystemManager has its timers updated at A and B, and battle mode is
	// constructed and initialized at X, you can see the amount of time between
	// X and B (how much time passed on the wait timers in round 1) is significantly
	// smaller than the time between A and B.  Hence the extra boost to the stamina
	// portrait's location

	//FIX ME This will not work in the future (i.e. paralysis)...realized this
	//after writing all the above crap
	SystemManager->UpdateTimers();

	_initialized = true;
} // void BattleMode::_Initialize()



void BattleMode::_ShutDown() {
	_battle_music[0].StopMusic();

	for (uint32 i = 0; i < _character_actors.size(); ++i) {
		_character_actors[i]->UpdateGlobalActorStats();
	}

	// This call will clear the input state
	InputManager->EventHandler();

	// Remove this BattleMode instance from the game stack
	ModeManager->Pop();
}



void BattleMode::_TallyRewards() {
	GlobalEnemy *gbe;

	std::map<string, uint32>::iterator it;
	std::map<uint32, GlobalObject*>::iterator it2;
	//Tally up the xp, money, and get the list of items
	_victory_sp = GetNumberOfEnemies();
	for (uint32 i = 0; i < GetNumberOfEnemies(); ++i)
	{
		gbe = GetEnemyActorAt(i)->GetActor();
		_victory_xp += gbe->GetExperiencePoints();
		_victory_money += gbe->GetDrunesDropped();

	// Roots: this code is now defunct. Refer to the GlobalEnemy class to determine items dropped
// 		if (RandomFloat() * 100 <= gbe->GetChanceToDrop())
// 		{
// 			//Added here so that we can display the list on victory
// 			//FIX ME later
// 			GlobalManager->AddToInventory(gbe->GetItemDropped());
// 
// 			it2 = GlobalManager->GetInventory()->find(gbe->GetItemDropped());
// 			it = _victory_items.find(MakeStandardString(it2->second->GetName()));
// 
// 			if (it != _victory_items.end())
// 			{
// 				++it->second;
// 			}
// 			else
// 			{
// 				_victory_items.insert(make_pair(MakeStandardString(it2->second->GetName()), 1));
// 			}
// 		}
	}

	uint32 num_alive = 0;

	for (uint32 i = 0; i < GetNumberOfCharacters(); ++i)
	{
		if (GetPlayerCharacterAt(i)->IsAlive())
			++num_alive;
	}

	//Divvy the XP between surviving party members
	_victory_xp /= num_alive;
}

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Update Code
////////////////////////////////////////////////////////////////////////////////

void BattleMode::Update() {
	_battle_over = (_NumberEnemiesAlive() == 0) || (_NumberCharactersAlive() == 0);
	_victorious_battle = (_NumberEnemiesAlive() == 0);

	// ----- (1): If the battle is over, only execute this small block of update code
	if (_battle_over) {
		if (_finish_window->GetState() == FINISH_INVALID) { // Indicates that the battle has just now finished
			_action_window->Reset();
			_finish_window->Initialize(_victorious_battle);

			if (_victorious_battle) {
				_TallyRewards();
				PlayerVictory();
			}
		}

		// The FinishWindow::Update() function handles all update code when a battle is over.
		// The call to shut down battle mode is also made from within this call.
		_finish_window->Update();

		// Do not update other battle code if the battle has already ended
		return;
	} // if (_battle_over)

	// ----- (2): Update the state of all battle actors
	for (uint8 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->Update();
	}
	for (uint8 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->Update();
	}

	// ----- (3): Execute any scripts that are sitting in the queue
	if (_script_queue.size()) {
	//if (!_IsPerformingScript() && _script_queue.size() > 0) {
		std::list<private_battle::ScriptEvent*>::iterator it;
		bool ran_script = false;
		ScriptEvent* se;
		//for (uint8 i = 0; i < _script_queue.size(); i++)
		for (it = _script_queue.begin(); it != _script_queue.end(); it++)
		{
			se = (*it);//_script_queue.front();
			se->Update();
			//(*it).Update();
			//se._warm_up_time -= SystemManager->GetUpdateTime();
			if (se->GetWarmUpTime()->IsFinished() && !_IsPerformingScript())
			{
				SetPerformingScript(true,se);
				se->RunScript();
				ran_script = true;
				//Later have battle mode call UpdateActiveScriptEvent instead
				//_script_queue.pop_front();
			}
		}

		//Do this out here so iterator doesnt get screwed up mid-loop
		if (ran_script)
		{
			//If we used an item, immediately reconstruct the action list
			//This way if an item is used
			//if (se->GetItem())

			SetPerformingScript(false,NULL);
		}
	} // if (_script_queue.size())

	// ----- (4): Update various menus and other GUI graphics as appropriate
	if (_cursor_state == CURSOR_SELECT_ATTACK_POINT) {
		_attack_point_indicator.Update();
	}

	// ----- (5): Process user input depending upon which state the menu cursor is in
	switch (_cursor_state) {
		case CURSOR_IDLE:
			_UpdateCharacterSelection();
			break;
		case CURSOR_WAIT:
			break;
		case CURSOR_SELECT_TARGET:
			_UpdateTargetSelection();
			break;
		case CURSOR_SELECT_ATTACK_POINT:
			_UpdateAttackPointSelection();
			break;
		// TODO: What should be done for these two options?
		case CURSOR_SELECT_PARTY:
		default:
			break;
	} // switch (_cursor_state)

	// ----- (6): Update the action window if the player is making an action or target selection
	if (_action_window->GetState() != VIEW_INVALID)
		_action_window->Update();
} // void BattleMode::Update()



void BattleMode::_UpdateCharacterSelection() {
	// First check if there are any characters in the idle state. If there aren't, there's nothing left to do
	_selected_character_index = GetIndexOfFirstIdleCharacter();
	if (_selected_character_index == static_cast<int32>(INVALID_BATTLE_ACTOR_INDEX)) {
		return;
	}

	// TODO: Only freeze timers this when we've gone from having no idle characters to having at least one idle character
	// If the battle is running in wait mode, freeze all timers while the player selects an action
	if (ACTIVE_BATTLE_MODE == false)
		FreezeTimers();

	// If there is only one character alive, then we are sure that he/she is the character to select
	if (_NumberCharactersAlive() == 1) {
		_cursor_state = CURSOR_WAIT;
		_selected_character = GetPlayerCharacterAt(_selected_character_index);
		_action_window->Initialize(_selected_character);
		return;
	}

	// TODO: the code below doesn't work very well in practice (fight a battle with two characters and see what I mean)
	// it needs to be cleaned

	// Otherwise there are multiple characters, of which more than one may be idle

	// Handle user input commands: up, down, left, right, confirm
	if (InputManager->UpPress() || InputManager->RightPress()) {
		// Select the next character above the currently selected one
		// If no such character exists, the selected character will remain selected
		uint32 working_index = _selected_character_index;
		BattleCharacterActor *bca;

		while (working_index < GetNumberOfCharacters()) {
			bca = GetPlayerCharacterAt(working_index + 1);
			if (bca->IsAlive() && bca->GetWaitTime()->IsFinished() && !bca->IsQueuedToPerform()) {
				_selected_character_index = working_index + 1;
				break;
			}

			++working_index;
		}
	}
	else if (InputManager->DownPress() || InputManager->LeftPress()) {
		// Select the next character below the currently selected one.
		// If no such character exists, the selected character will remain selected
		uint32 working_index = _selected_character_index;
		BattleCharacterActor *bca;

		while (working_index > 0) {
			bca = GetPlayerCharacterAt(working_index + 1);
			if (bca->IsAlive() && bca->GetWaitTime()->IsFinished() && !bca->IsQueuedToPerform())
			{
				_selected_character_index = working_index - 1;
				break;
			}
			--working_index;
		}
	}
	else if (InputManager->ConfirmPress()) {
		_cursor_state = CURSOR_WAIT;
		_selected_character = GetPlayerCharacterAt(_selected_character_index);
		_action_window->Initialize(_selected_character);
	}
} // void BattleMode::_UpdateCharacterSelection()



void BattleMode::_UpdateTargetSelection() {
	if (InputManager->DownPress() || InputManager->LeftPress()) {
		if (_action_window->GetActionTargetType() != GLOBAL_TARGET_PARTY) {
			if (_action_window->IsActionTargetAlly()) {
				if (InputManager->DownPress()) {
					if (_selected_target->IsEnemy() == false) {
						if (_selected_target_index) {
							--_selected_target_index;
						}
						else {
							_selected_target_index = GlobalManager->GetActiveParty()->GetPartySize() - 1;
						}
						_selected_target = GetPlayerCharacterAt(_selected_target_index);
					}
					else {
						_selected_target_index = GetIndexOfNextAliveEnemy(false);
						_selected_target = GetEnemyActorAt(_selected_target_index);
					}
				}
			} // if (_action_window->IsActionTargetAlly())
			else {
				_selected_target_index = GetIndexOfNextAliveEnemy(false);
				_selected_target = GetEnemyActorAt(_selected_target_index);
			}
		}
	} // if (InputManager->DownPress() || InputManager->LeftPress())

	else if (InputManager->UpPress() || InputManager->RightPress()) {
		if (_action_window->GetActionTargetType() != GLOBAL_TARGET_PARTY) {
			if (_action_window->IsActionTargetAlly()) {
				if (InputManager->UpPress()) {
					if (_selected_target->IsEnemy() == false) {
						if (_selected_target_index < GlobalManager->GetActiveParty()->GetPartySize() - 1) {
							++_selected_target_index;
						}
						else {
							_selected_target_index = 0;
						}

						_selected_target = GetPlayerCharacterAt(_selected_target_index);
					}
					else {
						_selected_target_index = GetIndexOfNextAliveEnemy(true);
						_selected_target = GetEnemyActorAt(_selected_target_index);
					}
				}
			} // if (_action_window->IsActionTargetAlly())
			else {
				_selected_target_index = GetIndexOfNextAliveEnemy(true);
				_selected_target = GetEnemyActorAt(_selected_target_index);
			}
		}

	} // else if (InputManager->UpPress() || InputManager->RightPress())

	else if (InputManager->ConfirmPress()) {
		if (_action_window->GetActionTargetType() == GLOBAL_TARGET_ATTACK_POINT) {
			_cursor_state = CURSOR_SELECT_ATTACK_POINT;
			// TODO: Implement cursor memory for attack points here
			_selected_attack_point = 0;
		}
		else {
			// Create the script event to execute
			ScriptEvent* new_event;
			if (_action_window->GetActionCategory() != ACTION_TYPE_ITEM) {
				new_event = new ScriptEvent(_selected_character, _selected_target, _action_window->GetSelectedSkill());
			}
			else {
				GlobalItem* item = _action_window->GetSelectedItem();
				// NOTE: Don't know if decrementing the item count is the best approach to use here.
				// We decrement the count now so that if the next character wants to use items, they know
				// how many are available to use. If the current chararacter uses the item, then the decrement stays.
				// If count == 0, then it's removed from inventory...if item is not used (i.e. battle ends before use),
				// it is incremented back.
				item->DecrementCount(1);
				new_event = new ScriptEvent(_selected_character, _selected_target, item);
			}
			AddScriptEventToQueue(new_event);
			_selected_character->SetQueuedToPerform(true);
			_selected_target = NULL;
			_selected_character_index = GetIndexOfFirstIdleCharacter();
			_cursor_state = CURSOR_IDLE;

			// Resume battle timers if the battle is executing in wait mode
			if (ACTIVE_BATTLE_MODE == false)
				UnFreezeTimers();
		}
	} // else if (InputManager->ConfirmPress())

	else if (InputManager->CancelPress()) {
		_cursor_state = CURSOR_IDLE;
		_selected_target = NULL;
	}
} // void BattleMode::_UpdateTargetSelection()



void BattleMode::_UpdateAttackPointSelection() {
	BattleEnemyActor* e = GetEnemyActorAt(_selected_target_index);
	vector<GlobalAttackPoint*>* global_attack_points = e->GetActor()->GetAttackPoints();

	if (InputManager->ConfirmPress()) {
		ScriptEvent* new_event;
		if (_action_window->GetActionCategory() != ACTION_TYPE_ITEM) {
			new_event = new ScriptEvent(_selected_character, _selected_target, _action_window->GetSelectedSkill(),
				global_attack_points->at(_selected_attack_point));
		}
		else {
			GlobalItem* item = _action_window->GetSelectedItem();
			// NOTE: Don't know if decrementing the item count is the best approach to use here.
			// We decrement the count now so that if the next character wants to use items, they know
			// how many are available to use. If the current chararacter uses the item, then the decrement stays.
			// If count == 0, then it's removed from inventory...if item is not used (i.e. battle ends before use),
			// it is incremented back.
			item->DecrementCount(1);
			new_event = new ScriptEvent(_selected_character, _selected_target, item, global_attack_points->at(_selected_attack_point));
		}
		AddScriptEventToQueue(new_event);
		_selected_character->SetQueuedToPerform(true);
		_selected_target = NULL;
		_selected_character_index = GetIndexOfFirstIdleCharacter();
		_cursor_state = CURSOR_IDLE;
		_action_window->Reset();

		// Resume battle timers if the battle is executing in wait mode
		if (ACTIVE_BATTLE_MODE == false)
			UnFreezeTimers();
	}

	else if (InputManager->UpPress() || InputManager->RightPress()) {
		if (_selected_attack_point < global_attack_points->size() - 1) {
			_selected_attack_point++;
		}
		else if (_selected_attack_point == global_attack_points->size() - 1) {
			_selected_attack_point = 0;
		}
	}

	else if (InputManager->DownPress() || InputManager->LeftPress()) {
		if (_selected_attack_point > 0) {
			_selected_attack_point--;
		}
		else if (_selected_attack_point == 0) {
			_selected_attack_point = global_attack_points->size() - 1;
		}
	}

	else if (InputManager->CancelPress()) {
		_cursor_state = CURSOR_SELECT_TARGET;
	}
} // void BattleMode::_UpdateAttackPointSelection()

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Draw Code
////////////////////////////////////////////////////////////////////////////////

void BattleMode::Draw() {
	// Apply scene lighting if the battle has finished
	if (_battle_over) {
		if (_victorious_battle) {
			VideoManager->EnableSceneLighting(Color(0.914f, 0.753f, 0.106f, 1.0f)); // Golden color for victory
		}
		else {
			VideoManager->EnableSceneLighting(Color(1.0f, 0.0f, 0.0f, 1.0f)); // Red color for defeat
		}
	}

	_DrawBackgroundVisuals();
	_DrawBottomMenu();
	_DrawSprites();
	_DrawTimeMeter();

	if (_action_window->GetState() != VIEW_INVALID) {
		_action_window->Draw();
	}

	if (_battle_over) {
		_finish_window->Draw();
	}
} // void BattleMode::Draw()



void BattleMode::_DrawBackgroundVisuals() {
	// Draw the full-screen, static background image
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);
	VideoManager->Move(0, 0);
	VideoManager->DrawImage(_battle_background);

	// TODO: Draw other background objects and animations
} // void BattleMode::_DrawBackgroundVisuals()



void BattleMode::_DrawBottomMenu() {
	// Draw the static image for the lower menu
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(0, 0);
	VideoManager->DrawImage(_bottom_menu_image);

	// Draw the swap icon and any swap cards
	VideoManager->Move(6, 16);
	VideoManager->DrawImage(_swap_icon, Color::gray);
	VideoManager->Move(6, 68);
	for (uint8 i = 0; i < _current_number_swaps; i++) {
		VideoManager->DrawImage(_swap_card);
		VideoManager->MoveRelative(4, -4);
	}

	// Draw the selected character's portrait, blended according to the character's current HP level
	_selected_character->DrawPortrait();

	// Draw the status information of all character actors
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->DrawStatus();
	}
} // void BattleMode::_DrawBottomMenu()



void BattleMode::_DrawSprites() {
	// TODO: Draw sprites in order based on their x and y coordinates on the screen (bottom to top, then left to right)

	// Draw all character sprites
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->DrawSprite();
	}

	// Sort and draw the enemies
	//std::deque<private_battle::BattleEnemyActor*> sorted_enemy_actors = _enemy_actors;
 	//std::sort(sorted_enemy_actors.begin(), sorted_enemy_actors.end(), AscendingYSort());

	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->DrawSprite();
	}
} // void BattleMode::_DrawSprites()



void BattleMode::_DrawTimeMeter() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(1010, 128);
	VideoManager->DrawImage(_universal_time_meter);
	// Draw all character portraits
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	//BattleEnemyActor * e = GetEnemyActorAt(_selected_target_index);

	GLOBAL_TARGET target_type = GLOBAL_TARGET_INVALID;
	bool selected;// = false;

	if (_cursor_state == CURSOR_SELECT_TARGET || _cursor_state == CURSOR_SELECT_ATTACK_POINT)
	{
		if (_action_window->GetActionCategory() == ACTION_TYPE_ITEM)
		{
			target_type = _action_window->GetActionTargetType();
		}
		//FIX ME Work in option for flee
		else
		{
			//FIX ME
// 			target_type = action_window->GetActionTargetType();
		}
	}

	//FIX ME Below is the logic that should be used...requires change to UpdateTargetSelection code
	for (uint32 i = 0; i < _character_actors.size(); i++)
	{
		selected = false;

		if (_cursor_state == CURSOR_SELECT_TARGET || _cursor_state == CURSOR_SELECT_ATTACK_POINT)
		{
			//FIX ME Temp code.  Instead, check chosen item or skill's
			//Target Type.  If PARTY and _selected_target is a BattleCharacterActor,
			//loop through and highlight all characters
			if (target_type == GLOBAL_TARGET_PARTY)
			{
				if (!_selected_target->IsEnemy())
				{
					selected = true;
				}
			}
			else
			{
				if (!_selected_target->IsEnemy() && _selected_target == _character_actors[i])
				{
					selected = true;
				}
			}
		}

		_character_actors[i]->DrawTimePortrait(selected);
	}

	// Draw all enemy sprites
	// FIX ME use some logic for targeting highlight, loop on _selected_actor_arguments
	for (uint32 i = 0; i < _enemy_actors.size(); i++)
	{
		selected = false;

		if (_cursor_state == CURSOR_SELECT_TARGET || _cursor_state == CURSOR_SELECT_ATTACK_POINT)
		{
			//FIX ME Temp code.  Instead, check chosen item or skill's
			//Target Type.  If PARTY and _selected_target is a BattleCharacterActor,
			//loop through and highlight all characters
			if (target_type == GLOBAL_TARGET_PARTY)
			{
				if (_selected_target->IsEnemy())
				{
					selected = true;
				}
			}
			else
			{
				if (_selected_target->IsEnemy() && _selected_target == _enemy_actors[i])
				{
					selected = true;
				}
			}
		}
		_enemy_actors[i]->DrawTimePortrait(selected);
	}
} // void BattleMode::_DrawTimeMeter()

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Miscellaneous Code
////////////////////////////////////////////////////////////////////////////////

bool _TEMPIsA1Smaller(BattleEnemyActor* a1, BattleEnemyActor* a2) {
	if (a1->GetYLocation() - a1->GetActor()->GetSpriteHeight() < a2->GetYLocation() - a2->GetActor()->GetSpriteHeight())
		return true;

	return false;
}

// 
// Ascending Y sorting functor. We want to compare the actual objects, NOT pointers!
struct AscendingYSort {
	bool operator()(BattleEnemyActor* a1, BattleEnemyActor* a2)
	{
		//return ((*a1) < (*a2));
		return _TEMPIsA1Smaller(a1, a2);
	}
};



uint32 BattleMode::_NumberEnemiesAlive() const {
	uint32 enemy_count = 0;
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		if (_enemy_actors[i]->IsAlive()) {
			enemy_count++;
		}
	}
	return enemy_count;
}



uint32 BattleMode::_NumberCharactersAlive() const {
	uint32 character_count = 0;
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		if (_character_actors[i]->IsAlive()) {
			character_count++;
		}
	}
	return character_count;
}


// Sets whether an action is being performed or not
void BattleMode::SetPerformingScript(bool is_performing, ScriptEvent* se) {
	// Check if a script has just ended. Set the script to stop performing and pop the script from the front of the queue
	// ANDY: Only one script will be running at a time, so only need to check the incoming bool

	if (!is_performing) { // == false && _performing_script == true)
		// Remove the first scripted event from the queue
		// _script_queue.front().GetSource() is always either BattleEnemyActor or BattleCharacterActor
		//IBattleActor * source = dynamic_cast<IBattleActor*>(_script_queue.front().GetSource());
		//IBattleActor* source = _script_queue.front().GetSource();
		BattleActor* source = (*_active_se).GetSource();
		if (source) {
			source->SetQueuedToPerform(false);
			//ScriptEvent t = *_active_se;

			std::list<private_battle::ScriptEvent*>::iterator it = _script_queue.begin();
			while (it != _script_queue.end()) {
				if ((*it) == _active_se) {
					_script_queue.erase(it);
					break;
				}
				it++;
			}
			//_script_queue.erase(_active_se);
			//_script_queue.pop_front();
			//_script_queue.remove(t);
			//FIX ME Use char and enemy stats
			source->ResetWaitTime();
			_active_se = NULL;
		}
		else {
			cerr << "Invalid IBattleActor pointer in SetPerformingScript()" << endl;
			SystemManager->ExitGame();
		}
	}
	else {// if (is_performing && !_performing_script)
		if (se == NULL) {
			cerr << "Invalid IBattleActor pointer in SetPerformingScript()" << endl;
			SystemManager->ExitGame();
		}
	}

	_performing_script = is_performing;
	_active_se = se;
}



void BattleMode::RemoveScriptedEventsForActor(BattleActor * actor) {
//void BattleMode::RemoveScriptedEventsForActor(IBattleActor * actor) {
	std::list<private_battle::ScriptEvent*>::iterator it = _script_queue.begin();

	while (it != _script_queue.end()) {
		if ((*it)->GetSource() == actor) {
			if ((*it)->GetItem())
				(*it)->GetItem()->IncrementCount(1);
			it = _script_queue.erase(it);	//remove this location
		}
		else {
			it++;
		}
	}
}

void BattleMode::FreezeTimers()
{
	std::list<private_battle::ScriptEvent*>::iterator it = _script_queue.begin();
	//Pause scripts
	while (it != _script_queue.end())
	{
		if ((*it)->GetWarmUpTime()->IsRunning())
			(*it)->GetWarmUpTime()->Pause();

		++it;
	}

	//Pause characters
	for (uint32 i = 0; i < _character_actors.size(); ++i)
	{
		if (_character_actors.at(i)->GetWaitTime()->IsRunning())
			_character_actors.at(i)->GetWaitTime()->Pause();
	}

	//Pause enemies
	for (uint32 i = 0; i < _enemy_actors.size(); ++i)
	{
		if (_enemy_actors.at(i)->GetWaitTime()->IsRunning())
			_enemy_actors.at(i)->GetWaitTime()->Pause();
	}
}


void BattleMode::UnFreezeTimers()
{
	//FIX ME Do not unpause timers for paralyzed actors
	//Unpause scripts
	std::list<private_battle::ScriptEvent*>::iterator it = _script_queue.begin();
	//Pause scripts
	while (it != _script_queue.end())
	{
		if ((*it)->GetWarmUpTime()->IsPaused())
			(*it)->GetWarmUpTime()->Run();

		++it;
	}

	//Unpause characters
	for (uint32 i = 0; i < _character_actors.size(); ++i)
	{
		if (_character_actors.at(i)->GetWaitTime()->IsPaused())
			_character_actors.at(i)->GetWaitTime()->Run();
	}

	//Unpause enemies
	for (uint32 i = 0; i < _enemy_actors.size(); ++i)
	{
		if (_enemy_actors.at(i)->GetWaitTime()->IsPaused())
			_enemy_actors.at(i)->GetWaitTime()->Run();
	}
}


// Handle player victory
void BattleMode::PlayerVictory() {
	GlobalManager->AddDrunes(_victory_money);
	// Give some experience for each character in the party
	for (uint32 i = 0; i < _character_actors.size(); ++i) {
		GlobalCharacter* character = _character_actors.at(i)->GetActor();
		if (character->GetExperienceForNextLevel() < _victory_xp) {
			_victory_level = true;
// 			AudioManager->PlaySound("snd/level_up.wav");
			if (character->GetExperienceLevel() == 1) { // Character is upgrading to level 2
				_victory_skill = true;
				character->AddSkill(2);
			}
		}
		// ROOTS: This function returns a bool indicating if there was any character growth... needs to be checked
		character->AddExperiencePoints(_victory_xp);
		_character_actors.at(i)->SetSkillPoints(_character_actors.at(i)->GetSkillPoints() + _victory_sp);
	}
}


// Handle player defeat
void BattleMode::PlayerDefeat() {
	_ShutDown();
	ModeManager->PopAll();
	BootMode *BM = new BootMode();
	ModeManager->Push(BM);
}



void BattleMode::SwapCharacters(BattleCharacterActor * ActorToRemove, BattleCharacterActor * ActorToAdd) {
	// Remove 'ActorToRemove'
	for (std::deque < BattleCharacterActor * >::iterator it = _character_actors.begin(); it != _character_actors.end(); it++) {
		if (*it == ActorToRemove) {
			_character_actors.erase(it);
			break;
		}
	}

	// set location and origin to removing characters location and origin
	ActorToAdd->SetXOrigin(ActorToRemove->GetXOrigin());
	ActorToAdd->SetYOrigin(ActorToRemove->GetYOrigin());
	ActorToAdd->SetXLocation(static_cast<float>(ActorToRemove->GetXOrigin()));
	ActorToAdd->SetYLocation(static_cast<float>(ActorToRemove->GetYOrigin()));

	_character_actors.push_back(ActorToAdd);	//add the other character to battle
}



uint32 BattleMode::GetIndexOfFirstAliveEnemy() const {
	std::deque<private_battle::BattleEnemyActor*>::const_iterator it = _enemy_actors.begin();
	for (uint32 i = 0; it != _enemy_actors.end(); i++, it++) {
		if ((*it)->IsAlive()) {
			return i;
		}
	}

	//Should not be reached
	return INVALID_BATTLE_ACTOR_INDEX;
}


uint32 BattleMode::GetIndexOfLastAliveEnemy() const {
	std::deque<private_battle::BattleEnemyActor*>::const_iterator it = _enemy_actors.end()-1;
	for (int32 i = _enemy_actors.size()-1; i >= 0; i--, it--) {
		if ((*it)->IsAlive()) {
			return i;
		}
	}

	//Should not be reached
	return INVALID_BATTLE_ACTOR_INDEX;
}



uint32 BattleMode::GetIndexOfFirstIdleCharacter() const {
	BattleCharacterActor *bca;
	deque<BattleCharacterActor*>::const_iterator it = _character_actors.begin();

	for (uint32 i = 0; it != _character_actors.end(); i++, it++) {
		bca = (*it);
		if (!bca->IsQueuedToPerform() && bca->GetActor()->IsAlive() && bca->GetWaitTime()->IsFinished())
		{
			return i;
		}
	}

	return INVALID_BATTLE_ACTOR_INDEX;
}



uint32 BattleMode::GetIndexOfCharacter(BattleCharacterActor * const Actor) const {
	deque<BattleCharacterActor*>::const_iterator it = _character_actors.begin();
	for (int32 i = 0; it != _character_actors.end(); i++, it++) {
		if (*it == Actor)
			return i;
	}

	//Should not be reached
	return INVALID_BATTLE_ACTOR_INDEX;
}



uint32 BattleMode::GetIndexOfNextAliveEnemy(bool move_upward) const {
	//BattleCharacterActor *bca;

	if (move_upward)
	{
		for (uint32 i = _selected_target_index + 1; i < _enemy_actors.size(); ++i)
		{
			if (_enemy_actors[i]->IsAlive())
			{
				return i;
			}
		}
		for (uint32 i = 0; i <= _selected_target_index; ++i)
		{
			if (_enemy_actors[i]->IsAlive())
			{
				return i;
			}
		}
		
		//Should never be reached
		return INVALID_BATTLE_ACTOR_INDEX;
	}
	else
	{
		for (int32 i = static_cast<int32>(_selected_target_index) - 1; i >= 0; --i)
		{
			if (_enemy_actors[i]->IsAlive())
			{
				return i;
			}
		}
		for (int32 i = static_cast<int32>(_enemy_actors.size()) - 1; i >= static_cast<int32>(_selected_target_index); --i)
		{
			if (_enemy_actors[i]->IsAlive())
			{
				return i;
			}
		}
		
		//Should never be reached
		return INVALID_BATTLE_ACTOR_INDEX;
	}
}

} // namespace hoa_battle
