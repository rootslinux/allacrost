///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_dialogue.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for map mode dialogue.
*** ***************************************************************************/

// Allacrost utilities
#include "utils.h"

// Allacrost engines
#include "audio.h"
#include "input.h"
#include "mode_manager.h"

// Allacrost common
#include "dialogue.h"
#include "global.h"

// Other game mode headers
#include "menu.h"

// Local map mode headers
#include "map.h"
#include "map_dialogue.h"
#include "map_events.h"
#include "map_objects.h"
#include "map_sprites.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_gui;
using namespace hoa_input;
using namespace hoa_mode_manager;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_menu;
using namespace hoa_common;

namespace hoa_map {

namespace private_map {

// Used to indicate that no event is to take place for a particular dialogue line or option
const uint32 NO_DIALOGUE_EVENT = 0;

///////////////////////////////////////////////////////////////////////////////
// MapDialogue Class Functions
///////////////////////////////////////////////////////////////////////////////

MapDialogue::~MapDialogue() {
	if (_dialogue_start_event != nullptr) {
		delete _dialogue_start_event;
	}
	if (_dialogue_end_event != nullptr) {
		delete _dialogue_end_event;
	}

	for (uint32 i = 0; i < _line_records.size(); ++i) {
		if (_line_records[i] != nullptr)
			delete _line_records[i];
	}

	for (uint32 i = 0; i < _line_events.size(); ++i) {
		if (_line_events[i] != nullptr)
			delete _line_events[i];
	}
}


MapDialogue::MapDialogue(uint32 id) :
	CommonDialogue(id),
	_input_blocked(false),
	_restore_state(true),
	_dialogue_name("dialogue#" + hoa_utils::NumberToString(id)),
	_dialogue_start_event(nullptr),
	_dialogue_end_event(nullptr)
{}



MapDialogue* MapDialogue::Create(uint32 id) {
	MapDialogue* dialogue = new MapDialogue(id);
	MapMode::CurrentInstance()->GetDialogueSupervisor()->RegisterDialogue(dialogue);
	return dialogue;
}



void MapDialogue::AddLine(string text, uint32 speaker, int32 next_line) {
	CommonDialogue::AddLine(text, next_line);
	_speakers.push_back(speaker);
	_line_records.push_back(nullptr);
	_line_events.push_back(nullptr);
}



void MapDialogue::AddLineTiming(uint32 display_time) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function called when dialogue contained no lines" << endl;
		return;
	}

	_display_times[_line_count - 1] = display_time;
}



void MapDialogue::AddLineTiming(uint32 display_time, uint32 line) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function called when dialogue contained no lines" << endl;
		return;
	}

	if (line >= _line_count) {
		IF_PRINT_WARNING(MAP_DEBUG) << "invalid line index requested: " << line << endl;
		return;
	}

	_display_times[line] = display_time;
}



void MapDialogue::AddOption(string text) {
	AddOption(text, COMMON_DIALOGUE_NEXT_LINE);
}



void MapDialogue::AddOption(string text, int32 next_line) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add an option to a dialogue with no lines" << endl;
		return;
	}

	uint32 current_line = _line_count - 1;

	// If the line the options will be added to currently has no options, create a new instance of the MapDialogueOptions class to store the options in.
	if (_options[current_line] == nullptr) {
		_options[current_line] = new MapDialogueOptions();
	}

	MapDialogueOptions* options = dynamic_cast<MapDialogueOptions*>(_options[current_line]);
	options->AddOption(text, next_line);
}



void MapDialogue::AddOptionEvent(uint32 event_id, uint32 delay_ms) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add an option event to a dialogue with no lines" << endl;
		return;
	}

	if (_options[_line_count - 1] == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add an option event to a line that contained no options" << endl;
		return;
	}

	MapDialogueOptions* options = dynamic_cast<MapDialogueOptions*>(_options[_line_count - 1]);
	options->AddOptionEvent(event_id, delay_ms);
}



