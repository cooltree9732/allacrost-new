///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for shop mode interface
***
*** This code provides an interface for the user to purchase wares from a
*** merchant. This mode is usually entered from a map after discussing with a
*** shop keeper.
*** ***************************************************************************/

#include <iostream>

#include "defs.h"
#include "utils.h"

#include "audio.h"
#include "video.h"
#include "input.h"
#include "system.h"

#include "global.h"

#include "mode_manager.h"
#include "pause.h"

#include "shop.h"
#include "shop_root.h"
#include "shop_buy.h"
#include "shop_sell.h"
#include "shop_trade.h"
#include "shop_confirm.h"
#include "shop_leave.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_mode_manager;
using namespace hoa_shop::private_shop;
using namespace hoa_pause;

namespace hoa_shop {

bool SHOP_DEBUG = false;
// Initialize static class variable
ShopMode* ShopMode::_current_instance = NULL;

namespace private_shop {

// *****************************************************************************
// ***** ShopMedia class methods
// *****************************************************************************

// Initialize static class variable
ShopMedia* ShopMedia::_current_instance = NULL;

ShopMedia::ShopMedia() {
	if (_drunes_icon.Load("img/icons/drunes.png") == false)
		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load drunes icon image" << endl;

	if (_socket_icon.Load("img/menus/socket.png") == false)
		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load socket icon image" << endl;

	if (_equip_icon.Load("img/menus/equip.png") == false)
		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load equip icon image" << endl;

	if (ImageDescriptor::LoadMultiImageFromElementGrid(_elemental_icons, "img/icons/elemental_icons.png", 8, 9) == false) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load elemental icon images" << endl;
		return;
	}

	// TODO
// 	if (ImageDescriptor::LoadMultiImageFromElementGrid(_status_icons, "img/icons/status_icons.png", ROWS, COLS) == false) {
// 		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load status icon images" << endl;
// 		return;
// 	}

	_sounds["confirm"] = new SoundDescriptor();
	_sounds["cancel"] = new SoundDescriptor();
	_sounds["coins"] = new SoundDescriptor();
	_sounds["bump"] = new SoundDescriptor();

	uint32 sound_load_failures = 0;
	if (_sounds["confirm"]->LoadAudio("snd/confirm.wav") == false)
		sound_load_failures++;
	if (_sounds["cancel"]->LoadAudio("snd/cancel.wav") == false)
		sound_load_failures++;
	if (_sounds["coins"]->LoadAudio("snd/coins.wav") == false)
		sound_load_failures++;
	if (_sounds["bump"]->LoadAudio("snd/bump.wav") == false)
		sound_load_failures++;

	if (sound_load_failures > 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load " << sound_load_failures << " sounds needed by shop mode" << endl;
	}
}



void ShopMedia::Initialize() {
	// Temporary containers to hold all possible category text and icons
	vector<ustring> all_text;
	vector<StillImage> all_icons;

	all_text.push_back(MakeUnicodeString("Items"));
	all_text.push_back(MakeUnicodeString("Weapons"));
	all_text.push_back(MakeUnicodeString("Head Armor"));
	all_text.push_back(MakeUnicodeString("Torso Armor"));
	all_text.push_back(MakeUnicodeString("Arm Armor"));
	all_text.push_back(MakeUnicodeString("Leg Armor"));
	all_text.push_back(MakeUnicodeString("Shards"));
	all_text.push_back(MakeUnicodeString("Key Items"));
	all_text.push_back(MakeUnicodeString("All Wares"));

	if (ImageDescriptor::LoadMultiImageFromElementGrid(all_icons, "img/icons/object_category_icons.png", 3, 4) == false) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load object category icon images" << endl;
		return;
	}

