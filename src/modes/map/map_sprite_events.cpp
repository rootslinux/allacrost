///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_sprite_events.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for map mode sprite events
*** ***************************************************************************/

// Allacrost engines
#include "script.h"
#include "system.h"

// Local map mode headers
#include "map.h"
#include "map_events.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "map_sprite_events.h"

using namespace std;

using namespace hoa_script;
using namespace hoa_system;

namespace hoa_map {

namespace private_map {

// -----------------------------------------------------------------------------
// ---------- SpriteEvent Class Methods
// -----------------------------------------------------------------------------

SpriteEvent::SpriteEvent(uint32 event_id, EVENT_TYPE event_type, VirtualSprite* sprite) :
	MapEvent(event_id, event_type),
	_sprite(sprite)
{
	if (sprite == nullptr)
		IF_PRINT_WARNING(MAP_DEBUG) << "nullptr sprite object passed into constructor: " << event_id << endl;
}

// -----------------------------------------------------------------------------
// ---------- ChangePropertySpriteEvent Class Methods
// -----------------------------------------------------------------------------

ChangePropertySpriteEvent::ChangePropertySpriteEvent(uint32 event_id, VirtualSprite* sprite) :
	SpriteEvent(event_id, CHANGE_PROPERTY_SPRITE_EVENT, sprite),
	_sprite_list(1, sprite),
	_properties(),
	_relative_position_change(false),
	_updatable(false),
	_visible(false),
	_collidable(false),
	_context(MAP_CONTEXT_NONE),
	_x_position(0),
	_y_position(0),
	_x_offset(0.0f),
	_y_offset(0.0f),
	_direction(NORTH),
	_movement_speed(NORMAL_SPEED),
	_moving(false),
	_running(false),
	_stationary_movement(false)
{}



ChangePropertySpriteEvent* ChangePropertySpriteEvent::Create(uint32 event_id, VirtualSprite* sprite) {
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function received nullptr sprite argument when trying to create an event with id: " << event_id << endl;
		return nullptr;
	}

	ChangePropertySpriteEvent* event = new ChangePropertySpriteEvent(event_id, sprite);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}



ChangePropertySpriteEvent* ChangePropertySpriteEvent::Create(uint32 event_id, uint16 sprite_id) {
	VirtualSprite* sprite = MapMode::CurrentInstance()->GetObjectSupervisor()->GetSprite(sprite_id);
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "no sprite object was registered for the requested sprite_id (" << sprite_id << ") "
			<< "when trying to create an event with id: " << event_id << endl;
		return nullptr;
	}

	return Create(event_id, sprite);
}



void ChangePropertySpriteEvent::AddSprite(VirtualSprite* sprite) {
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function received nullptr sprite argument when trying to add to event id: " << GetEventID() << endl;
		return;
	}

	// Note that we don't bother to check if this sprite is a duplicate of one already in the list. It doesn't matter much since we will
	// just end up setting the properties for that sprite twice.
	_sprite_list.push_back(sprite);
}



void ChangePropertySpriteEvent::Position(int16 x_position, float x_offset, int16 y_position, float y_offset) {
	if (_relative_position_change == false) {
		if (x_position < 0) {
			IF_PRINT_WARNING(MAP_DEBUG) << "function received negative x_position value when relative positioning was disabled, event id: " << GetEventID() << endl;
			x_position = -x_position;
		}
		if (y_position < 0) {
			IF_PRINT_WARNING(MAP_DEBUG) << "function received negative y_position value when relative positioning was disabled, event id: " << GetEventID() << endl;
			y_position = -y_position;
		}
	}

	_x_position = x_position;
	_x_offset = x_offset;
	_y_position = y_position;
	_y_offset = y_offset;
}


