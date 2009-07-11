///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_utils.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for shop mode utility code.
***
*** This file contains common code that is shared among the various shop mode
*** classes.
*** ***************************************************************************/

#include "video.h"

#include "global.h"

#include "shop_utils.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_global;

namespace hoa_shop {

namespace private_shop {

// *****************************************************************************
// ***** ObjectInfoWindow
// ***************************************************************************

ObjectInfoWindow::ObjectInfoWindow() {
	_is_weapon = false;
	_is_armor  = false;

	// (1) Create the info window in the bottom right-hand section of the screen
	MenuWindow::Create(800.0f, 300.0f, ~VIDEO_MENU_EDGE_TOP);
	MenuWindow::SetPosition(112.0f, 184.0f);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();

	// (2) Initialize the object to NULL, so that no information is displayed
	_object = NULL;

	// (3) Initialize the description text box in the lower section of the window
	description.SetOwner(this);
	description.SetPosition(25.0f, 150.0f);
	description.SetDimensions(550.0f, 80.0f);
	description.SetDisplaySpeed(30);
	description.SetTextStyle(TextStyle());
	description.SetDisplayMode(VIDEO_TEXT_INSTANT);
	description.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

	// (4) Initialize the properties text box in the upper right section of the window
	properties.SetOwner(this);
	properties.SetPosition(450.0f, 217.0f);
	properties.SetDimensions(300.0f, 80.0f);
	properties.SetDisplaySpeed(30);
	properties.SetTextStyle(TextStyle());
	properties.SetDisplayMode(VIDEO_TEXT_INSTANT);
	properties.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

	// (5) Load character icons
	_LoadCharacterIcons();
}



ObjectInfoWindow::~ObjectInfoWindow() {
	MenuWindow::Destroy();
}



void ObjectInfoWindow::SetObject(GlobalObject* obj) {
	_object = obj;
	_is_weapon = false;
	_is_armor  = false;

	_usableBy.clear();
	_statVariance.clear();
	_metaVariance.clear();

	if (obj == NULL) {
		description.ClearText();
		properties.ClearText();
		return;
	}

	if (obj->GetObjectType() == GLOBAL_OBJECT_WEAPON ||
	    obj->GetObjectType() == GLOBAL_OBJECT_HEAD_ARMOR ||
	    obj->GetObjectType() == GLOBAL_OBJECT_TORSO_ARMOR ||
	    obj->GetObjectType() == GLOBAL_OBJECT_ARM_ARMOR ||
	    obj->GetObjectType() == GLOBAL_OBJECT_LEG_ARMOR) {
		uint32 partysize = GlobalManager->GetActiveParty()->GetPartySize();
		GlobalCharacter* ch;

		if(obj->GetObjectType() == GLOBAL_OBJECT_WEAPON) {
			_is_weapon = true;
		}
		else {
			_is_armor = true;
		}

		for(uint32 i = 0; i < partysize; i++) {
			ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(i));

			// If the currently selected character can equip this weapon, calculate the +/- effects the weapon/armor has on stats.
			if(static_cast<GlobalArmor*>(obj)->GetUsableBy() & ch->GetID()) {
				int32 variance = 0;
				int32 metaVariance = 0;
				switch(obj->GetObjectType()) {
					case GLOBAL_OBJECT_WEAPON:
						variance = static_cast<GlobalWeapon*>(obj)->GetPhysicalAttack() - ch->GetWeaponEquipped()->GetPhysicalAttack();
						metaVariance = static_cast<GlobalWeapon*>(obj)->GetMetaphysicalAttack() - ch->GetWeaponEquipped()->GetMetaphysicalAttack();
						break;
					case GLOBAL_OBJECT_ARM_ARMOR:
						variance = static_cast<GlobalArmor*>(obj)->GetPhysicalDefense() - ch->GetArmArmorEquipped()->GetPhysicalDefense();
						metaVariance = static_cast<GlobalArmor*>(obj)->GetMetaphysicalDefense() - ch->GetArmArmorEquipped()->GetMetaphysicalDefense();
						break;
					case GLOBAL_OBJECT_TORSO_ARMOR:
						variance = static_cast<GlobalArmor*>(obj)->GetPhysicalDefense() - ch->GetTorsoArmorEquipped()->GetPhysicalDefense();
						metaVariance = static_cast<GlobalArmor*>(obj)->GetMetaphysicalDefense() - ch->GetTorsoArmorEquipped()->GetMetaphysicalDefense();
						break;
					case GLOBAL_OBJECT_HEAD_ARMOR:
						variance = static_cast<GlobalArmor*>(obj)->GetPhysicalDefense() - ch->GetHeadArmorEquipped()->GetPhysicalDefense();
						metaVariance = static_cast<GlobalArmor*>(obj)->GetMetaphysicalDefense() - ch->GetHeadArmorEquipped()->GetMetaphysicalDefense();
					 	break;
					case GLOBAL_OBJECT_LEG_ARMOR:
						variance = static_cast<GlobalArmor*>(obj)->GetPhysicalDefense() - ch->GetLegArmorEquipped()->GetPhysicalDefense();
						metaVariance = static_cast<GlobalArmor*>(obj)->GetMetaphysicalDefense() - ch->GetLegArmorEquipped()->GetMetaphysicalDefense();
						break;
					default: break;
				}

				// Put variance info into the corresponding vectors for currently selected character's index.
				_usableBy.push_back(ch);
				_statVariance.push_back(variance);
				_metaVariance.push_back(metaVariance);
			}
			else {
				_usableBy.push_back(NULL);
				_statVariance.push_back(0);
				_metaVariance.push_back(0);
			}
		}
	}

