///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for map mode interface.
*** ***************************************************************************/

// Allacrost engines
#include "audio.h"
#include "script.h"
#include "input.h"
#include "system.h"

// Allacrost globals
#include "common.h"
#include "global.h"

// Other mode headers
#include "menu.h"
#include "pause.h"

// Local map mode headers
#include "map.h"
#include "map_dialogue.h"
#include "map_events.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "map_tiles.h"
#include "map_transition.h"
#include "map_treasure.h"
#include "map_zones.h"
#include "map_utils.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_common;
using namespace hoa_audio;
using namespace hoa_input;
using namespace hoa_mode_manager;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_video;
using namespace hoa_global;
using namespace hoa_menu;
using namespace hoa_pause;
using namespace hoa_map::private_map;

namespace hoa_map {

// Initialize static class variables
MapMode* MapMode::_current_instance = nullptr;

// The maximum value of the run stamina bar
const uint32 RUN_STAMINA_MAX = 10000;

// ****************************************************************************
// ********** MapMode Public Class Methods
// ****************************************************************************

MapMode::MapMode(string script_filename, int32 load_point) :
	GameMode(MAP_MODE),
	_data_filename(""),
	_script_filename(script_filename),
	_script_tablespace(""),
	_load_point(load_point),
	_global_record_group(nullptr),
	_local_record_group("local_map"), // The group name doesn't really matter since this record group is volatile
	_dialogue_supervisor(new DialogueSupervisor()),
	_event_supervisor(new EventSupervisor()),
	_object_supervisor(new ObjectSupervisor()),
	_tile_supervisor(new TileSupervisor()),
	_transition_supervisor(new TransitionSupervisor()),
	_treasure_supervisor(new TreasureSupervisor()),
	_camera(nullptr),
	_player_sprite(nullptr),
	_delta_x(0),
	_delta_y(0),
	_num_map_contexts(0),
	_run_disabled(false),
	_run_state(false),
	_run_stamina(RUN_STAMINA_MAX),
	_unlimited_stamina(false),
	_dialogue_icons_visible(false),
	_stamina_bar_visible(false),
	_current_track(0xFFFFFFFF) // intentionally invalid track number
{
	_current_instance = this;
	SetCommandDescriptions();

	// Disable any active visual effects
	VideoManager->DisableLightOverlay();
	VideoManager->DisableAmbientOverlay();
	VideoManager->DisableLightning();

	ResetState();
	PushState(STATE_EXPLORE);

	// Creates a unique record group identifier string by using the script's tablespace name prefixed with "map_"
	string group_name = "map_" + DetermineLuaFileTablespaceName(_script_filename);
	if (GlobalManager->DoesRecordGroupExist(group_name) == false) {
		GlobalManager->AddNewRecordGroup(group_name);
	}
	_global_record_group = GlobalManager->GetRecordGroup(group_name);

	// The virtual focus is added to the default object layer of the object supervisor. This means that the
	// object supervisor will destroy the object when the map exits, so the MapMode destructor doesn't need
	// to delete it.
	_virtual_focus = new VirtualSprite();
	_virtual_focus->SetXPosition(0, 0.0f);
	_virtual_focus->SetYPosition(0, 0.0f);
	_virtual_focus->SetMovementSpeed(NORMAL_SPEED);
	_object_supervisor->AddObject(_virtual_focus, DEFAULT_LAYER_ID);

	_intro_timer.Initialize(7000, 0);
	_intro_timer.EnableAutoUpdate(this);

	// The camera must never be nullptr (otherwise the game will crash), so initially set it to the virtual focus
	_camera = _virtual_focus;
	_camera_timer.Initialize(0, 1);

	// TODO: Load the map files in a seperate thread
	_LoadMapFiles();

	// Load miscellaneous map graphics
	vector<uint32> timings(16, 100); // holds the timing data for the new dialogue animation; 16 frames at 100ms each
	_dialogue_icon.SetDimensions(2.0f, 2.0f);
	if (_dialogue_icon.LoadFromFrameSize("img/misc/dialogue_icon.png", timings, 32, 32) == false)
		IF_PRINT_WARNING(MAP_DEBUG) << "failed to load the new dialogue icon image" << endl;

	if (_stamina_bar_background.Load("img/misc/stamina_bar_background.png", 227, 24) == false)
		IF_PRINT_WARNING(MAP_DEBUG) << "failed to load the the stamina bar background image" << endl;

	if (_stamina_bar_infinite_overlay.Load("img/misc/stamina_bar_infinite_overlay.png", 227, 24) == false)
		IF_PRINT_WARNING(MAP_DEBUG) << "failed to load the the stamina bar infinite overlay image" << endl;
}



MapMode::~MapMode() {
	for (uint32 i = 0; i < _music.size(); i++)
		_music[i].FreeAudio();
	_music.clear();

	for (uint32 i = 0; i < _sounds.size(); i++)
		_sounds[i].FreeAudio();
	_sounds.clear();

	for (uint32 i = 0; i < _enemies.size(); i++)
		delete _enemies[i];
	_enemies.clear();

	delete _dialogue_supervisor;
	delete _event_supervisor;
	delete _object_supervisor;
	delete _tile_supervisor;
	delete _transition_supervisor;
	delete _treasure_supervisor;

	_map_script.CloseFile();
}



void MapMode::SetCommandDescriptions() {
	_command_descriptions[UP_COMMAND] = UTranslate("Move character; Move cursor");
	_command_descriptions[DOWN_COMMAND] = UTranslate("Move character; Move cursor");
	_command_descriptions[LEFT_COMMAND] = UTranslate("Move character; Move cursor");
	_command_descriptions[RIGHT_COMMAND] = UTranslate("Move character; Move cursor");
	_command_descriptions[CONFIRM_COMMAND] = UTranslate("Interact with surroundings; Select menu option");
	_command_descriptions[CANCEL_COMMAND] = UTranslate("Toggle run/walk; Return to previous menu");
	_command_descriptions[MENU_COMMAND] = UTranslate("Enter party menu");
}



void MapMode::Reset() {
	// Reset video engine context properties
	VideoManager->SetCoordSys(0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);

	// Set the active instance pointer to this map
	MapMode::_current_instance = this;

	// Make the map location known globally to other code that may need to know this information
	GlobalManager->SetLocation(_map_name, _location_graphic.GetFilename());
	GlobalManager->SetMapLocationFilename(_script_filename);

	if (_music.size() > _current_track && _music[_current_track].GetState() != AUDIO_STATE_PLAYING) {
		_music[_current_track].Play();
	}

	_intro_timer.Run();
}



void MapMode::Update() {
	// TODO: this is a workaround to make sure that when we switch between two game modes, the update code references the correct instance of this class.
	// This needs a better solution (perhaps not relying on a static instance pointer at all)
	if (_current_instance != this)
		_current_instance = this;

	_dialogue_icon.Update();

	// Process quit/pause/help events so long as we are not in the middle of a transition
	if (CurrentState() != STATE_TRANSITION) {
		if (InputManager->QuitPress() == true) {
			ModeManager->Push(new PauseMode(hoa_pause::QUIT));
			return;
		}
		else if (InputManager->PausePress() == true) {
			ModeManager->Push(new PauseMode(hoa_pause::PAUSE));
			return;
		}
		else if (InputManager->HelpPress() == true) {
			ModeManager->Push(new PauseMode(hoa_pause::HELP));
			return;
		}
	}

	// ---------- (1) Update all animated tile images and map objects
	_tile_supervisor->Update();
	_object_supervisor->Update();
	_object_supervisor->SortObjectLayers();

	// ---------- (2) Update the active state of the map
	switch (CurrentState()) {
		case STATE_EXPLORE:
			_UpdateExplore();
			break;
		case STATE_SCENE:
			break;
		case STATE_DIALOGUE:
			_dialogue_supervisor->Update();
			break;
		case STATE_TREASURE:
			_camera->SetMoving(false);
			_treasure_supervisor->Update();
			break;
        case STATE_TRANSITION:
            _transition_supervisor->Update();
            break;
		default:
			IF_PRINT_WARNING(MAP_DEBUG) << "map was in an unknown state: " << CurrentState() << endl;
			ResetState();
			break;
	}

	// ---------- (3) Update the timers
	if (_camera_timer.IsRunning())
		_camera_timer.Update();

	// ---------- (4) Call the map script's update function
	if (_update_function) {
		_map_script.ExecuteFunction(_update_function);
	}

	// ---------- (5) Update all active map events
	_event_supervisor->Update();
} // void MapMode::Update()



void MapMode::Draw() {
	// TODO: this is a workaround to make sure that when we switch between two game modes, the draw code references the correct instance of this class.
	// This needs a better solution (perhaps not relying on a static instance pointer at all)
	if (_current_instance != this)
		_current_instance = this;

	_CalculateMapFrame();

	if (_draw_function)
		ScriptCallFunction<void>(_draw_function);
	else
		_DrawMapLayers();

	if (VideoManager->DEBUG_IsGraphicsDebuggingEnabled() == true) {
		_object_supervisor->DEBUG_DrawCollisionGrid(GetCurrentContext());
		_object_supervisor->DEBUG_DrawZoneOutlines(GetCurrentContext());
	}

	VideoManager->DrawOverlays();

	_DrawGUI();

	if (CurrentState() == STATE_DIALOGUE) {
		_dialogue_supervisor->Draw();
	}
	else if (CurrentState() == STATE_TREASURE) {
		_treasure_supervisor->Draw();
	}
}



void MapMode::ResetState() {
	_state_stack.clear();
	_state_stack.push_back(STATE_INVALID);
	IF_PRINT_DEBUG(MAP_DEBUG) << "map changing to state: " << _state_stack.back() << endl;
}



void MapMode::PushState(MAP_STATE state) {
	_state_stack.push_back(state);
	IF_PRINT_DEBUG(MAP_DEBUG) << "changed map state to: " << DEBUG_MapStateName(state) << endl;
}



void MapMode::PopState() {
	_state_stack.pop_back();
	if (_state_stack.empty() == true) {
		IF_PRINT_WARNING(MAP_DEBUG) << "stack was empty after operation, reseting state stack" << endl;
		_state_stack.push_back(STATE_INVALID);
	}
	IF_PRINT_DEBUG(MAP_DEBUG) << "changed map state to: " << DEBUG_MapStateName(_state_stack.back()) << endl;
}



MAP_STATE MapMode::CurrentState() {
	if (_state_stack.empty() == true) {
		IF_PRINT_WARNING(MAP_DEBUG) << "stack was empty, reseting state stack" << endl;
		_state_stack.push_back(STATE_INVALID);
	}
	return _state_stack.back();
}



void MapMode::AddZone(MapZone *zone) {
	_object_supervisor->_zones.push_back(zone);
}



bool MapMode::IsEnemyLoaded(uint32 id) const {
	for (uint32 i = 0; i < _enemies.size(); i++) {
		if (_enemies[i]->GetID() == id) {
			return true;
		}
	}
	return false;
}



void MapMode::PlayMusic(uint32 track_num) {
	if (_current_track <= _music.size()) {
		_music[_current_track].Stop();
	}

	_current_track = track_num;
	_music[_current_track].Play();
}



void MapMode::SetCamera(VirtualSprite* sprite, uint32 duration) {
	if (_camera == sprite) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Camera was moved to the same sprite" << endl;
		return;
	}