void ChangePropertySpriteEvent::_Start() {
	// When no properties were set by the user, this event effectively becomes a no-op
	if (_properties.none() == true)
		return;

	for (uint32 i = 0; i < _sprite_list.size(); ++i) {
		VirtualSprite* sprite = _sprite_list[i];
		MapSprite* map_sprite = nullptr;
		if (sprite->GetObjectType() != VIRTUAL_TYPE)
			map_sprite = dynamic_cast<MapSprite*>(sprite);
		for (uint32 bit = 0; bit < _properties.size(); ++bit) {
			if (_properties.test(bit) == true) {
				switch (bit) {
					case UPDATABLE:
						sprite->updatable = _updatable;
						break;
					case VISIBLE:
						sprite->visible = _visible;
						break;
					case COLLIDABLE:
						sprite->collidable = _collidable;
						break;
					case CONTEXT:
						sprite->SetContext(_context);
						break;
					case POSITION:
						if (_relative_position_change == false)
							sprite->SetPosition(static_cast<uint16>(_x_position), _x_offset, static_cast<uint16>(_y_position), _y_offset);
						else
							sprite->ModifyPosition(_x_position, _x_offset, _y_position, _y_offset);
						break;
					case DIRECTION:
						sprite->SetDirection(_direction);
						break;
					case MOVEMENTSPEED:
						sprite->SetMovementSpeed(_movement_speed);
						break;
					case MOVING:
						sprite->SetMoving(_moving);
						break;
					case RUNNING:
						sprite->SetRunning(_running);
						break;
					case STATIONARYMOVEMENT:
						if (map_sprite != nullptr)
							map_sprite->SetStationaryMovement(_stationary_movement);
						break;
					case REVERSEMOVEMENT:
						if (map_sprite != nullptr)
							map_sprite->SetReverseMovement(_reverse_movement);
						break;
					default:
						IF_PRINT_WARNING(MAP_DEBUG) << "unknown property bit set (" << bit << "), event id: " << GetEventID() << endl;
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------
// ---------- AnimateSpriteEvent Class Methods
// -----------------------------------------------------------------------------

AnimateSpriteEvent::AnimateSpriteEvent(uint32 event_id, VirtualSprite* sprite) :
	SpriteEvent(event_id, ANIMATE_SPRITE_EVENT, sprite),
	_current_frame(0),
	_display_timer(0),
	_loop_count(0),
	_number_loops(0)
{}



AnimateSpriteEvent::~AnimateSpriteEvent()
{}



AnimateSpriteEvent* AnimateSpriteEvent::Create(uint32 event_id, VirtualSprite* sprite) {
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function received nullptr sprite argument when trying to create an event with id: " << event_id << endl;
		return nullptr;
	}

	AnimateSpriteEvent* event = new AnimateSpriteEvent(event_id, sprite);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}



AnimateSpriteEvent* AnimateSpriteEvent::Create(uint32 event_id, uint16 sprite_id) {
	VirtualSprite* sprite = MapMode::CurrentInstance()->GetObjectSupervisor()->GetSprite(sprite_id);
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "no sprite object was registered for the requested sprite_id (" << sprite_id << ") "
			<< "when trying to create an event with id: " << event_id << endl;
		return nullptr;
	}

	return Create(event_id, sprite);
}



void AnimateSpriteEvent::_Start() {
	SpriteEvent::_Start();
	_current_frame = 0;
	_display_timer = 0;
	_loop_count = 0;
	dynamic_cast<MapSprite*>(_sprite)->SetCustomAnimation(true);
	dynamic_cast<MapSprite*>(_sprite)->SetCurrentAnimation(static_cast<uint8>(_frames[_current_frame]));
}



bool AnimateSpriteEvent::_Update() {
	_display_timer += SystemManager->GetUpdateTime();

	if (_display_timer > _frame_times[_current_frame]) {
		_display_timer = 0;
		_current_frame++;

		// Check if we are past the final frame to display in the loop
		if (_current_frame >= _frames.size()) {
			_current_frame = 0;

			// If this animation is not infinitely looped, increment the loop counter
			if (_number_loops >= 0) {
				_loop_count++;
				if (_loop_count > _number_loops) {
					_loop_count = 0;
					dynamic_cast<MapSprite*>(_sprite)->SetCustomAnimation(false);
					_sprite->ReleaseControl(this);
					return true;
				 }
			}
		}

		dynamic_cast<MapSprite*>(_sprite)->SetCurrentAnimation(static_cast<uint8>(_frames[_current_frame]));
	}

	return false;
}

// -----------------------------------------------------------------------------
// ---------- RandomMoveSpriteEvent Class Methods
// -----------------------------------------------------------------------------

RandomMoveSpriteEvent::RandomMoveSpriteEvent(uint32 event_id, VirtualSprite* sprite, uint32 move_time, uint32 direction_time) :
	SpriteEvent(event_id, RANDOM_MOVE_SPRITE_EVENT, sprite),
	_total_movement_time(move_time),
	_total_direction_time(direction_time),
	_movement_timer(0),
	_direction_timer(0)
{}



RandomMoveSpriteEvent::~RandomMoveSpriteEvent()
{}



RandomMoveSpriteEvent* RandomMoveSpriteEvent::Create(uint32 event_id, VirtualSprite* sprite, uint32 move_time, uint32 direction_time) {
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function received nullptr sprite argument when trying to create an event with id: " << event_id << endl;
		return nullptr;
	}

	RandomMoveSpriteEvent* event = new RandomMoveSpriteEvent(event_id, sprite, move_time, direction_time);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}




RandomMoveSpriteEvent* RandomMoveSpriteEvent::Create(uint32 event_id, uint16 sprite_id, uint32 move_time, uint32 direction_time) {
	VirtualSprite* sprite = MapMode::CurrentInstance()->GetObjectSupervisor()->GetSprite(sprite_id);
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "no sprite object was registered for the requested sprite_id (" << sprite_id << ") "
			<< "when trying to create an event with id: " << event_id << endl;
		return nullptr;
	}

	return Create(event_id, sprite, move_time, direction_time);
}



void RandomMoveSpriteEvent::_Start() {
	SpriteEvent::_Start();
	_sprite->SetRandomDirection();
	_sprite->SetMoving(true);
}



bool RandomMoveSpriteEvent::_Update() {
	_direction_timer += SystemManager->GetUpdateTime();
	_movement_timer += SystemManager->GetUpdateTime();

	// Check if we should change the sprite's direction
	if (_direction_timer >= _total_direction_time) {
		_direction_timer -= _total_direction_time;
		_sprite->SetRandomDirection();
	}

	if (_movement_timer >= _total_movement_time) {
		_movement_timer = 0;
		_sprite->SetMoving(false);
		_sprite->ReleaseControl(this);
		return true;
	}

	return false;
}



void RandomMoveSpriteEvent::_ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj) {
	// Try to adjust the sprite's position around the collision. If that fails, change the sprite's direction
	if (MapMode::CurrentInstance()->GetObjectSupervisor()->AdjustSpriteAroundCollision(_sprite, coll_type, coll_obj) == false) {
		_sprite->SetRandomDirection();
	}
}

