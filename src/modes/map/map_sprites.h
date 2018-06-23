///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_sprites.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for map mode sprite code.
*** *****************************************************************************/

#pragma once

// Allacrost utilities
#include "utils.h"
#include "defs.h"

// Allacrost engines
#include "video.h"

// Local map mode headers
#include "map_utils.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "map_zones.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief A special type of sprite with no physical image
***
*** The VirtualSprite is a special type of MapObject because it has no physical form (no image). Virtual sprites may be
*** manipulated to move around on the screen just like any other sprite. The visible and collidable properties are set
*** to false during construction. Some possible uses for a virtual sprite include:
***
*** - A mobile focusing point for the map camera
*** - An impassible map location for other sprites and objects in a specific context
***
*** \note The VirtualSprite class serves as a base class for all other types of sprites.
*** ***************************************************************************/
class VirtualSprite : public MapObject {
public:
	VirtualSprite();

	~VirtualSprite()
		{}

	//! \brief Updates the virtual object's position if it is moving, otherwise does nothing.
	virtual void Update();

	//! \brief Does nothing since virtual sprites have no image to draw
	virtual void Draw()
		{}

	/** \brief Used to check if a sprite is facing in a particular direction
	*** \param direction The direction to check. Should be one of the following four directional constants: NORTH, SOUTH, EAST, WEST
	*** \return True if the sprite is facing the direction being checked
	**/
	bool IsFacingDirection(uint16 direction) const;

	/** \brief Sets the sprite's direction to a random value
	*** This function is used mostly for the ActionRandomMove class.
	**/
	virtual void SetRandomDirection();

	/** \brief Calculates the distance the sprite should move given its velocity (speed and direction)
	*** \return A floating point value representing the distance moved
	*** \note This method does not check if the "moving" member is true but does factor in the "is_running"
	*** member in its calculation.
	**/
	float CalculateDistanceMoved();

	/** \brief Modifies the sprite's position and checks if the new position is valid
	*** \param direction The direction to modify the sprite's position in (NORTH, SOUTH, EAST, and WEST are the only valid values)
	*** \param distance The distance to move the sprite in the specified direction
	*** \param disable_collision_notification If true, do not generate new notification events for collisions (defaults to false)
	*** \return True if the sprite's position was successfully modified to a new valid location
	***
	*** After moving the sprite to its new position, collision detection will be run to check that the sprite did not collide with anything. If
	*** a collision was detected, the sprite's original position will be restored and a new MapCollisionNotificationEvent will be sent to the
	*** notification engine with information about the collision. The caller has the option to not submit a collision notification if they so choose.
	**/
	bool ModifySpritePosition(uint16 direction, float distance, bool disable_collision_notification = false);

	/** \brief Declares that an event is taking control over the sprite
	*** \param event The sprite event that is assuming control
	*** This function is not safe to call when there is an event already controlling the sprite.
	*** The previously controlling event should first release control (which will set the control_event
	*** member to nullptr) before a new event acquires it. The acquisition will be successful regardless
	*** of whether there is currently a controlling event or not, but a warning will be printed in the
	*** improper case.
	**/
	void AcquireControl(SpriteEvent* event);

	/** \brief Declares that an event is releasing control over the sprite
	*** \param event The sprite event that is releasing control
	*** The reason why the SpriteEvent has to pass a pointer to itself in this call is to make sure
	*** that this event is still controlling the sprite. If the control has switched to another event
	*** (because another event acquired it before this event released it), a warning will be printed
	*** and no change will be made (the control event will not change).
	**/
	void ReleaseControl(SpriteEvent* event);

	/** \brief Saves the state of the sprite
	*** Attributes saved: direction, speed, moving state
	**/
	virtual void SaveState();

	/** \brief Restores the saved state of the sprite
	*** Attributes restored: direction, speed, moving state
	**/
	virtual void RestoreState();

	/** \name Class Member Access Functions
	*** These functions are specifically written to enable Lua to access the members of this class.
	**/
	//@{
	uint16 GetDirection() const
		{ return _direction; }

	/** \note This method takes into account the current direction when setting the new direction
	*** in the case of diagonal movement. For example, if the sprite is currently facing north
	*** and this function indicates that the sprite should move northwest, it will face north
	*** during the northwest movement.
	**/
	virtual void SetDirection(uint16 direction);

	float GetMovementSpeed() const
		{ return _movement_speed; }

	virtual void SetMovementSpeed(float speed)
		{ _movement_speed = speed; }

