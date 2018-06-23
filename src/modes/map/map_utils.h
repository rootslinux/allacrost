///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_utils.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for map mode utility code
***
*** This code contains constants, enums, functions, and small classes that do not
*** necessarily fit under the core functionality of other map mode files, or are
*** shared between different sets of files. Likely all other map code will need to
*** include this file, but this file should never include other map headers.
***  Some things you can find here include:
***
*** - Assistant functionality to draw code
*** - Map transition managers
*** - Path finding
*** - Record data
*** - Notification defintions
*** *****************************************************************************/

#pragma once

// Allacrost utilities
#include "utils.h"
#include "defs.h"

// Allacrost engines
#include "mode_manager.h"
#include "notification.h"
#include "system.h"
#include "video.h"

namespace hoa_map {

//! Determines whether the code in the hoa_map namespace should print debug statements or not.
extern bool MAP_DEBUG;

namespace private_map {

/** \name Screen Coordiante System Constants
*** \brief Represents the size of the visible screen in map tiles and the collision grid
*** Every map tile is 32x32 pixels, and every collision grid element is one quarter of that
*** area (16x16). Thus the number of collision grid elements that compose the screen are
*** four times the number of tiles that are visible on the screen. This also means the number
*** of rows and columns of grid elements that encompass the screen are twice that of the
*** number of rows and columns of tiles.
**/
//@{
const float SCREEN_COLS = 32.0f;
const float SCREEN_ROWS = 24.0f;
const float HALF_SCREEN_COLS = SCREEN_COLS / 2;
const float HALF_SCREEN_ROWS = SCREEN_ROWS / 2;

const uint16 TILE_COLS = 16; // Number of tile columns that fit on the screen
const uint16 TILE_ROWS = 12; // Number of tile rows that fit on the screen
const uint16 HALF_TILE_COLS = TILE_COLS / 2;
const uint16 HALF_TILE_ROWS = TILE_ROWS / 2;

const uint16 GRID_LENGTH = 32; // Length of a grid element in pixels
const uint16 TILE_LENGTH = 64; // Length of a tile in pixels
const uint16 HALF_TILE_LENGTH = TILE_LENGTH / 2;
//@}

//! \brief The number of tiles that are found in a tileset image (512x512 pixel image containing 32x32 pixel tiles)
const uint32 TILES_PER_TILESET = 256;

//! \brief Indicates that no image is referenced at this tile location
const int32 UNREFERENCED_TILE = -1;

//! \brief Indicates that the tile drawn at this location should be the corresponding tile from the inhertiting context
const int32 INHERITED_TILE = -2;


/** \name Map State Enum
*** \brief Represents the current state of operation during map mode.
**/
//@{
enum MAP_STATE {
	STATE_INVALID          = 0,
	STATE_EXPLORE          = 1, //!< Standard state, player has control to move about the map
	STATE_SCENE            = 2, //!< Like the explore state but player has no control (input is ignored)
	STATE_DIALOGUE         = 3, //!< When a dialogue is active
	STATE_TREASURE         = 4, //!< Active when a treasure has been procured by the player
	STATE_TRANSITION       = 5, //!< State for when transitioning mode
	STATE_TOTAL            = 6
};
//@}


/** \name Map Context Constants
*** \brief Constants used to represent all 32 possible map contexts
***
*** Note that only one bit is set for each context. This is done so that the collision
*** grid for all contexts can be stored in a single integer. This also simplifies the
*** complexity of collision detection for map sprites.
***
*** \note Ideally these names would omit the "MAP_" affix. However, doing so creates some naming conflicts
*** with 64-bit versions of Windows so we cannot. In Lua though, we bind these names without the "MAP_" since
*** no conflict exists in the Lua environment.
**/
enum MAP_CONTEXT {
	MAP_CONTEXT_NONE  = 0x00000000,
	MAP_CONTEXT_01    = 0x00000001, // Also known as the base context
	MAP_CONTEXT_02    = 0x00000002,
	MAP_CONTEXT_03    = 0x00000004,
	MAP_CONTEXT_04    = 0x00000008,
	MAP_CONTEXT_05    = 0x00000010,
	MAP_CONTEXT_06    = 0x00000020,
	MAP_CONTEXT_07    = 0x00000040,
	MAP_CONTEXT_08    = 0x00000080,
	MAP_CONTEXT_09    = 0x00000100,
	MAP_CONTEXT_10    = 0x00000200,
	MAP_CONTEXT_11    = 0x00000400,
	MAP_CONTEXT_12    = 0x00000800,
	MAP_CONTEXT_13    = 0x00001000,
	MAP_CONTEXT_14    = 0x00002000,
	MAP_CONTEXT_15    = 0x00004000,
	MAP_CONTEXT_16    = 0x00008000,
	MAP_CONTEXT_17    = 0x00010000,
	MAP_CONTEXT_18    = 0x00020000,
	MAP_CONTEXT_19    = 0x00040000,
	MAP_CONTEXT_20    = 0x00080000,
	MAP_CONTEXT_21    = 0x00100000,
	MAP_CONTEXT_22    = 0x00200000,
	MAP_CONTEXT_23    = 0x00400000,
	MAP_CONTEXT_24    = 0x00800000,
	MAP_CONTEXT_25    = 0x01000000,
	MAP_CONTEXT_26    = 0x02000000,
	MAP_CONTEXT_27    = 0x04000000,
	MAP_CONTEXT_28    = 0x08000000,
	MAP_CONTEXT_29    = 0x10000000,
	MAP_CONTEXT_30    = 0x20000000,
	MAP_CONTEXT_31    = 0x40000000,
	MAP_CONTEXT_32    = 0x80000000,
	MAP_CONTEXT_ALL   = 0xFFFFFFFF
};

/** \name Map Context Transition Type Constants
*** \brief Constants that represent the various types of transition between map contexts that can occur
***
***
**/
enum MAP_CONTEXT_TRANSITION_TYPE {
	TRANSITION_NONE      = 0, // transition is instaneous, so therefore no transition takes place
	TRANSITION_BLEND     = 1, // both contexts are drawn on top of each other, and one fades to the other
	TRANSITION_COLOR     = 2  // original context does a screen fade to a color, then fades back to the new context
};

/** \name Map Zone Types
*** \brief Identifier types for the various classes of map zones
***
*** \todo This enum is currently not in use by zone classes. Evaluate whether or not such a type identifier enum
*** is necessary and either add them to the zone classes or remove this enum.
**/
enum ZONE_TYPE {
	ZONE_INVALID    = 0,
	ZONE_MAP        = 1,
	ZONE_CAMERA     = 2,
	ZONE_RESIDENT   = 3,
	ZONE_ENEMY      = 4,
	ZONE_CONTEXT    = 5,
	MAP_ZONE_TOTAL  = 6
};

//! \brief Used to identify the type of map object
enum MAP_OBJECT_TYPE {
	PHYSICAL_TYPE          = 0,
	VIRTUAL_TYPE           = 1,
	SPRITE_TYPE            = 2,
	ENEMY_TYPE             = 3,
	MAP_TREASURE_TYPE      = 4,
	GLIMMER_TREASURE_TYPE  = 5,
};


/** \name Map Sprite Speeds
*** \brief Common speeds for sprite movement.
*** These values are the time (in milliseconds) that it takes a sprite to walk
*** the distance of one map grid (16 pixels).
**/
//@{
const float VERY_SLOW_SPEED  = 225.0f;
const float SLOW_SPEED       = 190.0f;
const float NORMAL_SPEED     = 150.0f;
const float FAST_SPEED       = 110.0f;
const float VERY_FAST_SPEED  = 75.0f;
//@}


/** \name Sprite Direction Constants
*** \brief Constants used for determining sprite directions
*** Sprites are allowed to travel in eight different directions, however the sprite itself
*** can only be facing one of four ways: north, south, east, or west. Because of this, it
*** is possible to travel, for instance, northwest facing north <i>or</i> northwest facing west.
*** The "NW_NORTH" constant means that the sprite is traveling to the northwest and is
*** facing towards the north.
***
*** \note These constants include a series of shorthands (MOVING_NORTHWEST, FACING_NORTH) used
*** to check for movement and facing directions.
**/
//@{
const uint16 NORTH     = 0x0001;
const uint16 SOUTH     = 0x0002;
const uint16 WEST      = 0x0004;
const uint16 EAST      = 0x0008;
const uint16 NW_NORTH  = 0x0010;
const uint16 NW_WEST   = 0x0020;
const uint16 NE_NORTH  = 0x0040;
const uint16 NE_EAST   = 0x0080;
const uint16 SW_SOUTH  = 0x0100;
const uint16 SW_WEST   = 0x0200;
const uint16 SE_SOUTH  = 0x0400;
const uint16 SE_EAST   = 0x0800;
// Used to check for movement direction regardless of facing direction
const uint16 MOVING_NORTHWARD = NORTH | NW_NORTH | NW_WEST | NE_NORTH | NE_EAST;
const uint16 MOVING_SOUTHWARD = SOUTH | SW_SOUTH | SW_WEST | SE_SOUTH | SE_EAST;
const uint16 MOVING_EASTWARD = EAST | NE_EAST | NE_NORTH | SE_EAST | SE_SOUTH;
const uint16 MOVING_WESTWARD = WEST | NW_WEST | NW_NORTH | SW_WEST | SW_SOUTH;
const uint16 MOVING_NORTHWEST = NW_NORTH | NW_WEST;
const uint16 MOVING_NORTHEAST = NE_NORTH | NE_EAST;
const uint16 MOVING_SOUTHWEST = SW_SOUTH | SW_WEST;
const uint16 MOVING_SOUTHEAST = SE_SOUTH | SE_EAST;
const uint16 MOVING_ORTHOGONALLY = NORTH | SOUTH | EAST | WEST;
const uint16 MOVING_DIAGONALLY = MOVING_NORTHWEST | MOVING_NORTHEAST | MOVING_SOUTHWEST | MOVING_SOUTHEAST;
// Used to check for facing direction regardless of moving direction
const uint16 FACING_NORTH = NORTH | NW_NORTH | NE_NORTH;
const uint16 FACING_SOUTH = SOUTH | SW_SOUTH | SE_SOUTH;
const uint16 FACING_WEST = WEST | NW_WEST | SW_WEST;
const uint16 FACING_EAST = EAST | NE_EAST | SE_EAST;
//@}


/** \name Map Sprite Animation Constants
*** These constants are used to index the MapSprite#animations vector to display the correct
*** animation. The first 8 entries in this vector always represent the same sets of animations
*** for each map sprite. Not all sprites have running animations, so the next 4 entries in the
*** sprite's animation vector are not necessarily running animations.
**/
//@{
const uint32 ANIM_STANDING_SOUTH = 0;
const uint32 ANIM_STANDING_NORTH = 1;
const uint32 ANIM_STANDING_WEST  = 2;
const uint32 ANIM_STANDING_EAST  = 3;
const uint32 ANIM_WALKING_SOUTH  = 4;
const uint32 ANIM_WALKING_NORTH  = 5;
const uint32 ANIM_WALKING_WEST   = 6;
const uint32 ANIM_WALKING_EAST   = 7;
const uint32 ANIM_RUNNING_SOUTH  = 8;
const uint32 ANIM_RUNNING_NORTH  = 9;
const uint32 ANIM_RUNNING_WEST   = 10;
const uint32 ANIM_RUNNING_EAST   = 11;
const uint32 ANIM_ATTACKING_EAST = 12;
//@}


//! \brief Represents the various types of collisions which may occur for a sprite
enum COLLISION_TYPE {
	NO_COLLISION        = 0, //!< Indicates that no collision has occurred
	BOUNDARY_COLLISION  = 1, //!< Happens when the sprite attempts to move outside any of the map's boundaries
	GRID_COLLISION      = 2, //!< Condition when the sprite's collision rectangle overlaps an invalid element of the map's collision grid
	OBJECT_COLLISION    = 3, //!< Occurs when the sprite collides with another map object in the same object layer
};


//! \brief Identifiers for the similarly named classes of map events
enum EVENT_TYPE {
	INVALID_EVENT                   = 0,
	PUSH_MAP_STATE_EVENT            = 1,
	POP_MAP_STATE_EVENT             = 2,
	CAMERA_MOVE_EVENT               = 3,
	DIALOGUE_EVENT                  = 4,
	SHOP_EVENT                      = 5,
	SOUND_EVENT                     = 6,
	MAP_TRANSITION_EVENT            = 7,
	BATTLE_ENCOUNTER_EVENT          = 8,
	SCRIPTED_EVENT                  = 9,
	SCRIPTED_SPRITE_EVENT           = 10,
	CHANGE_PROPERTY_SPRITE_EVENT    = 11,
	CHANGE_DIRECTION_SPRITE_EVENT   = 12,
	PATH_MOVE_SPRITE_EVENT          = 13,
	RANDOM_MOVE_SPRITE_EVENT        = 14,
	ANIMATE_SPRITE_EVENT            = 15,
	TOTAL_EVENT                     = 16
};


//! \brief Defines the different states the dialogue can be in.
enum DIALOGUE_STATE {
	DIALOGUE_STATE_INACTIVE =  0, //!< Active when the dialogue window is in the process of displaying a line of text
	DIALOGUE_STATE_LINE     =  1, //!< Active when the dialogue window is in the process of displaying a line of text
	DIALOGUE_STATE_OPTION   =  2, //!< Active when player-selectable options are present in the dialogue window
};


//! \brief The maximum number of options that a line of dialogue can present to the player
const uint32 MAX_DIALOGUE_OPTIONS = 5;

//! \brief The number of milliseconds to take to fade out the map
const uint32 MAP_FADE_OUT_TIME = 2000;

//! \brief The standard number of milliseconds it takes for enemies to spawn in an enemy zone
const uint32 STANDARD_ENEMY_SPAWN_TIME = 3000;

//! \brief The stamina counter amounts when the guage is either empty or full
const uint32 STAMINA_EMPTY  = 0;
const uint32 STAMINA_FULL   = 10000;

//! \brief The only layer ID for both tile layers and object layers that is guaranteed to exist
const uint32 DEFAULT_LAYER_ID = 0;

//! \brief The default time to wait before enemies spawn on a map
const uint32 DEFAULT_ENEMY_SPAWN_TIME = 30000;

//! \brief Sprite ID for when dialogue has no speaker
const uint32 NO_SPRITE = 0;


/** \brief Returns the opposite facing direction of the direction given in parameter.
*** \return A direction that faces opposite to the argument direction
*** \note This is mostly used as an helper function to make sprites face each other in a conversation.
**/
uint16 CalculateOppositeDirection(const uint16 direction);


/** \brief Returns a string representation of a map state, useful in debugging
*** \param state The map state to use
*** \return The name of the state in string format (eg: STATE_DIALOGUE returns "dialogue")
**/
std::string DEBUG_MapStateName(MAP_STATE state);


/** \brief Returns a string representation of a map event's type, useful in debugging
*** \param type The map event type to use
*** \return The name of the event type in string format (eg: PUSH_MAP_STATE_EVENT returns "push map state")
**/
std::string DEBUG_EventTypeName(EVENT_TYPE type);


/** ****************************************************************************
*** \brief Represents a rectangular section of a map
***
*** This is a small class that is used to represent rectangular map areas. These
*** areas are used very frequently throughout the map code to check for collision
*** detection, determining objects that are within a certain radius of one
*** another, etc.
*** ***************************************************************************/
class MapRectangle {
public:
	MapRectangle() :
		left(0.0f), right(0.0f), top(0.0f), bottom(0.0f)
		{}