// -----------------------------------------------------------------------------
// ---------- PathMoveSpriteEvent Class Methods
// -----------------------------------------------------------------------------

PathMoveSpriteEvent::PathMoveSpriteEvent(uint32 event_id, VirtualSprite* sprite, int16 x_coord, int16 y_coord) :
	SpriteEvent(event_id, PATH_MOVE_SPRITE_EVENT, sprite),
	_relative_destination(false),
	_source_col(-1),
	_source_row(-1),
	_destination_col(x_coord),
	_destination_row(y_coord),
	_last_x_position(0),
	_last_y_position(0),
	_final_direction(0),
	_current_node(0)
{}



PathMoveSpriteEvent* PathMoveSpriteEvent::Create(uint32 event_id, VirtualSprite* sprite, int16 x_coord, int16 y_coord) {
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function received nullptr sprite argument when trying to create an event with id: " << event_id << endl;
		return nullptr;
	}

	PathMoveSpriteEvent* event = new PathMoveSpriteEvent(event_id, sprite, x_coord, y_coord);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}



PathMoveSpriteEvent* PathMoveSpriteEvent::Create(uint32 event_id, uint16 sprite_id, int16 x_coord, int16 y_coord) {
	VirtualSprite* sprite = MapMode::CurrentInstance()->GetObjectSupervisor()->GetSprite(sprite_id);
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "no sprite object was registered for the requested sprite_id (" << sprite_id << ") "
			<< "when trying to create an event with id: " << event_id << endl;
		return nullptr;
	}

	return Create(event_id, sprite, x_coord, y_coord);
}



