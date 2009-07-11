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
*** \brief   Header file for shop mode interface.
***
*** This code provides an interface for the user to purchase wares from a
*** merchant. This mode is usually entered from a map after speaking with a
*** store owner.
***
*** \todo Implement feature for shops to have limited quantity of wares to buy,
*** as well as the ability to re-generate more quantities as the time passes
*** between shops.
*** ***************************************************************************/

#ifndef __SHOP_HEADER__
#define __SHOP_HEADER__

#include "defs.h"
#include "utils.h"

#include "mode_manager.h"

#include "global.h"

#include "shop_utils.h"

namespace hoa_shop {

//! \brief Determines whether the code in the hoa_shop namespace should print debug statements or not.
extern bool SHOP_DEBUG;

namespace private_shop {

/** \brief A pointer to the currently active shop mode
*** This is used by the various shop classes so that they can refer back to the main class from which
*** they are a part of. This member is initially set to NULL. It is set whenever the ShopMode
*** constructor is invoked, and reset back to NULL when the ShopMode destructor is invoked.
**/
extern ShopMode* current_shop;

} // namespace private_shop

/** ****************************************************************************
*** \brief Handles the game execution while the player is shopping.
***
*** ShopMode allows the player to purchase items, weapons, armor, and other
*** objects. ShopMode consists of a captured screenshot which forms the
*** background image, upon which a series of menu windows are drawn. The
*** background image is of size 1024x768, and a 800x600 arrangement of windows
*** is drawn ontop of the middle of that image.
***
*** \note Shop states are a little confusing because of prompt/confirmation
*** windows. These windows appear when, for instance, the player attempts an
*** invalid operation (buy x0 quantity of an item). When this occurs, we make
*** the prompt window the current state, but also save the buy state because
*** we still want to draw the list of wares to purchase.
***
*** \note The recommended way to create and initialize this class is to call the
*** following methods.
***
*** -# ShopMode constructor
*** -# SetGreetingText()
*** -# SetPriceLevels()
*** -# AddObject() for each object to be sold
*** -# Wait for the Reset() method to be automatically called
*** ***************************************************************************/
class ShopMode : public hoa_mode_manager::GameMode {
public:
	ShopMode();

	~ShopMode();

	static ShopMode* CurrentInstance()
		{ return _current_instance; }

	/** \brief Resets appropriate settings. Called whenever the ShopMode object is made the active game mode.
	*** This function additionally constructs the inventory menu from the object list. Therefore, if you add
	*** an object to the inventory it won't be seen in the list until this function is called.
	**/
	void Reset();

	//! \brief Handles user input and updates the shop menu.
	void Update();

	//! \brief Handles the drawing of everything on the shop menu and makes sub-draw function calls as appropriate.
	void Draw();

	/** \brief Sets the greeting message from the shop/merchant
	*** \param greeting The text
	*** \note This method will only work if it is called before the shop is initialized. Calling it afterwards will
	*** result in no operation and a warning message
	**/
	void SetGreetingText(hoa_utils::ustring greeting);

	/** \brief Sets the buy and sell price levels for the shop
	*** \param buy_level The price level to set for wares that the player would buy from the shop
	*** \param sell_level The price level to set for wares that the player would sell to the shop
	*** \note This method will only work if it is called before the shop is initialized. Calling it afterwards will
	*** result in no operation and a warning message
	**/
	void SetPriceLevels(SHOP_PRICE_LEVEL buy_level, SHOP_PRICE_LEVEL sell_level);

	/** \brief Adds a new object for the shop to sell
	*** \param object_id The id number of the object to add
	***
	*** The newly added object won't be seen in the shop menu until the Reset() function is called.
	**/
	void AddObject(uint32 object_id);

	// Functions below this line are intended for use only by other shop mode classes

	/** \brief Loads data and prepares shop for initial use
	*** This function should only be called once, usually from the Load() method. If it is called more than
	*** once it will print a warning and refuse to execute a second time.
	**/
	void Initialize();

	/** \brief Called whenever the player successfully confirms a transaction
	*** This method processes the transaction, including modifying the party's drune count, adding/removing
	*** objects from the inventory, and auto equipping/un-equipping traded equipment. It also calls appropriate
	*** methods in the various shop interfaces to update their display lists with the updated inventory contents and
	*** shop stocks.
	**/
	void CompleteTransaction();

	/** \brief Updates the costs and sales totals
	*** \param costs_amount The amount to change the purchases cost member by
	*** \param sales_amount The amount to change the sales revenue member by
	***
	*** Obviously if one wishes to only update either costs or sales but not both, pass a zero value for the
	*** appropriate argument that should not be changed. This function should only be called when necessary because
	*** it also has to update the finance text in the shop's root interface, so the function does not just
	*** modify integer values but does have a small amount of computational overhead
	**/
	void UpdateFinances(int32 costs_amount, int32 sales_amount);

