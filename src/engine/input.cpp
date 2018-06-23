///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file   input.cpp
*** \author Tyler Olsen (Roots)
*** \brief  Source file for processing user input
*** **************************************************************************/

#include "input.h"
#include "video.h"
#include "script.h"

#include "mode_manager.h"
#include "system.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_script;
using namespace hoa_mode_manager;
using namespace hoa_system;

using namespace hoa_input::private_input;

template<> hoa_input::InputEngine* Singleton<hoa_input::InputEngine>::_singleton_reference = nullptr;

namespace hoa_input {

InputEngine* InputManager = nullptr;
bool INPUT_DEBUG = false;


hoa_utils::ustring InputEngine::_command_names[] = {
	UTranslate("Up"),
	UTranslate("Down"),
	UTranslate("Left"),
	UTranslate("Right"),
	UTranslate("Confirm"),
	UTranslate("Cancel"),
	UTranslate("Menu"),
	UTranslate("Swap"),
	UTranslate("Left Select"),
	UTranslate("Right Select"),
	UTranslate("Pause")
};



InputEngine::InputEngine() {
	IF_PRINT_DEBUG(INPUT_DEBUG) << "constructor invoked" << endl;

	_any_key_press        = false;
	_any_key_release      = false;
	_unmapped_key_press   = false;
	_last_axis_moved      = -1;
	_up_state             = false;
	_up_press             = false;
	_up_release           = false;
	_down_state           = false;
	_down_press           = false;
	_down_release         = false;
	_left_state           = false;
	_left_press           = false;
	_left_release         = false;
	_right_state          = false;
	_right_press          = false;
	_right_release        = false;
	_confirm_state        = false;
	_confirm_press        = false;
	_confirm_release      = false;
	_cancel_state         = false;
	_cancel_press         = false;
	_cancel_release       = false;
	_menu_state           = false;
	_menu_press           = false;
	_menu_release         = false;
	_swap_state           = false;
	_swap_press           = false;
	_swap_release         = false;
	_right_select_state   = false;
	_right_select_press   = false;
	_right_select_release = false;
	_left_select_state    = false;
	_left_select_press    = false;
	_left_select_release  = false;

	_pause_press          = false;
	_quit_press           = false;

	_joyaxis_x_first      = true;
	_joyaxis_y_first      = true;
	_joystick.js          = nullptr;
	_joystick.x_axis      = 0;
	_joystick.y_axis      = 1;
	_joystick.threshold   = 8192;
}



InputEngine::~InputEngine() {
	IF_PRINT_DEBUG(INPUT_DEBUG) << "destructor invoked" << endl;

	// If a joystick is open, close it before exiting
	if (_joystick.js != nullptr) {
		SDL_JoystickClose(_joystick.js);
	}
}



bool InputEngine::SingletonInitialize() {
	// Initialize the SDL joystick subsystem
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
		PRINT_ERROR << "failed to initailize the SDL joystick subsystem" << endl;
		return false;
	}

	return true;
}



const hoa_utils::ustring& InputEngine::CommandName(INPUT_STANDARD_COMMAND command) {
	static ustring empty_string = ustring();

	if (command <= COMMAND_INVALID || command >= COMMAND_TOTAL) {
		IF_PRINT_WARNING(INPUT_DEBUG) << "invalid command argument: " << command << endl;
		return empty_string;
	}
	else {
		return _command_names[command];
	}
}



void InputEngine::InitializeJoysticks() {
	// Attempt to initialize and setup the joystick system
	if (SDL_NumJoysticks() == 0) { // No joysticks found
		SDL_JoystickEventState(SDL_IGNORE);
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	}
	else { // At least one joystick exists
		SDL_JoystickEventState(SDL_ENABLE);
		// TODO: need to allow user to specify which joystick to open, if multiple exist
		_joystick.js = SDL_JoystickOpen(_joystick.joy_index);
	}
}



