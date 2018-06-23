///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    script.cpp
*** \author  Daniel Steuernol (Steu)
***          Tyler Olsen  (Roots)
***
*** \brief   Source file for the scripting engine.
*** ***************************************************************************/

#include <iostream>
#include <stdarg.h>

#include "script.h"

using namespace std;
using namespace luabind;

using namespace hoa_utils;
using namespace hoa_script::private_script;

template<> hoa_script::ScriptEngine* Singleton<hoa_script::ScriptEngine>::_singleton_reference = nullptr;

namespace hoa_script {

ScriptEngine* ScriptManager = nullptr;
bool SCRIPT_DEBUG = false;


string DetermineLuaFileTablespaceName(const string& filename) {
	if (filename.empty()) {
		IF_PRINT_WARNING(SCRIPT_DEBUG) << "function received an empty string argument" << endl;
		return "";
	}

	string tablespace_name;
	int32 last_slash_index = filename.find_last_of("/");
	int32 period_index = filename.find(".");
	tablespace_name = filename.substr(last_slash_index + 1, period_index - (last_slash_index + 1));

	if (tablespace_name.empty()) {
		IF_PRINT_WARNING(SCRIPT_DEBUG) << "function received an unexpected filename string: " << filename << endl;
		return "";
	}

	// Lua identifiers must start with a letter. If the filename starts with a nonalphabetic character,
	// our convention is to prepend "a" to the tablespace name in that file
	char first = tablespace_name[0];
	if ((first < 'A' || first > 'Z') && (first < 'a' || first > 'z')) {
		tablespace_name = 'a' + tablespace_name;
	}
	return tablespace_name;
}

//-----------------------------------------------------------------------------
// ScriptEngine Class Functions
//-----------------------------------------------------------------------------

ScriptEngine::ScriptEngine() {
	IF_PRINT_DEBUG(SCRIPT_DEBUG) << "ScriptEngine constructor invoked." << endl;

	// Initialize Lua and LuaBind
	_global_state = lua_open();
	luaL_openlibs(_global_state);
	luabind::open(_global_state);
}



ScriptEngine::~ScriptEngine() {
	IF_PRINT_DEBUG(SCRIPT_DEBUG) << "ScriptEngine destructor invoked." << endl;

	_open_files.clear();
	lua_close(_global_state);
	_global_state = nullptr;
}



bool ScriptEngine::SingletonInitialize() {
	// TODO: Open the user setting's file and apply those settings
	return true;
}



bool ScriptEngine::IsFileOpen(const std::string& filename) {
	if (_open_files.find(filename) != _open_files.end()) {
		return true;
	}

	return false;
}




bool ScriptEngine::ExecuteLuaFunction(const string& filename, const string& function_name, bool open_tablespace) {
	ReadScriptDescriptor script;

	if (script.OpenFile(filename) == false) {
	    return false;
	}

	if (script.DoesFunctionExist(function_name) == false) {
		IF_PRINT_WARNING(SCRIPT_DEBUG) << "failed to find function \"" << function_name << "\" to execute in file: "
			<< filename << endl;
		script.CloseFile();
		return false;
	}

	if (open_tablespace == true) {
		if (script.OpenTablespace() == "") {
			IF_PRINT_WARNING(SCRIPT_DEBUG) << "failed to open tablespace in file: " << filename << endl;
		}
		script.CloseFile();
		return false;
	}

	bool result = script.ExecuteFunction(function_name);
	script.CloseFile();
	return result;
}



void ScriptEngine::HandleLuaError(luabind::error& err) {
	lua_State *state = err.state();
	PRINT_ERROR << "a runtime Lua error has occured with the following error message:\n  " << endl;
	std::string k = lua_tostring(state, lua_gettop(state)) ;
	cerr << k << endl;
	lua_pop(state, 1);
}



void ScriptEngine::HandleCastError(luabind::cast_failed& err) {
	PRINT_ERROR << "the return value of a Lua function call could not be successfully converted "
		<< "to the specified C++ type: " << err.what() << endl;
}



void ScriptEngine::_AddOpenFile(ScriptDescriptor* sd) {
	// NOTE: This function assumes that the file is not already open

	_open_files.insert(make_pair(sd->_filename, sd));
	// Add the lua_State to the list of opened lua states if it is not already present
	if (sd->GetAccessMode() == SCRIPT_READ || sd->GetAccessMode() == SCRIPT_MODIFY) {
		ReadScriptDescriptor* rsd = dynamic_cast<ReadScriptDescriptor*>(sd);
		if (_open_threads.find(rsd->GetFilename()) == _open_threads.end())
			_open_threads[rsd->GetFilename()] = rsd->_lstack;
	}
}



void ScriptEngine::_RemoveOpenFile(ScriptDescriptor* sd) {
	// NOTE: Function assumes that the ScriptDescriptor file is already open
	_open_files.erase(sd->_filename);
}



lua_State *ScriptEngine::_CheckForPreviousLuaState(const std::string &filename) {
	return nullptr; // TEMP, see todo notes in script.h

	if (_open_threads.find(filename) != _open_threads.end())
		return _open_threads[filename];
	else
		return nullptr;
}


} // namespace hoa_script