	/** \brief Changes the active state of shop mode and prepares the interface of the new state
	*** \param new_state The state to change the shop to
	**/
	void ChangeState(private_shop::SHOP_STATE new_state);

	//! \brief Returns true if the user has indicated they wish to buy or sell any items
	bool HasPreparedTransaction() const
		{ return ((_total_costs != 0) || (_total_sales != 0)); }

	//! \brief Returns the number of drunes that the party would be left with after the marked purchases and sales
	uint32 GetTotalRemaining() const
		{ return (hoa_global::GlobalManager->GetDrunes() + _total_sales - _total_costs); }

	//! \name Class member access functions
	//@{
	bool IsInitialized() const
		{ return _initialized; }

	private_shop::SHOP_STATE GetState() const
		{ return _state; }

	SHOP_PRICE_LEVEL GetBuyPriceLevel() const
		{ return _buy_price_level; }

	SHOP_PRICE_LEVEL GetSellPriceLevel() const
		{ return _sell_price_level; }

	uint8 GetDealTypes() const
		{ return _deal_types; }

	uint32 GetTotalCosts() const
		{ return _total_costs; }

	uint32 GetTotalSales() const
		{ return _total_sales; }

	const std::vector<hoa_video::StillImage>& GetObjectCategoryImages() const
		{ return _object_category_images; }

	/** \brief Retrieves a shop sound object
	*** \param identifier The string identifier for the sound to retrieve
	*** \return A pointer to the SoundDescriptor, or NULL if no sound had the identifier name
	**/
	hoa_audio::SoundDescriptor* GetSound(std::string identifier);
	//@}

private:
	/** \brief A reference to the current instance of ShopMode
	*** This is used by other shop clases to be able to refer to the shop that they exist in. This member
	*** is NULL when no shop is active
	**/
	static ShopMode* _current_instance;

	//! \brief Set to true only after the shop has been initialized and is ready to be used by the player
	bool _initialized;

	//! \brief Keeps track of what windows are open to determine how to handle user input.
	private_shop::SHOP_STATE _state;

	//! \brief A bit vector that represents the types of merchandise that the shop deals in (items, weapons, etc)
	uint8 _deal_types;

	//! \brief The shop's price level of objects that the player buys
	SHOP_PRICE_LEVEL _buy_price_level;

	//! \brief The shop's price level of objects that the player sells
	SHOP_PRICE_LEVEL _sell_price_level;

	//! \brief The total cost of all marked purchases.
	uint32 _total_costs;

	//! \brief The total revenue that will be earned from all marked sales.
	uint32 _total_sales;

	/** \brief Contains the ids of all objects which are sold in this shop
	*** The map key is the object id and the value is not used for anything (currently).
	**/
	std::map<uint32, uint32> _object_map;

	/** \brief Contains all of the objects
	*** \note This container is temporary, and will be replaced with multiple containers (for each
	*** type of object) at a later time.
	**/
	std::vector<hoa_global::GlobalObject*> _buy_objects;

	/** \brief Contains all of the items
	*** \note This container is temporary, and will be replaced with multiple containers (for each
	*** type of object) at a later time.
	**/
	std::vector<hoa_global::GlobalObject*> _current_inv;

	/** \brief Contains quantities corresponding to _all_objects
	**/
	std::vector<uint32> _buy_objects_quantities;

	/** \brief Contains quantities corresponding to current inventory
	**/
	std::vector<uint32> _sell_objects_quantities;

	/** \name Shopping interfaces
	*** These are the class objects which are responsible for managing each state in shop mode
	**/
	//@{
	private_shop::ShopRootInterface* _root_interface;

	private_shop::ShopBuyInterface* _buy_interface;

	private_shop::ShopSellInterface* _sell_interface;

	private_shop::ShopTradeInterface* _trade_interface;

	private_shop::ShopConfirmInterface* _confirm_interface;
	//@}

	//! \brief Holds an image of the screen taken when the ShopMode instance was created
	hoa_video::StillImage _saved_screen;

	//! \brief Retains all icon images for each object category
	std::vector<hoa_video::StillImage> _object_category_images;

	//! \brief A map of the sounds used in shop mode
	std::map<std::string, hoa_audio::SoundDescriptor*> _shop_sounds;
}; // class ShopMode : public hoa_mode_manager::GameMode

} // namespace hoa_shop

#endif // __SHOP_HEADER__