bool InputEngine::RestoreDefaultKeys() {
	// Load the settings file
	string in_filename = GetSettingsFilename();
	ReadScriptDescriptor settings_file;
	if (settings_file.OpenFile(in_filename) == false) {
		PRINT_ERROR << "failed to open data file for reading: " << in_filename << endl;
		return false;
	}

	// Load all default keys from the table
	settings_file.OpenTable("settings");
	settings_file.OpenTable("key_defaults");
	_key.up           = SDL_GetKeyFromName(settings_file.ReadString("up").c_str());
	_key.down         = SDL_GetKeyFromName(settings_file.ReadString("down").c_str());
	_key.left         = SDL_GetKeyFromName(settings_file.ReadString("left").c_str());
	_key.right        = SDL_GetKeyFromName(settings_file.ReadString("right").c_str());
	_key.confirm      = SDL_GetKeyFromName(settings_file.ReadString("confirm").c_str());
	_key.cancel       = SDL_GetKeyFromName(settings_file.ReadString("cancel").c_str());
	_key.menu         = SDL_GetKeyFromName(settings_file.ReadString("menu").c_str());
	_key.swap         = SDL_GetKeyFromName(settings_file.ReadString("swap").c_str());
	_key.left_select  = SDL_GetKeyFromName(settings_file.ReadString("left_select").c_str());
	_key.right_select = SDL_GetKeyFromName(settings_file.ReadString("right_select").c_str());
	_key.pause        = SDL_GetKeyFromName(settings_file.ReadString("pause").c_str());
	settings_file.CloseTable();
	settings_file.CloseTable();

	settings_file.CloseFile();

	return true;
}



bool InputEngine::RestoreDefaultJoyButtons() {
	// Load the settings file
	string in_filename = GetSettingsFilename();
	ReadScriptDescriptor settings_file;
	if (settings_file.OpenFile(in_filename) == false) {
		PRINT_ERROR << "failed to open data file for reading: " << in_filename << endl;
		return false;
	}

	// Load all default buttons from the table
	settings_file.OpenTable("settings");
	settings_file.OpenTable("joystick_defaults");
	_joystick.confirm      = static_cast<uint32>(settings_file.ReadInt("confirm"));
	_joystick.cancel       = static_cast<uint32>(settings_file.ReadInt("cancel"));
	_joystick.menu         = static_cast<uint32>(settings_file.ReadInt("menu"));
	_joystick.swap         = static_cast<uint32>(settings_file.ReadInt("swap"));
	_joystick.left_select  = static_cast<uint32>(settings_file.ReadInt("left_select"));
	_joystick.right_select = static_cast<uint32>(settings_file.ReadInt("right_select"));
	_joystick.pause        = static_cast<uint32>(settings_file.ReadInt("pause"));
	_joystick.quit		   = static_cast<uint32>(settings_file.ReadInt("quit"));
	settings_file.CloseTable();
	settings_file.CloseTable();

	settings_file.CloseFile();

	return true;
}



void InputEngine::EventHandler() {
	SDL_Event event; // Holds the game event

	// Reset all of the press and release flags so that they don't get detected twice.
	_any_key_press = false;
	_any_key_release = false;
	_unmapped_key_press = false;

	_up_press             = false;
	_up_release           = false;
	_down_press           = false;
	_down_release         = false;
	_left_press           = false;
	_left_release         = false;
	_right_press          = false;
	_right_release        = false;
	_confirm_press        = false;
	_confirm_release      = false;
	_cancel_press         = false;
	_cancel_release       = false;
	_menu_press           = false;
	_menu_release         = false;
	_swap_press           = false;
	_swap_release         = false;
	_right_select_press   = false;
	_right_select_release = false;
	_left_select_press    = false;
	_left_select_release  = false;

	_pause_press = false;
	_quit_press = false;
	_help_press = false;

	// Loops until there are no remaining events to process
	while (SDL_PollEvent(&event)) {
		_event = event;
		if (event.type == SDL_QUIT) {
			_quit_press = true;
			break;
		}
		// Check if the window was iconified/minimized or restored
		else if (event.type == SDL_WINDOWEVENT) {
			// TEMP: pausing the game on a context switch between another application proved to
			// be rather annoying. The code which did this is commented out below. I think it would
			// be better if instead the application yielded for a certain amount of time when the
			// application looses context.

// 			if (event.active.state & SDL_APPACTIVE) {
// 				if (event.active.gain == 0) { // Window was iconified/minimized
// 					// Check if the game is in pause mode. Otherwise the player might put pause on,
// 					// minimize the window and then the pause is off.
// 					if (ModeManager->GetGameType() != PAUSE_MODE) {
// 						TogglePause();
// 					}
// 				}
// 				else if (ModeManager->GetGameType() == PAUSE_MODE) { // Window was restored
// 					TogglePause();
// 				}
// 			}
// 			else if (event.active.state & SDL_APPINPUTFOCUS) {
// 				if (event.active.gain == 0) { // Window lost keyboard focus (another application was made active)
// 					// Check if the game is in pause mode. Otherwise the player might put pause on,
// 					// minimize the window and then the pause is off.
// 					if (ModeManager->GetGameType() != PAUSE_MODE) {
// 						TogglePause();
// 					}
// 				}
// 				else if (ModeManager->GetGameType() == PAUSE_MODE) { // Window gain keyboard focus (not sure)
// 					TogglePause();
// 				}
// 			}
			break;
		}
		else if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
			_KeyEventHandler(event.key);
		}
		else {
			_JoystickEventHandler(event);
		}
	} // while (SDL_PollEvent(&event)
} // void InputEngine::EventHandler()



