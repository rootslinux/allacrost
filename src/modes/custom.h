///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2016 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    custom.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for custom game mode
*** **************************************************************************/

#pragma once

#include "defs.h"
#include "utils.h"

#include "mode_manager.h"
#include "script.h"

//! All code in custom mode are wrapped in this namespace
namespace hoa_custom {

/** ****************************************************************************
*** \brief A bare-bones mode that is almost completely implemented in a Lua script file
***
*** Custom modes are usually built for simple, short scenes such as displaying a background
*** graphic or text on a black screen. But as their name implies, custom modes are flexible
*** and can be designed to implement any sort of functionality desired. The scripts for custom
*** game mode implementations are usually found in lua/scripts/custom/.
***
*** \note There are several important things to keep in mind when implementing the Lua code for a custom mode:
*** - The video engine's coordinate system should be set appropriately on every call to Reset()
*** - User input needs to be handled in the Update() call. Otherwise expected changes such as quit and pause inputs by
***   the user will be ignored.
*** ***************************************************************************/
class CustomMode : public hoa_mode_manager::GameMode {
	friend void hoa_defs::BindModeCode();

public:
	//! \param script_filename The name of the Lua file that implements this custom mode
	CustomMode(const std::string& script_filename);

	~CustomMode();

	/** \brief Leaves the command descriptions to their default "(unused)"
	*** \note Because every custom mode is unique, it is up to the Lua implementation to properly set the command descriptions.
	**/
	void SetCommandDescriptions() {}

	//! \brief Executs the _reset_function. Called whenever CustomMode is made the active game mode
	void Reset();

	//! \brief Executes the _update_function to process game logic
	void Update();

	//! \brief Executes the _draw_function to draw elements to the screen
	void Draw();

	/** \brief Adds new option data to the custom mode instance
	*** \param option_key The string that identifies the option data
	*** \param option_value The value of the option
	***
	*** If an option with the key already exists, it will be overwritten. Note that if the option key doesn't match any of the options
	*** that the custom mode expects to see, it will have no effect on any data or operation of the mode. Options are typically added
	*** immediately after the class instance is created so that the Reset() function can load the option data and initialize the class properly.
	**/
	void AddOption(const std::string& option_key, const std::string& option_value)
		{ _options[option_key] = option_value; }

	/** \brief Retrieves existing option data
	*** \param option_key The string identifying the data to retrieve
	*** \return The option data string. If the option key does not exist, an empty string will be returned
	**/
	const std::string GetOption(const std::string& option_key) const;

private:
	/** \brief Initially false, this member will get set to true after the first call to Reset() is completed
	*** The purpose of this member is to make sure that data that needs to be loaded by the class is done so only once.
	*** It is not modified any other time, and is exposed in Lua as a read-only member.
	**/
	bool _load_complete;

	/** \brief A container of option strings that can be used to set data or define behavior of the custom mode script
	*** The addition of this container allows for better re-use of custom mode scripts. For example, a text string can be
	*** added as an option to allow the mode to display different text on the screen. The map key is used as a unique identifier
	*** for the option data. To know what option keys are available for a specific custom mode, you'll need to open up the Lua
	*** script file and see.
	**/
	std::map<std::string, std::string> _options;

	//! \brief The Lua file controlling this instance of CustomMode. The file remains open throughout the life of the class instance
	hoa_script::ReadScriptDescriptor _script_file;

	//! \brief A script function called whenever Reset() is invoked
	ScriptObject _reset_function;

	//! \brief A script function called whenever Update() is invoked
	ScriptObject _update_function;

	//! \brief A script function called whenever Draw() is invoked
	ScriptObject _draw_function;
}; // class CustomMode : public hoa_mode_manager::GameMode

} // namespace hoa_custom
