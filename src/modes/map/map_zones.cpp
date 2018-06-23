///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_zones.cpp
*** \author  Guillaume Anctil (Drakkoon)
*** \brief   Source file for map mode zones.
*** ***************************************************************************/

// Allacrost engines
#include "notification.h"
#include "video.h"

// Local map mode headers
#include "map.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "map_transition.h"
#include "map_zones.h"

using namespace std;
using namespace hoa_utils;

using namespace hoa_notification;
using namespace hoa_video;

namespace hoa_map {

namespace private_map {

// -----------------------------------------------------------------------------
// ---------- MapZone Class Functions
// -----------------------------------------------------------------------------

const Color MapZone::ZONE_OUTLINE_COLOR = Color(0.0f, 0.0f, 1.0f, 0.33f);



MapZone::MapZone(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row) :
	_active_contexts(MAP_CONTEXT_NONE)
{
	AddSection(left_col, right_col, top_row, bottom_row);
}



MapZone::MapZone(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row, MAP_CONTEXT contexts) :
	_active_contexts(contexts)
{
	AddSection(left_col, right_col, top_row, bottom_row);
}



void MapZone::AddSection(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row) {
	if (left_col >= right_col) {
		IF_PRINT_WARNING(MAP_DEBUG) << "left and right coordinates are mismatched: section will not be added" << endl;
		return;
	}

	if (top_row >= bottom_row) {
		IF_PRINT_WARNING(MAP_DEBUG) << "top and bottom coordinates are mismatched: section will not be added" << endl;
		return;
	}

	_sections.push_back(ZoneSection(left_col, right_col, top_row, bottom_row));
}



bool MapZone::IsInsideZone(uint16 pos_x, uint16 pos_y) const {
	// Verify each section of the zone and check if the position is within the section bounds.
	for (vector<ZoneSection>::const_iterator i = _sections.begin(); i != _sections.end(); ++i) {
		if (pos_x >= i->left_col && pos_x <= i->right_col &&
			pos_y >= i->top_row && pos_y <= i->bottom_row)
		{
			return true;
		}
	}
	return false;
}



void MapZone::_RandomPosition(uint16& x, uint16& y) {
	// Select a random ZoneSection
	uint16 i = RandomBoundedInteger(0, _sections.size() - 1);

	// Select a random x and y position inside that section
	x = RandomBoundedInteger(_sections[i].left_col, _sections[i].right_col);
	y = RandomBoundedInteger(_sections[i].top_row, _sections[i].bottom_row);
}



void MapZone::DEBUG_DrawZoneOutlines(MAP_CONTEXT context, const Color& outline_color) {
// 	const float OUTLINE_WIDTH = 2.0f;

	// Only draw the zone outline if it exists for the current context
	if (_active_contexts & context) {
		// Determine the boundaries of the 2D sub-matrix of visible collision grid elements
		// Note that it is possible that oen or two rows or columns of the collision grid might not actually be visible here since the map frame
		// only determines which tiles are visible, not the colllision grid elements within those tiles.
		const MapFrame& frame = MapMode::CurrentInstance()->GetMapFrame();
		const uint32 grid_row_start = frame.starting_row * 2;
		const uint32 grid_row_end = grid_row_start + frame.num_draw_rows * 2;
		const uint32 grid_col_start = frame.starting_col * 2;
		const uint32 grid_col_end = grid_col_start + frame.num_draw_cols * 2;

		// Draw coordinates that reprsent the top left corner of the top left grid element visible on the screen
		float top_left_x = frame.tile_x_start - 1.0f;
		float top_left_y = frame.tile_y_start - 2.0f;;

		for (ZoneSection section : _sections) {
			// If any portion of the zone section falls within a screen, draw an outline of the section
			bool no_intersection = (section.top_row >= grid_row_end || section.bottom_row <= grid_row_start
				|| section.left_col >= grid_col_end || section.right_col <= grid_col_start);
			if (no_intersection == false) {
				float section_width = section.right_col - section.left_col;
				float section_height = section.bottom_row - section.top_row;
				float draw_x = top_left_x + static_cast<float>(section.left_col) - static_cast<float>(grid_col_start) + (section_width / 2.0f);
				float draw_y = top_left_y + static_cast<float>(section.bottom_row) - static_cast<float>(grid_row_start);

				VideoManager->Move(draw_x, draw_y);
				VideoManager->DrawRectangle(section_width, section_height, outline_color);

				// TODO: In the future we want to call DrawRectangleOutline() instead of DrawRectangle() to better indicate the individual zone sections and overlap.
				// At the moment this function has a bug preventing it from working. Once it is fixed, uncomment the code below and replace it with the logic above for
				// DrawRectangle().
				//
				// Figure out the X/Y draw coordinates for this section. This is done by taking the relative difference for the coordinates of the
				// top left corner of the top left grid element visible on the screen. Because the zone section could be very large, some of these
				// coordinates could be very far off the screen.
// 				float left_side = top_left_x + 1.0f * (section.left_col - grid_col_start);
// 				float right_side = top_left_x + 1.0f * (section.right_col - grid_col_start + 1);
// 				float top_side = top_left_y + 1.0f * (section.top_row - grid_row_start);
// 				float bottom_side = top_left_y + 1.0f * (section.bottom_row - grid_row_start + 1);
// 				VideoManager->DrawRectangleOutline(left_side, right_side, top_side, bottom_side, OUTLINE_WIDTH, ENEMY_ZONE_OUTLINE_COLOR);
			}
		}
	}
}

// -----------------------------------------------------------------------------
// ---------- CameraZone Class Functions
// -----------------------------------------------------------------------------

CameraZone::CameraZone(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row) :
	MapZone(left_col, right_col, top_row, bottom_row),
	_camera_inside(false),
	_was_camera_inside(false),
	_player_sprite_inside(false),
	_was_player_sprite_inside(false)
{}



CameraZone::CameraZone(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row, MAP_CONTEXT contexts) :
	MapZone(left_col, right_col, top_row, bottom_row, contexts),
	_camera_inside(false),
	_was_camera_inside(false),
	_player_sprite_inside(false),
	_was_player_sprite_inside(false)
{}



void CameraZone::Update() {
	_was_camera_inside = _camera_inside;
	_was_player_sprite_inside = _player_sprite_inside;

	VirtualSprite* camera = MapMode::CurrentInstance()->GetCamera();
	if (camera == nullptr) {
		_camera_inside = false;
	}
	// Camera must share a context with the zone and be within its borders
	else if ((_active_contexts & camera->GetContext()) && (IsInsideZone(camera->x_position, camera->y_position) == true)) {
		_camera_inside = true;
	}
	else {
		_camera_inside = false;
	}

	VirtualSprite* player = MapMode::CurrentInstance()->GetPlayerSprite();
	if (player == nullptr) {
		_player_sprite_inside = false;
	}
	// Camera must share a context with the zone and be within its borders
	else if ((_active_contexts & player->GetContext()) && (IsInsideZone(player->x_position, player->y_position) == true)) {
		_player_sprite_inside = true;
	}
	else {
		_player_sprite_inside = false;
	}

	// Generate a notification event for any enter/exit change
	if (_was_camera_inside != _camera_inside || _was_player_sprite_inside != _player_sprite_inside) {
		NotificationManager->Notify(new CameraZoneNotificationEvent(this));
	}
}

// -----------------------------------------------------------------------------
// ---------- ResidentZone Class Functions
// -----------------------------------------------------------------------------

ResidentZone::ResidentZone(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row) :
	MapZone(left_col, right_col, top_row, bottom_row)
{}



ResidentZone::ResidentZone(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row, MAP_CONTEXT contexts) :
	MapZone(left_col, right_col, top_row, bottom_row, contexts)
{}



void ResidentZone::Update() {
	_entering_residents.clear();
	_exiting_residents.clear();

	// Holds a list of sprites that should be removed from the resident zone. This is necessary because we can't iterate through
	// the resident list and erase former residents without messing up the set iteration.
	vector<VirtualSprite*> remove_list;

	// Examine all residents to see if they still reside in the zone. If not, move them to the exiting residents list
	for (set<VirtualSprite*>::iterator i = _residents.begin(); i != _residents.end(); i++) {
		// Make sure that the resident is still in a context shared by the zone and located within the zone boundaries
		if ((((*i)->GetContext() & _active_contexts) == 0x0) || (IsInsideZone((*i)->x_position, (*i)->y_position) == false)) {
			remove_list.push_back(*i);
		}
	}

	for (uint32 i = 0; i < remove_list.size(); i++) {
		_exiting_residents.insert(remove_list[i]);
		_residents.erase(remove_list[i]);
	}
}



void ResidentZone::AddPotentialResident(VirtualSprite* sprite) {
	if (sprite == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function received nullptr argument" << endl;
		return;
	}

	// Check that sprite is not already a resident
	if (IsSpriteResident(sprite) == true) {
		return;
	}

	// Check that the sprite's context is compatible with this zone and is located within the zone boundaries
	if (sprite->GetContext() & _active_contexts) {
		if (IsInsideZone(sprite->x_position, sprite->y_position) == true) {
			_entering_residents.insert(sprite);
			_residents.insert(sprite);
		}
	}
}



bool ResidentZone::IsSpriteResident(uint32 object_id) const {
	return IsSpriteResident(MapMode::CurrentInstance()->GetObjectSupervisor()->GetSprite(object_id));
}



bool ResidentZone::IsCameraResident() const {
	return IsSpriteResident(MapMode::CurrentInstance()->GetCamera());
}



bool ResidentZone::IsSpriteEntering(uint32 object_id) const {
	return IsSpriteEntering(MapMode::CurrentInstance()->GetObjectSupervisor()->GetSprite(object_id));
}



bool ResidentZone::IsCameraEntering() const {
	return IsSpriteEntering(MapMode::CurrentInstance()->GetCamera());
}




bool ResidentZone::IsSpriteExiting(uint32 object_id) const {
	return IsSpriteExiting(MapMode::CurrentInstance()->GetObjectSupervisor()->GetSprite(object_id));
}



bool ResidentZone::IsCameraExiting() const {
	return IsSpriteExiting(MapMode::CurrentInstance()->GetCamera());
}



VirtualSprite* ResidentZone::_GetSpriteInSet(const set<VirtualSprite*>& local_set, uint32 index) const {
	if (index >= local_set.size()) {
		return nullptr;
	}

	uint32 counter = 0;
	for (set<VirtualSprite*>::const_iterator i = local_set.begin(); i != local_set.end(); i++) {
		if (index == counter) {
			return *i;
		}
		counter++;
	}

	IF_PRINT_WARNING(MAP_DEBUG) << "sprite not found after reaching end of set -- this should never happen" << endl;
	return nullptr;
}

// -----------------------------------------------------------------------------
// ---------- ContextZone Class Functions
// -----------------------------------------------------------------------------

ContextZone::ContextZone(MAP_CONTEXT one, MAP_CONTEXT two) :
	_context_one(one),
	_context_two(two)
{
	if (_context_one == _context_two) {
		PRINT_ERROR << "tried to create a ContextZone with two equal context values: " << _context_one << endl;
	}
	else if (_context_one == MAP_CONTEXT_NONE || _context_two == MAP_CONTEXT_NONE) {
		PRINT_ERROR << "tried to create a ContextZone without a valid context ID" << endl;
	}
}



void ContextZone::AddSection(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row) {
	(void)left_col;
	(void)right_col;
	(void)top_row;
	(void)bottom_row;
	IF_PRINT_WARNING(MAP_DEBUG) << "this method is invalid for this class and should not be called: section will not be added" << endl;
}



void ContextZone::AddSection(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row, bool context) {
	if (left_col >= right_col) {
		IF_PRINT_WARNING(MAP_DEBUG) << "left and right coordinates are mismatched: section will not be added" << endl;
		return;
	}

	if (top_row >= bottom_row) {
		IF_PRINT_WARNING(MAP_DEBUG) << "top and bottom coordinates are mismatched: section will not be added" << endl;
		return;
	}

	_sections.push_back(ZoneSection(left_col, right_col, top_row, bottom_row));
	_section_contexts.push_back(context);
}



void ContextZone::Update() {
	int16 index;

	// Check every ground object and determine if its context should be changed by this zone
	// TODO: get the object container from the proper layer, not just the default layer
	vector<MapObject*>* objects = MapMode::CurrentInstance()->GetObjectSupervisor()->_object_layers[DEFAULT_LAYER_ID].GetObjects();
	for (uint32 i = 0;	i < objects->size(); ++i) {
		// If the object does not have a context equal to one of the two switching contexts, do not examine it further
		if (objects->at(i)->GetContext() != _context_one && objects->at(i)->GetContext() != _context_two) {
			continue;
		}

		// If the object is inside the zone, set their context to that zone's context
		// (This may result in no change from the object's current context depending on the zone section)
		index = _IsInsideZone(objects->at(i));
		if (index >= 0) {
			MAP_CONTEXT section_context = _section_contexts[index] ? _context_one : _context_two;
			if (objects->at(i)->GetContext() != section_context) {
				objects->at(i)->SetContext(section_context);
				// If the camera is pointing at the object that just had its context changed, start the context transition
				if (MapMode::CurrentInstance()->GetCamera() == objects->at(i)) {
					MapMode::CurrentInstance()->GetTransitionSupervisor()->StartContextTransition(objects->at(i)->GetContext());
				}
			}
		}
	}
}



int16 ContextZone::_IsInsideZone(MapObject* object) {
	// NOTE: argument is not nullptr-checked here for performance reasons

	// Check each section of the zone to see if the object is located within
	for (uint16 i = 0; i < _sections.size(); i++) {
		if (object->x_position >= _sections[i].left_col && object->x_position <= _sections[i].right_col &&
			object->y_position >= _sections[i].top_row && object->y_position <= _sections[i].bottom_row)
		{
			return i;
		}
	}

	return -1;
}

// -----------------------------------------------------------------------------
// ---------- EnemyZone Class Functions
// -----------------------------------------------------------------------------

const Color EnemyZone::ENEMY_ZONE_OUTLINE_COLOR = Color(1.0f, 0.0f, 1.0f, 0.33f);



EnemyZone::EnemyZone() :
	MapZone(),
	_roaming_restrained(true),
	_spawning_disabled(false),
	_active_enemies(0),
	_spawn_timer(DEFAULT_ENEMY_SPAWN_TIME),
	_spawn_zone(nullptr)
{
	_active_contexts = MAP_CONTEXT_ALL;
	_spawn_timer.Run();
}



EnemyZone::EnemyZone(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row) :
	MapZone(left_col, right_col, top_row, bottom_row),
	_roaming_restrained(true),
	_spawning_disabled(false),
	_active_enemies(0),
	_spawn_timer(DEFAULT_ENEMY_SPAWN_TIME),
	_spawn_zone(nullptr)
{
	_active_contexts = MAP_CONTEXT_ALL;
	_spawn_timer.Run();
}



EnemyZone::EnemyZone(const EnemyZone& copy) :
	MapZone(copy)
{
	_roaming_restrained = copy._roaming_restrained;
	_active_enemies = copy._active_enemies;
	_spawn_timer = copy._spawn_timer;
	if (copy._spawn_zone == nullptr)
		_spawn_zone = nullptr;
	else
		_spawn_zone = new MapZone(*(copy._spawn_zone));
}



EnemyZone& EnemyZone::operator=(const EnemyZone& copy) {
	if (this == &copy) // Handle self-assignment case
		return *this;

	MapZone::operator=(copy);
	_roaming_restrained = copy._roaming_restrained;
	_active_enemies = copy._active_enemies;
	_spawn_timer = copy._spawn_timer;
	if (copy._spawn_zone == nullptr)
		_spawn_zone = nullptr;
	else
		_spawn_zone = new MapZone(*(copy._spawn_zone));

	return *this;
}



void EnemyZone::AddEnemy(EnemySprite* enemy, MapMode* map, uint8 count) {
	if (count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function called with a zero value count argument" << endl;
		return;
	}

	// Prepare the first enemy
	enemy->SetZone(this);
	// TODO: use proper layer ID instead of the default
	map->GetObjectSupervisor()->AddObject(enemy, DEFAULT_LAYER_ID);
	_enemies.push_back(enemy);

	// Create any additional copies of the enemy and add them as well
	for (uint8 i = 1; i < count; i++) {
		EnemySprite* copy = new EnemySprite(*enemy);
		copy->SetObjectID(map->GetObjectSupervisor()->GenerateObjectID());
		// Add a 10% random margin of error to make enemies look less synchronized
		copy->SetDirectionChangeTime(static_cast<uint32>(copy->GetDirectionChangeTime() * (1 + RandomFloat() * 10)));
		copy->Reset();

		// TODO: use proper layer ID instead of the default
		map->GetObjectSupervisor()->AddObject(copy, DEFAULT_LAYER_ID);
		_enemies.push_back(copy);
	}
}



void EnemyZone::AddSpawnSection(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row) {
	if (left_col >= right_col) {
		IF_PRINT_WARNING(MAP_DEBUG) << "left and right coordinates are mismatched: section will not be added" << endl;
		return;
	}

	if (top_row >= bottom_row) {
		IF_PRINT_WARNING(MAP_DEBUG) << "top and bottom coordinates are mismatched: section will not be added" << endl;
		return;
	}

	// Make sure that this spawn section fits entirely inside one of the roaming sections
	bool okay_to_add = false;
	for (uint32 i = 0; i < _sections.size(); i++) {
		if ((left_col >= _sections[i].left_col) && (right_col <= _sections[i].right_col)
			&& (top_row >= _sections[i].top_row) && (bottom_row <= _sections[i].bottom_row))
		{
			okay_to_add = true;
			break;
		}
	}

	if (okay_to_add == false) {
		IF_PRINT_WARNING(MAP_DEBUG) << "could not add section as it did not fit inside any single roaming zone section" << endl;
		return;
	}

	// Create the spawn zone if it does not exist and add the new section
	if (_spawn_zone == nullptr) {
		_spawn_zone = new MapZone(left_col, right_col, top_row, bottom_row);
	}
	else {
		_spawn_zone->AddSection(left_col, right_col, top_row, bottom_row);
	}
}



void EnemyZone::ForceSpawnAllEnemies() {
	for (uint32 i = 0; i < _enemies.size(); ++i) {
		if (_enemies[i]->GetState() == EnemySprite::INACTIVE) {
			_SpawnEnemy(i);
		}
	}
}



void EnemyZone::EnemyDead() {
	if (_active_enemies == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function called when no enemies were active" << endl;
	}
	else {
		--_active_enemies;
	}
}



void EnemyZone::Update() {
	// Enemy zones only update during the explore state
	if (MapMode::CurrentInstance()->CurrentState() != STATE_EXPLORE) {
		return;
	}
	if (_enemies.empty() == true)
		return;

	// Update the spawn regeneration timer. This must complete before another enemy is spawned in
	_spawn_timer.Update();
	if (_spawn_timer.IsFinished() == false) {
		return;
	}

	// Spawn another enemy only if we have inactive enemies available and spawning hasn't been disabled
	if (_spawning_disabled == true || _active_enemies >= _enemies.size()) {
		return;
	}

	// TODO: this should select a random inactive enemy, not just the first one
	uint32 enemy_index = 0;
	for (uint32 i = 0; i < _enemies.size(); i++) {
		if (_enemies[i]->GetState() == EnemySprite::INACTIVE) {
			enemy_index = i;
			break;
		}
	}

	_SpawnEnemy(enemy_index);
} // void EnemyZone::Update()



bool EnemyZone::_SpawnEnemy(uint32 enemy_index) {
	// When spawning an enemy in a random zone location, sometimes it is occupied by another
	// object or that section is unwalkable. We try only a few different spawn locations before
	// giving up. Otherwise this function could potentially take a noticable amount of time to complete
	const int8 SPAWN_RETRIES = 40;

	if (enemy_index >= _enemies.size()) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function called with an out-of-range index argument: " << enemy_index << endl;
		return false;
	}

