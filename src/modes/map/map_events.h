///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_events.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for map mode events and event processing
***
*** Events occur on map mode to alter the state of the map, present a scene to the
*** player, or do any other custom task we require. Events may be "chained" together
*** so that one event begins as another ends. Event class objects are created in the
*** Lua map script file and triggered. The CustomEvent class defined here allows an
*** event to be scripted completely within Lua. The other classes are more common to
*** use and define typical events we may want to see in a map, such as playing a sound.
*** ***************************************************************************/

#pragma once

// Allacrost utilities
#include "defs.h"
#include "utils.h"

// Allacrost engines
#include "script.h"

// Local map mode headers
#include "map_utils.h"
#include "map_sprites.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief A container class representing a link between two map events
***
*** Map events may trigger additional events to occur alongside it or following
*** it. This class represents a "link" between two events and describes how the
*** two events are linked. In an event link there is a parent event and a child
*** event. The parent and child events may begin at the same time, or the child
*** event may occur after the parent event starts, but the child will never
*** preceed the parent's start. This class only stores the event_id of the child,
*** and the link object is added as a member onto the parent event's class. When
*** the parent event gets processed, all links are examined and the children events
*** are prepared appropriately.
***
*** We use two pieces of information to determine when to start a child event relevant
*** to its parent. The first is a boolean value that indicates whether the child's
*** start is relative to the parent's start or the parent's finish. The second is a
*** time value that indicates how long to wait (in milliseconds) from the parent's
*** start/finish before starting the child event.
*** ***************************************************************************/
class EventLink {
public:
	EventLink(uint32 child_id, bool start, uint32 time) :
		child_event_id(child_id), launch_at_start(start), launch_timer(time) {}

	~EventLink()
		{}

	//! \brief The ID of the child event in this link
	uint32 child_event_id;

	//! \brief The event will launch relative to the parent event's start if true, or its finish if false
	bool launch_at_start;

	//! \brief The amount of milliseconds to wait before launching the event (0 means launch instantly)
	uint32 launch_timer;
}; // class EventLink


/** ****************************************************************************
*** \brief An abstract class representing an event that occurs on a map
***
*** An event can be virtually anything from playing a sound to moving a sprite
*** to beginning a dialogue. Events do not necessarily inform the user (through
*** visual or audio means) that an event has occurred. They may be employed to
*** change the state of a map without the player's knowledge. This is an abstract
*** class because common types of events (such as beginning a dialogue) are implemented
*** in C++ code while Lua is used to represent not-so-common types of events.
***
*** All events have a unique, non-zero, unsigned integer value that serve to
*** distinguish the events from one another (an ID number). Events can also contain any
*** number of "links" to children events, which are events which launch simultaneously
*** with or some time after the parent event. Events are processed via two
*** functions. _Start() is called only one when the event begins. _Update() is called
*** once for every iteration of the main game loop until this function returns a true
*** value, indicating that the event is finished.
***
*** Events can also store any number of changes to make to either the global or local record
*** groups for the map. These changes are applied every time the event's _Start() function is
*** called.
*** ***************************************************************************/
class MapEvent {
	friend class EventSupervisor;
public:
	uint32 GetEventID() const
		{ return _event_id; }

	EVENT_TYPE GetEventType() const
		{ return _event_type; }

	/** \brief Declares a child event to be launched immediately at the start of this event
	*** \param child_event_id The event id of the child event
	**/
	void AddEventLinkAtStart(uint32 child_event_id)
		{ _AddEventLink(child_event_id, true, 0); }

	/** \brief Declares a child event to be launched after the start of this event
	*** \param child_event_id The event id of the child event
	*** \param launch_time The number of milliseconds to wait before launching the child event
	**/
	void AddEventLinkAtStart(uint32 child_event_id, uint32 launch_time)
		{ _AddEventLink(child_event_id, true, launch_time); }