	// Determine which categories are used in this shop and populate the true containers with that data
	uint8 deal_types = ShopMode::CurrentInstance()->GetDealTypes();
	uint8 bit_x = 0x01; // Used to do a bit-by-bit analysis of the obj_types variable
	for (uint8 i = 0; i < GLOBAL_OBJECT_TOTAL; i++, bit_x <<= 1) {
		// Check if the type is available by doing a bit-wise comparison
		if (deal_types & bit_x) {
			_object_category_names.push_back(all_text[i]);
			_object_category_icons.push_back(all_icons[i]);
		}
	}

	// If here is more than one category, add the text/icon for all wares
	if (_object_category_names.size() > 1) {
		_object_category_names.push_back(all_text[8]);
		_object_category_icons.push_back(all_icons[8]);
	}
}



StillImage* ShopMedia::GetElementalIcon(GLOBAL_ELEMENTAL element_type, GLOBAL_INTENSITY intensity) {
	// Row/col coordinates for where the specific icon can be found in the multi image array
	uint32 row = 0, col = 0;

	// Elemental type determines the icon's row
	switch (element_type) {
		case GLOBAL_ELEMENTAL_FIRE:
			row = 0;
			break;
		case GLOBAL_ELEMENTAL_WATER:
			row = 1;
			break;
		case GLOBAL_ELEMENTAL_VOLT:
			row = 2;
			break;
		case GLOBAL_ELEMENTAL_EARTH:
			row = 3;
			break;
		case GLOBAL_ELEMENTAL_SLICING:
			row = 4;
			break;
		case GLOBAL_ELEMENTAL_SMASHING:
			row = 5;
			break;
		case GLOBAL_ELEMENTAL_MAULING:
			row = 6;
			break;
		case GLOBAL_ELEMENTAL_PIERCING:
			row = 7;
			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid elemental type: " << element_type << endl;
			return NULL;
	}

	// Intensity determines the icon's column
	switch (intensity) {
		case GLOBAL_INTENSITY_POS_EXTREME:
			col = 0;
			break;
		case GLOBAL_INTENSITY_POS_GREATER:
			col = 1;
			break;
		case GLOBAL_INTENSITY_POS_MODERATE:
			col = 2;
			break;
		case GLOBAL_INTENSITY_POS_LESSER:
			col = 3;
			break;
		case GLOBAL_INTENSITY_NEUTRAL:
			col = 4;
			break;
		case GLOBAL_INTENSITY_NEG_LESSER:
			col = 5;
			break;
		case GLOBAL_INTENSITY_NEG_MODERATE:
			col = 6;
			break;
		case GLOBAL_INTENSITY_NEG_GREATER:
			col = 7;
			break;
		case GLOBAL_INTENSITY_NEG_EXTREME:
			col = 8;
			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid intensity level: " << intensity << endl;
			return NULL;
	}

	return &(_elemental_icons[(row * GLOBAL_INTENSITY_TOTAL) + col]);
}



SoundDescriptor* ShopMedia::GetSound(string identifier) {
	map<string, SoundDescriptor*>::iterator sound = _sounds.find(identifier);
	if (sound != _sounds.end()) {
		return sound->second;
	}
	else {
		return NULL;
	}
}

} // namespace private_shop

// *****************************************************************************
// ***** ShopMode class methods
// *****************************************************************************

