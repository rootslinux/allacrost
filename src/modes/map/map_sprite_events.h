///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2017 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_sprite_events.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for map mode sprite events
***
*** Sprite events are a specialized form of a map event that affect the properties
*** of sprites. This could be something as simple as changing the direction that the
*** sprite is facing, or computing a path for the sprite to move and controlling
*** it as it moves along the path. They derive from MapEvent and are managed and
*** processed in the same way as all other types of events.
*** ***************************************************************************/

#pragma once

// Allacrost utilities
#include "defs.h"
#include "utils.h"

// Allacrost engines
#include "script.h"

// Local map mode headers
#include "map_events.h"
#include "map_utils.h"
#include "map_sprites.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief An abstract event class that represents an event controlling a sprite
***
*** Sprite events are special types of events that control a sprite (of any type)
*** on a map. Technically they are more like controllers than events, in that they
*** take control of a sprite and direct how its state should change, whether that
*** be their direction, movement, and/or display. All sprite events are connected
*** to one (and only one) sprite. When the event takes control over the sprite,
*** it notifies the sprite object which grabs a pointer to the SpriteEvent.
***
*** For a deriving class to be implemented properly, it must do two things.
*** # In the _Start method, call SpriteEvent::_Start() before any other code
*** # Before returning true in the _Update() method, call _sprite->ReleaseControl(this)
***
*** \note It is important to keep in mind that all map sprites have their update
*** function called before map events are updated. This can have implications for
*** changing some members of the sprite object inside the _Start() and _Update() methods
*** as these methods are called <i>after</i> the sprite's own Update() method. Keep
*** this property in mind when designing a derived sprite event class.
*** ***************************************************************************/
class SpriteEvent : public MapEvent {
protected:
	/** \param event_id The ID of this event
	*** \param event_type The type of this event
	*** \param sprite A pointer to the sprite that this event will control
	**/
	SpriteEvent(uint32 event_id, EVENT_TYPE event_type, VirtualSprite* sprite);

	virtual ~SpriteEvent()
		{}

	//! \brief A pointer to the map sprite that the event controls
	VirtualSprite* _sprite;

	//! \brief Acquires control of the sprite that the event will operate on
	virtual void _Start()
		{ _sprite->AcquireControl(this); }

	//! \brief Updates the state of the sprite and returns true if the event is finished
	virtual bool _Update() = 0;
}; // class SpriteEvent : public MapEvent


/** ****************************************************************************
*** \brief A simple event used to modify various properties of one or more sprites
***
*** During event sequences, it is frequently the case that we desire a change in the
*** properties of a sprite. For example, changing their direction to face a sound,
*** or to stop movement. This class serves as a means to make those instant changes
*** to a sprite's properties.
***
*** One unique aspect of this class is that it allows you to add more than one sprite,
*** and all sprites will be affected by the same property changes at the same time. This
*** means that you don't need to create a single event for each sprite, although if you want
*** the same properties to change but at different times, you'll need to create several events
*** to achieve that.
***
*** \note Some of the properties you can change with this event only affect MapSprite objects,
*** or those that derive from MapSprite. Check the methods for a note to see if the property
*** applies to all sprites, or only MapSprite and MapSprite-derived objects.
*** ***************************************************************************/
class ChangePropertySpriteEvent : public SpriteEvent {
protected:
	//! \brief Represent indexes into a bit vector of properties that are set to change
	enum PROPERTY_NAME {
		UPDATABLE           =  0,
		VISIBLE             =  1,
		COLLIDABLE          =  2,
		CONTEXT             =  3,
		POSITION            =  4,
		DIRECTION           =  5,
		MOVEMENTSPEED       =  6,
		MOVING              =  7,
		RUNNING             =  8,
		STATIONARYMOVEMENT  =  9,
		REVERSEMOVEMENT     = 10,
	};

public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite to enact the event on
	*** \return A pointer to the instance of the event created
	**/
	static ChangePropertySpriteEvent* Create(uint32 event_id, VirtualSprite* sprite);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite_id The ID of the sprite to enact the event on
	*** \return A pointer to the instance of the event created
	**/
	static ChangePropertySpriteEvent* Create(uint32 event_id, uint16 sprite_id);

	/** \brief Adds another sprite to have its properties modified by this event
	*** \param sprite A pointer to the sprite to update
	**/
	void AddSprite(VirtualSprite* sprite);

	/** \brief Indicates that position changes are relative to the sprite's current position
	*** By default, all position changes are in absolute coordinates on the map. Calling this function indicates that position changes
	*** are instead relative to the sprite's current position. You should call this function before calling Position(), otherwise
	***
	**/
	void PositionChangeRelative()
		{ _relative_position_change = true;}

