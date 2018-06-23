///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2016 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    custom.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for custom mode code
*** **************************************************************************/

#include "custom.h"

#include "mode_manager.h"
#include "script.h"

using namespace std;
using namespace hoa_utils;

using namespace hoa_mode_manager;
using namespace hoa_script;

namespace hoa_custom {

CustomMode::CustomMode(const std::string& script_filename) :
	GameMode(CUSTOM_MODE),
	_load_complete(false),
	_options()
{
	if (_script_file.OpenFile(script_filename) == false) {
		PRINT_ERROR << "Failed to open custom mode script file: " << script_filename << endl;
		return;
	}
	std::string tablespace = DetermineLuaFileTablespaceName(script_filename);
	_script_file.OpenTable(tablespace);
	_reset_function = _script_file.ReadFunctionPointer("Reset");
	_update_function = _script_file.ReadFunctionPointer("Update");
	_draw_function = _script_file.ReadFunctionPointer("Draw");
	_script_file.CloseTable();
}



CustomMode::~CustomMode() {
	_script_file.CloseFile();
}



void CustomMode::Reset() {
	// A pointer to the class instance is passed in to the reset function so that the Lua script can access the members and methods
	ScriptCallFunction<void>(_reset_function, this);
	_load_complete = true;
}



void CustomMode::Update() {
	_script_file.ExecuteFunction(_update_function);}



void CustomMode::Draw() {
	_script_file.ExecuteFunction(_draw_function);
}



const std::string CustomMode::GetOption(const std::string& option_key) const {
	std::map<std::string, std::string>::const_iterator option = _options.find(option_key);
	if (option != _options.end())
		return option->second;
	else
		return "";
}

} // namespace hoa_custom
