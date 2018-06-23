///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_dialogue.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for map mode dialogue
*** ***************************************************************************/

#pragma once

// Allacrost utilities
#include "utils.h"
#include "defs.h"

// Allacrost engines
#include "script.h"
#include "video.h"

// Allacrost common
#include "dialogue.h"
#include "gui.h"

// Local map mode headers
#include "map_utils.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief Represents a dialogue that occurs between one or more sprites on a map
***
*** MapDialogue is a more specialized version of CommonDialogue. Like CommonDialogue,
*** a dialogue has multiple lines, non-linear line sequencing support, timed line support,
*** and selectable option support. Map dialogues also have the following properties.
***
*** - Every line has a speaker, which must point to an existing sprite object on the map.
*** The name and portrait of this sprite (if available) is used in the dialogue display.
*** - A record name is automatically generated for the dialogue based on its ID, and this
*** record is entered into the map's global record group in order to keep track of how many
*** times a player has seen a dialogue.
*** - The beginning or end of a dialogue may trigger a map event.
*** - Lines can modify the global or local record groups for a map, or launch map events.
*** Selected options can also enact these changes.
***
*** There are various other properties that you can set for a map dialogue object, such as
*** declaring that user input should be ignored while the dialogue is active, or modifying
*** the state of the sprites for the line speakers.
***
*** \note Map events can be set to occur under four situations: when a dialogue begins, when it
*** ends, when a line of dialogue ends, or when a dialogue option is selected by the player.
*** In each of these cases, only a single map event may be specified. If you need more than one
*** event to occur, you must make use of the chaining feature in map events to chain one or more
*** secondary events off of the event being triggered in the dialogue
***
*** \note Because the map will enter into the DIALOGUE_STATE while a dialogue is active, be
*** very careful about pushing or popping map states in events that get executed during a
*** dialogue.
*** ***************************************************************************/
class MapDialogue : public hoa_common::CommonDialogue {
public:
	~MapDialogue();

	/** \brief Creates an instance of the class and registers it with the dialogue supervisor
	*** \param event_id The unique ID of the dialogue to be created
	*** \return A pointer to the instance of the event created
	**/
	static MapDialogue* Create(uint32 id);

	/** \brief Specifies an event to trigger when the dialogue begins
	*** \param event_id The ID of the event to trigger
	**/
	void AddEventAtStart(uint32 event_id)
		{ _AddDialogueEvent(event_id, 0, true); }

	/** \brief Specifies an event to trigger when the dialogue begins
	*** \param event_id The ID of the event to trigger
	*** \param launch_timing The number of milliseconds to wait after the dialogue begins before triggering this event
	**/
	void AddEventAtStart(uint32 event_id, uint32 launch_timing)
		{ _AddDialogueEvent(event_id, launch_timing, true); }

	/** \brief Specifies an event to trigger when the dialogue ends
	*** \param event_id The ID of the event to trigger
	**/
	void AddEventAtEnd(uint32 event_id)
		{ _AddDialogueEvent(event_id, 0, false); }

	/** \brief Specifies an event to trigger when the dialogue ends
	*** \param event_id The ID of the event to trigger
	*** \param launch_timing The number of milliseconds to wait after the dialogue ends before triggering this event
	**/
	void AddEventAtEnd(uint32 event_id, uint32 launch_timing)
		{ _AddDialogueEvent(event_id, launch_timing, false); }

	/** \brief Adds a new line of text to the dialogue
	*** \param text The text to show on the screen
	*** \param speaker The object ID of the sprite speaking this line
	***
	*** The following line properties are set when using this call:
	*** - proceed to next sequential line, no display time
	**/
	void AddLine(std::string text, uint32 speaker)
		{ AddLine(text, speaker, hoa_common::COMMON_DIALOGUE_NEXT_LINE); }