	description.SetDisplayText(_object->GetDescription());

	// Determine what properties to display depending on what type of object this is
	switch (obj->GetObjectType()) {
		case GLOBAL_OBJECT_WEAPON:
			GlobalWeapon *weapon;
			weapon = dynamic_cast<GlobalWeapon*>(obj);
			properties.SetDisplayText("PHYS ATK: " + NumberToString(weapon->GetPhysicalAttack()) + "\n" + "META ATK: " + NumberToString(weapon->GetMetaphysicalAttack()) + "\n" +
				"Equippable by: "
			);
			break;
		case GLOBAL_OBJECT_HEAD_ARMOR:
		case GLOBAL_OBJECT_TORSO_ARMOR:
		case GLOBAL_OBJECT_ARM_ARMOR:
		case GLOBAL_OBJECT_LEG_ARMOR:
			GlobalArmor *armor;
			armor = dynamic_cast<GlobalArmor*>(obj);
			properties.SetDisplayText("           DEF: " + NumberToString(armor->GetPhysicalDefense()) + "\n" + "META DEF: " + NumberToString(armor->GetMetaphysicalDefense())
			);
			break;
		default:
			properties.ClearText();
			break;
	}
} // void ObjectInfoWindow::SetObject(GlobalObject* obj)



void ObjectInfoWindow::Draw() {
	MenuWindow::Draw();
	if (_object == NULL) {
		return;
	}

	VideoManager->Move(350.0f, 240.0f);
	// Draw the object's icon and name
	_object->GetIconImage().Draw();
	VideoManager->MoveRelative(60.0f, 20.0f);
	VideoManager->Text()->Draw(_object->GetName());

	if(_is_weapon || _is_armor) {
		hoa_utils::ustring atk_or_def;
		if(_is_weapon) atk_or_def = MakeUnicodeString("ATK:");
		if(_is_armor)  atk_or_def = MakeUnicodeString("DEF:");

		VideoManager->Move(335.0f, 110.0f);

		for(uint32 i = 0; i < /*CHANGE TO PARTYSIZE*/_usableBy.size(); i++) {

			// if selected character is able to equip this item
			if(_usableBy[i] != NULL) {
				//VideoManager->Text()->Draw(_usableBy[i]->GetName());
				_character_icons[i].Draw();
				VideoManager->MoveRelative(47.0f, 32.0f);

				VideoManager->Text()->Draw(atk_or_def, TextStyle("default", Color::white, VIDEO_TEXT_SHADOW_DARK));
				VideoManager->MoveRelative(47.0f, 0.0f);
				if(_statVariance[i] > 0) {
					VideoManager->Text()->Draw("+" + NumberToString(_statVariance[i]), TextStyle("default", Color::green, VIDEO_TEXT_SHADOW_DARK));
				}
				else if(_statVariance[i] == 0) {
					VideoManager->Text()->Draw("+" + NumberToString(_statVariance[i]), TextStyle("default", Color::gray, VIDEO_TEXT_SHADOW_DARK) );
				}
				else if(_statVariance[i] < 0) {
					VideoManager->MoveRelative(2.0f, 0.0f); // OCD + Allignment problem :)
					VideoManager->Text()->Draw(NumberToString(_statVariance[i]), TextStyle("default", Color::red, VIDEO_TEXT_SHADOW_DARK) );
					VideoManager->MoveRelative(-2.0f, 0.0f);
				}

				VideoManager->MoveRelative(-47.0f, -32.0f);
				VideoManager->Text()->Draw("MET: ", TextStyle("default", Color::white, VIDEO_TEXT_SHADOW_DARK));
				VideoManager->MoveRelative(47.0f, 0.0f);

				if(_metaVariance[i] > 0) {
					VideoManager->Text()->Draw("+" + NumberToString(_metaVariance[i]), TextStyle("default", Color::green, VIDEO_TEXT_SHADOW_DARK));
				}
				else if(_metaVariance[i] == 0) {
					VideoManager->Text()->Draw("+" + NumberToString(_metaVariance[i]), TextStyle("default", Color::gray, VIDEO_TEXT_SHADOW_DARK) );
				}
				else if(_metaVariance[i] < 0) {
					VideoManager->MoveRelative(2.0f, 0.0f); // OCD + Allignment problem :)
					VideoManager->Text()->Draw(NumberToString(_metaVariance[i]), TextStyle("default", Color::red, VIDEO_TEXT_SHADOW_DARK) );
					VideoManager->MoveRelative(-2.0f, 0.0f);
				}
				VideoManager->MoveRelative(30.0f, 0.0f);
			}
			// if selected character can't equip this item
			else {
				_character_icons_bw[i].Draw();
				VideoManager->MoveRelative(124.0f, 0.0f);
			}


		}
	}
	// Draw the object's description and stats text boxes
	description.Draw();
	properties.Draw();
}


void ObjectInfoWindow::_LoadCharacterIcons() {
	uint32 partysize = GlobalManager->GetActiveParty()->GetPartySize();		// Number of characters in party
	GlobalCharacter* ch;  													// Used to point to individual character
	hoa_video::StillImage icon;												// Stores color icon image
	hoa_video::StillImage icon_bw;											// Store b&w icon image

	for(uint32 i = 0; i < partysize; i++) {
			ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(i));

			// load color character icon
			if (icon.Load("img/icons/actors/characters/" + ch->GetFilename() + ".png", 45.0f, 45.0f) == false)
				cerr << "SHOPMODE: Couldn't load character icon: " + ch->GetFilename() + ".png" << endl;

			// load black and white character icon
			if (icon_bw.Load("img/icons/actors/characters/" + ch->GetFilename() + "_bw.png", 45.0f, 45.0f) == false)
				cerr << "SHOPMODE: Couldn't load character icon: " + ch->GetFilename() + "_bw.png" << endl;

			// put color and black and white icons into appropiate vectors
			_character_icons.push_back(icon);
			_character_icons_bw.push_back(icon_bw);
	}
}

} // namespace private_shop

} // namespace hoa_shop