void PathMoveSpriteEvent::SetRelativeDestination(bool relative) {
	if (MapMode::CurrentInstance()->GetEventSupervisor()->IsEventActive(GetEventID()) == true) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted illegal operation while event was active: " << GetEventID() << endl;
		return;
	}

	_relative_destination = relative;
	_path.clear();
}



void PathMoveSpriteEvent::SetDestination(int16 x_coord, int16 y_coord) {
	if (MapMode::CurrentInstance()->GetEventSupervisor()->IsEventActive(GetEventID()) == true) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted illegal operation while event was active: " << GetEventID() << endl;
		return;
	}

	_destination_col = x_coord;
	_destination_row = y_coord;
	_path.clear();
}



void PathMoveSpriteEvent::SetFinalDirection(uint16 direction) {
	if ((direction != NORTH) && (direction != SOUTH) && (direction != EAST) && (direction != WEST))
		IF_PRINT_WARNING(MAP_DEBUG) << "non-standard direction specified (" << direction << ") "
			<< "for an event with id: " << GetEventID() << endl;

	_final_direction = direction;
}



void PathMoveSpriteEvent::_Start() {
	SpriteEvent::_Start();

	_current_node = 0;
	_last_x_position = _sprite->x_position;
	_last_y_position = _sprite->y_position;

	// Set and check the source position
	_source_col = _sprite->x_position;
	_source_row = _sprite->y_position;
	if (_source_col < 0 || _source_row < 0) {
		// TODO: Also check if the source position is beyond the maximum row/col map boundaries
		IF_PRINT_WARNING(MAP_DEBUG) << "sprite position is invalid" << endl;
		_path.clear();
		return;
	}

	// Set and check the destination position
	if (_relative_destination == false) {
		_destination_node.col = _destination_col;
		_destination_node.row = _destination_row;
	}
	else {
		_destination_node.col = _source_col + _destination_col;
		_destination_node.row = _source_row + _destination_row;
	}

	// TODO: check if destination node exceeds map boundaries
	if (_destination_node.col < 0 || _destination_node.row < 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "invalid destination coordinates" << endl;
		_path.clear();
		return;
	}

	// TODO: If we already have a path from this source to this destination, re-use it and do not compute a new path

	if (MapMode::CurrentInstance()->GetObjectSupervisor()->FindPath(_sprite, _path, _destination_node) == true) {
		_sprite->SetMoving(true);
		_SetSpriteDirection();
		// TODO: if a sprite starts their path when their offsets are non-zero, the pathfinding algorithm always assumes
		// the sprite is at those offsets for each X/Y position. This can cause the sprite to not find a valid path that
		// they could otherwise fit through. Investigate ways to improve this here, or within the FindPath() algorithm.
	}
	else {
		IF_PRINT_WARNING(MAP_DEBUG) << "failed to find a path for sprite with id: " << _sprite->GetObjectID() << endl;
		_path.clear();
	}
}



bool PathMoveSpriteEvent::_Update() {
	if (_path.empty() == true) {
		PRINT_ERROR << "no path to destination: [" << _destination_col << ", " << _destination_row << "]" << endl;
		return true;
	}

	// Check if the sprite has arrived at the position of the current node
	if (_sprite->x_position == _path[_current_node].col && _sprite->y_position == _path[_current_node].row) {
		_current_node++;

		// When the current node index is at the end of the path, the event is finished
		if (_current_node >= _path.size() - 1) {
			// TODO: don't finish here: instead move the sprite to the specified offset within the grid element then finish
			_sprite->SetMoving(false);
			_sprite->ReleaseControl(this);
			if (_final_direction != 0)
				_sprite->SetDirection(_final_direction);

			// TODO: As soon as the sprite's position is X/Y, the path is completed. However, this can result in different
			// final positions depending on what direction the sprite was walking when reaching X/Y, and what their speed was.
			// In other words, while their position will be X/Y, their x and y offsets can range from 0.0f - 0.999f.
			// Figure out a way here for the sprite to always end in the same exact position, possibly by continuing to move them
			// until their final offsets are 0.0f.
			return true;
		}
		else {
			_SetSpriteDirection();
		}
	}
	// If the sprite has moved to a new position other than the next node, adjust its direction so it is trying to move to the next node
	else if ((_sprite->x_position != _last_x_position) || (_sprite->y_position != _last_y_position)) {
		_last_x_position = _sprite->x_position;
		_last_y_position = _sprite->y_position;
		_SetSpriteDirection();
	}

	return false;
}