	/** \brief Adds a new line of text to the dialogue
	*** \param text The text to show on the screen
	*** \param speaker The object ID of the sprite speaking this line
	*** \param next_line The line of dialogue which should follow this one
	***
	*** The following line properties are set when using this call:
	*** - no display time
	**/
	void AddLine(std::string text, uint32 speaker, int32 next_line);

	/** \brief Adds a new line of text to the dialogue without a speaker
	*** \param text The text to show on the screen
	***
	*** The following line properties are set when using this call:
	*** - proceed to next sequential line, no display time
	**/
	void AddLine(std::string text)
		{ AddLine(text,NO_SPRITE, hoa_common::COMMON_DIALOGUE_NEXT_LINE); }

	/** \brief Sets a a display time for the last line of dialogue added
	*** \param display_time The number of milliseconds that the line should be displayed for
	**/
	void AddLineTiming(uint32 display_time);

	/** \brief Sets a a display time for a specific line of dialogue
	*** \param display_time The number of milliseconds that the line should be displayed for
	*** \param line The index of the line to set the timing for
	**/
	void AddLineTiming(uint32 display_time, uint32 line);

	/** \brief Adds a record to be set on the global record group once the line begins
	*** \param record_name The name of the record to set
	*** \param record_value The value of the record to set
	**/
	void AddLineGlobalRecord(const std::string& record_name, int32 record_value)
		{ _AddLineRecord(record_name, record_value, true); }

	/** \brief Adds a record to be set on the local record group once the line begins
	*** \param record_name The name of the record to set
	*** \param record_value The value of the record to set
	**/
	void AddLineLocalRecord(const std::string& record_name, int32 record_value)
		{ _AddLineRecord(record_name, record_value, false); }

	/** \brief Adds an event to the most recently added line of text that will be launched when the line begins
	*** \param event_id The ID of the event to add
	**/
	void AddLineEventAtStart(uint32 event_id)
		{ _AddLineEvent(event_id, 0, true); }

	/** \brief Adds an event to the most recently added line of text that will be launched when the line begins
	*** \param event_id The ID of the event to add
	*** \param start_timing The number of milliseconds to wait before starting the event
	**/
	void AddLineEventAtStart(uint32 event_id, uint32 start_timing)
		{ _AddLineEvent(event_id, start_timing, true); }

	/** \brief Adds an event to the most recently added line of text that will be launched when the line ends
	*** \param event_id The ID of the event to add
	**/
	void AddLineEventAtEnd(uint32 event_id)
		{ _AddLineEvent(event_id, 0, false); }

	/** \brief Adds an event to the most recently added line of text that will be launched when the line ends
	*** \param event_id The ID of the event to add
	*** \param start_timing The number of milliseconds to wait before starting the event
	**/
	void AddLineEventAtEnd(uint32 event_id, uint32 start_timing)
		{ _AddLineEvent(event_id, start_timing, false); }

	/** \brief Adds an option to the most recently added line of text
	*** \param text The text for this particular option
	*** \note If no lines have been added to the dialogue yet, this option will not be added and a warning will be issued
	***
	*** The following option properties are set when using this call:
	*** - proceed to next sequential line
	**/
	void AddOption(std::string text);

	/** \brief Adds an option to the most recently added line of text
	*** \param text The text for this particular option
	*** \param next_line The index value of the next line of dialogue to display should this option be selected
	*** \note If no lines have been added to the dialogue yet, this option will not be added and a warning will be issued
	**/
	void AddOption(std::string text, int32 next_line);

	/** \brief Adds a record to be set for the global record group to the most recently added option
	*** \param record_name The name of the record to set
	*** \param record_value The value of the record to set
	**/
	void AddOptionGlobalRecord(const std::string& record_name, int32 record_value)
		{ _AddOptionRecord(record_name, record_value, true); }

	/** \brief Adds a record to be set for the local record group to the most recently added option
	*** \param record_name The name of the record to set
	*** \param record_value The value of the record to set
	**/
	void AddOptionLocalRecord(const std::string& record_name, int32 record_value)
		{ _AddOptionRecord(record_name, record_value, false); }