	MapRectangle(float l, float r, float t, float b) :
		left(l), right(r), top(t), bottom(b)
		{}

	//! \brief The four edges of the rectangle's area
	float left, right, top, bottom;

	/** \brief Determines if two rectangle objects intersect with one another
	*** \param first A reference to the first rectangle object
	*** \param second A reference to the second rectangle object
	*** \return True if the two rectangles intersect at any location
	***
	*** This function assumes that the rectangle objects hold map collision grid
	*** coordinates, where the top of the rectangle is a smaller number than the
	*** bottom of the rectangle and the left is a smaller number than the right.
	**/
	static bool CheckIntersection(const MapRectangle& first, const MapRectangle& second);
}; // class MapRectangle


/** ****************************************************************************
*** \brief Retains information about how the next map frame should be drawn.
***
*** This class is used by the MapMode class to determine how the next map frame
*** should be drawn. This includes which tiles will be visible and the offset
*** coordinates for the screen. Map objects also use this information to determine
*** where (and if) they should be drawn.
***
*** \note The MapMode class keeps an active object of this class with the latest
*** information about the map. It should be the only instance of this class that is
*** needed.
*** ***************************************************************************/
class MapFrame {
public:
	//! \brief The column and row indeces of the starting tile to draw (the top-left tile).
	int16 starting_col, starting_row;