void PathMoveSpriteEvent::_SetSpriteDirection() {
	uint16 direction = 0;

	if (_sprite->y_position > _path[_current_node].row) { // Need to move north
		direction |= NORTH;
	}
	else if (_sprite->y_position < _path[_current_node].row) { // Need to move south
		direction |= SOUTH;
	}

	if (_sprite->x_position > _path[_current_node].col) { // Need to move west
		direction |= WEST;
	}
	else if (_sprite->x_position < _path[_current_node].col) { // // Need to move east
		direction |= EAST;
	}

	// Determine if the sprite is moving diagonally to the next node. If so, we have to determine which direction
	// the sprite should face during this movement as well
	if ((direction & (NORTH | SOUTH)) && (direction & (WEST | EAST))) {
		uint16 sprite_direction = _sprite->GetDirection();
		if (direction == (NORTH | WEST)) {
			if (sprite_direction & FACING_NORTH || sprite_direction & FACING_EAST)
				direction = NW_NORTH;
			else
				direction = NW_WEST;
		}
		else if (direction == (NORTH | EAST)) {
			if (sprite_direction & FACING_NORTH || sprite_direction & FACING_WEST)
				direction = NE_NORTH;
			else
				direction = NE_EAST;
		}
		else if (direction == (SOUTH | WEST)) {
			if (sprite_direction & FACING_SOUTH || sprite_direction & FACING_EAST)
				direction = SW_SOUTH;
			else
				direction = SW_WEST;
		}
		else if (direction == (SOUTH | EAST)) {
			if (sprite_direction & FACING_SOUTH || sprite_direction & FACING_WEST)
				direction = SE_SOUTH;
			else
				direction = SE_EAST;
		}
	}

	_sprite->SetDirection(direction);
}



void PathMoveSpriteEvent::_ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj) {
	// Boundary and grid collisions should not occur on a pre-calculated path. If these conditions do occur,
	// we terminate the path event immediately. The conditions may occur if, for some reason, the map's boundaries
	// or collision grid are modified after the path is calculated
	if (coll_type == BOUNDARY_COLLISION || coll_type == GRID_COLLISION) {
		if (MapMode::CurrentInstance()->GetObjectSupervisor()->AdjustSpriteAroundCollision(_sprite, coll_type, coll_obj) == false) {
			IF_PRINT_WARNING(MAP_DEBUG) << "boundary or grid collision occurred on a pre-calculated path movement" << endl;
		}
		// Wait
// 		_path.clear(); // This path is obviously not a correct one so we should trash it
// 		_sprite->ReleaseControl(this);
// 		MapMode::CurrentInstance()->GetEventSupervisor()->TerminateEvent(GetEventID());
		return;
	}

	// If the code has reached this point, then we are dealing with an object collision

	// Determine if the obstructing object is blocking the destination of this path
	bool destination_blocked = MapMode::CurrentInstance()->GetObjectSupervisor()->IsPositionOccupiedByObject(_destination_node.row, _destination_node.col, coll_obj);

	switch (coll_obj->GetObjectType()) {
		case PHYSICAL_TYPE:
		case MAP_TREASURE_TYPE:
			// If the object is a static map object and blocking the destination, give up and terminate the event
			if (destination_blocked == true) {
				IF_PRINT_WARNING(MAP_DEBUG) << "path destination was blocked by a non-sprite map object" << endl;
				_path.clear(); // This path is obviously not a correct one so we should trash it
				_sprite->ReleaseControl(this);
				MapMode::CurrentInstance()->GetEventSupervisor()->TerminateEvent(GetEventID());
				// Note that we will retain the path (we don't clear() it), hoping that next time the object is moved

			}
			// Otherwise, try to find an alternative path around the object
			else {
				// TEMP: try a movement adjustment to get around the object
				MapMode::CurrentInstance()->GetObjectSupervisor()->AdjustSpriteAroundCollision(_sprite, coll_type, coll_obj);
				// TODO: recalculate and find an alternative path around the object
			}
			break;

		case VIRTUAL_TYPE:
		case SPRITE_TYPE:
		case ENEMY_TYPE:
			if (destination_blocked == true) {
				// Do nothing but wait for the obstructing sprite to move out of the way
				return;

				// TODO: maybe we should use a timer here to determine if a certain number of seconds have passed while waiting for the obstructiong
				// sprite to move. If that timer expires and the destination is still blocked by the sprite, we could give up on reaching the
				// destination and terminate the path event
			}

			else {
				// TEMP: try a movement adjustment to get around the object
				MapMode::CurrentInstance()->GetObjectSupervisor()->AdjustSpriteAroundCollision(_sprite, coll_type, coll_obj);
			}
			break;

		default:
			IF_PRINT_WARNING(MAP_DEBUG) << "collision object was of an unknown object type: " << coll_obj->GetObjectType() << endl;
	}
} // void PathMoveSpriteEvent::_ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj)

