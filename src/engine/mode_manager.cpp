///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file   mode_manager.cpp
*** \author Tyler Olsen (Roots)
*** \brief  Source file for game mode processing
*** **************************************************************************/

#include "mode_manager.h"
#include "system.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_system;

template<> hoa_mode_manager::ModeEngine* Singleton<hoa_mode_manager::ModeEngine>::_singleton_reference = nullptr;

namespace hoa_mode_manager {

ModeEngine* ModeManager = nullptr;
bool MODE_MANAGER_DEBUG = false;

////////////////////////////////////////////////////////////////////////////////
// ModeEngine class methods
////////////////////////////////////////////////////////////////////////////////

ModeEngine::ModeEngine() {
	IF_PRINT_DEBUG(MODE_MANAGER_DEBUG) << "constructor invoked" << endl;
	_pop_count = 0;
	_state_change = false;
	_debug_graphics_enabled = false;
}



ModeEngine::~ModeEngine() {
	IF_PRINT_DEBUG(MODE_MANAGER_DEBUG) << "destructor invoked" << endl;
	// Delete any game modes on the stack
	while (_game_stack.size() != 0) {
		delete _game_stack.back();
		_game_stack.pop_back();
	}

	// Delete any game modes on the push stack
	while (_push_stack.size() != 0) {
		delete _push_stack.back();
		_push_stack.pop_back();
	}
}



bool ModeEngine::SingletonInitialize() {
	// Delete any game modes on the stack
	while (_game_stack.size() != 0) {
		delete _game_stack.back();
		_game_stack.pop_back();
	}

	// Delete any game modes on the push stack
	while (_push_stack.size() != 0) {
		delete _push_stack.back();
		_push_stack.pop_back();
	}

	// Reset the pop counter
	_pop_count = 0;

	return true;
}



void ModeEngine::Pop() {
	_pop_count++;
	_state_change = true;
}



void ModeEngine::PopAll() {
	_pop_count = static_cast<uint32>(_game_stack.size());
	if (_pop_count > 0)
		_state_change = true;
}



void ModeEngine::Push(GameMode* new_mode) {
	if (new_mode == nullptr) {
		IF_PRINT_DEBUG(MODE_MANAGER_DEBUG) << "function received null argument" << endl;
		return;
	}
	for (GameMode* existing_mode : _game_stack) {
		if (new_mode == existing_mode) {
			IF_PRINT_DEBUG(MODE_MANAGER_DEBUG) << "attempted to push a mode that already existed on the stack" << endl;
			return;
		}
	}
	for (GameMode* existing_mode : _push_stack) {
		if (new_mode == existing_mode) {
			IF_PRINT_DEBUG(MODE_MANAGER_DEBUG) << "attempted to push a mode that had already been pushed" << endl;
			return;
		}
	}

	_push_stack.push_back(new_mode);
	_state_change = true;
}



GAME_MODE_TYPE ModeEngine::GetModeType() {
	if (_game_stack.empty())
		return INVALID_MODE;
	else
		return _game_stack.back()->mode_type;
}



GAME_MODE_TYPE ModeEngine::GetModeType(uint32 index) {
	if (_game_stack.size() < index)
		return INVALID_MODE;
	else
		return _game_stack.at(_game_stack.size() - index)->mode_type;
}



GameMode* ModeEngine::GetTop() {
	if (_game_stack.empty())
		return nullptr;
	else
		return _game_stack.back();
}



GameMode* ModeEngine::GetMode(uint32 index) {
	if (_game_stack.size() < index)
		return nullptr;
	else
		return _game_stack.at(_game_stack.size() - index);
}



bool ModeEngine::IsModeTypeInStack(GAME_MODE_TYPE type) {
	for (uint32 i = 0; i < _game_stack.size(); ++i) {
		if (_game_stack.at(i)->mode_type == type)
			return true;
	}

	return false;
}



void ModeEngine::Update() {
	// If a Push() or Pop() function was called, we need to adjust the state of the game stack.
	if (_state_change == true) {
		// Pop however many game modes we need to from the top of the stack
		while (_pop_count != 0) {
			if (_game_stack.empty()) {
				IF_PRINT_WARNING(MODE_MANAGER_DEBUG) << "tried to pop off more game modes than were on the stack" << endl;
				_pop_count = 0; // Reset the pop count so we don't continue to try to pop off more modes in future calls to Update()
				break; // Exit the loop
			}
			delete _game_stack.back();
			_game_stack.pop_back();
			_pop_count--;
		}

		// Push any new game modes onto the true game stack.
		while (_push_stack.size() != 0) {
			_game_stack.push_back(_push_stack.back());
			_push_stack.pop_back();
		}

		// Make sure there is a game mode on the stack, otherwise we'll get a segementation fault.
		if (_game_stack.empty()) {
			IF_PRINT_WARNING(MODE_MANAGER_DEBUG) << "game stack is empty; exiting application" << endl;
			SystemManager->ExitGame();
		}

		// Call the newly active game mode's Reset() function to re-initialize the game mode
		_game_stack.back()->Reset();

		// Reset the state change variable
		_state_change = false;

		// Call the system manager and tell it that the active game mode changed so it can update timers accordingly
		SystemManager->ExamineSystemTimers();

		// Re-initialize the game update timer so that the new active game mode does not begin with any update time to process
		SystemManager->InitializeUpdateTimer();
	} // if (_state_change == true)

	// Call the Update function on the top stack mode (the active game mode)
	_game_stack.back()->Update();
}



void ModeEngine::Draw() {
	if (_game_stack.size() == 0) {
		return;
	}

	_game_stack.back()->Draw();
}



void ModeEngine::DEBUG_PrintStack() {
	PRINT_DEBUG << "printing game stack" << endl;
	if (_game_stack.size() == 0) {
		cout << "*** game stack is empty ***" << endl;
		return;
	}

	cout << "*** top of stack ***" << endl;
	for (int32 i = static_cast<int32>(_game_stack.size()) - 1; i >= 0; i--)
		cout << " index: " << i << " type: " << _game_stack[i]->mode_type << endl;
	cout << "*** bottom of stack ***" << endl;
}

} // namespace hoa_mode_manager