	/** \brief Adds an event to the most recently added option
	*** \param event_id The ID of the event to execute should this option be selected
	**/
	void AddOptionEvent(uint32 event_id)
		{ AddOptionEvent(event_id, 0); }

	/** \brief Adds an event to the most recently added option
	*** \param event_id The ID of the event to execute should this option be selected
	*** \param start_timing The number of milliseconds to wait before starting the event
	**/
	void AddOptionEvent(uint32 event_id, uint32 start_timing);

	/** \brief Commits records and starts events for a specific line
	*** \param current_line The line of the dialogue to process
	*** \param begin_or_end If true, only start events that are to occur the beginning of the line. Otherwise start the end line events
	***
	*** Note that records are only committed when a line begins (begin_or_end is set to true).
	**/
	void ProcessLineActions(uint32 current_line, bool begin_or_end);

	/** \brief Checks all the data stored by the dialogue class to ensure that it is acceptable and ready for use
	*** \return True if the validation was successful, false if any problems were discovered
	***
	*** \note This function should not be called until after all map sprites and map events have been added. The function checks that each
	*** speaker is valid and stored in the map's object list, so if you perform this check before you've added all the speakers
	*** to the object list of MapMode, the validation will fail.
	**/
	bool Validate();

	/** \brief Returns the object ID of the speaker for the line specified (or zero if the line index was invalid or if no speaker)
	***	\todo Need to determine a way to differentitate an invalid index or no speaker (possibly print warning message for invalid index)
	**/
	uint32 GetLineSpeaker(uint32 line) const
		{ if (line >= _line_count) return 0; else return _speakers[line]; }

	//! \name Class Member Access Functions
	//@{
	std::string GetDialogueName() const
		{ return _dialogue_name; }

	bool IsInputBlocked() const
		{ return _input_blocked; }

	bool IsRestoreState() const
		{ return _restore_state; }

	MapEventData* GetDialogueStartEvent() const
		{ return _dialogue_start_event; }

	MapEventData* GetDialogueEndEvent() const
		{ return _dialogue_end_event; }

	void SetInputBlocked(bool blocked)
		{ _input_blocked = blocked; }

	void SetRestoreState(bool restore)
		{ _restore_state = restore; }
	//@}

protected:
	//! \param id The id number to represent the dialogue, which should be unique to other dialogue ids within this map
	MapDialogue(uint32 id);

	//! \brief If true, dialogue will ignore user input and instead execute independently
	bool _input_blocked;

	//! \brief If true, the state of map sprites participating in this dialogue will be reset after the dialogue completes
	bool _restore_state;

	//! \brief The event name for this dialogue that is stored in the saved game file, of the form "dialogue#"
	std::string _dialogue_name;

	//! \brief Pointer to an optional event that may start as soon as the dialogue begins
	MapEventData* _dialogue_start_event;

	//! \brief Pointer to an optional event that may start as soon as a dialogue ends
	MapEventData* _dialogue_end_event;

	//! \brief Contains object ID numbers that declare the speaker of each line
	std::vector<uint32> _speakers;

	//! \brief Maintains the list of map records that may be set after each line of the dialogue
	std::vector<MapRecordData*> _line_records;

	//! \brief Maintains the list of map events that may activate after each line of the dialogue
	std::vector<MapEventData*> _line_events;

	/** \brief Adds either an event to trigger either when the dialogue begins or ends
	*** \param event_id The ID of the event to add
	*** \param start_timing The number of milliseconds to wait before launching the event
	*** \param launch_at_start If true, the event will launch when the dialogue begins. Otherwise it launches when the dialogue ends.
	**/
	void _AddDialogueEvent(uint32 event_id, uint32 delay_timing, bool launch_at_start);

	/** \brief Adds a record to the most recently added line
	*** \param record_name The name of the record to set
	*** \param record_value The value of the record to set
	*** \param is_global If true, the record will be set to the global record group. Otherwise the local record group will be used.
	**/
	void _AddLineRecord(const std::string& record_name, int32 record_value, bool is_global);

