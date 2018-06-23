///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
* \file    menu_equip.cpp
* \author  Daniel Steuernol (Steu)
* \author  Andy Gardner (ChopperDave)
* \brief   Source file for equip menu.
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
// EquipWindow Class
////////////////////////////////////////////////////////////////////////////////

EquipWindow::EquipWindow() :
    _active_box(EQUIP_ACTIVE_NONE)
{
    // Start in equip mode by default
    _remove_mode = false;
    // Initialize option boxes
    _InitCharSelect();
    _InitEquipmentSelect();
    _InitEquipmentList();
}



EquipWindow::~EquipWindow() {
}


void EquipWindow::SetRemoveMode(bool remove_mode) {
    _remove_mode = remove_mode;
}



void EquipWindow::Activate(bool new_status) {

    //Activate window and first option box...or deactivate both
    if (new_status) {
        _active_box = EQUIP_ACTIVE_CHAR;
        _char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
    }
    else {
        _active_box = EQUIP_ACTIVE_NONE;
        _char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
    }
}



void EquipWindow::_InitEquipmentList() {
    // Set up the inventory option box
    // 	_equip_list.SetCellSize(180.0f, 30.0f);

    _equip_list.SetPosition(500.0f, 170.0f);
    _equip_list.SetDimensions(400.0f, 360.0f, 1, 255, 1, 6);
    _equip_list.SetTextStyle(TextStyle("text20"));

    _equip_list.SetCursorOffset(-52.0f, -20.0f);
    _equip_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
    _equip_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
    _equip_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
    // Update the equipment list
    _UpdateEquipList();
    if (_equip_list.GetNumberOptions() > 0) {
        _equip_list.SetSelection(0);
    }
    // Initially hide the cursor
    _equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}



void EquipWindow::_InitCharSelect() {
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

    //Use blank strings....won't be seen anyway
    for (uint32 i = 0; i < size; i++) {
        options.push_back(MakeUnicodeString(" "));
    }

    //Set options, selection and cursor state
    _char_select.SetOptions(options);
    _char_select.SetSelection(0);
    _char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

} // void EquipWindow::InitCharSelect()



void EquipWindow::_InitEquipmentSelect() {
    //Set params
    _equip_select.SetPosition(680.0f, 145.0f);
    _equip_select.SetDimensions(105.0f, 350.0f, 1, EQUIP_CATEGORY_SIZE, 1, EQUIP_CATEGORY_SIZE);
    _equip_select.SetTextStyle(TextStyle("text20"));

    _equip_select.SetCursorOffset(-132.0f, -20.0f);
    _equip_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
    _equip_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
    _equip_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);

    //Set options and default selection

    _equip_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
    _UpdateEquipList();
    _equip_select.SetSelection(EQUIP_WEAPON);
} // void EquipWindow::_InitEquipmentSelect()



