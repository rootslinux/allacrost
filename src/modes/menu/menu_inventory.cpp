///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
* \file    menu_inventory.cpp
* \author  Daniel Steuernol (Steu)
* \author  Andy Gardner (ChopperDave)
* \brief   Source file for inventory menu.
*****************************************************************************/

#include <iostream>
#include <sstream>

#include "audio.h"
#include "input.h"
#include "system.h"

#include "menu.h"

using namespace std;
using namespace hoa_menu::private_menu;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_gui;
using namespace hoa_global;
using namespace hoa_input;
using namespace hoa_system;

namespace hoa_menu {

	namespace private_menu {

////////////////////////////////////////////////////////////////////////////////
// InventoryWindow Class
////////////////////////////////////////////////////////////////////////////////

InventoryWindow::InventoryWindow() : _active_box(ITEM_ACTIVE_NONE) {
	_InitCategory();
	_InitInventoryItems();
	_InitCharSelect();

	//Initializes the description textbox for the bottom window
	_description.SetOwner(this);
	_description.SetPosition(30.0f, 525.0f);
	_description.SetDimensions(800.0f, 80.0f);
	_description.SetDisplaySpeed(30);
	_description.SetTextStyle(TextStyle("text20"));
	_description.SetDisplayMode(VIDEO_TEXT_INSTANT);
	_description.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

} // void InventoryWindow::InventoryWindow

InventoryWindow::~InventoryWindow() {}

//Initializes the list of items
void InventoryWindow::_InitInventoryItems() {
	// Set up the inventory option box
	_inventory_items.SetPosition(500.0f, 170.0f);
	_inventory_items.SetDimensions(400.0f, 360.0f, 1, 255, 1, 6);
	_inventory_items.SetTextStyle(TextStyle("text20"));
	_inventory_items.SetCursorOffset(-52.0f, -20.0f);
	_inventory_items.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_inventory_items.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_inventory_items.Scissoring(true, false);
		// Update the item text
	_UpdateItemText();
	if (_inventory_items.GetNumberOptions() > 0) {
		_inventory_items.SetSelection(0);
	}
	VideoManager->MoveRelative(-65, 20);
	// Initially hide the cursor
	_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}
	//Initalizes character select
void InventoryWindow::_InitCharSelect() {
	//character selection set up
	vector<ustring> options;
	uint32 size = GlobalManager->GetActiveParty()->GetPartySize();
	_char_select.SetPosition(72.0f, 109.0f);
	_char_select.SetDimensions(360.0f, 432.0f, 1, 4, 1, 4);
	_char_select.SetCursorOffset(-50.0f, -6.0f);
	_char_select.SetTextStyle(TextStyle("text20"));
	_char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);