	uint16 x, y; // Used to retain random position coordinates in the zone
	int8 retries = SPAWN_RETRIES; // Number of times to try finding a valid spawning location
	bool collision; // Holds the result of a collision detection check

	// Select a random position inside the zone to place the spawning enemy. To do this, we need to enable the collidable
	// property of the enemy sprite.
	bool saved_collidable = _enemies[enemy_index]->collidable;
	_enemies[enemy_index]->collidable = true; // This must be temporarily
	MapZone* spawning_zone = nullptr;
	if (HasSeparateSpawnZone() == false) {
		spawning_zone = this;
	}
	else {
		spawning_zone = _spawn_zone;
	}

	// Try to find a suitable spawn location
	do {
		spawning_zone->_RandomPosition(x, y);
		_enemies[enemy_index]->SetXPosition(x, 0.0f);
		_enemies[enemy_index]->SetYPosition(y, 0.0f);
		collision = MapMode::CurrentInstance()->GetObjectSupervisor()->DetectCollision(_enemies[enemy_index], nullptr);
	} while (collision && --retries > 0);

	// If we didn't find a suitable spawning location, reset the collision info
	// on the enemy sprite and we will retry on the next call to this function
	if (collision) {
		_enemies[enemy_index]->collidable = saved_collidable;
		return false;
	}
	// Otherwise, spawn the enemy and reset the spawn timer
	else {
		_spawn_timer.Reset();
		_spawn_timer.Run();
		_enemies[enemy_index]->ChangeState(EnemySprite::SPAWN);
		_active_enemies++;
		return true;
	}
}

} // namespace private_map

} // namespace hoa_map