	if (duration > 0) {
		_delta_x = _camera->ComputeXLocation() - sprite->ComputeXLocation();
		_delta_y = _camera->ComputeYLocation() - sprite->ComputeYLocation();
		_camera_timer.Reset();
		_camera_timer.SetDuration(duration);
		_camera_timer.Run();
	}
	_camera = sprite;
}



void MapMode::AddTileLayerToOrder(uint32 layer_id) {
	TileLayer* layer = _tile_supervisor->GetTileLayer(layer_id);
	if (layer == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "tried to add a tile layer with an invalid layer ID: " << layer_id << endl;
		return;
	}

	_layer_order.push_back(layer);
}



void MapMode::AddObjectLayerToOrder(uint32 layer_id) {
	ObjectLayer* layer = _object_supervisor->GetObjectLayer(layer_id);
	if (layer == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "tried to add an object layer with an invalid layer ID: " << layer_id << endl;
		return;
	}

	_layer_order.push_back(layer);
}



MAP_CONTEXT MapMode::GetCurrentContext() const {
	return _camera->GetContext();
}



void MapMode::MoveVirtualFocus(uint16 x, uint16 y) {
	_virtual_focus->SetXPosition(x, 0.0f);
	_virtual_focus->SetYPosition(y, 0.0f);
}