	//Use a blank string so the cursor has somewhere to point
	//String is overdrawn by char portraits, so no matter
	for (uint32 i = 0; i < size; i++) {
		options.push_back(MakeUnicodeString(" "));
	}
		_char_select.SetOptions(options);
	_char_select.SetSelection(0);
	_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}
	//Initalizes the available item categories
void InventoryWindow::_InitCategory() {
	_item_categories.SetPosition(458.0f, 120.0f);
	_item_categories.SetDimensions(448.0f, 30.0f, ITEM_CATEGORY_SIZE, 1, ITEM_CATEGORY_SIZE, 1);
	_item_categories.SetTextStyle(TextStyle("text20"));
	_item_categories.SetCursorOffset(-52.0f, -20.0f);
	_item_categories.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_item_categories.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_item_categories.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	vector<ustring> options;
	options.push_back(UTranslate("All"));
	options.push_back(UTranslate("Itm"));
	options.push_back(UTranslate("Wpn"));
	options.push_back(UTranslate("Hlm"));
	options.push_back(UTranslate("Tor"));
	options.push_back(UTranslate("Arm"));
	options.push_back(UTranslate("Leg"));
	options.push_back(UTranslate("Key"));

	_item_categories.SetOptions(options);
	_item_categories.SetSelection(ITEM_ALL);
	_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}
	// Activates/deactivates inventory window
void InventoryWindow::Activate(bool new_status) {
	// Set new status
	if (new_status) {
		_active_box = ITEM_ACTIVE_CATEGORY;
		// Update cursor state
		_item_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	}
	else {
		//FIX ME: Play N/A noise
		_active_box = ITEM_ACTIVE_NONE;
		_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	}
}

// Updates the window
void InventoryWindow::Update() {

    //bool cancel = false;
    if (GlobalManager->GetInventory()->size() == 0)
    {
        // no more items in inventory, exit inventory window
        Activate(false);
        return;
    }

        // Points to the active option box
        OptionBox *active_option = nullptr;
        _inventory_items.Update(SystemManager->GetUpdateTime()); //For scrolling

    switch (_active_box) {
        case ITEM_ACTIVE_CATEGORY:
            active_option = &_item_categories;
            break;
        case ITEM_ACTIVE_CHAR:
            active_option = &_char_select;
            break;
        case ITEM_ACTIVE_LIST:
            active_option = &_inventory_items;
            break;
    }

    // Handle the appropriate input events
    if (InputManager->ConfirmPress())
        {
            active_option->InputConfirm();
        }
        else if (InputManager->CancelPress())
        {
            active_option->InputCancel();
        }
        else if (InputManager->LeftPress())
        {
            active_option->InputLeft();
        }
        else if (InputManager->RightPress())
        {
            active_option->InputRight();
        }
        else if (InputManager->UpPress())
        {
            active_option->InputUp();
        }
        else if (InputManager->DownPress())
        {
            active_option->InputDown();
        }

        uint32 event = active_option->GetEvent();
        active_option->Update();
        // Handle confirm/cancel presses differently for each window
        switch (_active_box) {
        case ITEM_ACTIVE_NONE:
            break;

        case ITEM_ACTIVE_CATEGORY:
        {
            // Activate the item list for this category
            if (event == VIDEO_OPTION_CONFIRM) {
                if (_inventory_items.GetNumberOptions() > 0) {
                    _inventory_items.SetSelection(0);
                    _item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
                    _inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
                    _description.SetDisplayText(_item_objects[0]->GetDescription());
                    _active_box = ITEM_ACTIVE_LIST;
                    MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
                } // if _inventory_items.GetNumberOptions() > 0
            } // if VIDEO_OPTION_CONFIRM
              // Deactivate inventory
            else if (event == VIDEO_OPTION_CANCEL) {
                MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
                _item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
                Activate(false);
            } // if VIDEO_OPTION_CANCEL
            break;
        } // case ITEM_ACTIVE_CATEGORY

        case ITEM_ACTIVE_LIST:
        {
            // Activate the character select for application
            if (event == VIDEO_OPTION_CONFIRM) {
                GlobalObject* obj = _item_objects[_inventory_items.GetSelection()];
                if (obj->GetObjectType() == GLOBAL_OBJECT_ITEM) {
                    GlobalItem* item = dynamic_cast<GlobalItem*>(obj);
                    if (item->IsUsableInField() == true) {
                        _active_box = ITEM_ACTIVE_CHAR;
                        _inventory_items.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
                        _char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
                        MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
                    }
                }

            } // if VIDEO_OPTION_CONFIRM
              // Return to category selection
            else if (event == VIDEO_OPTION_CANCEL) {
                _active_box = ITEM_ACTIVE_CATEGORY;
                _inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
                _item_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
                MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
            } // else if VIDEO_OPTION_CANCEL
            else if (event == VIDEO_OPTION_BOUNDS_UP || VIDEO_OPTION_BOUNDS_DOWN) {
                _description.SetDisplayText(_item_objects[_inventory_items.GetSelection()]->GetDescription());
            } // else if VIDEO_OPTION_BOUNDS_UP
            break;
        } // case ITEM_ACTIVE_LIST

        case ITEM_ACTIVE_CHAR:
        {
            // Use the item on the chosen character
            if (event == VIDEO_OPTION_CONFIRM) {
                GlobalObject* obj = _item_objects[_inventory_items.GetSelection()];
                if (obj->GetObjectType() == GLOBAL_OBJECT_ITEM) {
                    GlobalItem *item = (GlobalItem*)GlobalManager->RetrieveFromInventory(obj->GetID());
                    const ScriptObject* script_function = item->GetFieldUseFunction();
                    if (script_function == nullptr) {
                        IF_PRINT_WARNING(MENU_DEBUG) << "item did not have a menu use function" << endl;
                    }
                    else {
                        if (IsTargetParty(item->GetTargetType()) == true) {
                            GlobalParty *ch_party = GlobalManager->GetActiveParty();
                            ScriptCallFunction<void>(*script_function, ch_party);
                            item->DecrementCount();
                        } // if GLOBAL_TARGET_PARTY
                        else { // Use on a single character only
                            GlobalCharacter *ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));
                            ScriptCallFunction<void>(*script_function, ch);
                            item->DecrementCount();
                            // Go back to inventory list after
                        }
                        // If they are anymore items in the category then go back to item select
                        if (_inventory_items.GetNumberOptions() > 0) {
                            _active_box = ITEM_ACTIVE_LIST;
                            _inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
                            _char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

                            if (item->GetCount() < 1 && static_cast<int32>(_inventory_items.GetNumberOptions() - 1) == _inventory_items.GetSelection()) {
                                _inventory_items.SetSelection(_inventory_items.GetSelection() - 1);
                            }
                        }// If they aren't anymore irems in the category then go to category select
                        else {
                            _active_box = ITEM_ACTIVE_CATEGORY;
                            _inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
                            _char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
                            _item_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
                        }
                    }
                } // if GLOBAL_OBJECT_ITEM
            } // if VIDEO_OPTION_CONFIRM
              // Return to item selection
            else if (event == VIDEO_OPTION_CANCEL) {
                _active_box = ITEM_ACTIVE_LIST;
                _inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
                _char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
                MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
            } // if VIDEO_OPTION_CANCEL
            break;
        } // case ITEM_ACTIVE_CHAR
        } // switch (_active_box)

          // Update the item list
        _UpdateItemText();
} // void InventoryWindow::Update()

  // Updates the item list
