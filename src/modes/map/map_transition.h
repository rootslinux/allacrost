///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_transition.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for map transition management
*** *****************************************************************************/

#pragma once

// Allacrost utilities
#include "utils.h"
#include "defs.h"

// Allacrost engines
#include "mode_manager.h"
#include "system.h"
#include "video.h"

// Map mode headers
#include "map_utils.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief Manages data and actions related to map transitions
***
*** There are two forms of transitioning in map mode. The first is transitioning from one map
*** context to another. The second is transitioning from the map to another game mode (which
*** could be another map, a battle, etc). Being in either sort of transition state can be tricky
*** because typically we want to do things like prevent player actions, certain types of inputs,
*** etc. Both forms of transition have two states first starting with a "fade out" state followed
*** by a "fade in" state.
***
*** There are a lot of configuration options for transitions including time, color, and so on.
*** Rather than have a long function call with several arguments, only two argument are mandatory:
*** the context or game mode, and the total amount of transition time. All other settings must be
*** changed via the "Set*()" methods priopr to starting the transition. Note that these settings can
*** only be set when no transition is in effect. After the transition completes, all settings are
*** returned to their default values automatically and will need to be set again for the next transition.
*** This reduces any unexpected behavior from settings for a previous transition effecting another, and
*** also reduces the number of set calls required as the default behavior is usually what you want.
***
*** In summary, to use this class do the following:
*** -# Before starting your transition, ensure that there is no other transition currently active
*** -# Call the various Set* methods to change any default transition behavior as desired
*** -# Call either StartContextTransition() or StartGameModeTransition()
*** -# Continually call Update() until it returns true, indicating the transition is finished
***
*** \note This class serves as an assistant to the MapMode class and as such, it does not implement all of
*** the required functionality for transitions. MapMode is responsible for calling the Update() method and
*** for making any changes related to things like stopping camera movement or changing the map state.
***
*** \todo In addition to color transitions, for context transitions it would be nice if we could draw both
*** contexts and fade one into the other.
***
*** \bug Issues can arise for context transitions where a sprite's properties are changed. In particular, if
*** the user passes in a pointer to the virtual focus as the sprite to change, the camera move won't work
*** correctly since the virtual focus is used during that movement phase. There are other ways errors can
*** occur with this class, such as using a context or direction value that doesn't map to an actual enumerated
*** value, or setting a sprite's positions to be outside the boundaries of the map. In general, some additional
*** error checking should be considered here.
*** ***************************************************************************/
class TransitionSupervisor {
public:
	//! \brief The default number of milliseconds it takes to complete a map context transition
	static const uint32 DEFAULT_CONTEXT_TRANSITION_TIME = 500;

	//! \brief The default number of milliseconds it takes to transition to a new game mode
	static const uint32 DEFAULT_MODE_TRANSITION_TIME = 250;

	TransitionSupervisor()
		{ _RestoreDefaultSettings(); }

	~TransitionSupervisor();

	/** \brief Begins a transition to a new context
	*** \param context The new context to transition to
	*** \return True if the transition started successfully. False is returned if a transition is already
	*** in effect or the input arguments are invalid.
	**/
	bool StartContextTransition(private_map::MAP_CONTEXT context)
		{ return StartContextTransition(context, DEFAULT_CONTEXT_TRANSITION_TIME); }

	/** \brief Begins a transition to a new context
	*** \param context The new context to transition to
	*** \param time The number of milliseconds that the full transition should take (if 0, the default time will be used)
	*** \return True if the transition started successfully. False is returned if a transition is already
	*** in effect or the input arguments are invalid.
	**/
	bool StartContextTransition(private_map::MAP_CONTEXT context, uint32 time);

	bool StartGameModeTransition(hoa_mode_manager::GameMode* mode)
		{ return StartGameModeTransition(mode, DEFAULT_MODE_TRANSITION_TIME); }

	/** \brief Begins a transition to a new game mode
	*** \param context The new mode to transition to (will be pushed to the game stack)
	*** \param time The number of milliseconds that the full transition should take (if 0, the default time will be used)
	*** \return True if the transition started successfully. False is returned if a transition is already
	*** in effect or the input arguments are invalid.
	**/
	bool StartGameModeTransition(hoa_mode_manager::GameMode* mode, uint32 time);

	/** \brief Updates an active transition (by updating the timer)
	*** \return True if the transition finished after this update
	*** \note If this is called when a transition is not active, false is returned and no action takes place
	**/
	bool Update();

	bool IsTransitionActive()
		{ return _timer.IsRunning(); }

	/** \brief Used to set the color that the transition should use when fading the screen
	*** \param color The color to use
	*** \note The alpha value in the color is ignored as it gets set automatically for the transition effect
	**/
	void SetTransitionColor(hoa_video::Color& color);

	//! \brief Used to terminate the current map mode instance by popping it off the game stack once the new mode is pushed to the game stack
	void SetTerminateMapOnCompletion();

	/** \brief Instructs camera movement instructions that the transition should follow
	*** \param sprite The sprite that the camera is pointing to when the transition begins (normally this will be the player-controlled sprite)
	*** \param x_position The new X position to set for the sprite
	*** \param y_position The new Y position to set for the sprite
	*** \param relative_position If true, the X/Y positions are relative to the sprite's current position. Otherwise they are absolute coordinates
	*** \param direction The new direction that the sprite should be facing. A zero value indicates to leave the current direction unchanged
	***
	*** This method will only have an effect on context transitions, not mode transitions. What it does is the following:
	*** -# When the transition begins, stop any movement of the sprite, set the map's virtual focus to the position of the sprite,
	***    and point the camera at the virtual focus.
	*** -# Begin moving the virtual focus to the desired X/Y position for the first half of the transition
	*** -# After reaching the half-way point of the transition, set the properties of the sprite to the new desired position and direction
	***
	*** The end result is a gradual pan as the screen is fading out from the original context, followed by a motionless fade-in to the new context.
	*** This is the typical desired behavior when switching between one context to the next, as there is usually a staircase, doorway, or other
	*** object being passed through to arrive at the new context.
	**/
	void SetContextCameraChanges(VirtualSprite* sprite, float x_position, float y_position, bool relative_position, uint16 direction);

private:
	//! \brief While transitioning between two contexts, holds the value of the context we are changing to
	private_map::MAP_CONTEXT _next_context;

	//! \brief When in the MAP_TRANSITION state, holds a pointer to the game mode to be transitioned to
	hoa_mode_manager::GameMode* _next_mode;

	//! \brief Holds the color to transition with if doing a color transition type
	hoa_video::Color _transition_color;

	/** \brief If set to true, removes the active instance of MapMode from the game stack and destroyes it when the next mode transition completes
	*** \note This setting is only valid for a mode transition. If it is used during a context transition, no effect will take place
	**/
	bool _terminate_map_on_completion;

	//! \brief For context transitions, holds a pointer to the sprite that should have its properties updated as the transition completes
	VirtualSprite* _context_sprite;

	//! \brief The new X position to set the _context_sprite to as the transition completes
	float _sprite_x_position;

	//! \brief The new Y position to set the _context_sprite to as the transition completes
	float _sprite_y_position;

	//! \brief The new direction to set the _context_sprite to as the transition completes
	uint16 _sprite_direction;

	//! \brief Set to true when the transition is half-way complete
	bool _second_phase_active;

	//! \brief A timer used to transition between two contexts smoothly
	hoa_system::SystemTimer _timer;

	//! \brief Restores all class members to their default values
	void _RestoreDefaultSettings();
};

} // namespace private_map

} // namespace hoa_map

