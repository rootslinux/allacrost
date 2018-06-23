///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_transition.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for map transition code
*** *****************************************************************************/

#include "utils.h"

#include "mode_manager.h"
#include "video.h"

#include "map.h"
#include "map_sprites.h"
#include "map_transition.h"
#include "map_utils.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_mode_manager;
using namespace hoa_video;

namespace hoa_map {

namespace private_map {

///////////////////////////////////////////////////////////////////////////////
// TransitionSupervisor Class Functions
///////////////////////////////////////////////////////////////////////////////

TransitionSupervisor::~TransitionSupervisor() {
	if (_next_mode != nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "_next_mode not null" << endl;
		delete _next_mode;
	}
}



bool TransitionSupervisor::StartContextTransition(private_map::MAP_CONTEXT context, uint32 time) {
	if (IsTransitionActive()) {
		return false;
	}

	if (context == MAP_CONTEXT_NONE || context == MAP_CONTEXT_ALL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "invalid context argument: " << context << endl;
		return false;
	}
	if (context == MapMode::CurrentInstance()->GetCurrentContext()) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to transition to the same context: " << context << endl;
		return false;
	}
	if (static_cast<uint32>(context) > static_cast<uint32>(1 << (MapMode::CurrentInstance()->GetNumMapContexts() - 1))) {
		IF_PRINT_WARNING(MAP_DEBUG) << "context argument exceeds the map's context range (" << context << ")" << endl;
		return false;
	}
	_next_context = context;

	if (_context_sprite != nullptr) {
		_context_sprite->SetMoving(false);
		// Set the virtual focus to where the sprite should be positioned in the next context at the end of the transition.
		VirtualSprite* focus = MapMode::CurrentInstance()->GetVirtualFocus();
		focus->SetContext(_context_sprite->GetContext()); // Leave the focus in the original context
		focus->SetXPosition(GetFloatInteger(_sprite_x_position), GetFloatFraction(_sprite_x_position));
		focus->SetYPosition(GetFloatInteger(_sprite_y_position), GetFloatFraction(_sprite_y_position));
		// This will being moving the camera from its current focus on the sprite to the location of the virtual focus
		MapMode::CurrentInstance()->SetCamera(focus, _timer.GetDuration() / 2);
	}

	MapMode::CurrentInstance()->PushState(STATE_TRANSITION);
	_timer.Initialize(time == 0 ? DEFAULT_CONTEXT_TRANSITION_TIME : time, 0);
	_timer.Run();
	VideoManager->FadeScreen(_transition_color, _timer.GetDuration() / 2); 	// Fade out the screen to the color for the first half of the timer

	return true;
}



bool TransitionSupervisor::StartGameModeTransition(hoa_mode_manager::GameMode* mode, uint32 time) {
	if (IsTransitionActive()) {
		return false;
	}

	if (mode == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "mode argument was nullptr" << endl;
		return false;
	}

	MapMode::CurrentInstance()->PushState(STATE_TRANSITION);
	_timer.Initialize(time == 0 ? DEFAULT_MODE_TRANSITION_TIME : time, 0);
	_timer.Run();
	VideoManager->FadeScreen(_transition_color, _timer.GetDuration());

	return true;
}



bool TransitionSupervisor::Update() {
	if (IsTransitionActive()) {
		_timer.Update();

		if (_timer.IsFinished()) {
			if (_next_mode != nullptr) {
				ModeManager->Pop();
				ModeManager->Push(_next_mode);

				// Fade the screen back in so that the new mode will be visible
				VideoManager->FadeScreen(Color::clear, _timer.GetDuration());
			}

			_RestoreDefaultSettings();
			MapMode::CurrentInstance()->PopState();
			return true;
		}
		else if (_second_phase_active == false && _timer.PercentComplete() >= 0.5f) {
			// For context transitions, we perform some updates when the transition is halfway complete. The screen should be completely dark
			// with the opaque transition color at this point, so the user doesn't see any of these changes until the screen fades back in.
			if (_next_context != MAP_CONTEXT_NONE) {
				// Fade the screen back in so that the new context will become visible
				_transition_color.SetAlpha(0.0f); // Set the alpha to zero so we can fade out the color
				VideoManager->FadeScreen(_transition_color, _timer.GetDuration() - _timer.GetTimeExpired());

				// If we're updating a sprite during the transition, now set the sprite to its new position and point the map camera back to it
				if (_context_sprite != nullptr) {
					_context_sprite->SetContext(_next_context);

					VirtualSprite* focus = MapMode::CurrentInstance()->GetVirtualFocus();
					uint16 integer;
					float offset;
					focus->GetXPosition(integer, offset);
					_context_sprite->SetXPosition(integer, offset);
					focus->GetYPosition(integer, offset);
					_context_sprite->SetYPosition(integer, offset);

					if (_sprite_direction != 0) {
						_context_sprite->SetDirection(_sprite_direction);
					}

					MapMode::CurrentInstance()->SetCamera(_context_sprite);
				}
				else {
					// Set the context of the map camera to change the context
					MapMode::CurrentInstance()->GetCamera()->SetContext(_next_context);
				}
			}

			_second_phase_active = true;
		}
	}

	return false;
}



void TransitionSupervisor::SetTransitionColor(hoa_video::Color& color) {
	if (IsTransitionActive() == false) {
		return;
	}

	_transition_color = color;
	_transition_color.SetAlpha(1.0f);
}



void TransitionSupervisor::SetTerminateMapOnCompletion() {
	if (IsTransitionActive() == false) {
		return;
	}

	_terminate_map_on_completion = true;
}



void TransitionSupervisor::SetContextCameraChanges(VirtualSprite* sprite, float x_position, float y_position, bool relative_position, uint16 direction) {
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function received nullptr sprite argument" << endl;
		return;
	}
	if (sprite == MapMode::CurrentInstance()->GetVirtualFocus()) {
		IF_PRINT_WARNING(MAP_DEBUG) << "using virtual focus sprite during a context change will result in incorrect camera movement" << endl;
	}

	_context_sprite = sprite;


	if (relative_position == false) {
		_sprite_x_position = x_position;
		_sprite_y_position = y_position;
	}
	else {
		uint16 integer;
		float offset;
		sprite->GetXPosition(integer, offset);
		_sprite_x_position = static_cast<float>(integer) + offset + x_position;
		sprite->GetYPosition(integer, offset);
		_sprite_y_position = static_cast<float>(integer) + offset + y_position;
	}

	_sprite_direction = direction;
}



void TransitionSupervisor::_RestoreDefaultSettings() {
	_next_context = MAP_CONTEXT_NONE;
	_next_mode = nullptr;
	_transition_color = Color::black;
	_terminate_map_on_completion = false;
	_context_sprite = nullptr;
	_sprite_x_position = 0.0f;
	_sprite_y_position = 0.0f;
	_sprite_direction = 0;
	_second_phase_active = false;
	_timer.Initialize(0, 0);
}

} // namespace private_map

} // namespace hoa_map