	/** \brief Declares a child event to be launched immediately at the end of this event
	*** \param child_event_id The event id of the child event
	**/
	void AddEventLinkAtEnd(uint32 child_event_id)
		{ _AddEventLink(child_event_id, false, 0); }

	/** \brief Declares a child event to be launched after the end of this event
	*** \param child_event_id The event id of the child event
	*** \param launch_time The number of milliseconds to wait before launching the child event
	**/
	void AddEventLinkAtEnd(uint32 child_event_id, uint32 launch_time)
		{ _AddEventLink(child_event_id, false, launch_time); }

	/** \brief Adds a record to be set on the global record group once the event starts
	*** \param record_name The name of the record to set
	*** \param record_value The value of the record to set
	**/
	void AddGlobalRecord(const std::string& record_name, int32 record_value)
		{ _AddRecord(record_name, record_value, true); }

	/** \brief Adds a record to be set on the local record group once the event starts
	*** \param record_name The name of the record to set
	*** \param record_value The value of the record to set
	**/
	void AddLocalRecord(const std::string& record_name, int32 record_value)
		{ _AddRecord(record_name, record_value, true); }

protected:
	//! \param id The ID for the map event (a zero value is invalid)
	MapEvent(uint32 id, EVENT_TYPE type) :
		_event_id(id), _event_type(type), _event_records(nullptr) {}

	virtual ~MapEvent()
		{ if (_event_records != nullptr) delete _event_records; }

	/** \brief Starts the event
	*** This function is only called once per event execution
	**/
	virtual void _Start() = 0;

	/** \brief Updates the event progress and checks if the event has finished
	*** \return True if the event is finished
	*** This function is called as many times as needed until the event has finished. The contents
	*** of this function may do more than simply check if the event is finished. It may also execute
	*** code for the event with the goal of eventually brining the event to a finished state.
	**/
	virtual bool _Update() = 0;

private:
	//! \brief A unique ID number for the event. A value of zero is invalid
	uint32 _event_id;

	//! \brief Identifier for the class type of this event
	EVENT_TYPE _event_type;

	//! \brief All child events of this class, represented by EventLink objects
	std::vector<EventLink> _event_links;

	//! \brief Holds changes to the local or global map records that may take place when the event is started
	MapRecordData* _event_records;

	/** \brief Declares a child event to be linked to this event
	*** \param child_event_id The event id of the child event
	*** \param launch_at_start The child starts relative to the start of the event if true, its finish if false
	*** \param launch_time The number of milliseconds to wait before launching the child event
	**/
	void _AddEventLink(uint32 child_event_id, bool launch_at_start, uint32 launch_time)
		{ _event_links.push_back(EventLink(child_event_id, launch_at_start, launch_time)); }

	/** \brief Adds a record to set when the event starts
	*** \param record_name The name of the record to set
	*** \param record_value The value of the record to set
	*** \param is_global If true, the record will be set to the global record group. Otherwise the local record group will be used.
	**/
	void _AddRecord(const std::string& record_name, int32 record_value, bool is_global);

	//! \brief Commits any stored records to the correct record group. Should only be called when _Start() is invoked
	void _CommitRecords()
		{ if (_event_records != nullptr) _event_records->CommitRecords(); }

}; // class MapEvent


/** ****************************************************************************
*** \brief Used to activate a new map state by pushing to the top of the state stack
***
*** This class exists simply because changing the map state is a very simple and
*** common operation. SCENE_STATE is the most frequently pushed state on maps. You must
*** be careful about which state you push, as certain states expect other properties to
*** be active when they are. For example, STATE_DIALOGUE assumes there is an active dialogue,
*** and simply pushing the state does not also activate a dialogue.
*** ***************************************************************************/
class PushMapStateEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param state The map state that should be activated when this event runs
	*** \return A pointer to the instance of the event created
	**/
	static PushMapStateEvent* Create(uint32 event_id, MAP_STATE state);

	//! \brief If invoked, the sprite pointed to by the map camera will cease moving when the event is started
	void StopCameraMovement()
		{ _stop_camera_movement = true; }