	bool IsMoving() const
		{ return _moving; }

	virtual void SetMoving(bool moving)
		{ _moving = moving; }

	bool IsRunning() const
		{ return _running; }

	virtual void SetRunning(bool running)
		{ _running = running; }

	bool IsStateSaved() const
		{ return _state_saved; }
	//@}

protected:
	/** \brief A bit-mask for the sprite's draw orientation and direction vector.
	*** This member determines both where to move the sprite (8 directions) and
	*** which way the sprite is facing (4 directions). See the Sprite direction
	*** constants for the values that this member may be set to.
	**/
	uint16 _direction;

	//! \brief The speed at which the sprite moves around the map.
	float _movement_speed;

	/** \brief Set to true when the sprite is currently in motion.
	*** \note This does not necessarily mean that the sprite actually is moving, but rather
	*** that the sprite is <i>trying</i> to move in a certain direction.
	**/
	bool _moving;

	//! \brief True when the sprite movement is running, false and movement will be by walking
	bool _running;

	//! \brief A pointer to the event that is controlling the action of this sprite. If nullptr, no event is controlling
	SpriteEvent* _control_event;

	/** \name Saved state attributes
	*** These attributes are used to save and restore the state of a VirtualSprite
	**/
	//@{
	//! \brief Indicates if the other saved members are valid because the state has recently been saved
	bool _state_saved;
	uint16 _saved_direction;
	float _saved_movement_speed;
	bool _saved_moving;
	//@}

	/** \brief Determines an appropriate resolution when the sprite collides with an obstruction
	*** \param coll_type The type of collision that has occurred
	*** \param coll_obj A pointer to the MapObject that the sprite has collided with, if any
	**/
	void _ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj);
}; // class VirtualSprite : public MapObject


/** ****************************************************************************
*** \brief A mobile map object with which the player can interact with.
***
*** Map sprites are animate, mobile, living map objects. Although there is
*** but this single class to represent all the map sprites in the game, they can
*** divided into types such as NPCs, friendly creatures, and enemies. The fact
*** that there is only one class for representing several sprite types is the
*** reason why many of the class members are pointers. For example, we don't
*** need dialogue for a dog sprite.
*** ***************************************************************************/
class MapSprite : public VirtualSprite {
public:
	MapSprite();

	~MapSprite();

	/** \brief Creates an instance of the class and registers it with the object supervisor
	*** \param object_id The object ID of this sprite
	*** \return A pointer to the instance of the sprite created
	**/
	static MapSprite* Create(int16 object_id);
// 		{ return MapObject::_Create<MapSprite>(object_id); }

	// ---------- Public methods

	/** \brief Loads the image containing the standard animations for the sprite
	*** \param filename The name of the image file holding the standard walking animations
	*** \return False if there was a problem loading the sprite.
	**/
	bool LoadStandardAnimations(std::string filename);

	/** \brief Loads the image containing the running animations for the sprite
	*** \param filename The name of the image file
	*** \return False if the animations were not created successfully.
	**/
	bool LoadRunningAnimations(std::string filename);

	/** \brief Loads the image containing the attack animations for the sprite
	*** \param filename The name of the image file
	*** \return False if the animations were not created successfully.
	**/
	bool LoadAttackAnimations(std::string filename);

	void LoadFacePortrait(std::string pn);

	//! \brief Updates the sprite's position and state.
	virtual void Update();

	//! \brief Draws the sprite frame in the appropriate position on the screen, if it is visible.
	virtual void Draw();

    //! \brief Draws the dialogue icon at the top of the sprite
    virtual void DrawDialog();

	/** \brief Sets the sprite's direction to a random value and updates the new animation appropriately
	*** \note Do not call this function until after you have loaded all of the standard sprite animations.
	*** Otherwise you will get a seg fault.
	**/
	void SetRandomDirection()
		{ VirtualSprite::SetRandomDirection(); _ChangeCurrentAnimation(); }

	bool IsStationaryMovement() const
		{ return _stationary_movement; }

	void SetStationaryMovement(bool stationary)
		{ if (_stationary_movement != stationary) { _stationary_movement = stationary; _ChangeCurrentAnimation(); } }

	bool IsReverseMovement() const
		{ return _reverse_movement; }

	/** \note Enabling reverse movment will instantly flip the direction the sprite appears to be facing, even if they are
	*** stationary. If you want to reverse the movement but not have the sprite suddenly change their facing direction, you'll
	*** need to also call SetDirection() with the opposite direction that you want the sprite to be facing.
	**/
	void SetReverseMovement(bool reverse)
		{ if (_reverse_movement != reverse) { _reverse_movement = reverse; _ChangeCurrentAnimation();} }