	/** \brief Functions that set the property of the same name as the function.
	*** Once you call these functions, there's no way to "cancel" the change from occuring to that property.
	**/
	//@{
	void Updatable(bool updatable)
		{ _properties.set(UPDATABLE); _updatable = updatable; }

	void Visible(bool visible)
		{ _properties.set(VISIBLE); _visible = visible; }

	void Collidable(bool collidable)
		{ _properties.set(COLLIDABLE); _collidable = collidable; }

	void Context(MAP_CONTEXT context)
		{ _properties.set(CONTEXT); _context = context; }

	/** \note If you pass in a negative value to this function before PositionChangeRelative() is called, a warning will be printed
	*** and the negative values will be converted to be positive. This function also sets the x/y offsets to 0.0f.
	**/
	void Position(int16 x_position, int16 y_position)
		{ Position(x_position, 0.0f, y_position, 0.0f); }

	/** \note If you pass in a negative value to this function before PositionChangeRelative() is called, a warning will be printed
	*** and the negative values will be converted to be positive.
	**/
	void Position(int16 x_position, float x_offset, int16 y_position, float y_offset);

	void Direction(uint16 direction)
		{ _properties.set(DIRECTION); _direction = direction; }

	void MovementSpeed(float movement_speed)
		{ _properties.set(MOVEMENTSPEED); _movement_speed = movement_speed; }

	void Moving(bool moving)
		{ _properties.set(MOVING); _moving = moving; }

	void Running(bool running)
		{ _properties.set(RUNNING); _running = running; }

	//! \note This function will only apply to sprites that are not VirtualSprite types
	void StationaryMovement(bool stationary_movement)
		{ _properties.set(STATIONARYMOVEMENT); _stationary_movement = stationary_movement; }

	//! \note This function will only apply to sprites that are not VirtualSprite types
	void ReverseMovement(bool reverse_movement)
		{ _properties.set(REVERSEMOVEMENT); _reverse_movement = reverse_movement; }
	//@}


protected:
	ChangePropertySpriteEvent(uint32 event_id, VirtualSprite* sprite);

	~ChangePropertySpriteEvent()
		{}

	//! \brief The list of sprites that will be modified. Guaranteed to contain at least one sprite.
	std::vector<VirtualSprite*> _sprite_list;

	//! \brief A bit-mask used to identify which properties of a sprite should be updated
	std::bitset<16> _properties;

	//! \brief When true, positional changes will be relative to the sprite's current position
	bool _relative_position_change;

	//! \brief Idenitcally named to the properties found in the following classes: MapObject, VirtualSprite, MapSprite
	//@{
	bool _updatable;
	bool _visible;
	bool _collidable;
	MAP_CONTEXT _context;
	//! \note X/Y position are stored as signed integers here because they can be used for relative movement.
	int16 _x_position, _y_position;
	float _x_offset, _y_offset;
	uint16 _direction;
	float _movement_speed;
	bool _moving;
	bool _running;
	bool _stationary_movement;
	bool _reverse_movement;
	//@}

	//! \brief Sets the desired properties for all sprites
	void _Start();

	//! \brief Always returns true, immediately terminating the event
	bool _Update()
		{ return true; }
}; // class ChangePropertySpriteEvent : public SpriteEvent


/** ****************************************************************************
*** \brief Displays specific sprite frames for a certain period of time
***
*** This event displays a certain animation of a sprite for a specified amount of time.
*** Its primary purpose is to allow complete control over how a sprite appears to the
*** player and to show the sprite interacting with its surroundings, such as flipping
*** through a book taken from a bookshelf. Looping of these animations is also supported.
***
*** \note You <b>must</b> add at least one frame to this object
***
*** \note These actions can not be used with VirtualSprite objects, since this
*** class explicitly needs animation images to work and virtual sprites have no
*** images.
*** ***************************************************************************/
class AnimateSpriteEvent : public SpriteEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite_id The ID of the sprite to enact the event on
	*** \param sprite A pointer to the sprite to enact the event on
	*** \return A pointer to the instance of the event created
	**/
	static AnimateSpriteEvent* Create(uint32 event_id, VirtualSprite* sprite);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite_id The ID of the sprite to enact the event on
	*** \return A pointer to the instance of the event created
	**/
	static AnimateSpriteEvent* Create(uint32 event_id, uint16 sprite_id);

	/** \brief Adds a new frame to the animation set
	*** \param frame The index of the sprite's animations to display
	*** \param time The amount of time, in milliseconds, to display this frame
	**/
	void AddFrame(uint16 frame, uint32 time)
		{ _frames.push_back(frame); _frame_times.push_back(time); }

	/** \brief Sets the loop
	***
	**/
	void SetLoopCount(int32 count)
		{ _loop_count = count; }