void MapDialogue::ProcessLineActions(uint32 current_line, bool begin_or_end) {
	if (current_line >= _line_count) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to process actions for a line that didn't exist: " << current_line << endl;
		return;
	}

	MapRecordData* record_data = _line_records[current_line];
	MapEventData* event_data = _line_events[current_line];
	if (record_data != nullptr && begin_or_end == true) {
		record_data->CommitRecords();
	}
	if (event_data != nullptr) {
		event_data->StartEvents(begin_or_end);
	}
}



bool MapDialogue::Validate() {
	if (CommonDialogue::Validate() == false) {
		// The CommonDialogue::Validate() call will print the appropriate warning if debugging is enabled (common code debugging that is)
		return false;
	}

	// Construct containers that hold all unique sprite and event ids for this dialogue
	set<uint32> sprite_ids;
	set<uint32> event_ids;
	for (uint32 i = 0; i < _line_count; i++) {
		sprite_ids.insert(_speakers[i]);
	}

	// Check that all sprites and events referrenced by the dialogue exist
	for (set<uint32>::iterator i = sprite_ids.begin(); i != sprite_ids.end(); i++) {
		if (MapMode::CurrentInstance()->GetObjectSupervisor()->GetSprite(*i) == nullptr) {
			IF_PRINT_WARNING(MAP_DEBUG) << "Validation failed for dialogue #" << _dialogue_id
				<< ": dialogue referenced invalid sprite with id: " << *i << endl;
			return false;
		}
	}

	for (uint32 i = 0; i < _line_events.size(); i++) {
		if (_line_events[i] != nullptr && _line_events[i]->ValidateEvents() == false) {
			return false;
		}
	}

	return true;
}



void MapDialogue::_AddDialogueEvent(uint32 event_id, uint32 delay_timing, bool launch_at_start) {
	if (launch_at_start == true) {
		if (_dialogue_start_event == nullptr) {
			_dialogue_start_event = new MapEventData();
		}
		else {
			IF_PRINT_WARNING(MAP_DEBUG) << "a dialogue start event was already set; overriding with new event" << endl;
		}
		_dialogue_start_event->AddEvent(event_id, delay_timing, launch_at_start);
	}
	else {
		if (_dialogue_end_event == nullptr) {
			_dialogue_end_event = new MapEventData();
		}
		else {
			IF_PRINT_WARNING(MAP_DEBUG) << "a dialogue end event was already set; overriding with new event" << endl;
		}
		_dialogue_end_event->AddEvent(event_id, delay_timing, launch_at_start);
	}
}



void MapDialogue::_AddLineRecord(const std::string& record_name, int32 record_value, bool is_global) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add a line record to a dialogue with no lines" << endl;
		return;
	}

	MapRecordData* record_data = _line_records[_line_count - 1];
	// If no record data exists for this line, we have to construct a new object to hold it
	if (record_data == nullptr) {
		record_data = new MapRecordData();
		_line_records[_line_count - 1] = record_data;
	}

	if (is_global == true)
		record_data->AddGlobalRecord(record_name, record_value);
	else
		record_data->AddLocalRecord(record_name, record_value);
}



void MapDialogue::_AddLineEvent(uint32 event_id, uint32 start_timing, bool launch_at_start) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add a line event to a dialogue with no lines" << endl;
		return;
	}

	MapEventData* event_data = _line_events[_line_count -1];
	// If no event data exists for this line, we have to construct a new object to hold it
	if (event_data == nullptr) {
		event_data = new MapEventData();
		_line_events[_line_count - 1] = event_data;
	}

	event_data->AddEvent(event_id, start_timing, launch_at_start);
}



