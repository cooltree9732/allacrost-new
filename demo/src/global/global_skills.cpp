////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    global_skills.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \brief   Source file for global game skills.
 *****************************************************************************/

#include <iostream>

#include "utils.h"
#include "video.h"
#include "script.h"
#include "battle_actors.h"

#include "global.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_script;



namespace hoa_global {

// ****************************************************************************
// ***** GlobalElementalEffect
// ****************************************************************************

// ****************************************************************************
// ***** GlobalStatusEffect
// ****************************************************************************

bool GlobalStatusEffect::IncrementIntensity(uint8 amount) {
	// Intensity can not be increased beyond the upper bound "extreme"
	if (_intensity == GLOBAL_INTENSITY_POS_EXTREME) {
		return false;
	}

	if (amount == 0) {
		if (GLOBAL_DEBUG) fprintf(stderr, "WARNING: passed 0 for amount argument to increase intensity of status effect\n");
		return false;
	}

// Raging_Hog: changed these _intensity -variables to _intesity_level -variables
	if (amount < 10) {
		// _intensity += amount;
		if (_intensity > GLOBAL_INTENSITY_POS_EXTREME) {
			_intensity = GLOBAL_INTENSITY_POS_EXTREME;
			return false;
		}
		else {
			return true;
		}
	}
	// This is done to protect against the possibility of an overflow condition
	else {
		if (GLOBAL_DEBUG) fprintf(stderr, "WARNING: amount argument was > 10 to increase intensity of status effect\n");

		if (_intensity != GLOBAL_INTENSITY_POS_EXTREME) {
			_intensity = GLOBAL_INTENSITY_POS_EXTREME;
		}
		return false;
	}
} // bool GlobalStatusEffect::IncrementIntensity(uint8 amount)



bool GlobalStatusEffect::DecrementIntensity(uint8 amount) {
	if (_intensity == GLOBAL_INTENSITY_INVALID) {
		return false;
	}

	if (amount == 0) {
		if (GLOBAL_DEBUG) fprintf(stderr, "WARNING: passed 0 for amount argument to decrease intensity of status effect\n");
		return false;
	}

	if (amount <= _intensity) {
		// _intensity -= amount;
		return true;
	}
	// This is done to protect against the possibility of an overflow condition
	else {
		if (_intensity != GLOBAL_INTENSITY_NEUTRAL) {
			_intensity = GLOBAL_INTENSITY_NEUTRAL;
		}
		return false;
	}
} // bool GlobalStatusEffect::DecrementIntensity(uint8 amount)

// ****************************************************************************
// ***** GlobalSkill
// ****************************************************************************

GlobalSkill::GlobalSkill(uint32 id) : _id(id) {
	// TODO: Use the id to look up the approrpriate script and load the skill data

	// TEMP: Only one type of skill is defined: Sword Slash
	/*_name = MakeUnicodeString("Sword Slash");
	_type = GLOBAL_SKILL_ATTACK;
	_target_type = GLOBAL_TARGET_ATTACK_POINT;
	_sp_required = 0;
	_warmup_time = 0;
	_cooldown_time = 0;
	_level_required = 1;*/
	_Load();
}

void GlobalSkill::_Load() {
	// A pointer to the skill script which will be used to load this skill
	ReadScriptDescriptor *skill_script = NULL;

	if (_id >= 1 && _id <= 10000) {
		_type = GLOBAL_SKILL_ATTACK;
		skill_script = &(GlobalManager->_attack_skills_script);
	}
	else if (_id >= 10001 && _id <= 20000) {
		_type = GLOBAL_SKILL_DEFEND;
		skill_script = &(GlobalManager->_defend_skills_script);
	}
	else if (_id >= 20001 && _id <= 30001) {
		_type = GLOBAL_SKILL_SUPPORT;
		skill_script = &(GlobalManager->_support_skills_script);
	}
	else {
		_type = GLOBAL_SKILL_INVALID;
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalSkill::_Load() failed because it had an invalid id value: " << _id << endl;
		_id = 0;
		return;
	}

	// Load the item data from the script
	if (skill_script->DoesTableExist(_id) == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalSkill::_Load() failed because there was no skill defined for the id: " << _id << endl;
		_id = 0;
		return;
	}
	skill_script->OpenTable(_id);

	_name = MakeUnicodeString(skill_script->ReadString("name"));
	_description = MakeUnicodeString(skill_script->ReadString("description"));
	_target_type = static_cast<GLOBAL_TARGET>(skill_script->ReadInt("target_type"));
	_sp_required = skill_script->ReadInt("sp_required");
	_warmup_time = skill_script->ReadInt("warmup_time");
	_cooldown_time = skill_script->ReadInt("cooldown_time");
	_level_required = skill_script->ReadInt("level_required");
	_usage = static_cast<GLOBAL_USE>(skill_script->ReadInt("usage"));
	_target_alignment = static_cast<GLOBAL_ALIGNMENT>(skill_script->ReadInt("target_alignment"));
	if (skill_script->DoesFunctionExist("BattleExecute"))
		_battle_execute_function = skill_script->ReadFunctionPointer("BattleExecute");
	if (skill_script->DoesFunctionExist("MenuExecut"))
 		_menu_execute_function = skill_script->ReadFunctionPointer("MenuExecute");

	skill_script->CloseTable();

	if (skill_script->IsErrorDetected()) {
		cerr << "GLOBAL ERROR: GlobalSkill::_Load() experienced errors when reading Lua data: " << endl;
		cerr << skill_script->GetErrorMessages() << endl;
	}
} // void GlobalSkill::_Load()



void GlobalSkill::BattleExecute(hoa_battle::private_battle::BattleActor* target, hoa_battle::private_battle::BattleActor* instigator) {
	if (_battle_execute_function.is_valid() == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalSkill::BattleExecute() failed because the Lua function "
				<< "pointer was invalid for skill: " << _id << endl;
		return;
	}

	if (GetSPRequired() > instigator->GetSkillPoints()) {
		cerr << "GLOBAL ERROR: GlobalSkill::BattleExecute() failed because there was an insufficient amount of "
			<< "skill points to execute the skill" << endl;
	}

	ScriptCallFunction<void>(_battle_execute_function, target, instigator);
}



void GlobalSkill::MenuExecute(GlobalCharacter* target, GlobalCharacter* instigator) {
	if (_menu_execute_function.is_valid() == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalSkill::MenuExecute() failed because the Lua function "
				<< "pointer was invalid for skill: " << _id << endl;
		return;
	}

	if (GetSPRequired() > instigator->GetSkillPoints()) {
		cerr << "GLOBAL ERROR: GlobalSkill::MenuExecute() failed because there was an insufficient amount of "
			<< "skill points to execute the skill" << endl;
		return;
	}

// 	ScriptCallFunction<void>(_menu_execute_function, target, instigator);
}



GlobalSkill::~GlobalSkill() {
	// TODO: Close the Lua script referenced by the _function member?

	for (uint32 i = 0; i < _elemental_effects.size(); i++) {
		delete _elemental_effects[i];
	}
	_elemental_effects.empty();

	for (uint32 i = 0; i < _status_effects.size(); i++) {
		delete _status_effects[i].second;
	}
	_status_effects.empty();
}

} // namespace hoa_global