protected:
	/** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite to move
	**/
	AnimateSpriteEvent(uint32 event_id, VirtualSprite* sprite);

	~AnimateSpriteEvent();

	//! \brief Index to the current frame to display from the frames vector
	uint32 _current_frame;

	//! \brief Used to count down the display time of the current frame
	uint32 _display_timer;

	//! \brief A counter for the number of animation loops that have been performed
	int32 _loop_count;

	/** \brief The number of times to loop the display of the frame set before finishing
	*** A value less than zero indicates to loop forever. Be careful with this,
	*** because that means that the action would never arrive at the "finished"
	*** state.
	***
	*** \note The default value of this member is zero, which indicates that the
	*** animations will not be looped (they will run exactly once to completion).
	**/
	int32 _number_loops;

	/** \brief Holds the sprite animations to display for this action
	*** The values contained here are indeces to the sprite's animations vector
	**/
	std::vector<uint16> _frames;

	/** \brief Indicates how long to display each frame
	*** The size of this vector should be equal to the size of the frames vector
	**/
	std::vector<uint32> _frame_times;

	//! \brief Calculates a path for the sprite to move to the destination
	void _Start();

	//! \brief Returns true when the sprite has reached the destination
	bool _Update();
}; // class AnimateSpriteEvent : public SpriteEvent


/** ****************************************************************************
*** \brief An event which randomizes movement of a sprite
*** ***************************************************************************/
class RandomMoveSpriteEvent : public SpriteEvent {
	friend class VirtualSprite;

public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite to enact the event on
	*** \param move_time The total amount of time that this event should take
	*** \param direction_time The amount of time to wait before changing the sprite's direction randomly
	*** \return A pointer to the instance of the event created
	**/
	static RandomMoveSpriteEvent* Create(uint32 event_id, VirtualSprite* sprite, uint32 move_time, uint32 direction_time);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite_id The ID of the sprite to enact the event on
	*** \param move_time The total amount of time that this event should take
	*** \param direction_time The amount of time to wait before changing the sprite's direction randomly
	*** \return A pointer to the instance of the event created
	**/
	static RandomMoveSpriteEvent* Create(uint32 event_id, uint16 sprite_id, uint32 move_time, uint32 direction_time);

protected:
	RandomMoveSpriteEvent(uint32 event_id, VirtualSprite* sprite, uint32 move_time, uint32 direction_time);

	~RandomMoveSpriteEvent();

	/** \brief The amount of time (in milliseconds) to perform random movement before ending this action
	*** Set this member to hoa_system::INFINITE_TIME in order to continue the random movement
	*** forever. The default value of this member will be set to 10 seconds if it is not specified.
	**/
	uint32 _total_movement_time;

	/** \brief The amount of time (in milliseconds) that the sprite should continue moving in its current direction
	*** The default value for this timer is 1.5 seconds (1500ms).
	**/
	uint32 _total_direction_time;

	//! \brief A timer which keeps track of how long the sprite has been in random movement
	uint32 _movement_timer;

	//! \brief A timer which keeps track of how long the sprite has been moving around since the last change in direction.
	uint32 _direction_timer;

	//! \brief Calculates a path for the sprite to move to the destination
	void _Start();

	//! \brief Returns true when the sprite has reached the destination
	bool _Update();

	/** \brief Tries to adjust the sprite's position around the collision. Will randomally change the sprite's direction if that fails.
	*** \param coll_type The type of collision that has occurred
	*** \param coll_obj A pointer to the MapObject that the sprite has collided with, if any
	**/
	void _ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj);
}; // class RandomMoveSpriteEvent : public SpriteEvent


/** ****************************************************************************
*** \brief An event which moves a sprite to a destination
***
*** This class allows for both absolute and relative destinations. Absolute destinations
*** are defined by specifying an X,Y coordinate on the map to move the sprite to. A relative
*** destination is the change in the X and Y directions to move the sprite from their current
*** position. The default destination type is absolute.
***
*** Using event linking, it is very simple to create an event chain where a sprite
*** travels between multiple destinations, or multiple sprites travel to multiple
*** destinations.
*** ***************************************************************************/
class PathMoveSpriteEvent : public SpriteEvent {
	friend class VirtualSprite;

public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite to move
	*** \param x_coord The X coordinate to move the sprite to
	*** \param y_coord The Y coordinate to move the sprite to
	*** \return A pointer to the instance of the event created
	**/
	static PathMoveSpriteEvent* Create(uint32 event_id, VirtualSprite* sprite, int16 x_coord, int16 y_coord);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite_id The ID of the sprite that is to be moved
	*** \param x_coord The X coordinate to move the sprite to
	*** \param y_coord The Y coordinate to move the sprite to
	*** \return A pointer to the instance of the event created
	**/
	static PathMoveSpriteEvent* Create(uint32 event_id, uint16 sprite_id, int16 x_coord, int16 y_coord);