void MapDialogue::_AddOptionRecord(const std::string& record_name, int32 record_value, bool is_global) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add an option record to a dialogue with no lines" << endl;
		return;
	}

	if (_options[_line_count - 1] == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add an option record to a line that contained no options" << endl;
		return;
	}


	MapDialogueOptions* options = dynamic_cast<MapDialogueOptions*>(_options[_line_count - 1]);
	options->AddOptionRecord(record_name, record_value, is_global);
}

///////////////////////////////////////////////////////////////////////////////
// MapDialogueOptions Class Functions
///////////////////////////////////////////////////////////////////////////////

MapDialogueOptions::~MapDialogueOptions() {
	for (uint32 i = 0; i < _option_records.size(); ++i) {
		if (_option_records[i] != nullptr)
			delete _option_records[i];
	}

	for (uint32 i = 0; i < _option_events.size(); ++i) {
		if (_option_events[i] != nullptr)
			delete _option_events[i];
	}
}



void MapDialogueOptions::AddOption(string text) {
	AddOption(text, COMMON_DIALOGUE_NEXT_LINE);
}



void MapDialogueOptions::AddOption(string text, int32 next_line) {
	CommonDialogueOptions::AddOption(text, next_line);
	_option_records.push_back(nullptr);
	_option_events.push_back(nullptr);
}



void MapDialogueOptions::AddOptionRecord(const std::string& record_name, int32 record_value, bool is_global) {
	if (GetNumberOptions() == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add an option event when no options were available" << endl;
		return;
	}

	MapRecordData* record_data = _option_records.back();
	// If no record data exists for this option, we have to construct a new object to hold it
	if (record_data == nullptr) {
		record_data = new MapRecordData();
		_option_records[_option_records.size() - 1] = record_data;
	}

	if (is_global == true) {
		record_data->AddGlobalRecord(record_name, record_value);
	}
	else {
		record_data->AddLocalRecord(record_name, record_value);
	}
}



void MapDialogueOptions::AddOptionEvent(uint32 event_id, uint32 start_timing) {
	if (GetNumberOptions() == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add an option event when no options were available" << endl;
		return;
	}

	MapEventData* event_data = _option_events.back();
	// If no event data exists for this option, we have to construct a new object to hold it
	if (event_data == nullptr) {
		event_data = new MapEventData();
		_option_events[_option_events.size() - 1] = event_data;
	}

	// Note that all event data added uses launch_at_start set to true.
	event_data->AddEvent(event_id, start_timing, true);
}



void MapDialogueOptions::ProcessOptionActions(uint32 option) {
	if (option > GetNumberOptions()) {
		IF_PRINT_WARNING(MAP_DEBUG) << "invalid option argument: " << option << endl;
		return;
	}

	MapRecordData* record_data = _option_records[option];
	if (record_data != nullptr)
		record_data->CommitRecords();

	MapEventData* event_data = _option_events[option];
	if (event_data != nullptr)
		event_data->StartEvents(true);
}

///////////////////////////////////////////////////////////////////////////////
// DialogueSupervisor Class Functions
///////////////////////////////////////////////////////////////////////////////

DialogueSupervisor::DialogueSupervisor() :
	_state(DIALOGUE_STATE_INACTIVE),
	_current_dialogue(nullptr),
	_current_line(0),
	_current_options(nullptr),
	_line_timer(),
	_dialogue_window()
{
	_dialogue_window.SetPosition(512.0f, 760.0f);
}



DialogueSupervisor::~DialogueSupervisor() {
	_current_dialogue = nullptr;
	_current_options = nullptr;

	// Delete all dialogues
	for (map<uint32, MapDialogue*>::iterator i = _dialogues.begin(); i != _dialogues.end(); i++) {
		delete i->second;
	}
	_dialogues.clear();
}