ShopMode::ShopMode() :
	_initialized(false),
	_state(SHOP_STATE_ROOT),
	_deal_types(0),
	_buy_price_level(SHOP_PRICE_STANDARD),
	_sell_price_level(SHOP_PRICE_STANDARD),
	_total_costs(0),
	_total_sales(0),
	_shop_media(NULL),
	_root_interface(NULL),
	_buy_interface(NULL),
	_sell_interface(NULL),
	_trade_interface(NULL),
	_confirm_interface(NULL)
{
	mode_type = MODE_MANAGER_SHOP_MODE;
	_current_instance = this;

	// ---------- (1): Create the menu windows and set their properties
	_top_window.Create(800.0f, 96.0f, ~VIDEO_MENU_EDGE_BOTTOM);
	_top_window.SetPosition(112.0f, 684.0f);
	_top_window.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_top_window.SetDisplayMode(VIDEO_MENU_INSTANT);
	_top_window.Show();

	_middle_window.Create(800.0f, 400.0f, VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_TOP | VIDEO_MENU_EDGE_BOTTOM);
	_middle_window.SetPosition(112.0f, 604.0f);
	_middle_window.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_middle_window.SetDisplayMode(VIDEO_MENU_INSTANT);
	_middle_window.Show();

	_bottom_window.Create(800.0f, 140.0f, ~VIDEO_MENU_EDGE_TOP);
	_bottom_window.SetPosition(112.0f, 224.0f);
	_bottom_window.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_bottom_window.SetDisplayMode(VIDEO_MENU_INSTANT);
	_bottom_window.Show();

	// (2) Create the list of shop actions
	_action_options.SetOwner(&_top_window);
	_action_options.SetPosition(80.0f, 90.0f);
	_action_options.SetDimensions(640.0f, 30.0f, 5, 1, 5, 1);
	_action_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_TOP);
	_action_options.SetTextStyle(TextStyle("title28"));
	_action_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_action_options.SetCursorOffset(-55.0f, 30.0f);
	_action_options.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);

	vector<ustring> option_text;
	option_text.push_back(MakeUnicodeString("Buy"));
	option_text.push_back(MakeUnicodeString("Sell"));
	option_text.push_back(MakeUnicodeString("Trade"));
	option_text.push_back(MakeUnicodeString("Confirm"));
	option_text.push_back(MakeUnicodeString("Leave"));

	_action_options.AddOption(option_text[0]);
	_action_options.AddOption(option_text[1]);
	_action_options.AddOption(option_text[2]);
	_action_options.AddOption(option_text[3]);
	_action_options.AddOption(option_text[4]);
	_action_options.SetSelection(0);

	_action_titles.push_back(TextImage(option_text[0], TextStyle("title28")));
	_action_titles.push_back(TextImage(option_text[1], TextStyle("title28")));
	_action_titles.push_back(TextImage(option_text[2], TextStyle("title28")));
	_action_titles.push_back(TextImage(option_text[3], TextStyle("title28")));
	_action_titles.push_back(TextImage(option_text[4], TextStyle("title28")));

	// (3) Create the financial table text
	_finance_table.SetOwner(&_top_window);
	_finance_table.SetPosition(80.0f, 45.0f);
	_finance_table.SetDimensions(640.0f, 20.0f, 4, 1, 4, 1);
	_finance_table.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_finance_table.SetTextStyle(TextStyle("text22"));
	_finance_table.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	// Initialize all four options with an empty string that will be overwritten by the following method call
	for (uint32 i = 0; i < 4; i++)
		_finance_table.AddOption(ustring());
	UpdateFinances(0, 0);

	_shop_media = new ShopMedia();
	_root_interface = new RootInterface();
	_buy_interface = new BuyInterface();
	_sell_interface = new SellInterface();
	_trade_interface = new TradeInterface();
	_confirm_interface = new ConfirmInterface();
	_leave_interface = new LeaveInterface();

	try {
		_screen_backdrop = VideoManager->CaptureScreen();
	}
	catch (Exception e) {
		IF_PRINT_WARNING(SHOP_DEBUG) << e.ToString() << endl;
	}
} // ShopMode::ShopMode()



ShopMode::~ShopMode() {
	for (uint32 i = 0; i < _created_objects.size(); i++) {
		delete(_created_objects[i]);
	}
	_created_objects.clear();

	delete _shop_media;
	delete _root_interface;
	delete _buy_interface;
	delete _sell_interface;
	delete _trade_interface;
	delete _confirm_interface;
	delete _leave_interface;

	_top_window.Destroy();
	_middle_window.Destroy();
	_bottom_window.Destroy();

	if (_current_instance == this) {
		_current_instance = NULL;
		ShopMedia::SetCurrentInstance(NULL);
	}
}



