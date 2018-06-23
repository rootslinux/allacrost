///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file   mode_manager.h
*** \author Tyler Olsen (Roots)
*** \brief  Header file for game mode processing
*** **************************************************************************/

#pragma once

#include "utils.h"
#include "defs.h"

#include "input.h"
#include "system.h"

//! \brief All calls to the mode management code are wrapped inside this namespace
namespace hoa_mode_manager {

//! \brief The singleton pointer responsible for maintaining and updating the game mode state.
extern ModeEngine* ModeManager;

//! \brief Determines whether the code in the hoa_mode_manager namespace should print debug statements or not.
extern bool MODE_MANAGER_DEBUG;

/** \brief Enumerate types of game modes
***
*** Each of these values correspond to a specific game mode class, except for INVALID_MODE.
**/
enum GAME_MODE_TYPE {
	INVALID_MODE  = 0,
	BATTLE_MODE   = 1,
	BOOT_MODE     = 2,
	CUSTOM_MODE   = 3,
	MAP_MODE      = 4,
	MENU_MODE     = 5,
	PAUSE_MODE    = 6,
	SAVE_MODE     = 7,
	SCENE_MODE    = 8,
	SHOP_MODE     = 9,
	TEST_MODE     = 10,
	WORLD_MODE    = 11,
	TOTAL_MODE    = 12
};


/** ***************************************************************************
*** \brief An abstract class that all game mode classes inherit from.
***
*** The GameMode class is the mandatory base class for a mode of operation in the game. The ModeEngine class manages all of the GameMode class
*** objects. Although this class contains nothing more than a single enum identifiying the type of game mode, derived classes have a plethora
*** of data that they manage. As such, copy, move, and delete operators are either removed or privatized.
***
*** \note This is \b important. Never, under any circumstances, should you ever invoke the delete function on a pointer to this object or its
*** related subclasses. The reason is that all of the memory reference handling is done by the ModeEngine class. If you attempt to ignore this
*** warning you \b will generate a segmentation fault.
*** **************************************************************************/
class GameMode {
	friend class ModeEngine;

public:
	virtual ~GameMode() {}

	GAME_MODE_TYPE GetModeType() const
		{ return mode_type; }

	//! \brief Returns a copy of all of the game mode's command descriptions
	const std::vector<hoa_utils::ustring> GetCommandDescriptions() const
		{ return _command_descriptions; }

	/** \brief Sets the text stored in the _command_descriptions array
	***
	*** This function should only be called once within the inheriting class constructor. This pure virtual function exists to require creators
	*** of game modes to remember to set these values. The function should not change the size of _command_descriptions.
	**/
	virtual void SetCommandDescriptions() = 0;

	/** \brief Resets the state of the class.
	***
	*** This function is called whenever the game mode is made active (ie, it is made the new active game mode
	*** on the top of the game modestack). This includes when the game mode is first created and pushed onto the
	*** game stack, so in that manner it can also be viewed as a helper function to the constructor.
	**/
	virtual void Reset() = 0;

	//! \brief Updates the state of the game mode.
	virtual void Update() = 0;

	//! \brief Draws the next screen frame for the game mode.
	virtual void Draw() = 0;

protected:
	GameMode() : GameMode(INVALID_MODE) {}

	//! \param type The mode_type to set the new GameMode object to.
	GameMode(GAME_MODE_TYPE type) : mode_type(type), _command_descriptions(hoa_input::COMMAND_TOTAL - 1, hoa_system::UTranslate("(unused)")) {}

	GameMode(const GameMode& other) = delete;

	GameMode& operator=(const GameMode& other) = delete;

	GameMode& operator=(GameMode&&) = delete;

	//! \brief Indicates what 'mode' this object is in (what type of inherited class).
	GAME_MODE_TYPE mode_type;