	//! \brief The number of columns and rows of tiles to draw on the screen.
	uint8 num_draw_cols, num_draw_rows;

	//! \brief The x and y position screen coordinates to start drawing tiles from.
	float tile_x_start, tile_y_start;

	/** \brief The position coordinates of the screen edges.
	*** These members are in terms of the map grid 16x16 pixel coordinates that map objects use.
	*** The presense of these coordinates make it easier for map objects to figure out whether or
	*** not they should be drawn on the screen. Note that these are <b>not</b> used as drawing
	*** cursor positions, but rather are map grid coordinates indicating where the screen edges lie.
	**/
	MapRectangle screen_edges;
}; // class MapFrame


/** ****************************************************************************
*** \brief Abstract representation of a layer of visible items on a map
***
*** This simple abstract class is used to maintain the order of drawable layers of tiles and objects on the map.
*** Its sole purpose is to allow for an ordered container to hold pointers to the different layer types.
*** ***************************************************************************/
class MapLayer {
public:
	//! \brief Updates all the objects on this layer (animations, position, etc.)
	virtual void Update() = 0;

	/** \brief Draws the layer to the screen
	*** \param context Only elements matching this context will be drawn
	**/
	virtual void Draw(MAP_CONTEXT context) const = 0;
}; // class MapLayer


/** ****************************************************************************
*** \brief A container class for node information in pathfinding.
***
*** This class is used in the MapMode#_FindPath function to find an optimal
*** path from a given source to a destination. The path finding algorithm
*** employed is A* and thus many members of this class are particular to the
*** implementation of that algorithm.
*** ***************************************************************************/
class PathNode {
public:
	/** \brief The grid coordinates for this node
	*** These coordinates correspond to the collision grid, where each element
	*** is a 16x16 pixel space on the map.
	**/
	int16 row, col;