void EquipWindow::Update() {
    // Points to the active option box
    OptionBox *active_option = nullptr;

    //choose correct menu
    switch (_active_box) {
    case EQUIP_ACTIVE_CHAR:
        active_option = &_char_select;
        break;
    case EQUIP_ACTIVE_SELECT:
        active_option = &_equip_select;
        break;
    case EQUIP_ACTIVE_LIST:
        active_option = &_equip_list;
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
    switch (_active_box) {
    //Choose character
    case EQUIP_ACTIVE_CHAR:
        if (event == VIDEO_OPTION_CONFIRM) {
            _active_box = EQUIP_ACTIVE_SELECT;
            _char_select.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
            _equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
            MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
        }
        else if (event == VIDEO_OPTION_CANCEL) {
            Activate(false);
            MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
        }
        break;

    //Choose equipment to replace
    case EQUIP_ACTIVE_SELECT:
        if (event == VIDEO_OPTION_CONFIRM) {
            _active_box = EQUIP_ACTIVE_LIST;
            _UpdateEquipList();
            // If in remove mode just remove the item and do not go to replace it
            if (_remove_mode == true) {
                GlobalCharacter* ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));

                switch (_equip_select.GetSelection()) {
                case EQUIP_WEAPON:
                {
                    if (ch->GetWeaponEquipped() != nullptr) {
                        GlobalManager->AddToInventory(ch->EquipWeapon(nullptr));
                        MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
                    }
                    else {
                        MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
                    }
                    break; }
                case EQUIP_HEADGEAR:
                {
                    if (ch->GetHeadArmorEquipped() != nullptr) {
                        GlobalManager->AddToInventory(ch->EquipHeadArmor(nullptr));
                        MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
                    }
                    else {
                        MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
                    }
                    break; }
                case EQUIP_BODYARMOR:
                {
                    if (ch->GetTorsoArmorEquipped() != nullptr) {
                        GlobalManager->AddToInventory(ch->EquipTorsoArmor(nullptr));
                        MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
                    }
                    else {
                        MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
                    }
                    break; }
                case EQUIP_OFFHAND:
                {
                    if (ch->GetArmArmorEquipped() != nullptr) {
                        GlobalManager->AddToInventory(ch->EquipArmArmor(nullptr));
                        MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
                    }
                    else {
                        MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
                    }
                    break; }
                case EQUIP_LEGGINGS:
                {
                    if (ch->GetLegArmorEquipped() != nullptr) {
                        GlobalManager->AddToInventory(ch->EquipLegArmor(nullptr));
                        MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
                    }
                    else {
                        MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
                    }
                    break; }
                }
                _active_box = EQUIP_ACTIVE_SELECT;
            }
            else if (_equip_list.GetNumberOptions() > 0) {
                _equip_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
                _equip_list.SetSelection(0);
                _equip_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
                MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
            }
            else {
                _active_box = EQUIP_ACTIVE_SELECT;
                MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
            }
        }
        else if (event == VIDEO_OPTION_CANCEL) {
            _active_box = EQUIP_ACTIVE_CHAR;
            _char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
            _equip_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
            MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
        }
        break;

    //Choose replacement
    case EQUIP_ACTIVE_LIST:
        if (event == VIDEO_OPTION_CONFIRM) {
            GlobalCharacter* ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));
            uint32 id_num;

            switch (_equip_select.GetSelection()) {
            case EQUIP_WEAPON:
            {	GlobalWeapon* wpn = GlobalManager->GetInventoryWeapons()->at(_equip_list.GetSelection());
            if (wpn->GetUsableBy() & ch->GetID()) {
                id_num = wpn->GetID();
                GlobalManager->AddToInventory(ch->EquipWeapon((GlobalWeapon*)GlobalManager->RetrieveFromInventory(id_num)));
            }
            else {
                MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
            }
            break; }

            case EQUIP_HEADGEAR:
            {	GlobalArmor* hlm = GlobalManager->GetInventoryHeadArmor()->at(_equip_list.GetSelection());
            if (hlm->GetUsableBy() & ch->GetID()) {
                id_num = hlm->GetID();
                GlobalManager->AddToInventory(ch->EquipHeadArmor((GlobalArmor*)GlobalManager->RetrieveFromInventory(id_num)));
            }
            else {
                MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
            }
            break; }

            case EQUIP_BODYARMOR:
            {	GlobalArmor* arm = GlobalManager->GetInventoryTorsoArmor()->at(_equip_list.GetSelection());
            if (arm->GetUsableBy() & ch->GetID()) {
                id_num = arm->GetID();
                GlobalManager->AddToInventory(ch->EquipTorsoArmor((GlobalArmor*)GlobalManager->RetrieveFromInventory(id_num)));
            }
            else {
                MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
            }
            break; }

            case EQUIP_OFFHAND:
            {	GlobalArmor* shld = GlobalManager->GetInventoryArmArmor()->at(_equip_list.GetSelection());
            if (shld->GetUsableBy() & ch->GetID()) {
                id_num = shld->GetID();
                GlobalManager->AddToInventory(ch->EquipArmArmor((GlobalArmor*)GlobalManager->RetrieveFromInventory(id_num)));
            }
            else {
                MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
            }
            break; }

            case EQUIP_LEGGINGS:
            {	GlobalArmor* lgs = GlobalManager->GetInventoryLegArmor()->at(_equip_list.GetSelection());
            if (lgs->GetUsableBy() & ch->GetID()) {
                id_num = lgs->GetID();
                GlobalManager->AddToInventory(ch->EquipLegArmor((GlobalArmor*)GlobalManager->RetrieveFromInventory(id_num)));
            }
            else {
                MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
            }
            break; }

            default:
                cout << "MENU ERROR: _equip_select.GetSelection value is invalid: " << _equip_select.GetSelection() << endl;
                break;
            } // switch _equip_select.GetSelection()

            _active_box = EQUIP_ACTIVE_SELECT;
            _equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
            _equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
            MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
        } // if VIDEO_OPTION_CONFIRM
        else if (event == VIDEO_OPTION_CANCEL) {
            _active_box = EQUIP_ACTIVE_SELECT;
            MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
            _equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
            _equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
        } // else if VIDEO_OPTION_CANCEL
        break;
    } // switch _active_box

    _UpdateEquipList();
} // void EquipWindow::Update()