void MapMode::MoveVirtualFocus(uint16 x, uint16 y, uint32 duration) {
	if (_camera != _virtual_focus) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempt to move camera although on different sprite" << endl;
	}
	else {
		if (duration > 0) {
			_delta_x = _virtual_focus->ComputeXLocation() - static_cast<float>(x);
			_delta_y = _virtual_focus->ComputeYLocation() - static_cast<float>(y);
			_camera_timer.Reset();
			_camera_timer.SetDuration(duration);
			_camera_timer.Run();
		}
		MoveVirtualFocus(x, y);
	}
}

// ****************************************************************************
// ********** MapMode Private Class Methods
// ****************************************************************************

void MapMode::_LoadMapFiles() {
	IF_PRINT_DEBUG(MAP_DEBUG) << "Loading map script: " << _script_filename << endl;

	// ---------- (1) Open the map script file and read in the map data file name
	if (_map_script.OpenFile(_script_filename) == false) {
		PRINT_ERROR << "failed to open map script file: " << _script_filename << endl;
		return;
	}
	_script_tablespace = DetermineLuaFileTablespaceName(_script_filename);
	_map_script.OpenTable(_script_tablespace);
	_data_filename = _map_script.ReadString("data_file");

	// ---------- (2) Open the map data file and load its contents into the appropriate supervisor classes
	ReadScriptDescriptor map_data;
	if (map_data.OpenFile(_data_filename) == false) {
		PRINT_ERROR << "failed to open map data file: " << _data_filename << endl;
		return;
	}

	map_data.OpenTable(DetermineLuaFileTablespaceName(_data_filename));
	_num_map_contexts = map_data.ReadUInt("number_map_contexts");
	_tile_supervisor->Load(map_data);
	_object_supervisor->Load(map_data);
	map_data.CloseAllTables();
	map_data.CloseFile();

	// ---------- (3) Load all necessary content from the map script file
	// Read the map's location graphic and name
	if (_location_graphic.Load(_map_script.ReadString("location_filename")) == false) {
		PRINT_ERROR << "failed to load location graphic image: " << _location_graphic.GetFilename() << endl;
	}
	_map_name = MakeUnicodeString(_map_script.ReadString("map_name"));

	// Load map sounds and music
	vector<string> sound_filenames;
	_map_script.ReadStringVector("sound_filenames", sound_filenames);

	for (uint32 i = 0; i < sound_filenames.size(); i++) {
		_sounds.push_back(SoundDescriptor());
		if (_sounds.back().LoadAudio(sound_filenames[i]) == false) {
			PRINT_ERROR << "failed to load map sound: " << sound_filenames[i] << endl;
		}
	}

	vector<string> music_filenames;
	_map_script.ReadStringVector("music_filenames", music_filenames);
	_music.resize(music_filenames.size(), MusicDescriptor());
	for (uint32 i = 0; i < music_filenames.size(); i++) {
		if (_music[i].LoadAudio(music_filenames[i]) == false) {
			PRINT_ERROR << "failed to load map music: " << music_filenames[i] << endl;
		}
	}

	// Create all of the GlobalEnemy objects for any enemy that may appear on this map
	if (_map_script.DoesTableExist("enemy_ids") == true) {
		vector<int32> enemy_ids;
		_map_script.ReadIntVector("enemy_ids", enemy_ids);
		for (uint32 i = 0; i < enemy_ids.size(); i++) {
			_enemies.push_back(new GlobalEnemy(enemy_ids[i]));
		}
	}

	// ---------- (4) Call the map script's Load function and get a reference to all other script functions used
	ScriptObject map_table(luabind::from_stack(_map_script.GetLuaState(), hoa_script::private_script::STACK_TOP));
	ScriptObject function = map_table["Load"];
	try {
	    ScriptCallFunction<void>(function, this);
	}
	// TODO: this catch block isn't catching the exceptions thrown from Lua.
	catch (luabind::error e) {
		PRINT_ERROR << "caught Luabind error while loading map script" << endl;
		ScriptManager->HandleLuaError(e);
	}

	_update_function = _map_script.ReadFunctionPointer("Update");
	_draw_function = _map_script.ReadFunctionPointer("Draw");

	// ---------- (5) Prepare all sprite dialogues
	// This is done at this stage because the map script's load function creates the sprite and dialogue objects. Only after
	// both sets are created can we determine which sprites have active dialogue.

	// TODO: Need to figure out a new function appropriate for this code?
	// TEMP: The line below is very bad to do, but is necessary for the UpdateDialogueStatus function to work correctly
	_current_instance = this;
	for (map<uint16, MapObject*>::iterator i = _object_supervisor->_all_objects.begin(); i != _object_supervisor->_all_objects.end(); i++) {
		if (i->second->GetType() == SPRITE_TYPE) {
			MapSprite* sprite = dynamic_cast<MapSprite*>(i->second);
			sprite->UpdateDialogueStatus();
		}
	}

	_map_script.CloseAllTables();
}