	//! \name Path Scoring Members
	//@{
	//! \brief The total score for this node (f = g + h).
	int16 f_score;

	//! \brief The score for this node relative to the source.
	int16 g_score;

	//! \brief The Manhattan distance from this node to the destination.
	int16 h_score;
	//@}

	//! \brief The grid coordinates for the parent of this node
	int16 parent_row, parent_col;

	// ---------- Methods

	PathNode() : row(-1), col(-1), f_score(0), g_score(0), h_score(0), parent_row(0), parent_col(0)
		{}

	PathNode(int16 r, int16 c) : row(r), col(c), f_score(0), g_score(0), h_score(0), parent_row(0), parent_col(0)
		{}

	//! \brief Overloaded comparison operator checks that row and col members are equal
	bool operator==(const PathNode& that) const
		{ return ((this->row == that.row) && (this->col == that.col)); }

	//! \brief Overloaded comparison operator checks that row or col members are unequal
	bool operator!=(const PathNode& that) const
		{ return ((this->row != that.row) || (this->col != that.col)); }

	//! \brief Overloaded comparison operator only used for path finding, compares the two f_scores
	bool operator<(const PathNode& that) const
		{ return this->f_score > that.f_score; }
}; // class PathNode


/** ****************************************************************************
*** \brief A simple class used for holding data to be set into either the global or local map records
***
*** This class is used by code that wants to set records only after a certain action occurs. For example,
*** when the player chooses a particular option in a dialogue, or a map event is started.
*** \note The CommonRecordGroups that are modified by this data are members of the current MapMode instance,
*** named _global_record_group and _local_record_group.
*** ***************************************************************************/
class MapRecordData {
public:
	MapRecordData() :
		_global_records(), _local_records() {}