protected:
	PushMapStateEvent(uint32 event_id, MAP_STATE state) :
		MapEvent(event_id, PUSH_MAP_STATE_EVENT), _state(state), _stop_camera_movement(false) {}

	~PushMapStateEvent()
		{}

	//! \brief The new map state to push
	MAP_STATE _state;

	//! \brief Determines if the camera should stop any active movement when the event is started
	bool _stop_camera_movement;

	//! \brief Pushes the state to the map state stack
	void _Start();

	//! \brief No operation (always returns true)
	bool _Update()
		{ return true; }
}; // class PushMapStateEvent


/** ****************************************************************************
*** \brief Removes the active map state and restores the previous state
***
*** Like PushMapStateEvent, the purpose of this simple class is because removing
*** the active map state is a frequent and common operation that map scripts require.
*** The class makes no assumptions about the state stack when it runs, so it is entirely
*** up to the user to make sure that a desired and valid state is waiting below the
*** active state. If the map state stack is empty when the pop operation is called,
*** it will behave in the manner documented by MapMode::PopState().
*** ***************************************************************************/
class PopMapStateEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \return A pointer to the instance of the event created
	**/
	static PopMapStateEvent* Create(uint32 event_id);


protected:
	PopMapStateEvent(uint32 event_id) :
		MapEvent(event_id, POP_MAP_STATE_EVENT) {}

	~PopMapStateEvent()
		{}

	//! \brief Pops the top state from the map state stack
	void _Start();

	//! \brief No operation (always returns true)
	bool _Update()
		{ return true; }
}; // class PopMapStateEvent


/** ****************************************************************************
*** \brief Moves the map camera to focus on an object
***
*** Camera movement is a frequent operation done in map scripting. This class is used
*** to easily setup and create events that perform camera movement. For convenience,
*** the class also allows you to move to a specific X/Y position on the map by altering
*** the position of the map's virtual focus object to these coordinates and setting the
*** camera on the virtual focus.
***
*** Camera movement may be done instantly or over a period of time. If the movement
*** is not instant, the event will complete when the camera has finished moving.
*** ***************************************************************************/
class CameraMoveEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param focus The sprite that the camera should move its focus to
	*** \param move_time The number of milliseconds to spend moving the camera. Zero moves it instantly
	*** \return A pointer to the instance of the event created
	**/
	static CameraMoveEvent* Create(uint32 event_id, VirtualSprite* focus, uint32 move_time);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param x_position The x position to move the camera to
	*** \param y_position The y position to move the camera to
	*** \param move_time The number of milliseconds to spend moving the camera. Zero moves it instantly
	*** \return A pointer to the instance of the event created
	***
	*** \note This version of Create() automatically moves the map's virtual focus to the requested position
	*** and focuses the camera on the virtual focus. Don't call this function if you don't want to make any
	*** changes to the current state of the virtual focus.
	**/
	static CameraMoveEvent* Create(uint32 event_id, uint32 x_position, uint32 y_position, uint32 move_time);

	/** \brief Used to set the context of the sprite that the camera will point to during this event
	*** \param context The context to set the camera sprite
	**/
	void SetCameraContext(MAP_CONTEXT context)
		{ _camera_context = context; }

protected:
	CameraMoveEvent(uint32 event_id, VirtualSprite* focus, uint32 x_position, uint32 y_position, uint32 move_time);

	~CameraMoveEvent()
		{}

	//! \brief The object that the camera should focus on when it moves
	VirtualSprite* _focus;

	//! \brief The context to set the map camera to before beginning the move event. Unused if equal to MAP_CONTEXT_NONE
	MAP_CONTEXT _camera_context;

	/** \brief X/Y coordinates on the map where the virtual focus should be moved to
	*** \note This is only used when focus is nullptr
	**/
	uint32 _x_position, _y_position;

	//! \brief The number of milliseconds to finish the camera movement. If zero, the movement is instantaneous
	uint32 _move_time;

	//! \brief Begins the camera movement, and may also alter the position of the virtual focus
	void _Start();

	//! \brief Returns true once the camera movement has completed
	bool _Update();
}; // class CameraMoveEvent


