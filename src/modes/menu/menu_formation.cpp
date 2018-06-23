///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
* \file    menu_formation.cpp
* \author  Daniel Steuernol (Steu)
* \author  Andy Gardner (ChopperDave)
* \brief   Source file for formation menu.
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
// FormationWindow Class
////////////////////////////////////////////////////////////////////////////////

FormationWindow::FormationWindow() : _active_box(FORM_ACTIVE_NONE) {
	_InitCharSelect();
}


FormationWindow::~FormationWindow() {
}


void FormationWindow::_InitCharSelect() {
	//character selection set up
	std::vector<ustring> options;
	uint32 size = GlobalManager->GetActiveParty()->GetPartySize();

	_char_select.SetPosition(72.0f, 109.0f);
	_char_select.SetDimensions(360.0f, 432.0f, 1, 4, 1, 4);
	_char_select.SetCursorOffset(-50.0f, -6.0f);
	_char_select.SetTextStyle(TextStyle("text20"));
	_char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);

	_second_char_select.SetPosition(72.0f, 109.0f);
	_second_char_select.SetDimensions(360.0f, 432.0f, 1, 4, 1, 4);
	_second_char_select.SetCursorOffset(-50.0f, -6.0f);
	_second_char_select.SetTextStyle(TextStyle("text20"));
	_second_char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_second_char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_second_char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);


	// Use blank string so cursor can point somewhere
	for (uint32 i = 0; i < size; i++) {
		options.push_back(MakeUnicodeString(" "));
	}

	_char_select.SetOptions(options);
	_char_select.SetSelection(0);
	_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

	_second_char_select.SetOptions(options);
	_second_char_select.SetSelection(0);
	_second_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

}


void FormationWindow::Update() {
	// Points to the active option box
	OptionBox *active_option = nullptr;

	//choose correct menu
	switch (_active_box) {
		case FORM_ACTIVE_CHAR:
			active_option = &_char_select;
			break;
		case FORM_ACTIVE_SECOND:
			active_option = &_second_char_select;
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
		case FORM_ACTIVE_CHAR:
			if (event == VIDEO_OPTION_CONFIRM) {
				_active_box = FORM_ACTIVE_SECOND;
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
				_second_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
			}
			else if (event == VIDEO_OPTION_CANCEL) {
				Activate(false);
				MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
			}
			break;

		case FORM_ACTIVE_SECOND:
			if (event == VIDEO_OPTION_CONFIRM) {
				// TODO: Implement Character Switch
				_active_box = FORM_ACTIVE_CHAR;
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				_second_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			}
			else if (event == VIDEO_OPTION_CANCEL) {
				_active_box = FORM_ACTIVE_CHAR;
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				_second_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
			}
			break;
	} // switch
	_char_select.Update();
}


void FormationWindow::Draw() {
	MenuWindow::Draw();
	_char_select.Draw();
	_second_char_select.Draw();
}


void FormationWindow::Activate(bool new_status) {
	if (new_status) {
		_active_box = FORM_ACTIVE_CHAR;
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	}
	else {
		_active_box = FORM_ACTIVE_NONE;
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		_second_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	}
}

} // namespace private_menu

} // namespace hoa_menu