	/** \brief Adds a new record to set for the global map record group
	*** \param record_name The name of the record to set
	*** \param record_value The value to set for the record
	**/
	void AddGlobalRecord(const std::string& record_name, int32 record_value)
		{ _global_records.push_back(std::make_pair(record_name, record_value)); }

	/** \brief Adds a new record to set for the local map record group
	*** \param record_name The name of the record to set
	*** \param record_value The value to set for the record
	**/
	void AddLocalRecord(const std::string& record_name, int32 record_value)
		{ _local_records.push_back(std::make_pair(record_name, record_value)); }

	//! \brief Sets the global and/or local records into their corresponding map record groups
	void CommitRecords();

private:
	//! \brief A list of string/integer pairs to set for the map's global record
	std::vector<std::pair<std::string, int32> > _global_records;

	//! \brief A list of string/integer pairs to set for the map's local record
	std::vector<std::pair<std::string, int32> > _local_records;
}; // class MapRecordData


/** ****************************************************************************
*** \brief A simple class used for holding data related to launching map events
***
*** This class stores a list of events to start and any time delay to wait before actually starting
*** the event. Additionally, a boolean is provided to mimic the fact that events started by other
*** events can be started at the same time as the parent event, or after the parent event completes.
*** So if there is a code construct with a beginning and an end point (say, displaying a line of dialogue),
*** then this boolean can be used to start the event at the same time as the dialogue, or when the dialogue
*** ends.
*** ***************************************************************************/
class MapEventData {
public:
	MapEventData()
		{}