	/** \brief Adds a new reference to a dialogue that the sprite uses
	*** \param dialogue_id The ID number of the dialogue
	*** \note It is valid for a dialogue to be referenced more than once by a sprite
	**/
	void AddDialogueReference(uint32 dialogue_id);

    /** \brief Removes all dialogue references from a sprite
    **/
	void ClearDialogueReferences();

	/** \brief Removes a specific dialogue reference from a sprite
	*** \param dialogue_id The ID number of the dialogue that should be removed
	**/
	void RemoveDialogueReference(uint32 dialogue_id);

	/** \brief Begins a new dialogue with this sprite using its next referenced dialogue
	***
	*** If the sprite has no dialogues referenced or has dialogues that are referenced but are unavailable,
	*** a warning will be printed and no dialogue will take place. It is the caller's responsibility to first
	*** check that the sprite has dialogue available.
	**/
	void InitiateDialogue();

	//! \brief Updates all dialogue status members based on the status of all referenced dialogues
	void UpdateDialogueStatus();

	//! \brief Increments the next_dialogue member to index the proceeding dialogue
	void IncrementNextDialogue();

	/** \brief Sets the next dialogue member for the sprite
	*** \param next The index value of the dialogue_references vector to set the next_dialogue member to
	*** \note You can not set the next_dialogue member to a negative number. This could cause run-time errors if it was supported here.
	**/
	void SetNextDialogue(uint16 next);

	/** \brief This method will save the state of a sprite.
	*** Attributes saved: direction, speed, moving state, name
	*** current animation.
	**/
	virtual void SaveState();

	/** \brief This method will load the saved state of a sprite.
	*** Attributes loaded: direction, speed, moving state, name
	*** current animation.
	*** \return false if there was no saved state, true otherwise.
	**/
	virtual void RestoreState();

	//! \name Class Member Access Functions
	//@{
	/** \note Do not call this function until after you have loaded all of the standard sprite animations.
	*** Otherwise you will get a seg fault.
	**/
	void SetDirection(uint16 direction)
		{ if (_direction != direction) { VirtualSprite::SetDirection(direction); _ChangeCurrentAnimation(); } }

	void SetMoving(bool moving)
		{ if (_moving != moving) { _moving = moving; _ChangeCurrentAnimation(); } }

	void SetRunning(bool running)
		{ if (_running != running) { _running = running; _ChangeCurrentAnimation(); } }

	// TODO: needs to be a ustring
	void SetName(std::string na)
		{ _name = hoa_utils::MakeUnicodeString(na); }

	void SetCurrentAnimation(uint8 anim)
		{ _current_animation = anim; }

	uint8 GetCurrentAnimation() const
		{ return _current_animation; }

	hoa_video::AnimatedImage& GetCurrentAnimation()
		{ return _animations[_current_animation]; }

	/** \brief Retrieves the image corresponding to a particular animation
	*** \return A pointer to the AnimatedImage. Returns nullptr if the requested animation did not exist
	**/
	hoa_video::AnimatedImage* GetAnimation(uint8 animation)
		{ return (animation >= _animations.size() ? nullptr : &(_animations.at(animation))); }

	bool HasAvailableDialogue() const
		{ return _has_available_dialogue; }

	bool HasUnseenDialogue() const
		{ return _has_unseen_dialogue; }

	hoa_utils::ustring& GetName()
		{ return _name; }

	hoa_video::StillImage* GetFacePortrait() const
		{ return _face_portrait; }

	//! \brief Returns the next dialogue to reference (negative value returned if no dialogues are referenced)
	int16 GetNextDialogue() const
		{ return _next_dialogue; }

	//! \brief Gets the ID value of the dialogue that will be the next to be referenced by the sprite
	uint32 GetNextDialogueID() const // TODO: check invalid indexing
		{ return _dialogue_references[_next_dialogue]; }

	//! \brief Returns the number of dialogues referenced by the sprite (including duplicates)
	uint16 GetNumberDialogueReferences() const
		{ return _dialogue_references.size(); }

	//! \brief Set to true for a custom animation
	void SetCustomAnimation(bool on_or_off)
		{ _custom_animation_on = on_or_off; }
	//@}


protected:
	//! \brief The name of the sprite, as seen by the player in the game.
	hoa_utils::ustring _name;

