///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
* \file    menu_views.cpp
* \author  Daniel Steuernol (Steu)
* \author  Andy Gardner (ChopperDave)
* \brief   Source file for status menu.
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
// StatusWindow Class
////////////////////////////////////////////////////////////////////////////////

StatusWindow::StatusWindow() :
    _char_select_active(false)
{
    // Get party size for iteration
    uint32 partysize = GlobalManager->GetActiveParty()->GetPartySize();
    StillImage portrait;
    GlobalCharacter* ch;

    // Set up the full body portrait
    for (uint32 i = 0; i < partysize; i++) {
        ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(i));
        portrait.SetStatic(true);

        string portrait_filename = "img/portraits/full/" + ch->GetFilename() + "_large.png";
        if (DoesFileExist(portrait_filename) == false) {
            IF_PRINT_WARNING(MENU_DEBUG) << "character portrait image file did not exist: " << portrait_filename << endl;
            portrait.Load("");
        }
        else {
            portrait.Load(portrait_filename);
        }
        _full_portraits.push_back(portrait);
        _full_portraits.push_back(portrait);
    }

    // Init char select option box
    _InitCharSelect();
} // StatusWindow::StatusWindow()


StatusWindow::~StatusWindow() {

}

// Activate/deactivate window
void StatusWindow::Activate(bool new_value) {
    _char_select_active = new_value;

    if (_char_select_active)
        _char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
    else
        _char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}

void StatusWindow::_InitCharSelect() {
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

    // Use blank string so cursor can point somewhere
    for (uint32 i = 0; i < size; i++) {
        options.push_back(MakeUnicodeString(" "));
    }

    _char_select.SetOptions(options);
    _char_select.SetSelection(0);
    _char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}

// Updates the status window
void StatusWindow::Update() {
    // check input values
    if (InputManager->UpPress())
    {
        _char_select.InputUp();
    }
    else if (InputManager->DownPress())
    {
        _char_select.InputDown();
    }
    else if (InputManager->CancelPress())
    {
        _char_select.InputCancel();
    }

    if (_char_select.GetEvent() == VIDEO_OPTION_CANCEL) {
        Activate(false);
        MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
    }
    _char_select.Update();
} // void StatusWindow::Update()


  // Draws the status window
void StatusWindow::Draw() {
    MenuWindow::Draw();

    GlobalCharacter* ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));

    // Set drawing system
    VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);

    // window top corner is 432, 99
    VideoManager->Move(565, 130);

    //Draw character name and level
    VideoManager->SetDrawFlags(VIDEO_X_CENTER, 0);
    VideoManager->Text()->Draw(ch->GetName());

    VideoManager->MoveRelative(0, 25);
    VideoManager->Text()->Draw(UTranslate("Experience Level: ") + MakeUnicodeString(NumberToString(ch->GetExperienceLevel())));

    VideoManager->SetDrawFlags(VIDEO_X_LEFT, 0);

    //Draw all character stats
    VideoManager->MoveRelative(-55, 60);
    VideoManager->Text()->Draw(UTranslate("HP: ") + MakeUnicodeString(NumberToString(ch->GetHitPoints()) +
        " (-" + NumberToString(ch->GetHitPointFatigue()) + "), Max: " + NumberToString(ch->GetMaxHitPoints())));

    VideoManager->MoveRelative(0, 25);
    VideoManager->Text()->Draw(UTranslate("SP: ") + MakeUnicodeString(NumberToString(ch->GetSkillPoints()) +
        " (-" + NumberToString(ch->GetSkillPointFatigue()) + ")"));

    VideoManager->MoveRelative(0, 25);
    VideoManager->Text()->Draw(UTranslate("XP to Next: ") + MakeUnicodeString(NumberToString(ch->GetExperienceForNextLevel())));

    VideoManager->MoveRelative(0, 25);
    VideoManager->Text()->Draw(UTranslate("Strength: ") + MakeUnicodeString(NumberToString(ch->GetStrength())));

    VideoManager->MoveRelative(0, 25);
    VideoManager->Text()->Draw(UTranslate("Vigor: ") + MakeUnicodeString(NumberToString(ch->GetVigor())));

    VideoManager->MoveRelative(0, 25);
    VideoManager->Text()->Draw(UTranslate("Fortitude: ") + MakeUnicodeString(NumberToString(ch->GetFortitude())));

    VideoManager->MoveRelative(0, 25);
    VideoManager->Text()->Draw(UTranslate("Protection: ") + MakeUnicodeString(NumberToString(ch->GetProtection())));

    VideoManager->MoveRelative(0, 25);
    VideoManager->Text()->Draw(UTranslate("Stamina: ") + MakeUnicodeString(NumberToString(ch->GetStamina())));

    VideoManager->MoveRelative(0, 25);
    VideoManager->Text()->Draw(UTranslate("Resilience: ") + MakeUnicodeString(NumberToString(ch->GetResilience())));

    VideoManager->MoveRelative(0, 25);
    VideoManager->Text()->Draw(UTranslate("Agility: ") + MakeUnicodeString(NumberToString(ch->GetAgility())));

    VideoManager->MoveRelative(0, 25);
    VideoManager->Text()->Draw(UTranslate("Evade: ") + MakeUnicodeString(NumberToString(ch->GetEvade()) + "%"));

    //Draw character full body portrait
    VideoManager->Move(855, 145);
    VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_TOP, 0);

    _full_portraits[_char_select.GetSelection()].Draw();

    VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);

    _char_select.Draw();
} // void StatusWindow::Draw()

} // namespace private_menu

} // namespace hoa_menu