void EquipWindow::_UpdateEquipList() {
    GlobalCharacter* ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));
    std::vector<ustring> options;

    if (_active_box == EQUIP_ACTIVE_LIST) {
        uint32 gearsize = 0;
        //		vector<hoa_global::GlobalWeapon*> weapons;
        //		vector<hoa_global::GlobalArmor*> armor;

        switch (_equip_select.GetSelection()) {
        case EQUIP_WEAPON:
            gearsize = GlobalManager->GetInventoryWeapons()->size();

            for (uint32 j = 0; j < gearsize; j++) {
                options.push_back(GlobalManager->GetInventoryWeapons()->at(j)->GetName());
            }

            break;

        case EQUIP_HEADGEAR:
            gearsize = GlobalManager->GetInventoryHeadArmor()->size();

            for (uint32 j = 0; j < gearsize; j++) {
                options.push_back(GlobalManager->GetInventoryHeadArmor()->at(j)->GetName());
            }

            break;

        case EQUIP_BODYARMOR:
            gearsize = GlobalManager->GetInventoryTorsoArmor()->size();

            for (uint32 j = 0; j < gearsize; j++) {
                options.push_back(GlobalManager->GetInventoryTorsoArmor()->at(j)->GetName());
            }

            break;

        case EQUIP_OFFHAND:
            gearsize = GlobalManager->GetInventoryArmArmor()->size();

            for (uint32 j = 0; j < gearsize; j++) {
                options.push_back(GlobalManager->GetInventoryArmArmor()->at(j)->GetName());
            }

            break;

        case EQUIP_LEGGINGS:
            gearsize = GlobalManager->GetInventoryLegArmor()->size();

            for (uint32 j = 0; j < gearsize; j++) {
                options.push_back(GlobalManager->GetInventoryLegArmor()->at(j)->GetName());
            }

            break;
        } // switch
          // 		_equip_list.SetSize(1, gearsize);
        _equip_list.SetOptions(options);
    } // if EQUIP_ACTIVE_LIST

    else {
        // First, update the IMAGES of the equipped items
        _equip_images.clear();
        StillImage i;

        // TODO: Simplify into single function
        if (ch->GetWeaponEquipped() != nullptr) {
            i.Load(ch->GetWeaponEquipped()->GetIconImage().GetFilename(), 60, 60);
            _equip_images.push_back(i);
        }
        else {
            i.Load("img/icons/weapons/no_weapon.png", 60, 60);
            _equip_images.push_back(i);
        }

        if (ch->GetHeadArmorEquipped() != nullptr) {
            i.Load(ch->GetHeadArmorEquipped()->GetIconImage().GetFilename(), 60, 60);
            _equip_images.push_back(i);
        }
        else {
            i.Load("img/icons/armor/no_armor.png", 60, 60);
            _equip_images.push_back(i);
        }

        if (ch->GetTorsoArmorEquipped() != nullptr) {
            i.Load(ch->GetTorsoArmorEquipped()->GetIconImage().GetFilename(), 60, 60);
            _equip_images.push_back(i);
        }
        else {
            i.Load("img/icons/armor/no_armor.png", 60, 60);
            _equip_images.push_back(i);
        }

        if (ch->GetArmArmorEquipped() != nullptr) {
            i.Load(ch->GetArmArmorEquipped()->GetIconImage().GetFilename(), 60, 60);
            _equip_images.push_back(i);
        }
        else {
            i.Load("img/icons/armor/no_armor.png", 60, 60);
            _equip_images.push_back(i);
        }

        if (ch->GetLegArmorEquipped() != nullptr) {
            i.Load(ch->GetLegArmorEquipped()->GetIconImage().GetFilename(), 60, 60);
            _equip_images.push_back(i);
        }
        else {
            i.Load("img/icons/armor/no_armor.png", 60, 60);
            _equip_images.push_back(i);
        }
        // Now, update the NAMES of the equipped items

        if (ch->GetWeaponEquipped() != nullptr) {
            options.push_back(ch->GetWeaponEquipped()->GetName());
        }
        else {
            options.push_back(MakeUnicodeString(" "));
        }

        if (ch->GetHeadArmorEquipped() != nullptr) {
            options.push_back(ch->GetHeadArmorEquipped()->GetName());
        }
        else {
            options.push_back(MakeUnicodeString(" "));
        }

        if (ch->GetTorsoArmorEquipped() != nullptr) {
            options.push_back(ch->GetTorsoArmorEquipped()->GetName());
        }
        else {
            options.push_back(MakeUnicodeString(" "));
        }

        if (ch->GetArmArmorEquipped() != nullptr) {
            options.push_back(ch->GetArmArmorEquipped()->GetName());
        }
        else {
            options.push_back(MakeUnicodeString(" "));
        }

        if (ch->GetLegArmorEquipped() != nullptr) {
            options.push_back(ch->GetLegArmorEquipped()->GetName());
        }
        else {
            options.push_back(MakeUnicodeString(" "));
        }

        // 		_equip_select.SetSize(1, 5);
        _equip_select.SetOptions(options);
    }

} // void EquipWindow::UpdateEquipList()