void MapMode::_UpdateExplore() {
	// First go to menu mode if the user requested it
	if (InputManager->MenuPress()) {
		MenuMode *MM = new MenuMode();
		ModeManager->Push(MM);
		return;
	}

	// If the user requested a confirm event, check if there is a nearby object that the player may interact with
	// Interactions are currently limited to dialogue with sprites and opening of treasures
	if (InputManager->ConfirmPress()) {
		MapObject* obj = _object_supervisor->FindNearestObject(_camera);

		if (obj != nullptr) {
			if (obj->GetType() == SPRITE_TYPE) {
				MapSprite *sp = reinterpret_cast<MapSprite*>(obj);

				if (sp->HasAvailableDialogue()) {
					_camera->SetMoving(false);
					_camera->SetRunning(false);
					sp->InitiateDialogue();
					return;
				}
			}
			else if (obj->GetType() == MAP_TREASURE_TYPE) {
				MapTreasure* treasure = reinterpret_cast<MapTreasure*>(obj);

				if (treasure->GetTreasureContainer()->IsTaken() == false) {
				    _camera->SetMoving(false);
					treasure->Open();
				}
			}
			else if (obj->GetType() == GLIMMER_TREASURE_TYPE) {
				GlimmerTreasure* treasure = reinterpret_cast<GlimmerTreasure*>(obj);

				if (treasure->GetTreasureContainer()->IsTaken() == false) {
				    _camera->SetMoving(false);
					treasure->Acquire();
				}
			}
		}
	}

	// Check if the player has toggled the run state
	if (_run_disabled == false && InputManager->CancelPress() == true) {
		if (_run_state == true) {
			_run_state = false;
		}
		// Only enable the run state if we have at least a little bit of stamina
		else if (_run_stamina > 0) {
			_run_state = true;
		}
	}

	// Detect movement input from the user and update the stamina counter and run state appropriately
	if (InputManager->UpState() || InputManager->DownState() || InputManager->LeftState() || InputManager->RightState()) {
		_camera->SetMoving(true);
		_camera->SetRunning(_run_state);

		// Regenerate the stamina at 1/4 the consumption rate if the user is walking
		if (_run_state == false && _run_stamina < RUN_STAMINA_MAX) {
			_run_stamina += SystemManager->GetUpdateTime() / 2;
			if (_run_stamina > RUN_STAMINA_MAX)
				_run_stamina = RUN_STAMINA_MAX;
		}
		// Deplete stamina by the appropriate amount if the player is running and stamina is not unlimited
		if (_unlimited_stamina == false && _run_state == true) {
			uint32 deplete_amount = SystemManager->GetUpdateTime() * 2;
			if (_run_stamina > deplete_amount) {
				_run_stamina -= deplete_amount;
			}
			// When stamina is fully depleted, toggle off the run state
			else {
				_run_stamina = 0;
				_run_state = false;
			}
		}
	}
	else { // User is not moving
		_camera->SetMoving(false);
		_camera->SetRunning(false);

		// Regenerate the stamina at 1/2 the consumption rate
		if (_run_stamina < RUN_STAMINA_MAX) {
			_run_stamina += SystemManager->GetUpdateTime();
			if (_run_stamina > RUN_STAMINA_MAX)
				_run_stamina = RUN_STAMINA_MAX;
		}
	}

	// Determine the direction of movement. Priority of movement is given to: up, down, left, right.
	// In the case of diagonal movement, the direction that the sprite should face also needs to be deduced.
	if (_camera->IsMoving() == true) {
		if (InputManager->UpState())
		{
			if (InputManager->LeftState())
				_camera->SetDirection(MOVING_NORTHWEST);
			else if (InputManager->RightState())
				_camera->SetDirection(MOVING_NORTHEAST);
			else
				_camera->SetDirection(NORTH);
		}
		else if (InputManager->DownState())
		{
			if (InputManager->LeftState())
				_camera->SetDirection(MOVING_SOUTHWEST);
			else if (InputManager->RightState())
				_camera->SetDirection(MOVING_SOUTHEAST);
			else
				_camera->SetDirection(SOUTH);
		}
		else if (InputManager->LeftState()) {
			_camera->SetDirection(WEST);
		}
		else if (InputManager->RightState()) {
			_camera->SetDirection(EAST);
		}
	} // if (_camera->IsMoving() == true)
} // void MapMode::_UpdateExplore()



