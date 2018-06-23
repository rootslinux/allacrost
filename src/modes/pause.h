////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    pause.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for pause mode interface.
*** ***************************************************************************/

#pragma once

#include "defs.h"
#include "utils.h"

#include "mode_manager.h"
#include "video.h"

#include "gui.h"


//! \brief All calls to pause mode are wrapped in this namespace.
namespace hoa_pause {

//! \brief Determines whether the code in the hoa_pause namespace should print debug statements or not.
extern bool PAUSE_DEBUG;

//! \brief Represents the three possible "pause" states for PauseMode
enum PAUSE_STATE {
	PAUSE  = 1,
	HELP   = 2,
	QUIT   = 3
};


/** ****************************************************************************
*** \brief Handles the game operation after a pause, quit, or help request from the player
***
*** This mode is used for handling three different game states, all of which effectively pause the normal
*** operation of the game. None of these states have visual elements that consume the entire screen state,
*** so a screenshot of the game is taken and used as a background just prior to entering the paused state.
*** The three possible states for this mode are "pause", "help", and "quit". Note that it is possible for a
*** user to change between these states. For example, while in the "help" state if a user attempts to quit
*** the game, they will change to the quit state. This means that there should only ever be a single instance
*** of PauseMode on the game stack.
***
*** Pause State:
*** Simply displays "Paused" in the center of the screen.
***
*** Help State:
*** Displays player help information relevant to the mode that was at the top of the stack when the
*** instance of this class was created. This information is a mapping of all controls on the keyboard and
*** what each of those controls does in this mode. This information is provided by each mode implementation
*** in a static method.
***
*** Quit State:
*** Presents the user with three options that they can select from.
*** - "Quit Game" exits the application completely.
*** - "Quit to Main Menu" completely empties the game stack and returns the user to boot mode
*** - "Cancel" unpauses the game and pops this PauseMode instance from the game stack.
***
*** \note When the user enters this mode, the game will sleep for small periods
*** of time so that the application isn't using up 100% of the CPU.
***
*** \note If the user inputs another quit event when this mode is active and in the
*** quit state, the game will exit immediately. If the user inputs a quit event
*** when the quit state is not active, this will activate the quite state.
*** ***************************************************************************/
class PauseMode : public hoa_mode_manager::GameMode {
public:
	/** \brief The class constructor determines the state and settings that PauseMode should be created in
	*** \param state The initial state to set pause mode to
	*** \param pause_audio If set to true, the audio is paused when PauseMode becomes active and resumes when it exits (default == false)
	**/
	PauseMode(PAUSE_STATE state, bool pause_audio = false);

	~PauseMode();

	/** \brief Leaves command descriptions to their default "(unused)"
	*** Instead of setting descriptions for this mode, the descriptions are set to the mode that was at the top of the stack when this mode was constructed
	**/
	void SetCommandDescriptions() {}

	//! \brief Resets appropriate class members. Called whenever PauseMode is made the active game mode.
	void Reset();

	//! \brief Updates the game state by the amount of time that has elapsed
	void Update();

	//! \brief Draws the next frame to be displayed on the screen
	void Draw();

private:
	//! \brief The state that this mode is currently in
	PAUSE_STATE _state;

	//! \brief Set to true if the audio should be resumed when this mode finishes
	bool _audio_paused;

	//! \brief Holds the type of game mode that was at the top of the game stack when the instance of this mode was created
	hoa_mode_manager::GAME_MODE_TYPE _parent_mode_type;

	//! \brief A screen capture of the last frame rendered on the screen before PauseMode was invoked
	hoa_video::StillImage _screen_capture;

	//! \brief A color used to dim the background screen capture image
	hoa_video::Color _dim_color;

	//! \brief "PAUSED" rendered as a text image texture
	hoa_video::TextImage _paused_text;

	//! \brief The list of selectabled quit options presented to the user while the mode is in the quit state
	hoa_gui::OptionBox _quit_options;

	//! \brief The GUI window holding all of the help content
	hoa_gui::MenuWindow _help_window;

	//! \brief Header for identifying the columns in the list of commands
	hoa_gui::OptionBox _help_commands_header;

	//! \brief Contains the name, key, and description of all possible player commands
	hoa_gui::OptionBox _help_commands;

	//! \brief A line of text explaining how to return to the game
	hoa_video::TextImage _help_return_text;
}; // class PauseMode : public hoa_mode_manager::GameMode

} // namespace hoa_pause