void DialogueSupervisor::Update() {
	if (_current_dialogue == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to update when no dialogue was active" << endl;
		return;
	}

	_line_timer.Update();

	switch (_state) {
		case DIALOGUE_STATE_LINE:
			_UpdateLine();
			break;
		case DIALOGUE_STATE_OPTION:
			_UpdateOptions();
			break;
		default:
			IF_PRINT_WARNING(MAP_DEBUG) << "dialogue supervisor was in an unknown state: " << _state << endl;
			_state = DIALOGUE_STATE_LINE;
			break;
	}
}



void DialogueSupervisor::Draw() {
	_dialogue_window.Draw();
}



void DialogueSupervisor::RegisterDialogue(MapDialogue* dialogue) {
	if (dialogue == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function received null argument" << endl;
		return;
	}

	if (GetDialogue(dialogue->GetDialogueID()) != nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "a dialogue was already registered with this ID: " << dialogue->GetDialogueID() << endl;
		delete dialogue;
	}
	else {
		_dialogues.insert(make_pair(dialogue->GetDialogueID(), dialogue));
	}
}



void DialogueSupervisor::BeginDialogue(uint32 dialogue_id) {
	MapDialogue* dialogue = GetDialogue(dialogue_id);

	if (dialogue == nullptr) {
		IF_PRINT_WARNING(COMMON_DEBUG) << "could not begin dialogue because none existed for id# " << dialogue_id << endl;
		return;
	}

	if (_current_dialogue != nullptr) {
		IF_PRINT_WARNING(COMMON_DEBUG) << "beginning a new dialogue while another dialogue is still active" << endl;
	}

	MapMode::CurrentInstance()->PushState(STATE_DIALOGUE);
	_current_line = 0;
	_current_dialogue = dialogue;
	if (_current_dialogue->GetDialogueStartEvent() != nullptr) {
		_current_dialogue->GetDialogueStartEvent()->StartEvents(true);
	}
	_BeginLine();
}



void DialogueSupervisor::EndDialogue() {
	if (_current_dialogue == nullptr) {
		IF_PRINT_WARNING(COMMON_DEBUG) << "tried to end a dialogue when there was no dialogue active" << endl;
		return;
	}

	_current_dialogue->IncrementTimesSeen();
	if (MapMode::CurrentInstance()->GetGlobalRecordGroup()->DoesRecordExist(_current_dialogue->GetDialogueName()) == false) {
		MapMode::CurrentInstance()->GetGlobalRecordGroup()->AddNewRecord(_current_dialogue->GetDialogueName(), _current_dialogue->GetTimesSeen());
	}
	else {
		MapMode::CurrentInstance()->GetGlobalRecordGroup()->SetRecord(_current_dialogue->GetDialogueName(), _current_dialogue->GetTimesSeen());
	}

	// We only want to call the RestoreState function *once* for each speaker, so first we have to construct a list of pointers
	// for all speakers without duplication (i.e. the case where a speaker spoke more than one line of dialogue).

	// Get a unique set of all sprites that participated in the dialogue
	set<MapSprite*> speakers;
	for (uint32 i = 0; i < _current_dialogue->GetLineCount(); i++) {
		if (_current_dialogue->GetLineSpeaker(i) != NO_SPRITE) {
			speakers.insert(dynamic_cast<MapSprite*>(MapMode::CurrentInstance()->GetObjectSupervisor()->GetObject(_current_dialogue->GetLineSpeaker(i))));
		}
		else {
			speakers.insert(nullptr);
		}
	}

	for (set<MapSprite*>::iterator i = speakers.begin(); i != speakers.end(); i++) {
		// Each sprite needs to know that this dialogue completed so that they can update their data accordingly
		if (*i != nullptr) {
			(*i)->UpdateDialogueStatus();

			// Restore the state (orientation, animation, etc.) of all speaker sprites if necessary
			if (_current_dialogue->IsRestoreState() == true) {
				if ((*i)->IsStateSaved() == true)
					(*i)->RestoreState();
			}
		}
	}

	// Pop STATE_DIALOGUE before the final event is triggered in case the event is going to modify the map state
	MapMode::CurrentInstance()->PopState();
	if (_current_dialogue->GetDialogueEndEvent() != nullptr) {
		_current_dialogue->GetDialogueEndEvent()->StartEvents(false);
	}

	_current_dialogue = nullptr;
	_current_options = nullptr;
}