void MapMode::_CalculateMapFrame() {
	// ---------- (1) Determine the center position coordinates for the camera
	float camera_x, camera_y; // Holds the final X, Y coordinates of the camera
	float x_pixel_length, y_pixel_length; // The X and Y length values that coorespond to a single pixel in the current coodinate system
	float rounded_x_offset, rounded_y_offset; // The X and Y position offsets of the camera, rounded to perfectly align on a pixel boundary

	uint16 current_x, current_y; // Actual position of the view, either the camera sprite or a point on the camera movement path
	float current_offset_x, current_offset_y; // Actual offset for the view

	// TODO: the call to GetPixelSize() will return the same result every time so long as the coordinate system did not change. If we never
	// change the coordinate system in map mode, then this should be done only once and the calculated values should be saved for re-use.
	// However, we've discussed the possiblity of adding a zoom feature to maps, in which case we need to continually re-calculate the pixel size
	VideoManager->GetPixelSize(x_pixel_length, y_pixel_length);

	if (_camera_timer.IsRunning() == false) {
	    _camera->GetXPosition(current_x, current_offset_x);
	    _camera->GetYPosition(current_y, current_offset_y);
	}
	else {
	    // Calculate path
	    float path_x = _camera->ComputeXLocation() + (1 - _camera_timer.PercentComplete()) * _delta_x;
	    float path_y = _camera->ComputeYLocation() + (1 - _camera_timer.PercentComplete()) * _delta_y;

	    current_x = GetFloatInteger(path_x);
	    current_y = GetFloatInteger(path_y);

	    current_offset_x = GetFloatFraction(path_x);
	    current_offset_y = GetFloatFraction(path_y);
	}

	rounded_x_offset = FloorToFloatMultiple(current_offset_x, x_pixel_length);
	rounded_y_offset = FloorToFloatMultiple(current_offset_y, y_pixel_length);
	camera_x = static_cast<float>(current_x) + rounded_x_offset;
	camera_y = static_cast<float>(current_y) + rounded_y_offset;

	// ---------- (2) Calculate all four screen edges and determine
	// Determine the draw coordinates of the top left corner using the camera's current position
	_map_frame.tile_x_start = 1.0f - rounded_x_offset;
	if (IsOddNumber(current_x))
		_map_frame.tile_x_start -= 1.0f;

	_map_frame.tile_y_start = 2.0f - rounded_y_offset;
	if (IsOddNumber(current_y))
		_map_frame.tile_y_start -= 1.0f;

	// The starting row and column of tiles to draw is determined by the map camera's position
	_map_frame.starting_col = (current_x / 2) - HALF_TILE_COLS;
	_map_frame.starting_row = (current_y / 2) - HALF_TILE_ROWS;

	_map_frame.screen_edges.top    = camera_y - HALF_SCREEN_ROWS;
	_map_frame.screen_edges.bottom = camera_y + HALF_SCREEN_ROWS;
	_map_frame.screen_edges.left   = camera_x - HALF_SCREEN_COLS;
	_map_frame.screen_edges.right  = camera_x + HALF_SCREEN_COLS;

	// ---------- (3) Check for boundary conditions and re-adjust as necessary so we don't draw outside the map area

	// Usually the map centers on the camera's position, but when the camera becomes too close to
	// the edges of the map, we need to modify the drawing properties of the frame.

	// Camera exceeds the left boundary of the map
	if (_map_frame.starting_col < 0) {
		_map_frame.starting_col = 0;
		_map_frame.tile_x_start = 1.0f;
		_map_frame.screen_edges.left = 0.0f;
		_map_frame.screen_edges.right = SCREEN_COLS;
	}
	// Camera exceeds the right boundary of the map
	else if (_map_frame.starting_col + TILE_COLS >= _tile_supervisor->GetColumnCount()) {
		_map_frame.starting_col = static_cast<int16>(_tile_supervisor->GetColumnCount() - TILE_COLS);
		_map_frame.tile_x_start = 1.0f;
		_map_frame.screen_edges.right = static_cast<float>(_object_supervisor->_num_grid_cols);
		_map_frame.screen_edges.left = _map_frame.screen_edges.right - SCREEN_COLS;
	}

	// Camera exceeds the top boundary of the map
	if (_map_frame.starting_row < 0) {
		_map_frame.starting_row = 0;
		_map_frame.tile_y_start = 2.0f;
		_map_frame.screen_edges.top = 0.0f;
		_map_frame.screen_edges.bottom = SCREEN_ROWS;
	}
	// Camera exceeds the bottom boundary of the map
	else if (_map_frame.starting_row + TILE_ROWS >= _tile_supervisor->GetRowCount()) {
		_map_frame.starting_row = static_cast<int16>(_tile_supervisor->GetRowCount() - TILE_ROWS);
		_map_frame.tile_y_start = 2.0f;
		_map_frame.screen_edges.bottom = static_cast<float>(_object_supervisor->_num_grid_rows);
		_map_frame.screen_edges.top = _map_frame.screen_edges.bottom - SCREEN_ROWS;
	}

	// ---------- (4) Determine the number of rows and columns of tiles that need to be drawn

	// When the tile images align perfectly with the screen, we can afford to draw one less row or column of tiles
	if (IsFloatInRange(_map_frame.tile_x_start, 0.999f, 1.001f)) {
		_map_frame.num_draw_cols = TILE_COLS;
	}
	else {
		_map_frame.num_draw_cols = TILE_COLS + 1;
	}
	if (IsFloatInRange(_map_frame.tile_y_start, 1.999f, 2.001f)) {
		_map_frame.num_draw_rows = TILE_ROWS;
	}
	else {
		_map_frame.num_draw_rows = TILE_ROWS + 1;
	}

	// Comment this out to print out map draw debugging info about once a second
// 	static int loops = 0;
// 	if (loops == 0) {
// 		printf("--- MAP DRAW INFO ---\n");
// 		printf("Pixel Size:        [%f, %f]\n", x_pixel_length, y_pixel_length);
// 		printf("Rounded offsets:   [%f, %f]\n", rounded_x_offset, rounded_y_offset);
// 		printf("Starting row, col: [%d, %d]\n", _map_frame.starting_row, _map_frame.starting_col);
// 		printf("# draw rows, cols: [%d, %d]\n", _map_frame.num_draw_rows, _map_frame.num_draw_cols);
// 		printf("Camera position:   [%f, %f]\n", camera_x, camera_y);
// 		printf("Tile draw start:   [%f, %f]\n", _map_frame.tile_x_start, _map_frame.tile_y_start);
// 		printf("Edges (T,B,L,R):   [%f, %f, %f, %f]\n", _map_frame.screen_edges.top, _map_frame.screen_edges.bottom,
// 			_map_frame.screen_edges.left, _map_frame.screen_edges.right);
// 	}
//
// 	if (loops >= 60) {
// 		loops = 0;
// 	}
// 	else {
// 		loops++;
// 	}
} // void MapMode::_CalculateMapFrame()