void ShopMode::Reset() {
	VideoManager->SetCoordSys(0.0f, 1024.0f, 0.0f, 768.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);

	_current_instance = this;
	_shop_media->SetCurrentInstance(_shop_media);
	if (IsInitialized() == false)
		Initialize();
}



void ShopMode::Initialize() {
	if (IsInitialized() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "shop was already initialized previously" << endl;
		return;
	}

	_initialized = true;

	// ---------- (1): Determine what types of objects the shop deals in based on the managed object list
	for (uint32 i = 0; i < _created_objects.size(); i++) {
		switch (_created_objects[i]->GetObjectType()) {
			case GLOBAL_OBJECT_ITEM:
				_deal_types |= DEALS_ITEMS;
				break;
			case GLOBAL_OBJECT_WEAPON:
				_deal_types |= DEALS_WEAPONS;
				break;
			case GLOBAL_OBJECT_HEAD_ARMOR:
				_deal_types |= DEALS_HEAD_ARMOR;
				break;
			case GLOBAL_OBJECT_TORSO_ARMOR:
				_deal_types |= DEALS_TORSO_ARMOR;
				break;
			case GLOBAL_OBJECT_ARM_ARMOR:
				_deal_types |= DEALS_ARM_ARMOR;
				break;
			case GLOBAL_OBJECT_LEG_ARMOR:
				_deal_types |= DEALS_LEG_ARMOR;
				break;
			case GLOBAL_OBJECT_SHARD:
				_deal_types |= DEALS_SHARDS;
				break;
			case GLOBAL_OBJECT_KEY_ITEM:
				_deal_types |= DEALS_KEY_ITEMS;
				break;
			default:
				IF_PRINT_WARNING(SHOP_DEBUG) << "unknown object type sold in shop: " << _created_objects[i]->GetObjectType() << endl;
				break;
		}
	}

	// ---------- (2): Add objects from the player's inventory to the list of shop objects
	map<uint32, GlobalObject*>* inventory = GlobalManager->GetInventory();
	for (map<uint32, GlobalObject*>::iterator i = inventory->begin(); i != inventory->end(); i++) {
		// Check if the object already exists in the shop list and if so, set its ownership count
		map<uint32, ShopObject>::iterator shop_obj_iter = _shop_objects.find(i->second->GetID());
		if (shop_obj_iter != _shop_objects.end()) {
			shop_obj_iter->second.IncrementOwnCount(i->second->GetCount());
		}
		// Otherwise, add the shop object to the list
		else {
			ShopObject new_shop_object(i->second, false);
			_shop_objects.insert(make_pair(i->second->GetID(), new_shop_object));
		}
	}

	// ---------- (3): Initialize pricing for all shop objects
	for (map<uint32, ShopObject>::iterator i = _shop_objects.begin(); i != _shop_objects.end(); i++) {
		i->second.SetPricing(_buy_price_level, _sell_price_level);
	}

	// ---------- (4): Initialize multimedia data
	_shop_media->Initialize();

	// ---------- (4): Initialize all shop interfaces
	_root_interface->Initialize();
	_buy_interface->Initialize();
	_sell_interface->Initialize();
	_trade_interface->Initialize();
	_confirm_interface->Initialize();
	_leave_interface->Initialize();
} // void ShopMode::Initialize()