	/** \brief Adds an event to the most recently added line
	*** \param event_id The ID of the event to add
	*** \param start_timing The number of milliseconds to wait before starting the event
	*** \param launch_at_start If true, the event will launch when the line begins. Otherwise it will launch when the line ends.
	**/
	void _AddLineEvent(uint32 event_id, uint32 start_timing, bool launch_at_start);

	/** \brief Adds a record to the most recently added option for the most recently added line
	*** \param record_name The name of the record to set
	*** \param record_value The value of the record to set
	*** \param is_global If true, the record will be set to the global record group. Otherwise the local record group will be used.
	**/
	void _AddOptionRecord(const std::string& record_name, int32 record_value, bool is_global);
}; // class MapDialogue : public hoa_common::CommonDialogue


/** ***************************************************************************************
*** \brief A container class for option sets presented during a map dialogue
***
*** When the player reads a dialogue, they may be presented with a small number of options to
*** select from when coming to a particular line. The selected option determines the next line
*** that will follow. Optionally, each particular option may trigger changes to the global or
*** local record group for the map, or start a map event when it is selected.
*** **************************************************************************************/
class MapDialogueOptions : public hoa_common::CommonDialogueOptions {
public:
	MapDialogueOptions()
		{}

	~MapDialogueOptions();

	/** \brief Adds a new option to the set of options
	*** \param text The text for the new option
	***
	*** The following option properties are set when using this call:
	*** - proceed to next sequential line, no event
	**/
	void AddOption(std::string text);

	/** \brief Adds a new option to the set of options
	*** \param text The text for the new option
	*** \param next_line An integer index of the next line of dialogue should this option be selected.
	***
	*** The following option properties are set when using this call:
	*** - no event
	**/
	void AddOption(std::string text, int32 next_line);

	/** \brief Adds a record to be set to the most recently added option
	*** \param record_name The name of the record to set
	*** \param record_value The value of the record to set
	*** \param is_global If true, the record will be set to the global record group. Otherwise the local record group will be used.
	**/
	void AddOptionRecord(const std::string& record_name, int32 record_value, bool is_global);

	/** \brief Adds an event to the most recently added option
	*** \param event_id The ID of the event to execute should this option be selected
	**/
	void AddOptionEvent(uint32 event_id)
		{ AddOptionEvent(event_id, 0); }

	/** \brief Adds an event to the most recently added option
	*** \param event_id The ID of the event to execute should this option be selected
	*** \param start_timing The number of milliseconds to wait before starting the event
	**/
	void AddOptionEvent(uint32 event_id, uint32 start_timing);

	//! \brief Returns the number of options stored by this class
	uint32 GetNumberOptions() const
		{ return _text.size(); }

	/** \brief Performs necessary actions for an option, updating any records and launching events appropriately
	*** \param option The index of the option to process
	**/
	void ProcessOptionActions(uint32 option);

private:
	//! \brief Holds any local or global records that are set when an option is selected
	std::vector<MapRecordData*> _option_records;

	//! \brief MapEvents that may be launched as a result of selecting each option
	std::vector<MapEventData*> _option_events;
}; // class MapDialogueOptions : public hoa_common::CommonDialogueOptions


/** ****************************************************************************
*** \brief Manages dialogue execution on maps
***
*** The MapMode class creates an instance of this class to handle all dialogue
*** processing that occurs on the map. This includes retention of the dialogue objects,
*** handling user input, display timing, and more. When a dialogue begins, the map state
*** changes to DIALOGUE_STATE via a push operation. This state is popped when the dialogue
*** finishes.
***
*** \todo Add support so that the player may backtrack through lines in a
*** dialogue (without re-processing selected options or previous script events).
*** ***************************************************************************/
class DialogueSupervisor {
public:
	DialogueSupervisor();

	~DialogueSupervisor();