void EquipWindow::Draw() {
    MenuWindow::Draw();
    _UpdateEquipList();

    //Draw option boxes
    _char_select.Draw();

    if (_active_box == EQUIP_ACTIVE_LIST) {
        _equip_list.Draw();
        VideoManager->Move(660.0f, 135.0f);
        VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
        switch (_equip_select.GetSelection()) {
        case EQUIP_WEAPON:
            VideoManager->Text()->Draw(UTranslate("Weapons"));
            break;
        case EQUIP_HEADGEAR:
            VideoManager->Text()->Draw(UTranslate("Headgear"));
            break;
        case EQUIP_BODYARMOR:
            VideoManager->Text()->Draw(UTranslate("Body Armor"));
            break;
        case EQUIP_OFFHAND:
            VideoManager->Text()->Draw(UTranslate("Offhand"));
            break;
        case EQUIP_LEGGINGS:
            VideoManager->Text()->Draw(UTranslate("Leggings"));
            break;
        }
    }
    else {
        _equip_select.Draw();

        //FIX ME: Use XML tags for formatting option boxes
        VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
        VideoManager->Move(450.0f, 170.0f);
        VideoManager->Text()->Draw(UTranslate("Weapon"));
        VideoManager->MoveRelative(0.0f, 70.0f);
        VideoManager->Text()->Draw(UTranslate("Headgear"));
        VideoManager->MoveRelative(0.0f, 70.0f);
        VideoManager->Text()->Draw(UTranslate("Body Armor"));
        VideoManager->MoveRelative(0.0f, 70.0f);
        VideoManager->Text()->Draw(UTranslate("Offhand"));
        VideoManager->MoveRelative(0.0f, 70.0f);
        VideoManager->Text()->Draw(UTranslate("Leggings"));

        VideoManager->MoveRelative(150.0f, -370.0f);

        for (uint32 i = 0; i < _equip_images.size(); i++) {
            VideoManager->MoveRelative(0.0f, 70.0f);
            _equip_images[i].Draw();
        }
    }

} // void EquipWindow::Draw()

} // namespace private_menu

} // namespace hoa_menu
