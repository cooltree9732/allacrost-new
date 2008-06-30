////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global_skills.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for global game skills.
*** ***************************************************************************/

#include <iostream>

#include "video.h"
#include "global.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_script;


namespace hoa_global {

using namespace private_global;

// -----------------------------------------------------------------------------
// GlobalSkill class
// -----------------------------------------------------------------------------

GlobalSkill::GlobalSkill(uint32 id) :
	_id(id),
	_battle_execute_function(NULL),
	_menu_execute_function(NULL)
{
	// A pointer to the skill script which will be used to load this skill
	ReadScriptDescriptor *skill_script = NULL;

	if (_id == 0) {
		_type = GLOBAL_SKILL_INVALID;
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalSkill constructor failed because it had an invalid id value: " << _id << endl;
		return;
	}
	else if (_id <= MAX_ATTACK_ID) {
		_type = GLOBAL_SKILL_ATTACK;
		skill_script = &(GlobalManager->_attack_skills_script);
	}
	else if (_id <= MAX_DEFEND_ID) {
		_type = GLOBAL_SKILL_DEFEND;
		skill_script = &(GlobalManager->_defend_skills_script);
	}
	else if (_id <= MAX_SUPPORT_ID) {
		_type = GLOBAL_SKILL_SUPPORT;
		skill_script = &(GlobalManager->_support_skills_script);
	}
	else {
		_type = GLOBAL_SKILL_INVALID;
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalSkill constructor failed because it had an invalid id value: " << _id << endl;
		_id = 0;
		return;
	}

	// make sure the script is open
	if (!skill_script->IsFileOpen())
		skill_script->OpenFile();

	// Load the skill properties from the script
	if (skill_script->DoesTableExist(_id) == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalSkill constructor failed because there was no skill defined for the id: " << _id << endl;
		_id = 0;
		return;
	}

	skill_script->OpenTable(_id);
	_name = MakeUnicodeString(skill_script->ReadString("name"));
	if (skill_script->DoesStringExist("description"))
		_description = MakeUnicodeString(skill_script->ReadString("description"));
	_sp_required = skill_script->ReadUInt("sp_required");
	_warmup_time = skill_script->ReadUInt("warmup_time");
	_cooldown_time = skill_script->ReadUInt("cooldown_time");
	_target_type = static_cast<GLOBAL_TARGET>(skill_script->ReadInt("target_type"));
	_target_ally = skill_script->ReadBool("target_ally");

	if (skill_script->DoesFunctionExist("BattleExecute")) {
		_battle_execute_function = new ScriptObject();
		*_battle_execute_function = skill_script->ReadFunctionPointer("BattleExecute");
	}
	if (skill_script->DoesFunctionExist("MenuExecute")) {
		_menu_execute_function = new ScriptObject();
		*_menu_execute_function = skill_script->ReadFunctionPointer("MenuExecute");
	}

	skill_script->CloseTable();

	if (skill_script->IsErrorDetected()) {
		cerr << "GLOBAL ERROR: GlobalSkill constructor experienced errors when reading Lua data. They are as follows: " << endl;
		cerr << skill_script->GetErrorMessages() << endl;
	}
} // GlobalSkill::GlobalSkill()



GlobalSkill::~GlobalSkill() {
	if (_battle_execute_function != NULL) {
		delete _battle_execute_function;
		_battle_execute_function = NULL;
	}

	if (_menu_execute_function != NULL) {
		delete _menu_execute_function;
		_menu_execute_function = NULL;
	}

// 	for (uint32 i = 0; i < _elemental_effects.size(); i++) {
// 		delete _elemental_effects[i];
// 	}
// 	_elemental_effects.empty();
// 
// 	for (uint32 i = 0; i < _status_effects.size(); i++) {
// 		delete _status_effects[i].second;
// 	}
// 	_status_effects.empty();
}



GlobalSkill::GlobalSkill(const GlobalSkill& copy) {
	_name = copy._name;
	_description = copy._description;
	_id = copy._id;
	_type = copy._type;
	_sp_required = copy._sp_required;
	_warmup_time = copy._warmup_time;
	_cooldown_time = copy._cooldown_time;
	_target_type = copy._target_type;
	_target_ally = copy._target_ally;

	// Make copies of valid ScriptObject function pointers
	if (copy._battle_execute_function == NULL)
		_battle_execute_function = NULL;
	else
		_battle_execute_function = new ScriptObject(*copy._battle_execute_function);

	if (copy._menu_execute_function == NULL)
		_menu_execute_function = NULL;
	else
		_menu_execute_function = new ScriptObject(*copy._menu_execute_function);
} // GlobalSkill::GlobalSkill(const GlobalSkill& copy)



GlobalSkill& GlobalSkill::operator=(const GlobalSkill& copy) {
	if (this == &copy) // Handle self-assignment case
		return *this;

	_name = copy._name;
	_description = copy._description;
	_id = copy._id;
	_type = copy._type;
	_sp_required = copy._sp_required;
	_warmup_time = copy._warmup_time;
	_cooldown_time = copy._cooldown_time;
	_target_type = copy._target_type;
	_target_ally = copy._target_ally;

	// Make copies of valid ScriptObject function pointers
	if (copy._battle_execute_function == NULL)
		_battle_execute_function = NULL;
	else
		_battle_execute_function = new ScriptObject(*copy._battle_execute_function);

	if (copy._menu_execute_function == NULL)
		_menu_execute_function = NULL;
	else
		_menu_execute_function = new ScriptObject(*copy._menu_execute_function);

	return *this;
} // GlobalSkill& GlobalSkill::operator=(const GlobalSkill& copy)

} // namespace hoa_global