// -----------------------------------------------------------------------------
// ---------- CustomSpriteEvent Class Methods
// -----------------------------------------------------------------------------

CustomSpriteEvent::CustomSpriteEvent(uint32 event_id, VirtualSprite* sprite, string start_name, string update_name) :
	SpriteEvent(event_id, SCRIPTED_SPRITE_EVENT, sprite),
	_start_function(nullptr),
	_update_function(nullptr)
{
	ReadScriptDescriptor& map_script = MapMode::CurrentInstance()->GetMapScript();
	MapMode::CurrentInstance()->OpenScriptTablespace(true);
	map_script.OpenTable("functions");
	if (start_name != "") {
		_start_function = new ScriptObject();
		*_start_function = map_script.ReadFunctionPointer(start_name);
	}
	if (update_name != "") {
		_update_function = new ScriptObject();
		*_update_function = map_script.ReadFunctionPointer(update_name);
	}
	map_script.CloseTable();
	map_script.CloseTable();

	if ((_start_function == nullptr) && (_update_function == nullptr)) {
		IF_PRINT_WARNING(MAP_DEBUG) << "no start or update functions were declared for event: " << event_id << endl;
	}
}



CustomSpriteEvent::~CustomSpriteEvent() {
	if (_start_function != nullptr) {
		delete _start_function;
		_start_function = nullptr;
	}
	if (_update_function != nullptr) {
		delete _update_function;
		_update_function = nullptr;
	}
}



CustomSpriteEvent::CustomSpriteEvent(const CustomSpriteEvent& copy) :
	SpriteEvent(copy)
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



CustomSpriteEvent& CustomSpriteEvent::operator=(const CustomSpriteEvent& copy) {
	if (this == &copy) // Handle self-assignment case
		return *this;

	SpriteEvent::operator=(copy);

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


CustomSpriteEvent* CustomSpriteEvent::Create(uint32 event_id, VirtualSprite* sprite, string start_name, string update_name) {
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function received nullptr sprite argument when trying to create an event with id: " << event_id << endl;
		return nullptr;
	}

	CustomSpriteEvent* event = new CustomSpriteEvent(event_id, sprite, start_name, update_name);
	MapMode::CurrentInstance()->GetEventSupervisor()->RegisterEvent(event);
	return event;
}


CustomSpriteEvent* CustomSpriteEvent::Create(uint32 event_id, uint16 sprite_id, string start_name, string update_name) {
	VirtualSprite* sprite = MapMode::CurrentInstance()->GetObjectSupervisor()->GetSprite(sprite_id);
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "no sprite object was registered for the requested sprite_id (" << sprite_id << ") "
			<< "when trying to create an event with id: " << event_id << endl;
		return nullptr;
	}

	return Create(event_id, sprite, start_name, update_name);
}



void CustomSpriteEvent::_Start() {
	if (_start_function != nullptr) {
		SpriteEvent::_Start();
		ScriptCallFunction<void>(*_start_function, _sprite);
	}
}



bool CustomSpriteEvent::_Update() {
	bool finished = false;
	if (_update_function != nullptr) {
		finished = ScriptCallFunction<bool>(*_update_function, _sprite);
	}
	else {
		finished = true;
	}

	if (finished == true) {
		_sprite->ReleaseControl(this);
	}
	return finished;
}

} // namespace private_map

} // namespace hoa_map