void MapMode::_DrawMapLayers() {
 	VideoManager->SetCoordSys(0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f);

	// TODO: blend context transitions need to be supported here, which will require drawing both previous and current
	// contexts and applying alpha to blend the two together

	for (uint32 i = 0; i < _layer_order.size(); ++i) {
		_layer_order[i]->Draw(GetCurrentContext());
	}
}



void MapMode::_DrawGUI() {
	// TODO: figure out what this color represents and create an approximate name for it
	const Color unknown(0.0196f, 0.207f, 0.0196f, 1.0f);
	const Color lighter_green(0.419f, 0.894f, 0.0f, 1.0f);
	const Color light_green(0.0196f, 0.207f, 0.0196f, 1.0f);
	const Color medium_green(0.0509f, 0.556f, 0.0509f, 1.0f);
	const Color darkish_green(0.352f, 0.4f, 0.352f, 1.0f);
	const Color dark_green(0.0196f, 0.207f, 0.0196f, 1.0f);
	const Color bright_yellow(0.937f, 1.0f, 0.725f, 1.0f);

	// ---------- (1) Draw dialog icons above each sprite that meets the visiblity criteria
	// We don't draw the icons when we are doing a transition because it doesn't look nice
	if (IsDialogueIconsVisible() == true && CurrentState() != STATE_TRANSITION)
		_object_supervisor->DrawDialogIcons();

	// ---------- (2) Draw the introductory location name and graphic if necessary
	VideoManager->PushState();
	VideoManager->SetCoordSys(0.0f, 1024.0f, 768.0f, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);

	if (_intro_timer.IsFinished() == false) {
		uint32 time = _intro_timer.GetTimeExpired();

		Color blend(1.0f, 1.0f, 1.0f, 1.0f);
		if (time < 2000) { // Fade in
			blend.SetAlpha((static_cast<float>(time) / 2000.0f));
		}
		else if (time > 5000) { // Fade out
			blend.SetAlpha(1.0f - static_cast<float>(time - 5000) / 2000.0f);
		}

		VideoManager->Move(512.0f, 100.0f);
		_location_graphic.Draw(blend);
		VideoManager->MoveRelative(0.0f, -80.0f);
		VideoManager->Text()->Draw(_map_name, TextStyle("title24", blend, VIDEO_TEXT_SHADOW_DARK));
	}

	// ---------- (3) Draw the stamina bar in the lower right corner
	if (_stamina_bar_visible == true) {
		// TODO: the code in this section needs better comments to explain what each coloring step is doing
		float fill_size = static_cast<float>(_run_stamina) / RUN_STAMINA_MAX;

		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);

		// Draw the background image
		VideoManager->Move(780, 747);
		_stamina_bar_background.Draw();
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);

		// Draw the base color of the bar
		VideoManager->Move(800, 740);
		VideoManager->DrawRectangle(200 * fill_size, 10, unknown);

		// Shade the bar with a faux lighting effect
		VideoManager->Move(800, 739);
		VideoManager->DrawRectangle(200 * fill_size, 2, dark_green);
		VideoManager->Move(800, 737);
		VideoManager->DrawRectangle(200 * fill_size, 7, darkish_green);

		// Only do this if the bar is at least 4 pixels long
		if ((200 * fill_size) >= 4) {
			VideoManager->Move(801, 739);
			VideoManager->DrawRectangle((200 * fill_size) -2, 1, darkish_green);

			VideoManager->Move(801, 738);
			VideoManager->DrawRectangle(1, 2, medium_green);
			VideoManager->Move(800 + (fill_size * 200 - 2), 738); // Automatically reposition to be at moving endcap
			VideoManager->DrawRectangle(1, 2, medium_green);
		}

		VideoManager->Move(800, 736);
		VideoManager->DrawRectangle(200 * fill_size, 5, medium_green);

		// Only do this if the bar is at least 4 pixels long
		if ((200 * fill_size) >= 4) {
			VideoManager->Move(801, 735);
			VideoManager->DrawRectangle(1, 1, lighter_green);
			VideoManager->Move(800 + (fill_size * 200 - 2), 735); // automatically reposition to be at moving endcap
			VideoManager->DrawRectangle(1, 1, lighter_green);
			VideoManager->Move(800, 734);
			VideoManager->DrawRectangle(200 * fill_size, 2, lighter_green);
		}

		// Only do this if the bar is at least 6 pixels long
		if ((200 * fill_size) >= 6) {
			VideoManager->Move(802, 733);
			VideoManager->DrawRectangle((200 * fill_size) - 4, 1, bright_yellow);
		}

		if (_unlimited_stamina) { // Draw the infinity symbol over the stamina bar
			VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
			VideoManager->Move(780, 747);
			_stamina_bar_infinite_overlay.Draw();
		}
	} // if (_stamina_bar_visible == true)

	VideoManager->PopState();
} // void MapMode::_DrawGUI()

} // namespace hoa_map