void ShopMode::Update() {
	// Pause and quit events have highest priority. If either type of event is detected, no other update processing will be done
	if (InputManager->QuitPress() == true) {
		ModeManager->Push(new PauseMode(true));
		return;
	}
	else if (InputManager->PausePress() == true) {
		ModeManager->Push(new PauseMode(false));
		return;
	}

	// When the state is at the root interface ,ShopMode needs to process user input and possibly change state
	if (_state == SHOP_STATE_ROOT) {
		SoundDescriptor* sound = NULL; // Used to hold pointers of sound objects to play

		if (InputManager->ConfirmPress()) {
			if (_action_options.GetSelection() < 0 || _action_options.GetSelection() > 4) {
				IF_PRINT_WARNING(SHOP_DEBUG) << "invalid selection in action window: " << _action_options.GetSelection() << endl;
				_action_options.SetSelection(0);
				return;
			}

			_action_options.InputConfirm();
			sound = ShopMedia::CurrentInstance()->GetSound("confirm");
			assert(sound != NULL);
			sound->Play();

			if (_action_options.GetSelection() == 0) { // Buy
				ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_BUY);
			}
			else if (_action_options.GetSelection() == 1) { // Sell
				ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_SELL);
			}
			else if (_action_options.GetSelection() == 2) { // Trade
				ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_TRADE);
			}
			else if (_action_options.GetSelection() == 3) { // Confirm
				ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_CONFIRM);
			}
			else if (_action_options.GetSelection() == 4) { // Leave
				ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_LEAVE);
			}
		}
		else if (InputManager->LeftPress()) {
			_action_options.InputLeft();
		}
		else if (InputManager->RightPress()) {
			_action_options.InputRight();
		}
	} // if (_state == SHOP_STATE_ROOT)

	// Update the active interface
	switch (_state) {
		case SHOP_STATE_ROOT:
			_root_interface->Update();
			break;
		case SHOP_STATE_BUY:
			_buy_interface->Update();
			break;
		case SHOP_STATE_SELL:
			_sell_interface->Update();
			break;
		case SHOP_STATE_TRADE:
			_trade_interface->Update();
			break;
		case SHOP_STATE_CONFIRM:
			_confirm_interface->Update();
			break;
		case SHOP_STATE_LEAVE:
			_leave_interface->Update();
			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid shop state: " << _state << ", reseting to root state" << endl;
			_state = SHOP_STATE_ROOT;
			break;
	} // switch (_state)
} // void ShopMode::Update()



void ShopMode::Draw() {
	// ---------- (1): Draw the background image. Set the system coordinates to the size of the window (same as the screen backdrop)
	VideoManager->SetCoordSys(0.0f, static_cast<float>(VideoManager->GetScreenWidth()), 0.0f, static_cast<float>(VideoManager->GetScreenHeight()));
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(0.0f, 0.0f);
	_screen_backdrop.Draw();

	// ---------- (2): Draw all menu windows
	VideoManager->SetCoordSys(0.0f, 1024.0f, 0.0f, 768.0f); // Restore the standard shop coordinate system before drawing the shop windows
	_top_window.Draw();
	_bottom_window.Draw();
	_middle_window.Draw(); // Drawn last because the middle window has the middle upper and lower window borders attached

	// ---------- (3): Draw the contents of the top window
	VideoManager->Move(130.0f, 605.0f);
	ShopMedia::CurrentInstance()->GetDrunesIcon()->Draw();
	VideoManager->MoveRelative(705.0f, 0.0f);
	ShopMedia::CurrentInstance()->GetDrunesIcon()->Draw();

	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(512.0f, 657.0f);
	switch (_state) {
		case SHOP_STATE_ROOT:
			_action_options.Draw();
			break;
		case SHOP_STATE_BUY:
			_action_titles[0].Draw();
			break;
		case SHOP_STATE_SELL:
			_action_titles[1].Draw();
			break;
		case SHOP_STATE_TRADE:
			_action_titles[2].Draw();
			break;
		case SHOP_STATE_CONFIRM:
			_action_titles[3].Draw();
			break;
		case SHOP_STATE_LEAVE:
			_action_titles[4].Draw();
			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid shop state: " << _state << endl;
			break;
	}

	// TODO: This method isn't working correctly (know idea why these coordinates work). When the call is fixed, the args should be updated
	VideoManager->DrawLine(-315.0f, -20.0f, 315.0f, -20.0f, 1.0f, Color::white);

	_finance_table.Draw();

	// ---------- (4): Call the draw function on the active interface to fill the contents of the other two windows
	switch (_state) {
		case SHOP_STATE_ROOT:
			_root_interface->Draw();
			break;
		case SHOP_STATE_BUY:
			_buy_interface->Draw();
			break;
		case SHOP_STATE_SELL:
			_sell_interface->Draw();
			break;
		case SHOP_STATE_TRADE:
			_trade_interface->Draw();
			break;
		case SHOP_STATE_CONFIRM:
			_confirm_interface->Draw();
			break;
		case SHOP_STATE_LEAVE:
			_leave_interface->Draw();
			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid shop state: " << _state << endl;
			break;
	}
} // void ShopMode::Draw()



