///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    menu.h
 * \author  Daniel Steuernol steu@allacrost.org
 * \date    Last Updated: January 24th, 2006
 * \brief   Header file for menu mode interface.
 *
 * This code handles the game event processing and frame drawing when the user
 * is in menu mode, (the main in-game menu). This mode's primary objectives
 * are to allow the user to view stastics about their party and manage inventory
 * and equipment.
 *
 *****************************************************************************/

#ifndef __MENU_HEADER__
#define __MENU_HEADER__

#include "utils.h"
#include <string>
#include <vector>
#include "video.h"
#include "defs.h"
#include "engine.h"
#include "gui.h"
#include "global.h"
#include "menu_views.h"

//! All calls to menu mode are wrapped in this namespace.
namespace hoa_menu {
	
//! Determines whether the code in the hoa_menu namespace should print debug statements or not.
extern bool MENU_DEBUG;

//! An internal namespace to be used only within the menu code. Don't use this namespace anywhere else!
namespace private_menu {

//! \name MenuMode OptionBox Main options
//! \brief Constants used for the menu in the main menu mode
//@{
const uint32 MAIN_INVENTORY = 0;
const uint32 MAIN_SKILLS 	= 1;
const uint32 MAIN_EQUIPMENT = 2;
const uint32 MAIN_STATUS 	= 3;
const uint32 MAIN_OPTIONS	= 4;
const uint32 MAIN_SAVE 		= 5;
const uint32 MAIN_EXIT		= 6;
const uint32 MAIN_SIZE		= 7;
//@}
	
//! \name MenuMode Inventory Menu options
//! \brief Constants used for the inventory menu
//@{
const uint32 INV_USE 	= 0;
const uint32 INV_SORT 	= 1;
const uint32 INV_CANCEL = 2;
const uint32 INV_SIZE 	= 3;
//@}
	
//! \name MenuMode Skills Menu options
//! \brief Constants used for the skills menu
//@{
const uint32 SKILLS_CANCEL 	= 0;
const uint32 SKILLS_SIZE	= 1;
//@}

//! \name MenuMode Equipment Menu options
//! \brief Constants used for the equipment menu
//@{
const uint32 EQUIP_EQUIP	= 0;
const uint32 EQUIP_REMOVE	= 1;
const uint32 EQUIP_CANCEL	= 2;
const uint32 EQUIP_SIZE		= 3;
//@}

//! \name MenuMode Status Menu options
//! \brief Constants used for the status menu
//@{
const uint32 STATUS_NEXT	= 0;
const uint32 STATUS_PREV	= 1;
const uint32 STATUS_CANCEL	= 2;
const uint32 STATUS_SIZE	= 3;
//@}

//! \name MenuMode Options Menu options
//! \brief Constants used for the options menu
//@{
const uint32 OPTIONS_EDIT	= 0;
const uint32 OPTIONS_SAVE	= 1;
const uint32 OPTIONS_CANCEL	= 2;
const uint32 OPTIONS_SIZE	= 3;
//@}

//! \name MenuMode Save Menu options
//! \brief Constants used for the save menu
//@{
const uint32 SAVE_SAVE		= 0;
const uint32 SAVE_CANCEL	= 1;
const uint32 SAVE_SIZE		= 2;
//@}
	
//! \name MenuMode OptionBox Show Flags
//! \brief Constants to determine which option box is currently showing.
//@{
const uint32 SHOW_MAIN 		= 0;
const uint32 SHOW_INVENTORY = 1;
const uint32 SHOW_SKILLS	= 2;
const uint32 SHOW_EQUIPMENT	= 3;
const uint32 SHOW_STATUS	= 4;
const uint32 SHOW_OPTIONS	= 5;
const uint32 SHOW_SAVE		= 6;
//@}

}

/*!****************************************************************************
 * \brief Responsible for managing the game when executing in the main in-game menu.
 *
 * This code in this class and its respective partner classes is arguably one of the
 * most complex pieces of the game to date. Basic functionality in this class has been
 * working for a while, but we still have much work to do here (namely, integrating
 * map scripts). I intend to more fully document the primary operational features of
 * this class at a later time, but I would like to wait until it is in a more finalized
 * state before I do so.
 *
 * \note 1) If you change the state of random_encounters from false to true, make 
 * sure to set a valid value (< 0) for steps_till_encounter. *I might change this later*
 * 
 * \note 2) Be careful with calling the MapMode constructor, for it changes the coordinate 
 * system of the video engine without warning. Only create a new instance of this class if
 * you plan to immediately push it on top of the game stack.
 *****************************************************************************/
class MenuMode : public hoa_engine::GameMode {
private:
	friend class hoa_data::GameData;

	hoa_video::StillImage _saved_screen;
	hoa_video::StillImage _location_picture;
	std::vector<hoa_video::StillImage> _menu_images;
	std::vector<hoa_audio::MusicDescriptor> _menu_music;
	std::vector<hoa_audio::SoundDescriptor> _menu_sound;
	
	//! \name Main Display Windows
	//@{
	//! \brief The windows that are displayed in the menu mode.
	CharacterWindow _character_window0;
	CharacterWindow _character_window1;
	CharacterWindow _character_window2;
	CharacterWindow _character_window3;
	hoa_video::MenuWindow _bottom_window;
	InventoryWindow _inventory_window;
	//@}
		
	//! \brief The current option box to display
	uint32 _current_menu_showing;
	
	//! A pointer to the current menu
	hoa_video::OptionBox *_current_menu;
	
	//! The top level options in boot mode
	hoa_video::OptionBox _main_options;
	//! \brief sub-menu option boxes
	//@{
	hoa_video::OptionBox _menu_inventory;
	hoa_video::OptionBox _menu_skills;
	hoa_video::OptionBox _menu_equipment;
	hoa_video::OptionBox _menu_status;
	hoa_video::OptionBox _menu_options;
	hoa_video::OptionBox _menu_save;
	//@}
	
	//! \brief Functions to set up the option boxes
	//@{
	void _SetupOptionBoxCommonSettings(hoa_video::OptionBox *ob);
	void _SetupMainOptionBox();
	void _SetupInventoryOptionBox();
	void _SetupSkillsOptionBox();
	void _SetupEquipmentOptionBox();
	void _SetupStatusOptionBox();
	void _SetupOptionsOptionBox();
	void _SetupSaveOptionBox();
	//@}
	
	//! \brief Handler functions to deal with events for all the different menus
	//@{
	void _HandleMainMenu();
	void _HandleInventoryMenu();
	void _HandleSkillsMenu();
	void _HandleEquipmentMenu();
	void _HandleStatusMenu();
	void _HandleOptionsMenu();
	void _HandleSaveMenu();
	//@}
	
	//! The name for the font to be used in the menu
	std::string _font_name;
public:
	MenuMode();
	~MenuMode();

	void Reset();
	void Update();
	void Draw();
};

} // namespace hoa_menu

#endif