/** ****************************************************************************
*** \brief An event which activates a dialogue on the map
***
*** Note that a dialogue may execute script actions, which would somewhat act
*** like events but technically are not events. Children events that are implemented
*** in Lua can take advantage of options selected by the player in these dialogues
*** to determine what events should follow down the event chain
***
*** Sometimes you may want a dialogue event to stop the camera from moving, especially
*** if it is the first event in an event chain. When this behavior is desired, call the
*** StopCameraMovement() method after creating the event object.
*** ***************************************************************************/
class DialogueEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param dialogue_id The ID of the dialogue to execute through this event
	*** \return A pointer to the instance of the event created
	**/
	static DialogueEvent* Create(uint32 event_id, uint32 dialogue_id);

	//! \brief Toggles whether or not camera movement should be stopped when the dialogue begins
	void SetStopCameraMovement(bool stop)
		{ _stop_camera_movement = stop; }

protected:
	DialogueEvent(uint32 event_id, uint32 dialogue_id);

	~DialogueEvent()
		{}

	//! \brief The ID of the dialogue to invoke
	uint32 _dialogue_id;

	//! \brief When true, any camera movement will be stopped when the event begins
	bool _stop_camera_movement;

	//! \brief Begins the dialogue
	void _Start();

	//! \brief Returns true when the last line of the dialogue has been read
	bool _Update();
}; // class DialogueEvent : public MapEvent


/** ****************************************************************************
*** \brief An event that creates an instance of ShopMode when started
***
*** \todo Several future shop mode features will likely need to be added to this
*** class. This includes limited availability of objects, market pricing, etc.
*** ***************************************************************************/
class ShopEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \return A pointer to the instance of the event created
	**/
	static ShopEvent* Create(uint32 event_id);

	/** \brief Adds a ware to the list of objects for sale
	*** \param object_id The ID of the GlobalObject to make available for purchase
	*** \param stock The amount of the object to make available for purchase
	*** \note All wares must be added before the _Start() method is called to ensure
	*** that the wares actually appear in shop mode.
	**/
	void AddWare(uint32 object_id, uint32 stock);

protected:
	//! \param event_id The ID of this event
	ShopEvent(uint32 event_id);

	~ShopEvent()
		{}

	//! \brief The GlobalObject IDs and stock count of all objects to be sold in the shop
	std::set<std::pair<uint32, uint32> > _wares;

	//! \brief Creates an instance of ShopMode and pushes it to the game mode stack
	void _Start();

	//! \brief Performs no operation (returns true)
	bool _Update();
}; // class ShopEvent : public MapEvent


/** ****************************************************************************
*** \brief Plays a sound. The event finishes when the sound stops
***
*** The suggested usage for initializing an object of this class is the following:
*** -# Call the class constructor
*** -# Call the GetSound() function to retrieve the SoundDescriptor object
*** -# Call SoundDescriptor methods to set the desired properties of the sound (looping, attenuation, etc)
***
*** After these steps are performed the event is ready to launch. The default properties
*** of the sound are the same as are in the default constructor of the SoundDescriptor
*** class. This includes no looping and no distance attenuation. The event will finish when the
*** sound finishes playing (when the sound state is AUDIO_STATE_STOPPED). Note that if looping is set
*** to infinite, the sound will never enter this state. It is possible to prematurely terminate this
*** event by calling the GetSound() method and invoking Stop() on the SoundDescriptor object that is
*** returned.
***
*** \note The MapMode class has a container of SoundDescriptor objects which should include all of
*** the sounds that may be used on a given map. This means that when a SoundEvent is created, the
*** sound file data will already be loaded by the audio engine.
***
*** \todo Support sounds with a position that employ distance attenuation. Perhaps
*** another derived class would be ideal to implement this, since sounds could possibly
*** be mobile (attached to sprites).
*** ***************************************************************************/
class SoundEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sound_filename The name of the sound file to load
	*** \return A pointer to the instance of the event created
	**/
	static SoundEvent* Create(uint32 event_id, std::string sound_filename);

	//! \brief Accessor which allows the properties of the sound to be customized
	hoa_audio::SoundDescriptor& GetSound()
		{ return _sound; }