string InputEngine::GetKeyName(INPUT_STANDARD_COMMAND command) const {
	switch (command) {
		case UP_COMMAND:
			return GetUpKeyName();
		case DOWN_COMMAND:
			return GetDownKeyName();
		case LEFT_COMMAND:
			return GetLeftKeyName();
		case RIGHT_COMMAND:
			return GetRightKeyName();
		case CONFIRM_COMMAND:
			return GetConfirmKeyName();
		case CANCEL_COMMAND:
			return GetCancelKeyName();
		case MENU_COMMAND:
			return GetMenuKeyName();
		case SWAP_COMMAND:
			return GetSwapKeyName();
		case LEFT_SELECT_COMMAND:
			return GetLeftSelectKeyName();
		case RIGHT_SELECT_COMMAND:
			return GetRightSelectKeyName();
		case PAUSE_COMMAND:
			return GetPauseKeyName();
		default:
			IF_PRINT_WARNING(INPUT_DEBUG) << "received invalid command argument: " << command << endl;
	}

	return "";
}



void InputEngine::_KeyEventHandler(SDL_KeyboardEvent& key_event) {
	if (key_event.type == SDL_KEYDOWN) { // Key was pressed
		_any_key_press = true;

		// CTRL key was held down
		if (key_event.keysym.mod & KMOD_CTRL || key_event.keysym.sym == SDLK_LCTRL || key_event.keysym.sym == SDLK_RCTRL) {
			_any_key_press = false; // We don't treat Ctrl+key presses as an "any key"

			if (key_event.keysym.sym == SDLK_a) {
				// Ctrl+A: "Advanced" display of video engine information
				VideoManager->ToggleAdvancedDisplay();
			}
			else if (key_event.keysym.sym == SDLK_f) {
				// Ctrl+F: "Fullscreen" toggle
				VideoManager->ToggleFullscreen();
				VideoManager->ApplySettings();
				return;
			}
			else if (key_event.keysym.sym == SDLK_g) {
				// Ctrl+G: "Graphical" debug toggle
				ModeManager->DEBUG_ToggleGraphicsEnabled();
				return;
			}
			else if (key_event.keysym.sym == SDLK_q) {
				// Ctrl+Q: "Quit" command requested
				_quit_press = true;
				return;
			}
			else if (key_event.keysym.sym == SDLK_r) {
				// Ctrl+R: "Rate" of frames drawn per second toggle
				VideoManager->ToggleFPS();
				return;
			}
			else if (key_event.keysym.sym == SDLK_s) {
				// Ctrl+S: "Screenshot" generation request
				static uint32 i = 1;
				string path = "";
				while (true) {
					path = hoa_utils::GetUserDataPath(true) + "screenshot_" + NumberToString<uint32>(i) + ".jpg";
					if (!DoesFileExist(path))
						break;
					i++;
				}
				VideoManager->MakeScreenshot(path);
				return;
			}
			else if (key_event.keysym.sym == SDLK_t) {
				// Ctrl+T: "Test" mode return request
				// This command is only processed only when a test mode instance is already on the stack. Otherwise it is ignored
				if (ModeManager->IsModeTypeInStack(TEST_MODE) == true) {
					// Removes all game modes from the stack except for the bottom most one, which should be the TestMode instance
					for (uint32 i = 1; i < ModeManager->GetModeStackSize(); ++i) {
						ModeManager->Pop();
					}
					// NOTE: Although it is rare, there may also be some game modes that are preparing to be pushed onto the stack
					// when this command is invoked. In that case, the newly pushed mode will be on the top, requiring the user to
					// enter this command once again. This bug is simple enough to get around but could be trick to provide a fix for
					// due to memory allocations of the modes about to be pushed. So for now this issue remains unaddressed.
				}
				return;
			}
			else if (key_event.keysym.sym == SDLK_x) {
				// Ctrl+X: "Texture" sheet display and cycle
				VideoManager->Textures()->DEBUG_NextTexSheet();
				return;
			}
			else if (key_event.keysym.sym == SDLK_F1) {
				// Ctrl+F1: Enable graphical debugging setting
				VideoManager->DEBUG_SetGraphicsDebuggingEnabled(!VideoManager->DEBUG_IsGraphicsDebuggingEnabled());
			}
		} // endif CTRL pressed

		else {
			// Note: a switch-case statement won't work here because _key.up is not an integer value
			if (key_event.keysym.sym == SDLK_ESCAPE) {
				_quit_press = true;
				return;
			}
			else if (key_event.keysym.sym == _key.up) {
				_up_state = true;
				_up_press = true;
				return;
			}
			else if (key_event.keysym.sym == _key.down) {
				_down_state = true;
				_down_press = true;
				return;
			}
			else if (key_event.keysym.sym == _key.left) {
				_left_state = true;
				_left_press = true;
				return;
			}
			else if (key_event.keysym.sym == _key.right) {
				_right_state = true;
				_right_press = true;
				return;
			}
			else if (key_event.keysym.sym == _key.confirm) {
				_confirm_state = true;
				_confirm_press = true;
				return;
			}
			else if (key_event.keysym.sym == _key.cancel) {
				_cancel_state = true;
				_cancel_press = true;
				return;
			}
			else if (key_event.keysym.sym == _key.menu) {
				_menu_state = true;
				_menu_press = true;
				return;
			}
			else if (key_event.keysym.sym == _key.swap) {
				_swap_state = true;
				_swap_press = true;
				return;
			}
			else if (key_event.keysym.sym == _key.left_select) {
				_left_select_state = true;
				_left_select_press = true;
				return;
			}
			else if (key_event.keysym.sym == _key.right_select) {
				_right_select_state = true;
				_right_select_press = true;
				return;
			}
			else if (key_event.keysym.sym == _key.pause) {
				_pause_press = true;
				return;
			}
			else if (key_event.keysym.sym == SDLK_F1) {
				_help_press = true;
				return;
			}
			else if (key_event.keysym.sym != SDLK_LCTRL && key_event.keysym.sym != SDLK_RCTRL) {
				_unmapped_key_press = true;
				return;
			}
		}
	}

	else { // Key was released
		_any_key_release = true;

		if (key_event.keysym.sym == _key.up) {
			_up_state = false;
			_up_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.down) {
			_down_state = false;
			_down_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.left) {
			_left_state = false;
			_left_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.right) {
			_right_state = false;
			_right_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.confirm) {
			_confirm_state = false;
			_confirm_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.cancel) {
			_cancel_state = false;
			_cancel_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.menu) {
			_menu_state = false;
			_menu_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.swap) {
			_swap_state = false;
			_swap_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.left_select) {
			_left_select_state = false;
			_left_select_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.right_select) {
			_right_select_state = false;
			_right_select_release = true;
			return;
		}
	}
} // void InputEngine::_KeyEventHandler(SDL_KeyboardEvent& key_event)