void ShopMode::AddObjectToBuyList(ShopObject* object) {
	if (object == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function was passed a NULL argument" << endl;
		return;
	}

	if (object->GetBuyCount() == 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be added had a buy count of zero" << endl;
	}

	uint32 object_id = object->GetObject()->GetID();
	pair<map<uint32, ShopObject*>::iterator, bool> ret_val;
	ret_val = _buy_list.insert(make_pair(object_id, object));
	if (ret_val.second == false) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be added already existed in buy list" << endl;
	}
}



void ShopMode::RemoveObjectFromBuyList(ShopObject* object) {
	if (object == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function was passed a NULL argument" << endl;
		return;
	}

	if (object->GetBuyCount() > 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be removed had a buy count that was non-zero" << endl;
	}

	uint32 object_id = object->GetObject()->GetID();
	map<uint32, ShopObject*>::iterator object_entry = _buy_list.find(object_id);
	if (object_entry == _buy_list.end()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be removed did not exist on the buy list" << endl;
	}
	else {
		_buy_list.erase(object_entry);
	}
}



void ShopMode::AddObjectToSellList(ShopObject* object) {
	if (object == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function was passed a NULL argument" << endl;
		return;
	}

	if (object->GetSellCount() == 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be added had a sell count of zero" << endl;
	}

	uint32 object_id = object->GetObject()->GetID();
	pair<map<uint32, ShopObject*>::iterator, bool> ret_val;
	ret_val = _sell_list.insert(make_pair(object_id, object));
	if (ret_val.second == false) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be added already existed in sell list" << endl;
	}
}



void ShopMode::RemoveObjectFromSellList(ShopObject* object) {
	if (object == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function was passed a NULL argument" << endl;
		return;
	}

	if (object->GetSellCount() > 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be removed had a sell count that was non-zero" << endl;
	}

	uint32 object_id = object->GetObject()->GetID();
	map<uint32, ShopObject*>::iterator object_entry = _sell_list.find(object_id);
	if (object_entry == _sell_list.end()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be removed did not exist on the sell list" << endl;
	}
	else {
		_sell_list.erase(object_entry);
	}
}



void ShopMode::CompleteTransaction() {
	for (map<uint32, ShopObject*>:: iterator i = _buy_list.begin(); i != _buy_list.end(); i++) {
		// TODO
	}

	for (map<uint32, ShopObject*>:: iterator i = _sell_list.begin(); i != _sell_list.end(); i++) {
		// TODO
	}

	map<uint32, GlobalObject*>* inventory = GlobalManager->GetInventory();
	for (map<uint32, GlobalObject*>::iterator i = inventory->begin(); i != inventory->end(); i++) {
		// TODO
	}
}