	/** \brief Used to toggle whether or not the destination provided in the constructor is relative or absolute
	*** \note Any previous existing paths are cleared when this function is called. If this function is called when
	*** the event is active, no change will take place.
	**/
	void SetRelativeDestination(bool relative);

	/** \brief Used to change the destination coordinates after the class object has been constructed
	*** \param x_coord The X coordinate to move the sprite to
	*** \param y_coord The Y coordinate to move the sprite to
	*** \note Any previous existing paths are cleared when this function is called. If this function is called when
	*** the event is active, no change will take place.
	**/
	void SetDestination(int16 x_coord, int16 y_coord);

	/** \brief Optionally indicates the facing direction to set for the sprite after the movement is complete
	*** \note The only directions you should set in the class constructor are: NORTH, SOUTH, EAST, and WEST.
	*** The other types of directions (which also infer movement) are unnecessary. Using a direction other than
	*** these four will result in a warning being printed.
	**/
	void SetFinalDirection(uint16 direction);

protected:
	PathMoveSpriteEvent(uint32 event_id, VirtualSprite* sprite, int16 x_coord, int16 y_coord);

	~PathMoveSpriteEvent()
		{}

	//! \brief When true, the destination coordinates are relative to the current position of the sprite. Otherwise the destination is absolute.
	bool _relative_destination;

	//! \brief Stores the source coordinates for the path movement (the sprite's position when the event is started).
	int16 _source_col, _source_row;

	//! \brief Stores the destination coordinates for the path movement. These may be either absolute or relative coordinates.
	int16 _destination_col, _destination_row;

	//! \brief Used to store the previous coordinates of the sprite during path movement, so as to set the proper direction of the sprite as it moves
	uint16 _last_x_position, _last_y_position;

	//! \brief Allows the ability of the event to optionally set the direction that the sprite should face after completing the path movement
	uint16 _final_direction;

	//! \brief An index to the path vector containing the node that the sprite currently occupies
	uint32 _current_node;

	//! \brief Holds the final destination coordinates for the path movement
	PathNode _destination_node;

	//! \brief Holds the path needed to traverse from source to destination
	std::vector<PathNode> _path;

	//! \brief Calculates a path for the sprite to move to the destination
	void _Start();

	//! \brief Returns true when the sprite has reached the destination
	bool _Update();

	//! \brief Sets the correct direction for the sprite to move to the next node in the path
	void _SetSpriteDirection();

	/** \brief Determines an appropriate resolution when the sprite collides with an obstruction
	*** \param coll_type The type of collision that has occurred
	*** \param coll_obj A pointer to the MapObject that the sprite has collided with, if any
	**/
	void _ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj);
}; // class PathMoveSpriteEvent : public SpriteEvent


/** ****************************************************************************
*** \brief A custom event which operates on a sprite
***
*** This class is a cross between a SpriteEvent and CustomEvent class. The class
*** inherits from SpriteEvent, but not from CustomEvent. The key feature of this
*** class is that it passes a pointer to a VirtualSprite object in the argument
*** list when it makes its Lua function calls. The Lua functions are then able
*** to take any allowable action on the sprite object. Otherwise, this class
*** behaves just like a standard CustomEvent class.
*** ***************************************************************************/
class CustomSpriteEvent : public SpriteEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite that will be passed to the Lua script functions
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
	static CustomSpriteEvent* Create(uint32 event_id, VirtualSprite* sprite, std::string start_name, std::string update_name);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite_id The id of the sprite that will be passed to the Lua script functions
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
	static CustomSpriteEvent* Create(uint32 event_id, uint16 sprite_id, std::string start_name, std::string update_name);

	CustomSpriteEvent(const CustomSpriteEvent& copy);

	CustomSpriteEvent& operator=(const CustomSpriteEvent& copy);

protected:
	CustomSpriteEvent(uint32 event_id, VirtualSprite* sprite, std::string start_name, std::string check_name);

	~CustomSpriteEvent();

	//! \brief A pointer to the Lua function that starts the event
	ScriptObject* _start_function;

	//! \brief A pointer to the Lua function that returns a boolean value if the event is finished
	ScriptObject* _update_function;

	//! \brief Calls the Lua _start_function, if one was defined
	void _Start();

	//! \brief Calls the Lua _update_function. If no update function was defined, does nothing and returns true
	bool _Update();
}; // class CustomSpriteEvent : public SpriteEvent

} // namespace private_map

} // namespace hoa_map
