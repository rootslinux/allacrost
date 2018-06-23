///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_sprites.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for map mode sprites.
*** ***************************************************************************/

// Allacrost utilities
#include "utils.h"

// Allacrost engines
#include "audio.h"
#include "mode_manager.h"
#include "notification.h"
#include "system.h"

// Allacrost globals
#include "global.h"

// Local map mode headers
#include "map.h"
#include "map_sprites.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "map_sprite_events.h"
#include "map_transition.h"

// Other game mode headers
#include "battle.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_mode_manager;
using namespace hoa_notification;
using namespace hoa_video;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_battle;

namespace hoa_map {

namespace private_map {

// ****************************************************************************
// ********** VirtualSprite class methods
// ****************************************************************************

VirtualSprite::VirtualSprite() :
	_direction(SOUTH),
	_movement_speed(NORMAL_SPEED),
	_moving(false),
	_running(false),
	_control_event(nullptr),
	_state_saved(false),
	_saved_direction(0),
	_saved_movement_speed(0.0f),
	_saved_moving(false)
{
	MapObject::_object_type = VIRTUAL_TYPE;
	visible = false;
	collidable = false;
}



void VirtualSprite::Update() {
	if (!updatable) {
		return;
	}

	// Determine if a movement event is controlling the sprite.
	if (_moving == false) {
		return;
// 		if (_control_event != nullptr) {
// 			EVENT_TYPE event_type = _control_event->GetEventType();
// 			if (event_type == PATH_MOVE_SPRITE_EVENT || event_type == RANDOM_MOVE_SPRITE_EVENT) {
// 				_moving = true;
// 			}
// 		}
// 		// If the sprite still isn't _moving, there's nothing more to update here
// 		if (_moving == false) {
// 			return;
// 		}
	}

	// Save the previous sprite's position temporarily
	float tmp_x = x_offset;
	float tmp_y = y_offset;

	float distance_moved = CalculateDistanceMoved();

	// TODO: Refactor this so that it calls ModifySpritePosition instead

	// Move the sprite the appropriate distance in the appropriate Y and X direction
	if (_direction & (NORTH | MOVING_NORTHWEST | MOVING_NORTHEAST))
		y_offset -= distance_moved;
	else if (_direction & (SOUTH | MOVING_SOUTHWEST | MOVING_SOUTHEAST))
		y_offset += distance_moved;
	if (_direction & (WEST | MOVING_NORTHWEST | MOVING_SOUTHWEST))
		x_offset -= distance_moved;
	else if (_direction & (EAST | MOVING_NORTHEAST | MOVING_SOUTHEAST))
		x_offset += distance_moved;

	MapObject* collision_object = nullptr;
	COLLISION_TYPE collision_type = NO_COLLISION;
	collision_type = MapMode::CurrentInstance()->GetObjectSupervisor()->DetectCollision(this, &collision_object);

	if (collision_type == NO_COLLISION) {
		CheckPositionOffsets();
	}
	else {
		MapCollisionNotificationEvent* event = new MapCollisionNotificationEvent(collision_type, this, collision_object);
		NotificationManager->Notify(event);

		// Restore the sprite's position. The _ResolveCollision() call that follows may find an alternative
		// position to move the sprite to.
		x_offset = tmp_x;
		y_offset = tmp_y;

		_ResolveCollision(collision_type, collision_object);
	}
}



bool VirtualSprite::IsFacingDirection(uint16 direction) const {
	switch (direction) {
		case NORTH:
			return (_direction & FACING_NORTH);
		case SOUTH:
			return (_direction & FACING_SOUTH);
		case EAST:
			return (_direction & FACING_EAST);
		case WEST:
			return (_direction & FACING_WEST);
		default:
			IF_PRINT_WARNING(MAP_DEBUG) << "function received invalid argument: " << direction << endl;
			return false;
	}
}



void VirtualSprite::SetRandomDirection() {
	switch (RandomBoundedInteger(1, 8)) {
		case 1:
			SetDirection(NORTH);
			break;
		case 2:
			SetDirection(SOUTH);
			break;
		case 3:
			SetDirection(EAST);
			break;
		case 4:
			SetDirection(WEST);
			break;
		case 5:
			SetDirection(MOVING_NORTHEAST);
			break;
		case 6:
			SetDirection(MOVING_NORTHWEST);
			break;
		case 7:
			SetDirection(MOVING_SOUTHEAST);
			break;
		case 8:
			SetDirection(MOVING_SOUTHWEST);
			break;
		default:
			IF_PRINT_WARNING(MAP_DEBUG) << "invalid randomized direction was chosen" << endl;
	}
}



float VirtualSprite::CalculateDistanceMoved() {
	float distance_moved = static_cast<float>(SystemManager->GetUpdateTime()) / _movement_speed;

	// Double the distance to move if the sprite is running
	if (_running == true)
		distance_moved *= 2.0f;
	// If the movement is diagonal, decrease the lateral movement distance by sin(45 degress)
	if (_direction & MOVING_DIAGONALLY)
		distance_moved *= 0.707f;

	return distance_moved;
}



bool VirtualSprite::ModifySpritePosition(uint16 direction, float distance, bool disable_collision_notification) {
	// Used to save the current position offset in case the adjustment fails
	float saved_offset;

	switch (direction) {
		case NORTH:
			saved_offset = y_offset;
			y_offset -= distance;
			break;
		case SOUTH:
			saved_offset = y_offset;
			y_offset += distance;
			break;
		case EAST:
			saved_offset = x_offset;
			x_offset += distance;
			break;
		case WEST:
			saved_offset = x_offset;
			x_offset -= distance;
			break;
		default:
			IF_PRINT_WARNING(MAP_DEBUG) << "invalid direction argument passed to this function: " << direction << endl;
			return false;
	}

	// Check for a collision in the newly adjusted position
	MapObject* collision_object = nullptr;
	COLLISION_TYPE collision_type = MapMode::CurrentInstance()->GetObjectSupervisor()->DetectCollision(this, &collision_object);
	if (collision_type != NO_COLLISION) {
		// Generate a notification that the collision occurred while the sprite is still in it's collision position
		if (disable_collision_notification == false) {
			MapCollisionNotificationEvent* event = new MapCollisionNotificationEvent(GRID_COLLISION, this, collision_object);
			NotificationManager->Notify(event);
			cout << event->DEBUG_PrintInfo() << endl;
		}

		// Restore the sprite's original position and give up any further efforts for movement adjustment
		if (direction & (NORTH | SOUTH)) {
			y_offset = saved_offset;
		}
		else {
			x_offset = saved_offset;
		}

		return false;
	}
	else {
		// The adjustment was successful, check the position offsets and state that the position has been changed
		CheckPositionOffsets();
		return true;
	}
} // bool VirtualSprite::_ModifySpritePosition(VirtualSprite* sprite, uint16 direction, float distance, bool disable_collision_notification)



void VirtualSprite::AcquireControl(SpriteEvent* event) {
	if (event == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument was nullptr" << endl;
		return;
	}

	if (_control_event != nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "a new event is acquiring control when the previous event has not "
			"released control over this sprite, object id: " << GetObjectID() << endl;
	}
	_control_event = event;
}



void VirtualSprite::ReleaseControl(SpriteEvent* event) {
	if (event == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument was nullptr" << endl;
		return;
	}

	if (_control_event == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "no event had control over this sprite, object id: " << GetObjectID() << endl;
	}
	else if (_control_event != event) {
		IF_PRINT_WARNING(MAP_DEBUG) << "a different event has control of this sprite, object id: " << GetObjectID() << endl;
	}
	else {
		_control_event = nullptr;
	}
}



void VirtualSprite::SaveState() {
	_state_saved = true;
	_saved_direction = _direction;
	_saved_movement_speed = _movement_speed;
	_saved_moving = _moving;
	if (_control_event != nullptr)
		MapMode::CurrentInstance()->GetEventSupervisor()->PauseEvent(_control_event->GetEventID());
}



void VirtualSprite::RestoreState() {
	if (_state_saved == false)
		IF_PRINT_WARNING(MAP_DEBUG) << "restoring state when no saved state was detected" << endl;

	_state_saved = false;
	 _direction = _saved_direction;
	 _movement_speed = _saved_movement_speed;
	 _moving = _saved_moving;
	 if (_control_event != nullptr)
		MapMode::CurrentInstance()->GetEventSupervisor()->ResumeEvent(_control_event->GetEventID());
}



void VirtualSprite::SetDirection(uint16 direction) {
	// Nothing complicated needed for lateral directions
	if (direction & (NORTH | SOUTH | EAST | WEST)) {
		_direction = direction;
	}
	// Otherwise if the direction is diagonal we must figure out which way the sprite should face.
	else if (direction & MOVING_NORTHWEST) {
		if (_direction & (FACING_NORTH | FACING_EAST))
			_direction = NW_NORTH;
		else
			_direction = NW_WEST;
	}
	else if (direction & MOVING_SOUTHWEST) {
		if (_direction & (FACING_SOUTH | FACING_EAST))
			_direction = SW_SOUTH;
		else
			_direction = SW_WEST;
	}
	else if (direction & MOVING_NORTHEAST) {
		if (_direction & (FACING_NORTH | FACING_WEST))
			_direction = NE_NORTH;
		else
			_direction = NE_EAST;
	}
	else if (direction & MOVING_SOUTHEAST) {
		if (_direction & (FACING_SOUTH | FACING_WEST))
			_direction = SE_SOUTH;
		else
			_direction = SE_EAST;
	}
	else {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to set an invalid direction: " << direction << endl;
	}
}



void VirtualSprite::_ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj) {
	// ---------- (1) First check for the case where the player has collided with a hostile enemy sprite
	if (coll_obj != nullptr) {
		EnemySprite* enemy = nullptr;
		if (this == MapMode::CurrentInstance()->GetCamera() && coll_obj->GetType() == ENEMY_TYPE) {
			enemy = reinterpret_cast<EnemySprite*>(coll_obj);
		}
		else if (coll_obj == MapMode::CurrentInstance()->GetCamera() && this->GetType() == ENEMY_TYPE) {
			enemy = reinterpret_cast<EnemySprite*>(this);
		}

		// If these two conditions are true, begin the battle
		if (enemy != nullptr && enemy->HasEnemyParties() == true && (enemy->GetState() == EnemySprite::ACTIVE || enemy->GetState() == EnemySprite::HUNT)
				&& MapMode::CurrentInstance()->AttackAllowed())
		{
			enemy->ChangeState(EnemySprite::INACTIVE);

			BattleMode *BM = new BattleMode();

			string battle_background = enemy->GetBattleBackgroundFile();
			if (battle_background != "")
				BM->GetMedia().SetBackgroundImage(battle_background);

			string enemy_battle_music = enemy->GetBattleMusicFile();
			if (enemy_battle_music != "")
				BM->GetMedia().SetBattleMusic(enemy_battle_music);

			const vector<uint32>& enemy_party = enemy->RetrieveRandomParty();
			for (uint32 i = 0; i < enemy_party.size(); i++) {
				BM->AddEnemy(enemy_party[i]);
			}

			string enemy_battle_script = enemy->GetBattleScriptFile();
			if (enemy_battle_script != "")
				BM->LoadBattleScript(enemy_battle_script);
			MapMode::CurrentInstance()->GetTransitionSupervisor()->StartGameModeTransition(BM);

			// TODO: some sort of map-to-battle transition animation sequence needs to start here
			return;
		}
	}

	// ---------- (2) Adjust the sprite's position if no event was controlling this sprite
	// This sprite is assumed in this case to be controlled by the player since sprites don't move by themselves
	if (_control_event == nullptr) {
		MapMode::CurrentInstance()->GetObjectSupervisor()->AdjustSpriteAroundCollision(this, coll_type, coll_obj);
		return;
	}

	// ---------- (3) Call the appropriate collision resolution function for the various control events
	EVENT_TYPE event_type = _control_event->GetEventType();
	if (event_type == PATH_MOVE_SPRITE_EVENT) {
		PathMoveSpriteEvent* path_event = dynamic_cast<PathMoveSpriteEvent*>(_control_event);
		path_event->_ResolveCollision(coll_type, coll_obj);
	}
	else if (event_type == RANDOM_MOVE_SPRITE_EVENT) {
		RandomMoveSpriteEvent* random_event = dynamic_cast<RandomMoveSpriteEvent*>(_control_event);
		random_event->_ResolveCollision(coll_type, coll_obj);
	}
	else {
		IF_PRINT_WARNING(MAP_DEBUG) << "collision occurred when sprite was controlled by a non-motion event" << endl;
	}
} // void VirtualSprite::_ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj)

// ****************************************************************************
// ********** MapSprite class methods
// ****************************************************************************

MapSprite::MapSprite() :
	_name(ustring()),
	_face_portrait(nullptr),
	_current_animation(ANIM_STANDING_SOUTH),
	_has_running_animations(false),
	_stationary_movement(false),
	_reverse_movement(false),
	_custom_animation_on(false),
	_saved_current_animation(0),
	_next_dialogue(-1),
	_has_available_dialogue(false),
	_has_unseen_dialogue(false)
{
	MapObject::_object_type = SPRITE_TYPE;
	visible = true;
	collidable = true;
}



MapSprite::~MapSprite() {
	if (_face_portrait != nullptr) {
		delete _face_portrait;
		_face_portrait = nullptr;
	}
}



MapSprite* MapSprite::Create(int16 object_id) {
	MapSprite* sprite = new MapSprite();
	sprite->SetObjectID(object_id);
	MapMode::CurrentInstance()->GetObjectSupervisor()->AddObject(sprite);
	return sprite;
}



bool MapSprite::LoadStandardAnimations(std::string filename) {
	// Prepare the four standing and four walking _animations
	for (uint8 i = 0; i < 8; i++)
		_animations.push_back(AnimatedImage());

	// TODO: dirty, dirty hack to support a sprite animation that doesn't have the standard 6 frames per direction
	// This needs to be fixed so sprites can have custom number of frames
	if (filename == "img/sprites/creatures/mak_hound.png") {
			// Load the multi-image, containing 32 frames total
		vector<StillImage> frames(32);
		for (uint8 i = 0; i < 32; i++)
			frames[i].SetDimensions(img_half_width * 2, img_height);

		if (ImageDescriptor::LoadMultiImageFromElementGrid(frames, filename, 4, 7) == false) {
			return false;
		}

		// Add standing frames to _animations
		_animations[ANIM_STANDING_SOUTH].AddFrame(frames[0], _movement_speed);
		_animations[ANIM_STANDING_NORTH].AddFrame(frames[7], _movement_speed);
		_animations[ANIM_STANDING_WEST].AddFrame(frames[14], _movement_speed);
		_animations[ANIM_STANDING_EAST].AddFrame(frames[21], _movement_speed);

		// Add walking frames to _animations
		_animations[ANIM_WALKING_SOUTH].AddFrame(frames[1], _movement_speed);
		_animations[ANIM_WALKING_SOUTH].AddFrame(frames[2], _movement_speed);
		_animations[ANIM_WALKING_SOUTH].AddFrame(frames[3], _movement_speed);
		_animations[ANIM_WALKING_SOUTH].AddFrame(frames[4], _movement_speed);
		_animations[ANIM_WALKING_SOUTH].AddFrame(frames[5], _movement_speed);
		_animations[ANIM_WALKING_SOUTH].AddFrame(frames[6], _movement_speed);

		_animations[ANIM_WALKING_NORTH].AddFrame(frames[8], _movement_speed);
		_animations[ANIM_WALKING_NORTH].AddFrame(frames[9], _movement_speed);
		_animations[ANIM_WALKING_NORTH].AddFrame(frames[10], _movement_speed);
		_animations[ANIM_WALKING_NORTH].AddFrame(frames[11], _movement_speed);
		_animations[ANIM_WALKING_NORTH].AddFrame(frames[12], _movement_speed);
		_animations[ANIM_WALKING_NORTH].AddFrame(frames[13], _movement_speed);

		_animations[ANIM_WALKING_WEST].AddFrame(frames[15], _movement_speed);
		_animations[ANIM_WALKING_WEST].AddFrame(frames[16], _movement_speed);
		_animations[ANIM_WALKING_WEST].AddFrame(frames[17], _movement_speed);
		_animations[ANIM_WALKING_WEST].AddFrame(frames[18], _movement_speed);
		_animations[ANIM_WALKING_WEST].AddFrame(frames[19], _movement_speed);
		_animations[ANIM_WALKING_WEST].AddFrame(frames[20], _movement_speed);

		_animations[ANIM_WALKING_EAST].AddFrame(frames[22], _movement_speed);
		_animations[ANIM_WALKING_EAST].AddFrame(frames[23], _movement_speed);
		_animations[ANIM_WALKING_EAST].AddFrame(frames[24], _movement_speed);
		_animations[ANIM_WALKING_EAST].AddFrame(frames[25], _movement_speed);
		_animations[ANIM_WALKING_EAST].AddFrame(frames[26], _movement_speed);
		_animations[ANIM_WALKING_EAST].AddFrame(frames[27], _movement_speed);
		return true;
	}

	// Load the multi-image, containing 24 frames total
	vector<StillImage> frames(24);
	for (uint8 i = 0; i < 24; i++)
		frames[i].SetDimensions(img_half_width * 2, img_height);

	if (ImageDescriptor::LoadMultiImageFromElementGrid(frames, filename, 4, 6) == false) {
		return false;
	}

	// Add standing frames to _animations
	_animations[ANIM_STANDING_SOUTH].AddFrame(frames[0], _movement_speed);
	_animations[ANIM_STANDING_NORTH].AddFrame(frames[6], _movement_speed);
	_animations[ANIM_STANDING_WEST].AddFrame(frames[12], _movement_speed);
	_animations[ANIM_STANDING_EAST].AddFrame(frames[18], _movement_speed);

	// Add walking frames to _animations
	_animations[ANIM_WALKING_SOUTH].AddFrame(frames[1], _movement_speed);
	_animations[ANIM_WALKING_SOUTH].AddFrame(frames[2], _movement_speed);
	_animations[ANIM_WALKING_SOUTH].AddFrame(frames[3], _movement_speed);
	_animations[ANIM_WALKING_SOUTH].AddFrame(frames[1], _movement_speed);
	_animations[ANIM_WALKING_SOUTH].AddFrame(frames[4], _movement_speed);
	_animations[ANIM_WALKING_SOUTH].AddFrame(frames[5], _movement_speed);

	_animations[ANIM_WALKING_NORTH].AddFrame(frames[7], _movement_speed);
	_animations[ANIM_WALKING_NORTH].AddFrame(frames[8], _movement_speed);
	_animations[ANIM_WALKING_NORTH].AddFrame(frames[9], _movement_speed);
	_animations[ANIM_WALKING_NORTH].AddFrame(frames[7], _movement_speed);
	_animations[ANIM_WALKING_NORTH].AddFrame(frames[10], _movement_speed);
	_animations[ANIM_WALKING_NORTH].AddFrame(frames[11], _movement_speed);

	_animations[ANIM_WALKING_WEST].AddFrame(frames[13], _movement_speed);
	_animations[ANIM_WALKING_WEST].AddFrame(frames[14], _movement_speed);
	_animations[ANIM_WALKING_WEST].AddFrame(frames[15], _movement_speed);
	_animations[ANIM_WALKING_WEST].AddFrame(frames[13], _movement_speed);
	_animations[ANIM_WALKING_WEST].AddFrame(frames[16], _movement_speed);
	_animations[ANIM_WALKING_WEST].AddFrame(frames[17], _movement_speed);

	_animations[ANIM_WALKING_EAST].AddFrame(frames[19], _movement_speed);
	_animations[ANIM_WALKING_EAST].AddFrame(frames[20], _movement_speed);
	_animations[ANIM_WALKING_EAST].AddFrame(frames[21], _movement_speed);
	_animations[ANIM_WALKING_EAST].AddFrame(frames[19], _movement_speed);
	_animations[ANIM_WALKING_EAST].AddFrame(frames[22], _movement_speed);
	_animations[ANIM_WALKING_EAST].AddFrame(frames[23], _movement_speed);

	return true;
} // bool MapSprite::LoadStandardAnimations(std::string filename)



bool MapSprite::LoadRunningAnimations(std::string filename) {
	// Prepare to add the four running _animations
	for (uint8 i = 0; i < 4; i++)
		_animations.push_back(AnimatedImage());

	// Load the multi-image, containing 24 frames total
	vector<StillImage> frames(24);
	for (uint8 i = 0; i < 24; i++)
		frames[i].SetDimensions(img_half_width * 2, img_height);

	if (ImageDescriptor::LoadMultiImageFromElementGrid(frames, filename, 4, 6) == false) {
		return false;
	}

	// Add walking frames to _animations
	_animations[ANIM_RUNNING_SOUTH].AddFrame(frames[1], _movement_speed);
	_animations[ANIM_RUNNING_SOUTH].AddFrame(frames[2], _movement_speed);
	_animations[ANIM_RUNNING_SOUTH].AddFrame(frames[3], _movement_speed);
	_animations[ANIM_RUNNING_SOUTH].AddFrame(frames[1], _movement_speed);
	_animations[ANIM_RUNNING_SOUTH].AddFrame(frames[4], _movement_speed);
	_animations[ANIM_RUNNING_SOUTH].AddFrame(frames[5], _movement_speed);

	_animations[ANIM_RUNNING_NORTH].AddFrame(frames[7], _movement_speed);
	_animations[ANIM_RUNNING_NORTH].AddFrame(frames[8], _movement_speed);
	_animations[ANIM_RUNNING_NORTH].AddFrame(frames[9], _movement_speed);
	_animations[ANIM_RUNNING_NORTH].AddFrame(frames[7], _movement_speed);
	_animations[ANIM_RUNNING_NORTH].AddFrame(frames[10], _movement_speed);
	_animations[ANIM_RUNNING_NORTH].AddFrame(frames[11], _movement_speed);

	_animations[ANIM_RUNNING_WEST].AddFrame(frames[13], _movement_speed);
	_animations[ANIM_RUNNING_WEST].AddFrame(frames[14], _movement_speed);
	_animations[ANIM_RUNNING_WEST].AddFrame(frames[15], _movement_speed);
	_animations[ANIM_RUNNING_WEST].AddFrame(frames[13], _movement_speed);
	_animations[ANIM_RUNNING_WEST].AddFrame(frames[16], _movement_speed);
	_animations[ANIM_RUNNING_WEST].AddFrame(frames[17], _movement_speed);

	_animations[ANIM_RUNNING_EAST].AddFrame(frames[19], _movement_speed);
	_animations[ANIM_RUNNING_EAST].AddFrame(frames[20], _movement_speed);
	_animations[ANIM_RUNNING_EAST].AddFrame(frames[21], _movement_speed);
	_animations[ANIM_RUNNING_EAST].AddFrame(frames[19], _movement_speed);
	_animations[ANIM_RUNNING_EAST].AddFrame(frames[22], _movement_speed);
	_animations[ANIM_RUNNING_EAST].AddFrame(frames[23], _movement_speed);

	_has_running_animations = true;
	return true;
} // bool MapSprite::LoadRunningAnimations(std::string filename)



bool MapSprite::LoadAttackAnimations(std::string filename) {
	// Prepare the four standing and four walking _animations
	for (uint8 i = 0; i < 8; i++)
		_animations.push_back(AnimatedImage());

	// Load the multi-image, containing 24 frames total
	vector<StillImage> frames(5);
	for (uint8 i = 0; i < 5; i++)
		frames[i].SetDimensions(img_half_width * 4, img_height);

	if (ImageDescriptor::LoadMultiImageFromElementGrid(frames, filename, 1, 5) == false) {
		return false;
	}

	// Add attack frames to _animations
	_animations[ANIM_ATTACKING_EAST].AddFrame(frames[0], _movement_speed);
	_animations[ANIM_ATTACKING_EAST].AddFrame(frames[1], _movement_speed);
	_animations[ANIM_ATTACKING_EAST].AddFrame(frames[2], _movement_speed);
	_animations[ANIM_ATTACKING_EAST].AddFrame(frames[3], _movement_speed);
	_animations[ANIM_ATTACKING_EAST].AddFrame(frames[4], _movement_speed);

	return true;
} // bool MapSprite::LoadAttackAnimations(std::string filename)



void MapSprite::LoadFacePortrait(std::string pn) {
	if (_face_portrait != nullptr) {
		delete _face_portrait;
	}

	_face_portrait = new StillImage();
	if (_face_portrait->Load(pn) == false) {
		delete _face_portrait;
		_face_portrait = nullptr;
		PRINT_ERROR << "failed to load face portrait" << endl;
	}
}



void MapSprite::Update() {
	// This call will update the sprite's position and perform collision detection
	VirtualSprite::Update();
// 	if (_animations.empty()) return;
// 	PRINT_DEBUG << "CA: " << _current_animation << ", size: " << _animations.size() << endl;
	_animations[_current_animation].Update();
}


// Draw the appropriate sprite frame at the correct position on the screen
void MapSprite::Draw() {
	if (MapObject::ShouldDraw() == true) {
		_animations[_current_animation].Draw();

		if (VideoManager->DEBUG_IsGraphicsDebuggingEnabled() == true)
			DEBUG_DrawCollisionBox();
	}
}



void MapSprite::DrawDialog() {
    // Update the alpha of the dialogue icon according to it's distance from the player sprite
	const float DIALOGUE_ICON_VISIBLE_RANGE = 10.0f;

    if (MapObject::ShouldDraw() == false)
		return;

	if (_has_available_dialogue == true && _has_unseen_dialogue == true && !MapMode::CurrentInstance()->IsCameraOnVirtualFocus()) {
		Color icon_color(1.0f, 1.0f, 1.0f, 0.0f);
		float icon_alpha = 1.0f - (fabs(ComputeXLocation() - MapMode::CurrentInstance()->GetCamera()->ComputeXLocation()) + fabs(ComputeYLocation() -
			MapMode::CurrentInstance()->GetCamera()->ComputeYLocation())) / DIALOGUE_ICON_VISIBLE_RANGE;

		if (icon_alpha <= 0.0f)
			return;
 		icon_color.SetAlpha(icon_alpha);

		// TODO: there's a bug here. The move relative assumes that the last draw position was for the current sprite's location, so it just moves the
		// cursor up above the head of the sprite to draw the icon. However, this is almost never the case, and we can't know what the current draw cursor
		// position is. We need to save the computed draw position of map objects when they are drawn, and then re-use that value here to draw the
		// icon.
		VideoManager->MoveRelative(0, -GetImgHeight());
		MapMode::CurrentInstance()->GetDialogueIcon().Draw(icon_color);
	}
}



void MapSprite::AddDialogueReference(uint32 dialogue_id) {
	_dialogue_references.push_back(dialogue_id);
    UpdateDialogueStatus();
	// TODO: The call above causes a warning to be printed out if the sprite has been created but the dialogue has not yet.
	// Map scripts typically create all sprites first (including their dialogue references) before creating the dialogues.
	// We need a safe way to add dialogue references to the sprite without causing these warnings to be printed when the
	// map is loading.
}



void MapSprite::ClearDialogueReferences() {
    _dialogue_references.clear();
    UpdateDialogueStatus();
}



void MapSprite::RemoveDialogueReference(uint32 dialogue_id) {
    // Remove all dialogues with the given reference (for the case, the same dialogue was add several times)
    for (uint32 i = 0; i < _dialogue_references.size(); i++) {
        if (_dialogue_references[i] == dialogue_id)
            _dialogue_references.erase(_dialogue_references.begin()+i);
    }
    UpdateDialogueStatus();
}



void MapSprite::InitiateDialogue() {
	if (_dialogue_references.empty() == true) {
		IF_PRINT_WARNING(MAP_DEBUG) << "sprite: " << object_id << " has no dialogue referenced" << endl;
		return;
	}

	SaveState();
	_moving = false;
	SetDirection(CalculateOppositeDirection(MapMode::CurrentInstance()->GetCamera()->GetDirection()));
	MapMode::CurrentInstance()->GetDialogueSupervisor()->BeginDialogue(_dialogue_references[_next_dialogue]);
	IncrementNextDialogue();
}



void MapSprite::UpdateDialogueStatus() {
	_has_available_dialogue = false;
	_has_unseen_dialogue = false;

	for (uint32 i = 0; i < _dialogue_references.size(); i++) {
		MapDialogue* dialogue = MapMode::CurrentInstance()->GetDialogueSupervisor()->GetDialogue(_dialogue_references[i]);
		if (dialogue == nullptr) {
			IF_PRINT_WARNING(MAP_DEBUG) << "sprite: " << object_id << " is referencing unknown dialogue: " << _dialogue_references[i] << endl;
			continue;
		}

		if (dialogue->IsAvailable()) {
			_has_available_dialogue = true;
			if (_next_dialogue < 0)
				_next_dialogue = i;
		}
		if (dialogue->HasAlreadySeen() == false) {
			_has_unseen_dialogue = true;
		}
	}

	// TODO: if the sprite has available, unseen dialogue and the _next_dialogue pointer is pointing to a dialogue that is already seen, change it
	// to point to the unseen available dialogue
}



void MapSprite::IncrementNextDialogue() {
	// Handle the case where no dialogue is referenced by the sprite
	if (_next_dialogue < 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function invoked when no dialogues were referenced by the sprite" << endl;
		return;
	}

	int16 last_dialogue = _next_dialogue;

	while (true) {
		_next_dialogue++;
		if (static_cast<uint16>(_next_dialogue) >= _dialogue_references.size())
			_next_dialogue = 0;

		MapDialogue* dialogue = MapMode::CurrentInstance()->GetDialogueSupervisor()->GetDialogue(_dialogue_references[_next_dialogue]);
		if (dialogue != nullptr && dialogue->IsAvailable() == true) {
			return;
		}
		// If this case occurs, all dialogues are now unavailable
		else if (_next_dialogue == last_dialogue) {
			IF_PRINT_WARNING(MAP_DEBUG) << "all referenced dialogues are now unavailable for this sprite" << endl;
			_has_available_dialogue = false;
			_has_unseen_dialogue = false;
			return;
		}
	}
}




void MapSprite::SetNextDialogue(uint16 next) {
	// If a negative value is passed in, this means the user wants to disable
	if (next >= _dialogue_references.size()) {
		IF_PRINT_WARNING(MAP_DEBUG) << "tried to set _next_dialogue to an value that was invalid (exceeds maximum bounds): " << next << endl;
	}
	else {
		_next_dialogue = static_cast<int16>(next);
	}
}



void MapSprite::SaveState() {
	VirtualSprite::SaveState();
	_saved_current_animation = _current_animation;
}



void MapSprite::RestoreState() {
	VirtualSprite::RestoreState();
	_current_animation = _saved_current_animation;
}



void MapSprite::_ChangeCurrentAnimation() {
	// Don't change the animation if a custom one has been selected
	if (_custom_animation_on)
		return;

	uint8 last_animation = _current_animation;
	bool stationary_animation = (_moving == false && _stationary_movement == false);

	// TODO: It would be nice to replace all this conditional logic with a lookup table to find the current animation
	if (stationary_animation == true) {
		if (_direction & FACING_NORTH) {
			_current_animation = ANIM_STANDING_NORTH;
		}
		else if (_direction & FACING_SOUTH) {
			_current_animation = ANIM_STANDING_SOUTH;
		}
		else if (_direction & FACING_WEST) {
			_current_animation = ANIM_STANDING_WEST;
		}
		else if (_direction & FACING_EAST) {
			_current_animation = ANIM_STANDING_EAST;
		}
	}
	else if (_has_running_animations == true && _running == true) {
		if (_direction & FACING_NORTH) {
			_current_animation = ANIM_RUNNING_NORTH;
		}
		else if (_direction & FACING_SOUTH) {
			_current_animation = ANIM_RUNNING_SOUTH;
		}
		else if (_direction & FACING_WEST) {
			_current_animation = ANIM_RUNNING_WEST;
		}
		else if (_direction & FACING_EAST) {
			_current_animation = ANIM_RUNNING_EAST;
		}
	}
	// All other cases use the walking animations
	else {
		if (_direction & FACING_NORTH) {
			_current_animation = ANIM_WALKING_NORTH;
		}
		else if (_direction & FACING_SOUTH) {
			_current_animation = ANIM_WALKING_SOUTH;
		}
		else if (_direction & FACING_WEST) {
			_current_animation = ANIM_WALKING_WEST;
		}
		else if (_direction & FACING_EAST) {
			_current_animation = ANIM_WALKING_EAST;
		}
	}
	// If movement animation is reversed, swap the current animation with it's directional opposite
	if (_reverse_movement == true) {
		switch (_current_animation) {
			case ANIM_STANDING_SOUTH:
				_current_animation = ANIM_STANDING_NORTH;
				break;
			case ANIM_STANDING_NORTH:
				_current_animation = ANIM_STANDING_SOUTH;
				break;
			case ANIM_STANDING_WEST:
				_current_animation = ANIM_STANDING_EAST;
				break;
			case ANIM_STANDING_EAST:
				_current_animation = ANIM_STANDING_WEST;
				break;
			case ANIM_WALKING_SOUTH:
				_current_animation = ANIM_WALKING_NORTH;
				break;
			case ANIM_WALKING_NORTH:
				_current_animation = ANIM_WALKING_SOUTH;
				break;
			case ANIM_WALKING_WEST:
				_current_animation = ANIM_WALKING_EAST;
				break;
			case ANIM_WALKING_EAST:
				_current_animation = ANIM_WALKING_WEST;
				break;
			case ANIM_RUNNING_SOUTH:
				_current_animation = ANIM_RUNNING_NORTH;
				break;
			case ANIM_RUNNING_NORTH:
				_current_animation = ANIM_RUNNING_SOUTH;
				break;
			case ANIM_RUNNING_WEST:
				_current_animation = ANIM_RUNNING_EAST;
				break;
			case ANIM_RUNNING_EAST:
				_current_animation = ANIM_RUNNING_WEST;
				break;
		}
	}

	// If the direction changed while _moving, update the animation timer on the new animated image to match the old one.
	// This is so that movement animations do not appear to "restart" when a sprite changes directions.
	if (stationary_animation == false && _current_animation != last_animation) {
		_animations[_current_animation].SetTimeProgress(_animations[last_animation].GetTimeProgress());
		_animations[last_animation].ResetAnimation();
	}

	// Reset the progress of the previous animation if the animation changed
	if (_current_animation != last_animation) {
		_animations[last_animation].ResetAnimation();
	}
} // void MapSprite::_ChangeCurrentAnimation()

// *****************************************************************************
// ********** EnemySprite class methods
// *****************************************************************************

EnemySprite::EnemySprite() :
	_state(INACTIVE),
	_spawned_state(HUNT),
	_zone(nullptr),
	_fade_color(1.0f, 1.0f, 1.0f, 0.0f),
	_pursuit_range(8.0f),
	_directional_change_time(2500),
	_fade_time(4000),
	_battle_music_file(""),
	_battle_background_file(""),
	_battle_script_file("")
{
	MapObject::_object_type = ENEMY_TYPE;
	visible = true;
	Reset();
}



EnemySprite* EnemySprite::Create(int16 object_id) {
	EnemySprite* sprite = new EnemySprite();
	sprite->SetObjectID(object_id);
	MapMode::CurrentInstance()->GetObjectSupervisor()->AddObject(sprite);
	return sprite;
}



void EnemySprite::Reset() {
	updatable = false;
	collidable = false;
	_state = INACTIVE;
	_state_timer.Reset();
	_fade_color.SetAlpha(0.0f);
	_returning_to_zone = false;
}



void EnemySprite::AddEnemy(uint32 enemy_id) {
	if (_enemy_parties.empty()) {
		IF_PRINT_WARNING(MAP_DEBUG) << "can not add new enemy when no parties have been declared" << endl;
		return;
	}

	_enemy_parties.back().push_back(enemy_id);

	// Make sure that the GlobalEnemy has already been created for this enemy_id
	if (MAP_DEBUG) {
		if (MapMode::CurrentInstance()->IsEnemyLoaded(enemy_id) == false) {
			PRINT_WARNING << "enemy to add has id " << enemy_id << ", which does not exist in MapMode::_enemies" << endl;
		}
	}
}



const std::vector<uint32>& EnemySprite::RetrieveRandomParty() {
	if (_enemy_parties.empty()) {
		PRINT_ERROR << "call invoked when no enemy parties existed, adding default party" << endl;
		_enemy_parties.push_back(vector<uint32>(1));
	}

	if (_enemy_parties.size() == 1) {
		return _enemy_parties[0];
	}
	else {
		return _enemy_parties[rand() % _enemy_parties.size()];
	}
}



void EnemySprite::ChangeState(ENEMY_STATE new_state) {
	if (_state == new_state)
		return;

	_state = new_state;
	switch (_state) {
		case INACTIVE:
			Reset();
			if (_zone)
				_zone->EnemyDead();
			break;
		case SPAWN:
			updatable = true;
			collidable = true;
			_state_timer.Initialize(_fade_time);
			_state_timer.Run();
			_fade_color.SetAlpha(0.0f);
			break;
		case ACTIVE:
			updatable = true;
			collidable = true;
			break;
		case HUNT:
			updatable = true;
			collidable = true;
			_moving = true;
			_state_timer.Initialize(_directional_change_time);
			_state_timer.Run();
			break;
		case DISSIPATE:
			_state_timer.Initialize(_fade_time);
			_state_timer.Run();
			_fade_color.SetAlpha(1.0f);
			break;
		default:
			PRINT_ERROR << "function received unknown state argument: " << _state << endl;
			updatable = false;
			collidable = false;
	}
}



void EnemySprite::Update() {
	switch (_state) {
		// Nothing should be done in this state. If the enemy has a zone, the zone will change the state back to spawning when appropriate
		case INACTIVE:
			break;

		// Gradually increase the fade color alpha while the sprite is fading in spawning
		case SPAWN:
			_state_timer.Update();
			if (_state_timer.IsFinished() == true) {
				_fade_color.SetAlpha(1.0f);
				ChangeState(_spawned_state);
			}
			else {
				_fade_color.SetAlpha(_state_timer.PercentComplete());
			}
			break;

		case ACTIVE:
			MapSprite::Update();
			break;

		// Set the sprite's direction so that it seeks to collide with the map camera's position
		case HUNT:
			// Holds the x and y deltas between the sprite and map camera coordinate pairs
			float xdelta, ydelta;
			_state_timer.Update();

			xdelta = ComputeXLocation() - MapMode::CurrentInstance()->GetPlayerSprite()->ComputeXLocation();
			ydelta = ComputeYLocation() - MapMode::CurrentInstance()->GetPlayerSprite()->ComputeYLocation();

			// If the sprite has moved outside of its zone and it should not, reverse the sprite's direction
			if (_zone != nullptr && _zone->IsInsideZone(x_position, y_position) == false && _zone->IsRoamingRestrained()) {
				// Make sure it wasn't already out (stuck on boundaries fix)
				if (_returning_to_zone == false) {
					SetDirection(CalculateOppositeDirection(GetDirection()));
					// The sprite is now finding its way back into the zone
					_returning_to_zone = true;
				}
			}
			// Otherwise, determine the direction that the sprite should move if the camera is within the sprite's aggression range
			else {
				_returning_to_zone = false;

				// Enemies will only pursue if the camera is inside the zone, or the zone is non-restrictive
				// TODO: this logic needs to be revisited; it is messy and should be cleaned up
				if (MapMode::CurrentInstance()->AttackAllowed() && (_zone == nullptr || (fabs(xdelta) <= _pursuit_range && fabs(ydelta) <= _pursuit_range
					 && (!_zone->IsRoamingRestrained() ||
					 _zone->IsInsideZone(MapMode::CurrentInstance()->GetPlayerSprite()->x_position, MapMode::CurrentInstance()->GetPlayerSprite()->y_position)))))
				{
					if (xdelta > -0.5 && xdelta < 0.5 && ydelta < 0)
						SetDirection(SOUTH);
					else if (xdelta > -0.5 && xdelta < 0.5 && ydelta > 0)
						SetDirection(NORTH);
					else if (ydelta > -0.5 && ydelta < 0.5 && xdelta > 0)
						SetDirection(WEST);
					else if (ydelta > -0.5 && ydelta < 0.5 && xdelta < 0)
						SetDirection(EAST);
					else if (xdelta < 0 && ydelta < 0)
						SetDirection(MOVING_SOUTHEAST);
					else if (xdelta < 0 && ydelta > 0)
						SetDirection(MOVING_NORTHEAST);
					else if (xdelta > 0 && ydelta < 0)
						SetDirection(MOVING_SOUTHWEST);
					else
						SetDirection(MOVING_NORTHWEST);
				}
				// If the sprite is not within the aggression range, pick a random direction to move
				else {
					if (_state_timer.IsFinished() == true) {
						// Sets to one of the 12 Sprite Direction Constants found in map_utils.h
						// TODO: this currently gives double the probabily of selecting the four types of directional movement. Rectify this
						SetDirection(1 << hoa_utils::RandomBoundedInteger(0, 11));
						_state_timer.Reset();
						_state_timer.Run();
					}
				}
			}

			// Roaming enemies are updated the same way as any other sprite in the explore state. In other states, they stop movement and
			// simply "walk in place".
			if (MapMode::CurrentInstance()->CurrentState() != STATE_EXPLORE) {
				_animations[_current_animation].Update();
			}
			else {
				MapSprite::Update();
			}
			break;

		// Gradually decrease the fade color alpha while the sprite is fading out and disappearing
		case DISSIPATE:
			_state_timer.Update();
			if (_state_timer.IsFinished() == true) {
				_fade_color.SetAlpha(0.0f);
				ChangeState(INACTIVE);
			}
			else {
				_fade_color.SetAlpha(1.0f - _state_timer.PercentComplete());
			}
			break;

		default:
			break;
	}
} // void EnemySprite::Update()



void EnemySprite::Draw() {
	if (_state == INACTIVE)
		return;

	if (MapObject::ShouldDraw() == true) {
		if (_state == SPAWN || _state == DISSIPATE) {
			_animations[_current_animation].Draw(_fade_color);
		}
		else {
			_animations[_current_animation].Draw();
		}

		if (VideoManager->DEBUG_IsGraphicsDebuggingEnabled() == true)
			DEBUG_DrawCollisionBox();
	}
}

} // namespace private_map

} // namespace hoa_map