void ShopMode::UpdateFinances(int32 costs_amount, int32 sales_amount) {
	int32 updated_costs = _total_costs + costs_amount;
	int32 updated_sales = _total_sales + sales_amount;

	if (updated_costs < 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "updated amount causes costs to become negative: " << costs_amount << endl;
		return;
	}
	if (updated_sales < 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "updated amount causes sales to become negative: " << sales_amount << endl;
		return;
	}
	if ((static_cast<int32>(GlobalManager->GetDrunes()) + updated_sales - updated_costs) < 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "updated costs and sales values cause negative balance: " << costs_amount << ", " << sales_amount << endl;
		return;
	}

	_total_costs = static_cast<uint32>(updated_costs);
	_total_sales = static_cast<uint32>(updated_sales);

	_finance_table.SetOptionText(0, MakeUnicodeString("Funds: " + NumberToString(GlobalManager->GetDrunes())));
	_finance_table.SetOptionText(1, MakeUnicodeString("Purchases: -" + NumberToString(ShopMode::CurrentInstance()->GetTotalCosts())));
	_finance_table.SetOptionText(2, MakeUnicodeString("Sales: +" + NumberToString(ShopMode::CurrentInstance()->GetTotalSales())));
	_finance_table.SetOptionText(3, MakeUnicodeString("Total: " + NumberToString(ShopMode::CurrentInstance()->GetTotalRemaining())));
}



void ShopMode::ChangeState(SHOP_STATE new_state) {
	if (_state == new_state) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "shop was already in the state to change to: " << _state << endl;
		return;
	}

	_state = new_state;

	// When state changes to the leave state, leave immediately if there are no marked purchases, sales, or trades
	if (_state == SHOP_STATE_LEAVE) {
		if ((GetTotalCosts() == 0) && (GetTotalSales() == 0)) {
			ModeManager->Pop();
		}
	}
} // void ShopMode::ChangeState(SHOP_STATE new_state)



void ShopMode::SetShopName(ustring name) {
	if (IsInitialized() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function called after shop was already initialized" << endl;
		return;
	}

	_root_interface->SetShopName(name);
}



void ShopMode::SetGreetingText(ustring greeting) {
	if (IsInitialized() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function called after shop was already initialized" << endl;
		return;
	}

	_root_interface->SetGreetingText(greeting);
}



void ShopMode::SetPriceLevels(SHOP_PRICE_LEVEL buy_level, SHOP_PRICE_LEVEL sell_level) {
	if (IsInitialized() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function called after shop was already initialized" << endl;
		return;
	}

	_buy_price_level = buy_level;
	_sell_price_level = sell_level;
}



void ShopMode::AddObject(uint32 object_id, uint32 stock) {
	if (IsInitialized() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function called after shop was already initialized" << endl;
		return;
	}

	if (stock == 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "added an object with a zero stock count" << endl;
		return;
	}

	if (object_id == private_global::OBJECT_ID_INVALID || object_id >= private_global::OBJECT_ID_EXCEEDS) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to add object with invalid id: " << object_id << endl;
		return;
	}

	if (_shop_objects.find(object_id) != _shop_objects.end()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to add object that already existed: " << object_id << endl;
		return;
	}

	GlobalObject* new_object = GlobalCreateNewObject(object_id, 1);
	_created_objects.push_back(new_object);
	ShopObject new_shop_object(new_object, true);
	new_shop_object.IncrementStockCount(stock);
	_shop_objects.insert(make_pair(object_id, new_shop_object));
}



void ShopMode::RemoveObject(uint32 object_id) {
	map<uint32, ShopObject>::iterator shop_iter = _shop_objects.find(object_id);
	if (shop_iter == _shop_objects.end()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to remove object that did not exist: " << object_id << endl;
		return;
	}

	if (shop_iter->second.IsSoldInShop() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "tried to remove object that is sold in shop: " << object_id << endl;
		return;
	}

	if (shop_iter->second.GetOwnCount() != 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object's ownership count was non-zero: " << object_id << endl;
		return;
	}

	_shop_objects.erase(shop_iter);
	_sell_list.erase(object_id);

	// TODO: call the sell interface and inform it that the object has been removed
}

} // namespace hoa_shop
