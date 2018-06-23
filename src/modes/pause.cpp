////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    pause.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for pause mode interface.
*** ***************************************************************************/

#include "audio.h"
#include "input.h"
#include "system.h"
#include "video.h"

#include "pause.h"
#include "boot.h"

using namespace std;
using namespace hoa_utils;

using namespace hoa_audio;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_video;

using namespace hoa_gui;

using namespace hoa_boot;


namespace hoa_pause {

bool PAUSE_DEBUG = false;

/** \name Quit Options Menu Constants
*** These constants reprent the OptionBox selection indeces of the three different options
*** presented to the player while the _quit_state member is active.
**/
//@{
const uint8 QUIT_GAME      = 0;
const uint8 QUIT_TO_BOOT   = 1;
const uint8 QUIT_CANCEL    = 2;
//@}



PauseMode::PauseMode(PAUSE_STATE state, bool pause_audio) :
	GameMode(PAUSE_MODE),
	_state(state),
	_audio_paused(pause_audio),
	_dim_color(0.35f, 0.35f, 0.35f, 1.0f) // A grayish opaque color
{
	GameMode* parent = ModeManager->GetTop();
	if (parent == nullptr) {
		IF_PRINT_WARNING(PAUSE_DEBUG) << "top of game mode stack returned a nullptr pointer" << endl;
		_parent_mode_type = INVALID_MODE;
	}
	else {
		_parent_mode_type = parent->GetModeType();
		_command_descriptions = parent->GetCommandDescriptions();
	}

	// Determine the type of game mode that instantiated this class (assumed to be at the top of the stack)
	_parent_mode_type = ModeManager->GetModeType();

	// Render the paused string in white text
	_paused_text.SetStyle(TextStyle("title28", Color::white, VIDEO_TEXT_SHADOW_BLACK));
	_paused_text.SetText(UTranslate("Paused"));

	// Initialize the quit options box
	_quit_options.SetPosition(512.0f, 384.0f);
	_quit_options.SetDimensions(750.0f, 50.0f, 3, 1, 3, 1);
	_quit_options.SetTextStyle(TextStyle("title24", Color::white, VIDEO_TEXT_SHADOW_BLACK));

	_quit_options.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_quit_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_quit_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_quit_options.SetCursorOffset(-58.0f, 18.0f);

	_quit_options.AddOption(UTranslate("Quit Game"));
	_quit_options.AddOption(UTranslate("Quit to Main Menu"));
	_quit_options.AddOption(UTranslate("Cancel"));
	_quit_options.SetSelection(QUIT_CANCEL);

	// Initialize help GUI elements
	_help_window.Create(880.0f, 640.0f);
	_help_window.SetPosition(512.0f, 384.0f);
	_help_window.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	if (_state == HELP) {
		_help_window.Show();
	}

	_help_commands_header.SetOwner(&_help_window);
	_help_commands_header.SetPosition(40.0f, 600.0f);
	_help_commands_header.SetDimensions(620.0f, 30.0f, 3, 1, 3, 1);
	_help_commands_header.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_help_commands_header.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_help_commands_header.SetTextStyle(TextStyle("title24"));
	_help_commands_header.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

	_help_commands_header.AddOption(UTranslate("Command"));
	_help_commands_header.AddOption(UTranslate("Key"));
	_help_commands_header.AddOption(UTranslate("Purpose"));

	// Total number of standard commands we will display (-1 because we don't include the "pause" command as a standard)
	uint32 standard_command_count = COMMAND_TOTAL - 1;
	// The number of additional commands we intend to display after the standard commands (+1 added for a "blank" command used as a separator)
	uint32 additional_command_count = 6;
	_help_commands.SetOwner(&_help_window);
	_help_commands.SetPosition(40.0f, 560.0f);
	_help_commands.SetDimensions(620.0f, 480.0f, 3, standard_command_count + additional_command_count, 3, standard_command_count + additional_command_count);
	_help_commands.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_help_commands.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_help_commands.SetTextStyle(TextStyle("text22"));
	_help_commands.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

	// Standard commands
	for (uint32 i = 0; i < standard_command_count; ++i) {
		INPUT_STANDARD_COMMAND command = static_cast<INPUT_STANDARD_COMMAND>(i);
		_help_commands.AddOption(InputManager->CommandName(command));
		_help_commands.AddOption(MakeUnicodeString(InputManager->GetKeyName(command)));
		_help_commands.AddOption(_command_descriptions.at(command));
	}

	// Additional commands. Insert a row of blank options to separate
	_help_commands.AddOption(ustring());
	_help_commands.AddOption(ustring());
	_help_commands.AddOption(ustring());
	_help_commands.AddOption(UTranslate("Pause"));
	_help_commands.AddOption(UTranslate("Spacebar"));
	_help_commands.AddOption(UTranslate("Pauses the game"));
	_help_commands.AddOption(UTranslate("Quit"));
	_help_commands.AddOption(UTranslate("Esc"));
	_help_commands.AddOption(UTranslate("Quit the application"));
	_help_commands.AddOption(UTranslate("Help"));
	_help_commands.AddOption(UTranslate("F1"));
	_help_commands.AddOption(UTranslate("Display command help"));
	_help_commands.AddOption(UTranslate("Fullscreen"));
	_help_commands.AddOption(UTranslate("Ctrl+F"));
	_help_commands.AddOption(UTranslate("Toggle between fullscreen or window"));
	_help_commands.AddOption(UTranslate("Screenshot"));
	_help_commands.AddOption(UTranslate("Ctrl+S"));
	_help_commands.AddOption(UTranslate("Save a screenshot of the game"));

	_help_return_text.SetStyle(TextStyle("title24"));
	_help_return_text.SetText(UTranslate("Press F1 to return to the game."));
}



PauseMode::~PauseMode() {
	if (_audio_paused == true)
		AudioManager->ResumeAudio();
}



void PauseMode::Reset() {
	if (_audio_paused == true)
		AudioManager->PauseAudio();

	// Save a copy of the current screen to use as the backdrop
	try {
		_screen_capture = VideoManager->CaptureScreen();
	}
	catch (Exception e) {
		IF_PRINT_WARNING(PAUSE_DEBUG) << e.ToString() << endl;
	}

	VideoManager->SetCoordSys(0.0f, VIDEO_STANDARD_RESOLUTION_WIDTH, 0.0f, VIDEO_STANDARD_RESOLUTION_HEIGHT);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
}



void PauseMode::Update() {
	// Don't eat up 100% of the CPU time while in pause mode. Put the process to sleep for 50ms
	SDL_Delay(50);

	if (_state == PAUSE) {
		if (InputManager->QuitPress() == true) {
			_state = QUIT;
		}
		else if (InputManager->PausePress() == true) {
			ModeManager->Pop();
		}
		else if (InputManager->HelpPress() == true) {
			_state = HELP;
			_help_window.Show();
		}
	}

	else if (_state == QUIT) {
		_quit_options.Update();

		if (InputManager->QuitPress() == true) {
			SystemManager->ExitGame();
		}
		else if (InputManager->HelpPress() == true) {
			_state = HELP;
			_help_window.Show();
		}

		else if (InputManager->ConfirmPress() == true) {
			switch (_quit_options.GetSelection()) {
				case QUIT_GAME:
					SystemManager->ExitGame();
					break;
				case QUIT_TO_BOOT:
					ModeManager->PopAll();
					ModeManager->Push(new BootMode());
					break;
				case QUIT_CANCEL:
					ModeManager->Pop();
					break;
				default:
					IF_PRINT_WARNING(PAUSE_DEBUG) << "unknown option selected: " << _quit_options.GetSelection() << endl;
					break;
			}
		}

		else if (InputManager->CancelPress() == true) {
			ModeManager->Pop();
		}

		else if (InputManager->LeftPress() == true) {
			_quit_options.InputLeft();
		}

		else if (InputManager->RightPress() == true) {
			_quit_options.InputRight();
		}
	}

	else if (_state == HELP) {
		if (InputManager->QuitPress() == true) {
			_help_window.Hide();
			_state = QUIT;
		}
		else if (InputManager->HelpPress() == true) {
			ModeManager->Pop();
		}
	}

	else {
		IF_PRINT_WARNING(PAUSE_DEBUG) << "invalid state: " << _state << ", terminating game mode" << endl;
		ModeManager->Pop();
	}
} // void PauseMode::Update()



void PauseMode::Draw() {
	// Set the coordinate system for the background and draw
	VideoManager->SetCoordSys(0.0f, _screen_capture.GetWidth(), 0.0f, _screen_capture.GetHeight());
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(0.0f, 0.0f);
	_screen_capture.Draw(_dim_color);

	// Re-set the coordinate system for everything else
	VideoManager->SetCoordSys(0.0f, VIDEO_STANDARD_RESOLUTION_WIDTH, 0.0f, VIDEO_STANDARD_RESOLUTION_HEIGHT);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(512.0f, 384.0f);

	if (_state == PAUSE) {
		_paused_text.Draw();
	}
	else if(_state == QUIT) {
		_quit_options.Draw();
	}
	else if (_state == HELP) {
		_help_window.Draw();

		// Don't draw any contents of the window until the window is fully shown
		if (_help_window.GetState() != VIDEO_MENU_STATE_SHOWN)
			return;

		// Draw the window contents, starting from the top and moving downward
		VideoManager->PushState();
		VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_TOP, 0);

		_help_commands_header.Draw();
		_help_commands.Draw();
		VideoManager->Move(512.0f, 120.0f);
		_help_return_text.Draw();

		VideoManager->PopState();
	}
} // void PauseMode::Draw()

} // namespace hoa_pause