void InputEngine::_JoystickEventHandler(SDL_Event& js_event) {
	if (js_event.type == SDL_JOYAXISMOTION) {
		if (js_event.jaxis.axis == _joystick.x_axis) {
			if (js_event.jaxis.value < -_joystick.threshold) {
				if (!_left_state) {
					_left_state = true;
					_left_press = true;
				}
			}
			else {
				_left_state = false;
			}

			if (js_event.jaxis.value > _joystick.threshold) {
				if (!_right_state) {
					_right_state = true;
					_right_press = true;
				}
			}
			else {
				_right_state = false;
			}
		}
		else if (js_event.jaxis.axis == _joystick.y_axis) {
			if (js_event.jaxis.value < -_joystick.threshold) {
				if (!_up_state) {
					_up_state = true;
					_up_press = true;
				}
			}
			else {
				_up_state = false;
			}

			if (js_event.jaxis.value > _joystick.threshold) {
				if (!_down_state) {
					_down_state = true;
					_down_press = true;
				}
			}
			else {
				_down_state = false;
			}
		}

		if (js_event.jaxis.value > _joystick.threshold
			|| js_event.jaxis.value < -_joystick.threshold)
			_last_axis_moved = js_event.jaxis.axis;
	} // if (js_event.type == SDL_JOYAXISMOTION)

	else if (js_event.type == SDL_JOYBUTTONDOWN) {

		_any_key_press = true;

		if (js_event.jbutton.button == _joystick.confirm) {
			_confirm_state = true;
			_confirm_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.cancel) {
			_cancel_state = true;
			_cancel_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.menu) {
			_menu_state = true;
			_menu_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.swap) {
			_swap_state = true;
			_swap_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.left_select) {
			_left_select_state = true;
			_left_select_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.right_select) {
			_right_select_state = true;
			_right_select_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.pause) {
			_pause_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.quit) {
			_quit_press = true;
			return;
		}
	} // else if (js_event.type == JOYBUTTONDOWN)

	else if (js_event.type == SDL_JOYBUTTONUP) {
		_any_key_press = false;
		_any_key_release = true;

		if (js_event.jbutton.button == _joystick.confirm) {
			_confirm_state = false;
			_confirm_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.cancel) {
			_cancel_state = false;
			_cancel_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.menu) {
			_menu_state = false;
			_menu_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.swap) {
			_swap_state = false;
			_swap_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.left_select) {
			_left_select_state = false;
			_left_select_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.right_select) {
			_right_select_state = false;
			_right_select_release = true;
			return;
		}
	} // else if (js_event.type == JOYBUTTONUP)

	// NOTE: SDL_JOYBALLMOTION and SDL_JOYHATMOTION are ignored for now. Should we process them?
} // void InputEngine::_JoystickEventHandler(SDL_Event& js_event)