protected:
	SoundEvent(uint32 event_id, std::string sound_filename);

	~SoundEvent();

	//! \brief The sound that this event will play
	hoa_audio::SoundDescriptor _sound;

	//! \brief Begins playback of the sound
	void _Start();

	//! \brief Returns true when the sound has finished playing, or finished looping
	bool _Update();
}; // class SoundEvent : public MapEvent


/** ****************************************************************************
*** \brief Transitions the game to a new map by fading the screen to black
***
*** When this event starts, it places the map in the state STATE_TRANSITION and
*** begins fading the screen to black. Once the screen fade is complete, it will
*** pop the current MapMode from the game stack, constructand push the new map,
*** and fade the screen back for the same amount of time. This means the new map
*** will begin when the screen is still fading back in.
***
*** By default the fade time used is MAP_FADE_OUT_TIME for both fading out and fading
*** in, and the default load point (0) will be used. These settings can both be changed
*** by the user.
*** ***************************************************************************/
class MapTransitionEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param filename The name of the map file to transition to
	*** \return A pointer to the instance of the event created
	**/
	static MapTransitionEvent* Create(uint32 event_id, std::string filename)
		{ return MapTransitionEvent::Create(event_id, filename, 0); }

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param filename The name of the map file to transition to
	*** \param load_point The load point to use for the map being transitioned to
	*** \return A pointer to the instance of the event created
	**/
	static MapTransitionEvent* Create(uint32 event_id, std::string filename, int32 load_point);

	/** \brief Allows the user to set a custom fade time
	*** \param fade_time The number of milliseconds to fade the map out. The same time is also used to fade the new map in.
	*** \note This function should not be called when the event is active. If so, a warning will be
	*** printed and no change will take place.
	**/
	void SetFadeTime(uint32 fade_time);

protected:
	MapTransitionEvent(uint32 event_id, std::string filneame, int32 load_point);

	~MapTransitionEvent()
		{}

	//! \brief The filename of the map to transition to
	std::string _transition_map_filename;

	//! \brief The load point for the transition map to use
	int32 _transition_map_load_point;

	//! \brief A timer used for fading out the current map
	hoa_system::SystemTimer _fade_timer;

	//! \brief Begins the transition process by fading out the screen and music
	void _Start();

	//! \brief Once the fading process completes, creates the new map mode to transition to
	bool _Update();
}; // class MapTransitionEvent : public MapEvent



/** ****************************************************************************
*** \brief Instantly starts a battle.
***
***
*** ***************************************************************************/
class BattleEncounterEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \return A pointer to the instance of the event created
	**/
	static BattleEncounterEvent* Create(uint32 event_id);

	void SetMusic(std::string filename);

	void SetBackground(std::string filename);

	void AddEnemy(uint32 enemy_id);

protected:
	BattleEncounterEvent(uint32 event_id);

	~BattleEncounterEvent();

	//! \brief ID numbers for enemies to generate
	std::vector<uint32> _enemy_ids;

	//! \brief Filename for battle music
	std::string _battle_music;

	//! \brief Filename for battle background
	std::string _battle_background;

	//! \brief Begins the transition to the battle
	void _Start();

	//! \brief Returns true once the map is no longer in the transition state
	bool _Update();
}; // class BattleEncounterEvent : public MapEvent