	//! \brief Processes user input and updates the state of the dialogue
	void Update();

	//! \brief Draws the dialogue window, text, portraits, and other visuals to the screen
	void Draw();

	/** \brief Adds a new dialogue to be managed by the supervisor
	*** \param dialogue Pointer to a CommonDialogue object that was created with the new operator
	***
	*** The dialogue to add must have a unique dialogue ID that is not already stored by this class
	*** instance. If a dialogue is found with the same ID, the dialogue will not be added and the
	*** dialogue object will be deleted from memory. All dialogues that are successfully added will
	*** be later deleted when this class' destructor is invoked, so make sure you only pass in objects
	*** that were created with the "new" operator.
	**/
	void RegisterDialogue(MapDialogue* dialogue);

	/** \brief Prepares the dialogue manager to begin processing a new dialogue
	*** \param dialogue_id The id number of the dialogue to begin
	**/
	void BeginDialogue(uint32 dialogue_id);

	//! \brief Immediately ends any dialogue that is taking place
	void EndDialogue();

	/** \brief Returns a pointer to the CommonDialogue with the requested ID value
	*** \param dialogue_id The identification number of the dialogue to retrieve
	*** \return A pointer to the dialogue requested, or nullptr if no such dialogue was found
	**/
	MapDialogue* GetDialogue(uint32 dialogue_id);

	//! \name Class member access functions
	//@{
	DIALOGUE_STATE GetDialogueState() const
		{ return _state; }

	MapDialogue* GetCurrentDialogue() const
		{ return _current_dialogue; }

	MapDialogueOptions* GetCurrentOptions() const
		{ return _current_options; }

	hoa_system::SystemTimer& GetLineTimer()
		{ return _line_timer; }

	uint32 GetCurrentLine() const
		{ return _current_line; }
	//@}

private:
	//! \brief Retains the current state of the dialogue execution
	DIALOGUE_STATE _state;

	//! \brief Contains all dialogues used in the map in a std::map structure. The dialogue IDs serve as the map keys
	std::map<uint32, MapDialogue*> _dialogues;

	//! \brief A pointer to the current piece of dialogue that is active
	MapDialogue* _current_dialogue;

	//! \brief Keeps track of which line is active for the current dialogue
	uint32 _current_line;

	//! \brief A pointer to the current set of options for the active dialogue line
	MapDialogueOptions* _current_options;

	//! \brief A timer employed for dialogues which have a display time limit
	hoa_system::SystemTimer _line_timer;

	//! \brief Holds the text and graphics that should be displayed for the dialogue
	hoa_common::CommonDialogueWindow _dialogue_window;

	// ---------- Private methods

	//! \brief Updates the dialogue when it is in the line state
	void _UpdateLine();

	//! \brief Updates the dialogue when it is in the option state
	void _UpdateOptions();

	/** \brief Begins the display of the line indexed by the value of _line_counter
	***
	*** This is called whenever a dialogue begins or is moved to the next line. Its duties include updating the
	*** dialogue state, dialogue window displays with data from the new line, and setting up the line timer.
	***
	*** \note This method does not check that the _line_counter member refers to a valid line. It is the caller's
	*** responsibility to ensure that _line_counter is valid prior to calling this method.
	**/
	void _BeginLine();

	/** \brief Finishes the current dialogue line and moves the dialogue forward to the next line
	***
	*** This function determines the next line that the dialogue should proceed to based on the properties of the
	*** current line. This includes "branching" to the appropriate next line based on the option selected by the player
	*** when options were enabled on the current line. Should the line counter become invalid or the dialogue is to end
	*** after the present line, this function will invoke the EndDialogue() method. In addition to proceeding to the next
	*** line, this method is also responsible for invoking any events that were to occur at the conclusion of the present
	*** line.
	**/
	void _EndLine();

	//! \brief Restores participating sprites to their state before this dialogue started
	void _RestoreSprites();
}; // class DialogueSupervisor

} // namespace private_map

} // namespace hoa_map