void InputEngine::_SetNewKey(SDL_Keycode & old_key, SDL_Keycode new_key) {
	if (_key.up == new_key) { // up key used already
		_key.up = old_key;
		old_key = new_key;
		return;
	}
	if (_key.down == new_key) { // down key used already
		_key.down = old_key;
		old_key = new_key;
		return;
	}
	if (_key.left == new_key) { // left key used already
		_key.left = old_key;
		old_key = new_key;
		return;
	}
	if (_key.right == new_key) { // right key used already
		_key.right = old_key;
		old_key = new_key;
		return;
	}
	if (_key.confirm == new_key) { // confirm key used already
		_key.confirm = old_key;
		old_key = new_key;
		return;
	}
	if (_key.cancel == new_key) { // cancel key used already
		_key.cancel = old_key;
		old_key = new_key;
		return;
	}
	if (_key.menu == new_key) { // menu key used already
		_key.menu = old_key;
		old_key = new_key;
		return;
	}
	if (_key.swap == new_key) { // swap key used already
		_key.swap = old_key;
		old_key = new_key;
		return;
	}
	if (_key.left_select == new_key) { // left_select key used already
		_key.left_select = old_key;
		old_key = new_key;
		return;
	}
	if (_key.right_select == new_key) { // right_select key used already
		_key.right_select = old_key;
		old_key = new_key;
		return;
	}
	if (_key.pause == new_key) { // pause key used already
		_key.pause = old_key;
		old_key = new_key;
		return;
	}

	old_key = new_key; // Otherwise simply overwrite the old value
} // InputEngine::_SetNewKey(SDLKey & old_key, SDLKey new_key)



void InputEngine::_SetNewJoyButton(uint32 & old_button, uint32 new_button) {
	if (_joystick.confirm == new_button) { // confirm button used already
		_joystick.confirm = old_button;
		old_button = new_button;
		return;
	}
	if (_joystick.cancel == new_button) { // cancel button used already
		_joystick.cancel = old_button;
		old_button = new_button;
		return;
	}
	if (_joystick.menu == new_button) { // menu button used already
		_joystick.menu = old_button;
		old_button = new_button;
		return;
	}
	if (_joystick.swap == new_button) { // swap button used already
		_joystick.swap = old_button;
		old_button = new_button;
		return;
	}
	if (_joystick.left_select == new_button) { // left_select button used already
		_joystick.left_select = old_button;
		old_button = new_button;
		return;
	}
	if (_joystick.right_select == new_button) { // right_select button used already
		_joystick.right_select = old_button;
		old_button = new_button;
		return;
	}
	if (_joystick.pause == new_button) { // pause button used already
		_joystick.pause = old_button;
		old_button = new_button;
		return;
	}

	old_button = new_button; // Otherwise simply overwrite the old value
} // InputEngine::_SetNewJoyButton(uint32 & old_button, uint32 new_button)


} // namespace hoa_input