	/** \brief Holds the translated descriptions of input commands specific to the game mode
	***
	*** The primarily use for this data is for the player help screen popup that can be displayed in PauseMode. The constructor for this class will
	*** initialize all of the descriptions here to "(unused)" so if a particular inheriting mode does not use a command, they do not need to provide
	*** a description.
	***
	*** \note The size of this vector is the total number of commands - 1 because we do not provide a description for the "pause" command, which
	*** should have consistent behavior regardless of the mode that it is being used in. The pause command can have a custom key mapping, but not
	*** a custom description.
	**/
	std::vector<hoa_utils::ustring> _command_descriptions;
}; // class GameMode


/** ***************************************************************************
*** \brief Manages and maintains all of the living game mode objects.
***
*** The ModeEngine class keeps a stack of GameMode objects, where the object on the top of the stack is the active GameMode.
*** There can only be one active game mode at any time. The Update() and Draw() functions for this class are wrapper calls
*** to the GameMode functions of the same name, and are invoked on the active game mode.
***
*** When a condition is encountered in which a game mode wishes to destroy itself and/or push a new mode onto the stack,
*** this does not occur until the next call to the ModeEngine#Update() function. The GameModeManager#push_stack retains
*** all the game modes we wish to push onto the stack on the next call to ModeEngine#Update(), and the
*** GameModeManager#pop_count member retains how many modes to delete and pop off the ModeEngine#game_stack. Pop operations
*** are always performed before push operations.
***
*** \note You might be wondering why the game stack uses a vector container rather than a stack container.
*** There are two reasons: the first being that we can't do a debug printout of the game_stack without
*** removing elements if a stack is used. The second reason is "just in case" we need to access a stack
*** element that is not on the top of the stack.
*** **************************************************************************/
class ModeEngine : public hoa_utils::Singleton<ModeEngine> {
	friend class hoa_utils::Singleton<ModeEngine>;

public:
	~ModeEngine();

	bool SingletonInitialize();

	//! \brief Increments by one the number of game modes to pop off the stack
	void Pop();

	/** \brief Removes all game modes from the stack on the next call to ModeEngine#Update().
	***
	*** This function sets the ModeEngine#pop_count member to the size of GameModeManager#game_stack.
	*** If there is no game mode in ModeEngine#push_stack before the next call to GameModeManager#Update(),
	*** The game will encounter a segmentation fault and die. Therefore, be careful with this function.
	***
	*** \note Typically this function is only used when the game exits, or when a programmer is smoking crack.
	**/
	void PopAll();

	/** \brief Pushes a new GameMode object on top of the stack.
	*** \param new_mode The new GameMode object that will go to the top of the stack.
	*** \note This should be obvious, but once you push a new object on the stack
	*** top, it will automatically become the new active game state.
	**/
	void Push(GameMode* new_mode);

	//! \brief Returns the number of game modes that are currently on the stack
	uint32 GetModeStackSize() const
		{ return _game_stack.size(); }

	/** \brief Gets the type of the currently active game mode.
	*** \return The value of the mode_type member of the GameMode object on the top of the stack.
	**/
	GAME_MODE_TYPE GetModeType();

	/** \brief Gets the type of a game mode in the stack.
	*** \return The value of the mode_type member of the GameMode object on the top of the stack.
	**/
	GAME_MODE_TYPE GetModeType(uint32 index);

	/** \brief Gets a pointer to the top game stack object.
	*** \return A pointer to the GameMode object on the top of the stack.
	**/
	GameMode* GetTop();

	/** \brief Gets a pointer to a game stack object.
	*** \return A pointer to the GameMode object at (index) from the top.
	**/
	GameMode* GetMode(uint32 index);

	/** \brief Examines the game mode stack to find if the stack contains any mode instances of the specified type
	*** \param type The type of game mode to search for in the stack
	*** \return True if one or more instances of the mode type are found, or false otherwise
	**/
	bool IsModeTypeInStack(GAME_MODE_TYPE type);

	//! \brief Checks if the game stack needs modes pushed or popped, then calls Update on the active game mode.
	void Update();

	//! \brief Calls the Draw() function on the active game mode.
	void Draw();

	//! \brief Prints the contents of the game_stack member to standard output.
	void DEBUG_PrintStack();

	//! \brief Returns true if the game mode should display any graphical debugging information on the screen
	bool DEBUG_IsGraphicsEnabled() const
		{ return _debug_graphics_enabled; }

	//! \brief Toggles the state of game mode graphical debugging
	void DEBUG_ToggleGraphicsEnabled()
		{ _debug_graphics_enabled = ~_debug_graphics_enabled; }

	//! \brief Sets game mode graphical debugging on or off
	void DEBUG_SetGraphicsEnabled(bool debug)
		{ _debug_graphics_enabled = debug; }

private:
	ModeEngine();

	/** \brief A stack containing all of the active game modes.
	***
	*** The back/last element of the vector is the top of the stack.
	**/
	std::vector<GameMode*> _game_stack;

	//! \brief A vector of game modes to push to the stack on the next call to ModeEngine#Update().
	std::vector<GameMode*> _push_stack;

	//! \brief The number of game modes to pop from the back of the stack on the next call to ModeEngine#Update().
	uint32 _pop_count;

	//! \brief True if a state change occured and we need to change the active game mode.
	bool _state_change;

	//! \brief Set to true if game modes should draw graphical debugging information
	bool _debug_graphics_enabled;
}; // class ModeEngine : public hoa_utils::Singleton<ModeEngine>

} // namespace hoa_mode_manager