void InventoryWindow::_UpdateItemText() {
    _item_objects.clear();
    _inventory_items.ClearOptions();

    switch (_item_categories.GetSelection()) {
    case ITEM_ALL:
    {
        std::map<uint32, GlobalObject*>* inv = GlobalManager->GetInventory();
        for (std::map<uint32, GlobalObject*>::iterator i = inv->begin(); i != inv->end(); i++) {
            _item_objects.push_back(i->second);
        }
    }
    break;

    case ITEM_ITEM:
        _item_objects = _GetItemVector(GlobalManager->GetInventoryItems());
        break;

    case ITEM_WEAPONS:
        _item_objects = _GetItemVector(GlobalManager->GetInventoryWeapons());
        break;

    case ITEM_HEAD_ARMOR:
        _item_objects = _GetItemVector(GlobalManager->GetInventoryHeadArmor());
        break;

    case ITEM_TORSO_ARMOR:
        _item_objects = _GetItemVector(GlobalManager->GetInventoryTorsoArmor());
        break;

    case ITEM_ARM_ARMOR:
        _item_objects = _GetItemVector(GlobalManager->GetInventoryArmArmor());
        break;

    case ITEM_LEG_ARMOR:
        _item_objects = _GetItemVector(GlobalManager->GetInventoryLegArmor());
        break;

    case ITEM_KEY:
        _item_objects = _GetItemVector(GlobalManager->GetInventoryKeyItems());
        break;
    }

    ustring text;
    std::vector<ustring> inv_names;

    for (size_t ctr = 0; ctr < _item_objects.size(); ctr++) {
        text = MakeUnicodeString("<" + _item_objects[ctr]->GetIconImage().GetFilename() + "><32>     ") +
            _item_objects[ctr]->GetName() + MakeUnicodeString("<R><350>" + NumberToString(_item_objects[ctr]->GetCount()) + "   ");
        inv_names.push_back(text);
    }

    _inventory_items.SetOptions(inv_names);
} // void InventoryWindow::UpdateItemText()



void InventoryWindow::Draw() {
    MenuWindow::Draw();

    // Update the item text in case the number of items changed.
    _UpdateItemText();

    // Draw char select option box
    _char_select.Draw();

    // Draw item categories option box
    _item_categories.Draw();

    // Draw item list
    _inventory_items.Draw();
} // bool InventoryWindow::Draw()

} // namespace private_menu

} // namespace hoa_menu