/** ****************************************************************************
*** \brief An event with its _Start() and _Update() functions implemented in Lua.
***
*** All events that do not fall into the other categories of events will be
*** implemented here. This event uses Lua functions to implement the _Start()
*** and _Update() functions (the C++ functions implemented here just call the
*** corresponding Lua functions). Note that any type of event can be implemented
*** in Lua, including alternative implementations of the other C++ event types.
*** You should use this event type only when the other event classes do not meet
*** your needs.
*** ***************************************************************************/
class CustomEvent : public MapEvent {
public:
	CustomEvent(const CustomEvent& copy);

	CustomEvent& operator=(const CustomEvent& copy);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param start_name The name of the start function to call
	*** \param update_name The name of the update function to call
	*** \return A pointer to the instance of the event created
	***
	*** \note Passing an empty string for either the start_name or update_name will result in
	*** no corresponding function defined. If no update function is defined, the call to _Update()
	*** will always return true, meaning that this event will end immediately after it starts.
	*** If both names are given empty string arguments, the event effectively does nothing and a
	*** warning message is printed out for this case.
	**/
	static CustomEvent* Create(uint32 event_id, std::string start_name, std::string update_name);

protected:
	CustomEvent(uint32 event_id, std::string start_name, std::string update_name);

	~CustomEvent();

	//! \brief A pointer to the Lua function that starts the event
	ScriptObject* _start_function;

	//! \brief A pointer to the Lua function that updates the status of an event, returning true when the event is finished
	ScriptObject* _update_function;

	//! \brief Calls the Lua _start_function if one was defined
	void _Start();

	//! \brief Calls the Lua _update_function. If no update function was defined, immediately returns true
	bool _Update();
}; // class CustomEvent : public MapEvent


/** ****************************************************************************
*** \brief Manages, processes, and launches map events
***
*** The EventSupervisor serves as an assistant to the MapMode class, much like the
*** other map supervisor classes. As such, this class is only instantiated as a member
*** of the MapMode class. The first responsibility of the EventSupervisor is to
*** retain all of the MapEvent objects that have been created. The second responsibility
*** of this class is to start and run events to their completion. Because events may be
*** linked together in a chain, starting one event may cause several other chains to start
*** as well. Each event can be thought of as the base of a n-length event chain, where n
*** is an integer greater than or equal to one.
***
*** When an event chain begins, the first (base) event of the chain is started.
*** Immediately after starting the first event, the supervisor will examine its event
*** links to determine which, if any, children events begin relative to the start of
*** the base event. If they are to start a certain time after the start of the parent
*** event, they are placed in a container and their countdown timers are initialized.
*** These timers will count down on every update call to the event manager and after
*** the timers expire, these events will be launched. When an active event ends, again
*** its event links are examined to determine if any children events exist that start
*** relative to the end of the parent event.
***
*** \todo What about the case when the same event is begun when the event is already
*** active? Should we prevent the case where an event is activated twice, print a
*** warning, or allow this situation and hope the programmer knows what they are doing?
***
*** In addition to supervisor events, this class also maintains a data log to track
*** various types of conditions that have occurred on the map. The data log is completely
*** separate from all event functionality, but is often used in combination with it. For example,
*** if the player has activated a switch three times, an event may be started. The data log
*** is a simple map container of string/integer pairs, and all stored data is permanently deleted
*** when the corresponding MapMode instance is destroyed.
*** ***************************************************************************/
class EventSupervisor {
public:
	EventSupervisor()
		{}

	~EventSupervisor();

	/** \brief Registers a map event object with the event supervisor
	*** \param new_event A pointer to the new event
	*** \note This function should be called for all events that are created
	**/
	void RegisterEvent(MapEvent* new_event);

	/** \brief Marks a specified event as active and immediately starts the event
	*** \param event_id The ID of the event to start
	**/
	void StartEvent(uint32 event_id);

	/** \brief Marks a specified event as active and immediately starts the event
	*** \param event A pointer to the event to begin
	**/
	void StartEvent(MapEvent* event);

