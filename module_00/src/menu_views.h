///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    menu_views.h
 * \author  Daniel Steuernol steu@allacrost.org
 * \date    Last Updated: January 24th, 2006
 * \brief   Header file for the different menu views.
 *
 * This code handles the different views that the user will see while the 
 * is in menu mode, (the main in-game menu). This mode's primary objectives
 * are to allow the user to view stastics about their party and manage inventory
 * and equipment.
 *
 *****************************************************************************/
 
#ifndef __MENU_VIEWS__
#define __MENU_VIEWS__

#include "utils.h"
#include <string>
#include <vector>
#include "video.h"
#include "defs.h"
#include "engine.h"
#include "gui.h"
#include "global.h"
#include "menu_views.h"

//! All menu mode calls are in this namespace
namespace hoa_menu {

//! This namespace is for private menu stuff only
namespace private_menu {
}

/*!***********************************************************************
 * \brief Represents the inventory window to browse the party's inventory
 *
 * There probably should only be one of these windows.  It will contain
 * all the necessary stuff to handle the party's inventory.
 *************************************************************************/
class InventoryWindow : public hoa_video::MenuWindow
{
private:
	//! OptionBox to display all the items
	hoa_video::OptionBox _inventory_items;
	//! Flag to specify if the inventory is active
	bool _inventory_active;
public:
	//! Class Constructor
	InventoryWindow();
	//! Class Destructor
	~InventoryWindow();

	//! Change the inventory active flag
	void Activate(bool new_status);
	bool IsActive() { return _inventory_active; }
	
	void Update();

	//! \brief Draw the inventory window
	//! Takes care of drawing the inventory window to the screen.
	bool Draw();
};

/*!***********************************************************************
 * \brief Represents an individual character window for the in-game menu.
 * 
 * There should be one of these windows for each character in the game.
 * It will contain all the information to be drawn for that character, and
 * also handles the placement of this.
 *************************************************************************/
class CharacterWindow : public hoa_video::MenuWindow
{
private:
	//! The name of the character that this window corresponds(sp?) to
	uint32 _char_id;
	//! The image of the character
	hoa_video::StillImage _portrait;
public:
	/*!
	 * \brief CharacterWindow Default constructor
	 */
	CharacterWindow();
	/*!
     * \brief CharacterWindow Destructor
	 */
	~CharacterWindow();

	//! Set the character for this window
	void SetCharacter(hoa_global::GlobalCharacter *character);

	/*!
	 * \brief render this window to the screen.
	 */
	bool Draw();
};

}

#endif