	/** \brief Adds a new set of event data to the class
	*** \param event_id The ID of the event to add (must be non-zero)
	*** \param start_timing The number of milliseconds to wait before starting the event. Default value is zero.
	*** \param launch_at_start Sets the launch boolean for determining under which conditions the event should be started. Default value is true.
	**/
	void AddEvent(uint32 event_id, uint32 start_timing = 0, bool launch_at_start = true);

	/** \brief Instructs the event supervisor to start the events referenced by the data
	*** \param launch_start Only events with the _launch_start property matching this value will be started
	**/
	void StartEvents(bool launch_start) const;

	/** \brief Examines all event ids to check that a corresponding event has already been constructed and registered with the event manager
	*** \return True if no invalid events were found
	**/
	bool ValidateEvents() const;

private:
	//! \brief The ids for each MapEvent referenced by the data
	std::vector<uint32> _event_ids;

	/** \brief The number of milliseconds to delay before the event actually starts (handled by the EventSupervisor call).
	*** A zero value will start the event immediately.
	**/
	std::vector<uint32> _start_timings;

	//! \brief Used in conjunction with the StartEvents() method to only start events that matching a boolean value
	std::vector<bool> _launch_start;
}; // class MapEventData


/** ****************************************************************************
*** \brief A notification event class describing sprite collisions
***
*** Whenever a sprite of any type moves on the map and has a collision, one of these
*** notification events is generated to describe the type and particulars about the
*** collision. This can be used by a map script to determine whether to play a sound,
*** switch the context of the sprite, or take some other action.
***
*** \note Because collision resolution changes the position of the sprite, you can
*** not rely on the position of the sprite when the notification event is being processed.
*** This is why this class has members that retain the position of the sprite as the collision
*** happened.
*** ***************************************************************************/
class MapCollisionNotificationEvent : public hoa_notification::NotificationEvent {
public:
	/** \param type The type of collision that occurred
	*** \param sprite The sprite that had the collision
	*** \note You should \b not use this constructor for object-type collisions
	**/
	MapCollisionNotificationEvent(COLLISION_TYPE type, VirtualSprite* sprite) :
		NotificationEvent("map", "collision"), collision_type(type), sprite(sprite), object(nullptr) { _CopySpritePosition(); }

	/** \param type The type of collision that occurred (should be COLLISION_OBJECT)
	*** \param sprite The sprite that had the collision
	*** \param object The object that the sprite collided with
	*** \note You should \b only use this constructor for object-type collisions
	**/
	MapCollisionNotificationEvent(COLLISION_TYPE type, VirtualSprite* sprite, MapObject* object) :
		NotificationEvent("map", "collision"), collision_type(type), sprite(sprite), object(object) { _CopySpritePosition(); }

	//! \brief Returns a string representation of the collision data stored in this object
	const std::string DEBUG_PrintInfo();

	//! \brief The type of collision that caused the notification to be generated
	COLLISION_TYPE collision_type;

	//! \brief The sprite that had the collision
	VirtualSprite* sprite;

	//! \brief Saved position data from the sprite at the time of the collision
	uint16 x_position, y_position;
	float x_offset, y_offset;

	//! \brief The object that the sprite collided with, if it was an object type collision. Otherwise will be nullptr
	MapObject* object;

private:
	//! \brief Retains the state of the sprite's position data in the class members
	void _CopySpritePosition();
}; // class MapCollisionNotificationEvent : public hoa_notification::NotificationEvent


/** ****************************************************************************
*** \brief A notification event class describing sprite collisions
***
*** Notifications generated only by CameraZone objects. Whenever one of these zones detects an
*** entry or exit from the zone by either the camera or player sprite, a notification is generated
*** with a pointer to the zone. When processing notifications, the user can then directly access
*** the pointer to the zone and its methods to figure out what generated the notification. The
*** pointer can be used to determine what zone or area on the map that it corresponds to.
*** ***************************************************************************/
class CameraZoneNotificationEvent : public hoa_notification::NotificationEvent {
public:
	/** \param type The type of collision that occurred
	*** \param sprite The sprite that had the collision
	*** \note You should \b not use this constructor for object-type collisions
	**/
	CameraZoneNotificationEvent(CameraZone* zone) :
		NotificationEvent("map", "camera-zone"), zone(zone) { }

	//! \brief Returns a string representation of the collision data stored in this object
	const std::string DEBUG_PrintInfo();

	//! \brief A pointer to the zone that generated the notification
	CameraZone* zone;
}; // class CameraZoneNotificationEvent : public hoa_notification::NotificationEvent

} // namespace private_map

} // namespace hoa_map
