///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_events.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for map mode events and event processing.
*** ***************************************************************************/

// Allacrost engines
#include "audio.h"
#include "mode_manager.h"
#include "script.h"
#include "system.h"
#include "video.h"

// Local map mode headers
#include "map.h"
#include "map_events.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "map_transition.h"

// Other mode headers
#include "shop.h"
#include "battle.h"

using namespace std;

using namespace hoa_audio;
using namespace hoa_mode_manager;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_video;

using namespace hoa_battle;
using namespace hoa_shop;

namespace hoa_map {

namespace private_map {

// -----------------------------------------------------------------------------
// ---------- MapEvent Class Methods
// -----------------------------------------------------------------------------

void MapEvent::_AddRecord(const std::string& record_name, int32 record_value, bool is_global) {
	if (_event_records == nullptr) {
		_event_records = new MapRecordData();
	}

	if (is_global == true)
		_event_records->AddGlobalRecord(record_name, record_value);
	else
		_event_records->AddLocalRecord(record_name, record_value);
}

// -----------------------------------------------------------------------------
// ---------- PushMapStateEvent Class Methods
// -----------------------------------------------------------------------------

PushMapStateEvent* PushMapStateEvent::Create(uint32 event_id, MAP_STATE state) {
	PushMapStateEvent* event = new PushMapStateEvent(event_id, state);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}



void PushMapStateEvent::_Start() {
	MapMode::CurrentInstance()->PushState(_state);
	if (_stop_camera_movement) {
		MapMode::CurrentInstance()->GetCamera()->SetMoving(false);
	}
}

// -----------------------------------------------------------------------------
// ---------- PopMapStateEvent Class Methods
// -----------------------------------------------------------------------------

PopMapStateEvent* PopMapStateEvent::Create(uint32 event_id) {
	PopMapStateEvent* event = new PopMapStateEvent(event_id);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}



void PopMapStateEvent::_Start() {
	MapMode::CurrentInstance()->PopState();
}

// -----------------------------------------------------------------------------
// ---------- CameraMoveEvent Class Methods
// -----------------------------------------------------------------------------

CameraMoveEvent* CameraMoveEvent::Create(uint32 event_id, VirtualSprite* focus, uint32 move_time) {
	if (focus == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function received nullptr argument" << endl;
		return nullptr;
	}

	CameraMoveEvent* event = new CameraMoveEvent(event_id, focus, 0, 0, move_time);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}



CameraMoveEvent* CameraMoveEvent::Create(uint32 event_id, uint32 x_position, uint32 y_position, uint32 move_time) {
	CameraMoveEvent* event = new CameraMoveEvent(event_id, nullptr, x_position, y_position, move_time);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}



CameraMoveEvent::CameraMoveEvent(uint32 event_id, VirtualSprite* focus, uint32 x_position, uint32 y_position, uint32 move_time) :
	MapEvent(event_id, CAMERA_MOVE_EVENT),
	_focus(focus),
	_camera_context(MAP_CONTEXT_NONE),
	_x_position(x_position),
	_y_position(y_position),
	_move_time(move_time)
{}



void CameraMoveEvent::_Start() {
	MapMode* map = MapMode::CurrentInstance();

	if (_focus != nullptr) {
		if (_camera_context != MAP_CONTEXT_NONE)
			_focus->SetContext(_camera_context);
		map->SetCamera(_focus, _move_time);
	}
	else {
		if (_camera_context != MAP_CONTEXT_NONE)
			map->GetVirtualFocus()->SetContext(_camera_context);
		map->MoveVirtualFocus(_x_position, _y_position);
		map->SetCamera(map->GetVirtualFocus(), _move_time);
	}
}



bool CameraMoveEvent::_Update() {
	if (_move_time == 0) {
		return true;
	}
	else {
		return MapMode::CurrentInstance()->IsCameraMoving();
	}
}

// -----------------------------------------------------------------------------
// ---------- DialogueEvent Class Methods
// -----------------------------------------------------------------------------

DialogueEvent::DialogueEvent(uint32 event_id, uint32 dialogue_id) :
	MapEvent(event_id, DIALOGUE_EVENT),
	_dialogue_id(dialogue_id),
	_stop_camera_movement(false)
{}



DialogueEvent* DialogueEvent::Create(uint32 event_id, uint32 dialogue_id) {
	DialogueEvent* event = new DialogueEvent(event_id, dialogue_id);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}


void DialogueEvent::_Start() {
	if (_stop_camera_movement == true) {
		MapMode::CurrentInstance()->GetCamera()->SetMoving(false);
		MapMode::CurrentInstance()->GetCamera()->SetRunning(false);
	}

	MapMode::CurrentInstance()->GetDialogueSupervisor()->BeginDialogue(_dialogue_id);
}



bool DialogueEvent::_Update() {
	MapDialogue* active_dialogue = MapMode::CurrentInstance()->GetDialogueSupervisor()->GetCurrentDialogue();
	if ((active_dialogue != nullptr) && (active_dialogue->GetDialogueID() == _dialogue_id))
		return false;
	else
		return true;
}

// -----------------------------------------------------------------------------
// ---------- ShopEvent Class Methods
// -----------------------------------------------------------------------------

ShopEvent::ShopEvent(uint32 event_id) :
	MapEvent(event_id, SHOP_EVENT)
{}



ShopEvent* ShopEvent::Create(uint32 event_id) {
	ShopEvent* event = new ShopEvent(event_id);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}



void ShopEvent::AddWare(uint32 object_id, uint32 stock) {
	_wares.insert(make_pair(object_id, stock));
}



void ShopEvent::_Start() {
	ShopMode* shop = new ShopMode();
	for (set<pair<uint32, uint32> >::iterator i = _wares.begin(); i != _wares.end(); i++) {
		shop->AddObject((*i).first, (*i).second);
	}
	ModeManager->Push(shop);
}



bool ShopEvent::_Update() {
	return true;
}

// -----------------------------------------------------------------------------
// ---------- SoundEvent Class Methods
// -----------------------------------------------------------------------------

SoundEvent::SoundEvent(uint32 event_id, string sound_filename) :
	MapEvent(event_id, SOUND_EVENT)
{
	if (_sound.LoadAudio(sound_filename) == false) {
		IF_PRINT_WARNING(MAP_DEBUG) << "failed to load sound event: " << sound_filename << endl;
	}
}



SoundEvent::~SoundEvent() {
	_sound.Stop();
}



SoundEvent* SoundEvent::Create(uint32 event_id, string sound_filename) {
	SoundEvent* event = new SoundEvent(event_id, sound_filename);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}



void SoundEvent::_Start() {
	_sound.Play();
}



bool SoundEvent::_Update() {
	if (_sound.GetState() == AUDIO_STATE_STOPPED) {
		// TODO: is it necessary to reset the loop counter and other properties here before returning?
		return true;
	}

	return false;
}

// -----------------------------------------------------------------------------
// ---------- MapTransitionEvent Class Methods
// -----------------------------------------------------------------------------

MapTransitionEvent::MapTransitionEvent(uint32 event_id, string filename, int32 load_point) :
	MapEvent(event_id, MAP_TRANSITION_EVENT),
	_transition_map_filename(filename),
	_transition_map_load_point(load_point)
{
	_fade_timer.Initialize(MAP_FADE_OUT_TIME, SYSTEM_TIMER_NO_LOOPS);
}



MapTransitionEvent* MapTransitionEvent::Create(uint32 event_id, string filename, int32 load_point) {
	MapTransitionEvent* event = new MapTransitionEvent(event_id, filename, load_point);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}



void MapTransitionEvent::SetFadeTime(uint32 fade_time)
{
	if (_fade_timer.GetState() != SYSTEM_TIMER_INITIAL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "can not set fade time when timer is active or finished" << endl;
		return;
	}

	_fade_timer.Initialize(fade_time, SYSTEM_TIMER_NO_LOOPS);
}



void MapTransitionEvent::_Start() {
	MapMode::CurrentInstance()->PushState(STATE_TRANSITION);
	_fade_timer.Reset();
	_fade_timer.Run();

	// TODO: The call below is a problem because if the user pauses while this event is in progress,
	// the screen fade will continue while in pause mode (it shouldn't). I think instead we'll have
	// to perform a manual fade of the screen, not allow the user to pause when map mode is in a transition state,
	// or use the notification engine to detect when the game
	// state changes to a mode other than map mode
	VideoManager->FadeScreen(Color::black, _fade_timer.GetDuration());

	// TODO: fade out the map music
}



bool MapTransitionEvent::_Update() {
	_fade_timer.Update();

	if (_fade_timer.IsFinished() == true) {
		ModeManager->Pop();
		try {
			MapMode *MM = new MapMode(_transition_map_filename, _transition_map_load_point);
			ModeManager->Push(MM);
		} catch (luabind::error e) {
			PRINT_ERROR << "Error loading map: " << _transition_map_filename << endl;
			ScriptManager->HandleLuaError(e);
		}
		// This will fade the screen back in from black
		VideoManager->FadeScreen(Color::clear, _fade_timer.GetDuration() / 2);
		return true;
	}

	return false;
}

// -----------------------------------------------------------------------------
// ---------- BattleEncounterEvent Class Methods
// -----------------------------------------------------------------------------

BattleEncounterEvent::BattleEncounterEvent(uint32 event_id) :
	MapEvent(event_id, BATTLE_ENCOUNTER_EVENT),
	_battle_music("mus/Confrontation.ogg"),
	_battle_background("img/backdrops/battle/desert.png")
{}



BattleEncounterEvent::~BattleEncounterEvent() {
}



BattleEncounterEvent* BattleEncounterEvent::Create(uint32 event_id) {
	BattleEncounterEvent* event = new BattleEncounterEvent(event_id);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}



void BattleEncounterEvent::SetMusic(std::string filename) {
	_battle_music = filename;
}



void BattleEncounterEvent::SetBackground(std::string filename) {
	_battle_background = filename;
}



void BattleEncounterEvent::AddEnemy(uint32 enemy_id) {
	_enemy_ids.push_back(enemy_id);
}



void BattleEncounterEvent::_Start() {
	BattleMode* batt_mode = new BattleMode();
	for (uint32 i = 0; i < _enemy_ids.size(); i++) {
		batt_mode->AddEnemy(_enemy_ids.at(i));
	}

	batt_mode->GetMedia().SetBackgroundImage(_battle_background);
	batt_mode->GetMedia().SetBattleMusic(_battle_music);

    MapMode::CurrentInstance()->GetTransitionSupervisor()->StartGameModeTransition(batt_mode);
}



bool BattleEncounterEvent::_Update() {
	if (MapMode::CurrentInstance()->CurrentState() != STATE_TRANSITION)
		return true;
	else
		return false;
}

// -----------------------------------------------------------------------------
// ---------- CustomEvent Class Methods
// -----------------------------------------------------------------------------

CustomEvent::CustomEvent(uint32 event_id, string start_name, string update_name) :
	MapEvent(event_id, SCRIPTED_EVENT),
	_start_function(nullptr),
	_update_function(nullptr)
{
	ReadScriptDescriptor& map_script = MapMode::CurrentInstance()->GetMapScript();
	MapMode::CurrentInstance()->OpenScriptTablespace(true);
	map_script.OpenTable("functions");
	if (start_name != "") {
		_start_function = new ScriptObject();
		*_start_function = map_script.ReadFunctionPointer(start_name);
		// The function object will be invalid if no function existed with the desired name
		if (_start_function->is_valid() == false) {
			IF_PRINT_WARNING(MAP_DEBUG) << "failed to find script function \"" << start_name << "\" for custom event (ID: " << event_id << ")" << endl;
			delete _start_function;
			_start_function = nullptr;
		}
	}
	if (update_name != "") {
		_update_function = new ScriptObject();
		*_update_function = map_script.ReadFunctionPointer(update_name);
		if (_update_function->is_valid() == false) {
			IF_PRINT_WARNING(MAP_DEBUG) << "failed to find script function \"" << update_name << "\" for custom event (ID: " << event_id << ")" << endl;
			delete _update_function;
			_update_function = nullptr;
		}
	}
	map_script.CloseTable();
	map_script.CloseTable();

	if ((_start_function == nullptr) && (_update_function == nullptr)) {
		IF_PRINT_WARNING(MAP_DEBUG) << "no start or update functions were declared for event: " << event_id << endl;
	}
}



CustomEvent::~CustomEvent() {
	if (_start_function != nullptr) {
		delete _start_function;
		_start_function = nullptr;
	}
	if (_update_function != nullptr) {
		delete _update_function;
		_update_function = nullptr;
	}
}


CustomEvent::CustomEvent(const CustomEvent& copy) :
	MapEvent(copy)
{
	if (copy._start_function == nullptr)
		_start_function = nullptr;
	else
		_start_function = new ScriptObject(*copy._start_function);

	if (copy._update_function == nullptr)
		_update_function = nullptr;
	else
		_update_function = new ScriptObject(*copy._update_function);
}



CustomEvent& CustomEvent::operator=(const CustomEvent& copy) {
	if (this == &copy) // Handle self-assignment case
		return *this;

	MapEvent::operator=(copy);

	if (copy._start_function == nullptr)
		_start_function = nullptr;
	else
		_start_function = new ScriptObject(*copy._start_function);

	if (copy._update_function == nullptr)
		_update_function = nullptr;
	else
		_update_function = new ScriptObject(*copy._update_function);

	return *this;
}



CustomEvent* CustomEvent::Create(uint32 event_id, string start_name, string update_name) {
	CustomEvent* event = new CustomEvent(event_id, start_name, update_name);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}



void CustomEvent::_Start() {
	if (_start_function != nullptr)
		ScriptCallFunction<void>(*_start_function);
}



bool CustomEvent::_Update() {
	if (_update_function != nullptr)
		return ScriptCallFunction<bool>(*_update_function);
	else
		return true;
}

// -----------------------------------------------------------------------------
// ---------- EventSupervisor Class Methods
// -----------------------------------------------------------------------------

EventSupervisor::~EventSupervisor() {
	_active_events.clear();
	_launch_events.clear();
	_event_history.clear();

	for (map<uint32, MapEvent*>::iterator i = _all_events.begin(); i != _all_events.end(); i++) {
		delete i->second;
	}
	_all_events.clear();
}



void EventSupervisor::RegisterEvent(MapEvent* new_event) {
	if (new_event == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument was nullptr" << endl;
		return;
	}

	if (GetEvent(new_event->_event_id) != nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "event with this ID already existed: " << new_event->_event_id << endl;
		return;
	}

	_all_events.insert(make_pair(new_event->_event_id, new_event));
}



void EventSupervisor::StartEvent(uint32 event_id) {
	MapEvent* event = GetEvent(event_id);
	if (event == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "no event with this ID existed: " << event_id << endl;
		return;
	}

	StartEvent(event);
}



void EventSupervisor::StartEvent(MapEvent* event) {
	if (event == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "nullptr argument passed to function" << endl;
		return;
	}

	IF_PRINT_DEBUG(MAP_DEBUG) << "Starting event: " << event->GetEventID() << " (" << DEBUG_EventTypeName(event->GetEventType())  << ")" << endl;

	_active_events.push_back(event);
	event->_Start();
	event->_CommitRecords(); // Commit any records for the event now that it has been started
	_event_history.emplace(event->GetEventID(), 0);
	_event_history[event->GetEventID()] += 1;
	_ExamineEventLinks(event, true);
}



void EventSupervisor::StartEvent(uint32 event_id, uint32 wait_time) {
	MapEvent* event = GetEvent(event_id);
	if (event == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "no event with this ID existed: " << event_id << endl;
		return;
	}

	if (wait_time == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "specified a wait_time of 0 for event_id: " << event_id << endl;
		StartEvent(event);
		return;
	}

	_launch_events.push_back(make_pair(static_cast<int32>(wait_time), event));
}



void EventSupervisor::StartEvent(MapEvent* event, uint32 wait_time) {
	if (event == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "nullptr argument passed to function" << endl;
		return;
	}

	if (wait_time == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "specified a wait_time of 0 for event with id: " << event->GetEventID() << endl;
		StartEvent(event);
		return;
	}

	_launch_events.push_back(make_pair(static_cast<int32>(wait_time), event));
}



void EventSupervisor::PauseEvent(uint32 event_id) {
	for (list<MapEvent*>::iterator i = _active_events.begin(); i != _active_events.end(); i++) {
		if ((*i)->_event_id == event_id) {
			_paused_events.push_back(*i);
			_active_events.erase(i);
			return;
		}
	}

	IF_PRINT_WARNING(MAP_DEBUG) << "operation failed because no active event was found corresponding to event id: " << event_id << endl;
}



void EventSupervisor::ResumeEvent(uint32 event_id) {
	for (list<MapEvent*>::iterator i = _paused_events.begin(); i != _paused_events.end(); i++) {
		if ((*i)->_event_id == event_id) {
			_active_events.push_back(*i);
			_paused_events.erase(i);
			return;
		}
	}

	IF_PRINT_WARNING(MAP_DEBUG) << "operation failed because no paused event was found corresponding to event id: " << event_id << endl;
}



void EventSupervisor::TerminateEvent(uint32 event_id) {
	// TODO: what if the event is in the active queue in more than one location?
	for (list<MapEvent*>::iterator i = _active_events.begin(); i != _active_events.end(); i++) {
		if ((*i)->_event_id == event_id) {
			MapEvent* terminated_event = *i;
			i = _active_events.erase(i);
			// We examine the event links only after the event has been removed from the active list
			_ExamineEventLinks(terminated_event, false);
			return;
		}
	}

	IF_PRINT_WARNING(MAP_DEBUG) << "attempted to terminate an event that was not active, id: " << event_id << endl;
}



void EventSupervisor::Update() {
	// Update all launch event timers and start all events whose timers have finished
	for (list<pair<int32, MapEvent*> >::iterator i = _launch_events.begin(); i != _launch_events.end();) {
		i->first -= SystemManager->GetUpdateTime();

		if (i->first <= 0) { // Timer has expired
			MapEvent* start_event = i->second;
			i = _launch_events.erase(i);
			// We begin the event only after it has been removed from the launch list
			StartEvent(start_event);
		}
		else
			++i;
	}

	// Check for active events which have finished
	for (list<MapEvent*>::iterator i = _active_events.begin(); i != _active_events.end();) {
		if ((*i)->_Update() == true) {
			MapEvent* finished_event = *i;
			i = _active_events.erase(i);
			// We examine the event links only after the event has been removed from the active list
			_ExamineEventLinks(finished_event, false);
		}
		else
			++i;
	}
}



bool EventSupervisor::IsEventActive(uint32 event_id) const {
	for (list<MapEvent*>::const_iterator i = _active_events.begin(); i != _active_events.end(); i++) {
		if ((*i)->_event_id == event_id) {
			return true;
		}
	}
	return false;
}



uint32 EventSupervisor::TimesEventStarted(uint32 event_id) const {
	map<uint32, uint32>::const_iterator i = _event_history.find(event_id);

	if (i == _event_history.end()) {
		return 0;
	}
	else
		return i->second;
}



MapEvent* EventSupervisor::GetEvent(uint32 event_id) const {
	map<uint32, MapEvent*>::const_iterator i = _all_events.find(event_id);

	if (i == _all_events.end())
		return nullptr;
	else
		return i->second;
}



void EventSupervisor::_ExamineEventLinks(MapEvent* parent_event, bool event_start) {
	for (uint32 i = 0; i < parent_event->_event_links.size(); i++) {
		EventLink& link = parent_event->_event_links[i];

		// Case 1: Start/finish launch member is not equal to the start/finish status of the parent event, so ignore this link
		if (link.launch_at_start != event_start) {
			continue;
		}
		// Case 2: The child event is to be launched immediately
		else if (link.launch_timer == 0) {
			StartEvent(link.child_event_id);
		}
		// Case 3: The child event has a timer associated with it and needs to be placed in the event launch container
		else {
			MapEvent* child = GetEvent(link.child_event_id);
			if (child == nullptr) {
				IF_PRINT_WARNING(MAP_DEBUG) << "can not launch child event, no event with this ID existed: " << link.child_event_id << endl;
				continue;
			}
			else {
				_launch_events.push_back(make_pair(static_cast<int32>(link.launch_timer), child));
			}
		}
	}
}

} // namespace private_map

} // namespace hoa_map