	/** \brief Begins an event after a specified wait period expires
	*** \param event_id The ID of the event to activate
	*** \param wait_time The number of milliseconds to wait before starting the event
	*** \note Passing a zero value for wait_time will result in a warning message and start the
	*** event immediately. If you wish to start the event immediately, use the version of StartEvent
	*** that does not require a wait_time to be specified.
	**/
	void StartEvent(uint32 event_id, uint32 wait_time);

	/** \brief Begins an event after a specified wait period expires
	*** \param event A pointer to the event to start
	*** \param wait_time The number of milliseconds to wait before starting the event
	*** \note Passing a zero value for wait_time will result in a warning message and start the
	*** event immediately. If you wish to start the event immediately, use the version of StartEvent
	*** that does not require a wait_time to be specified.
	**/
	void StartEvent(MapEvent* event, uint32 wait_time);

	/** \brief Pauses an active event by preventing the event from updating
	*** \param event_id The ID of the active event to pause
	*** If the event corresponding to the ID is not active, a warning will be issued and no change
	*** will occur.
	**/
	void PauseEvent(uint32 event_id);

	/** \brief Resumes a pausd evend
	*** \param event_id The ID of the active event to resume
	*** If the event corresponding to the ID is not paused, a warning will be issued and no change
	*** will occur.
	**/
	void ResumeEvent(uint32 event_id);

	/** \brief Terminates an event if it is active
	*** \param event_id The ID of the event to terminate
	*** \note If there is no active event that corresponds to the event ID, the function will do nothing.
	*** \note This function will <b>not</b> terminate or prevent the launching of any of the event's children.
	*** \note Use of this function is atypical and should be avoided. Termination of certain events before their completion
	*** can lead to memory leaks, errors, and other problems. Make sure that the event you are terminating will not cause
	*** any of these conditions.
	**/
	void TerminateEvent(uint32 event_id);

	//! \brief Updates the state of all active and launch events
	void Update();

	/** \brief Determines if a chosen event is active
	*** \param event_id The ID of the event to check
	*** \return True if the event is active, false if it is not or the event could not be found
	**/
	bool IsEventActive(uint32 event_id) const;

	/** \brief Returns the number of times an event has been started
	*** \param event_id The ID of the event to check
	*** \return Zero if the event has never been started, otherwise the number of times it has been started
	*** \note If an invalid ID argument is passed, zero will be returned
	**/
	uint32 TimesEventStarted(uint32 event_id) const;

	//! \brief Returns true if any events are active
	bool HasActiveEvent() const
		{ return !_active_events.empty(); }

	//! \brief Returns true if any events are being prepared to be launched after their timers expire
	bool HasLaunchEvent() const
		{ return !_launch_events.empty(); }

	/** \brief Returns a pointer to a specified event stored by this class
	*** \param event_id The ID of the event to retrieve
	*** \return A MapEvent pointer (which may need to be casted to the proper event type), or nullptr if no event was found
	**/
	MapEvent* GetEvent(uint32 event_id) const;

private:
	//! \brief A container for all map events, where the event's ID serves as the key to the std::map
	std::map<uint32, MapEvent*> _all_events;

	//! \brief A list of all events which have started but are not yet finished
	std::list<MapEvent*> _active_events;

	/** \brief A list of all events that are waiting on their launch timers to expire before being started
	*** The interger part of this std::pair is the countdown timer for this event to be launched
	**/
	std::list<std::pair<int32, MapEvent*> > _launch_events;

	//! \brief A list of all events which have been paused
	std::list<MapEvent*> _paused_events;

	/** \brief Maintains a history of how many times each event has been started
	*** The first integer is the event ID and the second is how many times that event has been started.
	*** It does not track how many times the event has completed or been terminated.
	**/
	std::map<uint32, uint32> _event_history;

	/** \brief A function that is called whenever an event starts or finishes to examine that event's links
	*** \param parent_event The event that has just started or finished
	*** \param event_start The event has just started if this member is true, or if it just finished it will be false
	**/
	void _ExamineEventLinks(MapEvent* parent_event, bool event_start);
}; // class EventSupervisor

} // namespace private_map

} // namespace hoa_map