	/** \brief A pointer to the face portrait of the sprite, as seen in dialogues and menus.
	*** \note Not all sprites have portraits, in which case this member will be nullptr
	**/
	hoa_video::StillImage* _face_portrait;

	//! \brief The index to the animations vector containing the current sprite image to display
	uint8 _current_animation;

	//! \brief Set to true if the sprite has running animations loaded
	bool _has_running_animations;

	//! \brief When true, the sprite will always be drawn with a movement animation, even if they are not physically moving
	bool _stationary_movement;

	//! \brief When true, the sprite standing and movement animations will be opposite of the direction the sprite is facing
	bool _reverse_movement;

	//! \brief True if a custom animation is currently in use
	bool _custom_animation_on;

	/** \name Saved state attributes
	*** These attributes are used to save and load the state of a VirtualSprite
	**/
	//@{
	uint8 _saved_current_animation;
	//@}

	/** \brief A vector containing all the sprite's various animations.
	*** The first four entries in this vector are the walking animation frames.
	*** They are ordered from index 0 to 3 as: down, up, left, right. Additional
	*** animations may follow.
	**/
	std::vector<hoa_video::AnimatedImage> _animations;

	//! \brief Contains the id values of all dialogues referenced by the sprite
	std::vector<uint32> _dialogue_references;

	/** \brief An index to the dialogue_references vector, representing the next dialogue the sprite should reference
	*** A negative value indicates that the sprite has no dialogue.
	**/
	int16 _next_dialogue;

	/** \brief True if the sprite references at least one available dialogue
	*** \note A dialogue may become unavailable if it reaches its max view count
	**/
	bool _has_available_dialogue;

	//! \brief True if at least one dialogue referenced by this sprite has not yet been viewed -and- is available to be viewed
	bool _has_unseen_dialogue;

	//! \brief Called when a change to the sprite takes place that may require a different animation to be displayed
	void _ChangeCurrentAnimation();
}; // class MapSprite : public VirtualSprite


/** ****************************************************************************
*** \brief A mobile map object that represents a hostile force
***
*** Enemy sprites have all the same features and functionality as a map sprite.
*** In addition to this, they have some additional data and methods that are commonly
*** needed for enemies encountered on a map, including:
***
*** - State information to determine if an enemy is spawning, dead, etc.
*** - Ability to be controlled by an EnemyZone, used for restricting the area where an enemy may roam
*** - Battle data to determine what enemies, music, etc. are loaded when a battle begins
***
*** Enemies typically are either spawned and controlled by their owning enemy zones,
*** or are controlled through scripting if they do not belong to any zones. You want to
*** be very careful about performing any custom scripting for an enemy controlled by a
*** zone, as the zone runs update logic that may counteract what you are trying to do
*** with an enemy.
***
*** Every enemy has a state that determines how the enemy is being updated and drawn.
*** This is true regardless of whether or not the enemy is controlled by a zone, but
*** the order of state logic defers between the two.
*
*** There are really two types of enemy sprites. The first type behave just like
*** map sprites and can have scripted movement sequences. The second type belong
*** to EnemyZones, where they fade into existence and pursue after the player's
*** sprite should the player enter the zone.
***
*** An enemy sprite in a zone can be in one of 3 states: SPAWNING, HOSTILE or DEAD.
*** In the spawning state, the enemy becomes gradually visible, is immobile, and
*** cannot be touched or attacked. In the hostile state, the enemies roams the map
*** and will cause a battle if touched by the player. In the dead state, the enemy
*** is invisible and waits for the EnemyZone to reset it in another position, so
*** that it may spawn once more.
*** ***************************************************************************/
class EnemySprite : public MapSprite {
public:
	//! \brief The possible states that the enemy sprite may be in
	enum ENEMY_STATE {
		INACTIVE,     //<! Enemy is in a "dead" state, waiting to be spawned or made active by a zone or script call
		SPAWN,        //<! Enemy is in the process of "fading in"
		ACTIVE,       //<! Fully visible and active. Behaves like a standard map sprite, even if inside a zone.
		HUNT,         //<! Roaming around and will pursue the player if they get too close
		DISSIPATE     //<! Enemy is in the process of disappearing, either due to death or a retreat
	};

	EnemySprite();

	/** \brief Creates an instance of the class and registers it with the object supervisor
	*** \param object_id The object ID of this sprite
	*** \return A pointer to the instance of the sprite created
	**/
	static EnemySprite* Create(int16 object_id);