MapDialogue* DialogueSupervisor::GetDialogue(uint32 dialogue_id) {
	map<uint32, MapDialogue*>::iterator location = _dialogues.find(dialogue_id);
	if (location == _dialogues.end()) {
		return nullptr;
	}
	else {
		return location->second;
	}
}



void DialogueSupervisor::_UpdateLine() {
	_dialogue_window.GetDisplayTextBox().Update();

	// If the line has a valid display time and the timer is finished, move on to the next line
	if ((_line_timer.GetDuration() > 0) && (_line_timer.IsFinished() == true)) {
		_EndLine();
		return;
	}

	// Set the correct indicator
	if (_current_dialogue->IsInputBlocked() || _current_options != nullptr || _dialogue_window.GetDisplayTextBox().IsFinished() == false) {
		_dialogue_window.SetIndicator(COMMON_DIALOGUE_NO_INDICATOR);
	}
	else if (_current_line == _current_dialogue->GetLineCount()-1) {
		_dialogue_window.SetIndicator(COMMON_DIALOGUE_LAST_INDICATOR);
	}
	else {
		_dialogue_window.SetIndicator(COMMON_DIALOGUE_NEXT_INDICATOR);
	}

	// If this dialogue does not allow user input, we are finished
	if (_current_dialogue->IsInputBlocked() == true) {
		return;
	}

	if (_dialogue_window.GetDisplayTextBox().IsFinished() == true && _current_options != nullptr) {
		_state = DIALOGUE_STATE_OPTION;
	}

	if (InputManager->ConfirmPress()) {
		// If the line is not yet finished displaying, display the rest of the text
		if (_dialogue_window.GetDisplayTextBox().IsFinished() == false) {
			_dialogue_window.GetDisplayTextBox().ForceFinish();
			// Proceed to option selection if the line has options
			if (_current_options != nullptr) {
				_state = DIALOGUE_STATE_OPTION;
			}
		}
		else {
			_EndLine();
		}
	}
}



void DialogueSupervisor::_UpdateOptions() {
	_dialogue_window.GetDisplayOptionBox().Update();

	if (InputManager->ConfirmPress()) {
		_dialogue_window.GetDisplayOptionBox().InputConfirm();
		_EndLine();
	}

	if (InputManager->UpPress()) {
		_dialogue_window.GetDisplayOptionBox().InputUp();
	}

	if (InputManager->DownPress()) {
		_dialogue_window.GetDisplayOptionBox().InputDown();
	}
}