	//! \brief Resets various members of the class so that the enemy is inactive, invisible, and does not produce a collision
	void Reset();

	//! \brief Updates the sprite's position and state.
	virtual void Update();

	//! \brief Draws the sprite frame in the appropriate position on the screen, if it is visible.
	virtual void Draw();

	/** \brief Adds a new empty vector to the _enemy_parties member
	*** \note Make sure to populate this vector by adding at least one enemy!
	**/
	void NewEnemyParty()
		{ _enemy_parties.push_back(std::vector<uint32>()); }

	/** \brief Adds an enemy with the specified ID to the last party in _enemy_parties
	*** \param enemy_id The ID of the enemy to add
	*** \note MapMode should have already loaded a GlobalEnemy with this ID and retained it within the MapMode#_enemies member.
	*** If this is not the case, this function will print a warning message.
	**/
	void AddEnemy(uint32 enemy_id);

	//! \brief Returns a reference to a random battle party of enemies
	const std::vector<uint32>& RetrieveRandomParty();

	//! \brief Returns true if the sprite has at least one party
	bool HasEnemyParties() const
		{ return !_enemy_parties.empty(); }

	/** \brief Changes the current state of the sprite and updates other class members appropriately
	*** \param new_state The state to change to. If it is the same as the current state, no change will happen
	***
	**/
	void ChangeState(ENEMY_STATE new_state);

	//! \name Class Member Access Functions
	//@{
	ENEMY_STATE GetState() const
		{ return _state; }

	ENEMY_STATE GetSpawnedState() const
		{ return _spawned_state; }

	EnemyZone* GetZone() const
		{ return _zone; }

	float GetPursuitRange() const
		{ return _pursuit_range; }

	uint32 GetDirectionChangeTime() const
		{ return _directional_change_time; }

	uint32 GetFadeTime() const
		{ return _fade_time; }

	std::string GetBattleMusicFile() const
		{ return _battle_music_file; }

	std::string GetBattleBackgroundFile() const
		{ return _battle_background_file; }

	std::string GetBattleScriptFile() const
		{ return _battle_script_file; }

	void SetSpawnedState(ENEMY_STATE state)
		{ _spawned_state = state; }

	void SetZone(EnemyZone* zone)
		{ _zone = zone; }

	void SetPursuitRange(float range)
		{ _pursuit_range = range; }

	void SetDirectionChangeTime(uint32 time)
		{ _directional_change_time = time; }

	void SetFadeTime(uint32 time)
		{ _fade_time = time; }

	void SetBattleMusicFile(const std::string& file)
		{ _battle_music_file = file; }

	void SetBattleBackgroundFile(const std::string& file)
		{ _battle_background_file = file; }

	void SetBattleScriptFile(const std::string& file)
		{ _battle_script_file = file; }
	//@}

private:
	//! \brief The state that the enemy sprite is currently in
	ENEMY_STATE _state;

	//! \brief The state that the sprite will be changed to after spawning completes (default == HUNT)
	ENEMY_STATE _spawned_state;

	//! \brief The zone that the enemy sprite belongs to
	private_map::EnemyZone* _zone;

	//! \brief Used by states for various purposes, including fading of the enemy sprite or determing direction changes
	hoa_system::SystemTimer _state_timer;

	//! \brief Used to gradually fade the sprite by adjusting the alpha channel during the SPAWN and DISSIPATE states
	hoa_video::Color _fade_color;

	//! \brief Determines the maximum distance from the player's character before the enemy begins pursuit
	float _pursuit_range;

	//! \brief The amount of time to wait before an enemy sprite changes movement direction in the ACTIVE_ZONED state
	uint32 _directional_change_time;

	//! \brief The total time to take to fade an enemy sprite during the SPAWN or DISSIPATE states
	uint32 _fade_time;

	//! \brief Set to true when the enemy has gone outside of its zone
	bool _returning_to_zone;

	//! \brief The filename of the music to play for the battle
	std::string _battle_music_file;

	//! \brief The background image to use for the battle
	std::string _battle_background_file;

	//! \brief The filename of the script to pass to the battle
	std::string _battle_script_file;

	/** \brief Contains the possible groups of enemies that may appear in a battle should the player encounter this enemy sprite
	*** The numbers contained within this member are ID numbers for the enemy. If the
	**/
	std::vector<std::vector<uint32> > _enemy_parties;
}; // class EnemySprite : public MapSprite

} // namespace private_map

} // namespace hoa_map