void DialogueSupervisor::_BeginLine() {
	_state = DIALOGUE_STATE_LINE;
	_current_options = dynamic_cast<MapDialogueOptions*>(_current_dialogue->GetLineOptions(_current_line));

	// Execute any actions that should occur when this line begins
	_current_dialogue->ProcessLineActions(_current_line, true);

	// Initialize the line timer
	if (_current_dialogue->GetLineDisplayTime(_current_line) >= 0) {
		_line_timer.Initialize(_current_dialogue->GetLineDisplayTime(_current_line));
		_line_timer.Run();
	}
	// If the line has no timer specified, set the line time to zero and put the timer in the finished state
	else {
		_line_timer.Initialize(0);
		_line_timer.Finish();
	}

	// Setup the text and graphics for the dialogue window
	_dialogue_window.Clear();
	_dialogue_window.GetDisplayTextBox().SetDisplayText(_current_dialogue->GetLineText(_current_line));

	if (_current_options != nullptr) {
		for (uint32 i = 0; i < _current_options->GetNumberOptions(); i++) {
			_dialogue_window.GetDisplayOptionBox().AddOption(_current_options->GetOptionText(i));
		}

		_dialogue_window.GetDisplayOptionBox().SetSelection(0);
	}

	// Executes normal code if the speaker is an actual sprite i.e speaker id is not NO_SPRITE
	if (_current_dialogue->GetLineSpeaker(_current_line) != NO_SPRITE) {
		MapObject* object = MapMode::CurrentInstance()->GetObjectSupervisor()->GetObject(_current_dialogue->GetLineSpeaker(_current_line));
		if (object == nullptr) {
			IF_PRINT_WARNING(MAP_DEBUG) << "dialogue #" << _current_dialogue->GetDialogueID()
				<< " referenced a sprite that did not exist with id: " << _current_dialogue->GetLineSpeaker(_current_line) << endl;
			return;
		}
		else if (object->GetType() != SPRITE_TYPE) {
			IF_PRINT_WARNING(MAP_DEBUG) << "dialogue #" << _current_dialogue->GetDialogueID()
				<< " referenced a map object which was not a sprite with id: " << _current_dialogue->GetLineSpeaker(_current_line) << endl;
			return;
		}
		else {
			MapSprite* speaker = dynamic_cast<MapSprite*>(object);
			_dialogue_window.GetNameText().SetText(speaker->GetName());
			_dialogue_window.SetPortraitImage(speaker->GetFacePortrait());
		}
	}
	else {
		_dialogue_window.GetNameText().SetText("");
		_dialogue_window.SetPortraitImage(nullptr);
	}


}



void DialogueSupervisor::_EndLine() {
	// Execute any actions that should occur when this line ends, or actions based on the selected option if the line had options
	_current_dialogue->ProcessLineActions(_current_line, false);
	if (_current_options != nullptr) {
		uint32 selected_option = _dialogue_window.GetDisplayOptionBox().GetSelection();
		_current_options->ProcessOptionActions(selected_option);
	}

	// Determine the next line to read
	int32 next_line = _current_dialogue->GetLineNextLine(_current_line);
	// If this line had options, the selected option next line overrides the line's next line that we set above
	if (_current_options != nullptr) {
		uint32 selected_option = _dialogue_window.GetDisplayOptionBox().GetSelection();
		next_line = _current_options->GetOptionNextLine(selected_option);
	}

	// --- Case 1: Explicitly setting the next line. Warn and end the dialogue if the line to move to is invalid
	if (next_line >= 0) {
		if (static_cast<uint32>(next_line) >= _current_dialogue->GetLineCount()) {
			IF_PRINT_WARNING(MAP_DEBUG) << "dialogue #" << _current_dialogue->GetDialogueID()
				<< " tried to set dialogue to invalid line. Current/next line values: {" << _current_line
				<< ", " << next_line << "}" << endl;
			next_line = COMMON_DIALOGUE_END;
		}
	}
	// --- Case 2: Request to incrementing the current line. If we're incrementing past the last line, end the dialogue
	else if (next_line == COMMON_DIALOGUE_NEXT_LINE) {
		next_line = _current_line + 1;
		if (static_cast<uint32>(next_line) >= _current_dialogue->GetLineCount()) {
			next_line = COMMON_DIALOGUE_END;
		}
	}
	// --- Case 3: Request to end the current dialogue
	else if (next_line == COMMON_DIALOGUE_END) {
		// Do nothing
	}
	// --- Case 4: Unknown negative value. Warn and end dialogue
	else {
		IF_PRINT_WARNING(MAP_DEBUG) << "dialogue #" << _current_dialogue->GetDialogueID()
			<< " unknown next line control value: " << next_line << endl;
		next_line = COMMON_DIALOGUE_END;
	}

	// Now either end the dialogue or move on to the next line
	if (next_line == COMMON_DIALOGUE_END) {
		EndDialogue();
	}
	else {
		_current_line = next_line;
		_BeginLine();
	}
}

} // namespace private_map

} // namespace hoa_map
