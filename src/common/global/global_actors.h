////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global_actors.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for global game actors
***
*** This file contains the implementation of "actors", which are living entities
*** in the game. Actors consist of playable characters and enemies in the game
*** that may participate in battles. Actors do not include NPCs found in towns or
*** other adversaries with which the player does not to battle with.
*** ***************************************************************************/

#pragma once

#include "defs.h"
#include "utils.h"

#include "video.h"

#include "global_utils.h"

namespace hoa_global {

/** ****************************************************************************
*** \brief Represents an actor that can participate in battles
***
*** This is an abstract parent class that both playable characters and enemies
*** inherit from in order to provide a consistent interface to the statistics
*** that characters and enemies share.
*** ***************************************************************************/
class GlobalActor {
public:
	GlobalActor();

	virtual ~GlobalActor();

	GlobalActor(const GlobalActor& copy);

	GlobalActor& operator=(const GlobalActor& copy);

	/** \brief Equips a new weapon on the actor
	*** \param weapon The new weapon to equip on the actor
	*** \return A pointer to the weapon that was previouslly equipped, or nullptr if no weapon was equipped.
	***
	*** This function will also automatically re-calculate all attack ratings, elemental, and status bonuses.
	**/
	GlobalWeapon* EquipWeapon(GlobalWeapon* weapon);

	/** \brief Equips a new armor on the actor
	*** \param armor The piece of armor to equip
	*** \param index The index into the _armor_equippd vector where to equip the armor
	*** \return A pointer to the armor that was previously equipped, or nullptr if no armor was equipped
	***
	*** This function will also automatically re-calculate all defense ratings, elemental, and status bonuses.
	*** If the index argument is invalid (out-of-bounds), the function will return the armor argument.
	**/
	GlobalArmor* EquipArmor(GlobalArmor* armor, uint32 index);

	/** \brief Adds a new skill to the actor's skill set
	*** \param skill_id The id number of the skill to add
	***
	*** No skill may be added more than once. If this case is detected or an error occurs when trying
	*** to load the skill data, it will not be added.
	**/
	virtual void AddSkill(uint32 skill_id) = 0;

	/** \brief Determines if the actor is "alive" and able to perform actions
	*** \return True if the character has a non-zero amount of hit points
	**/
	bool IsAlive() const
		{ return (_hit_points != 0); }

	//! \name Class member get functions
	//@{
	uint32 GetID() const
		{ return _id; }

	hoa_utils::ustring& GetName()
		{ return _name; }

	std::string& GetFilename()
		{ return _filename; }

	uint32 GetHitPoints() const
		{ return _hit_points; }

	uint32 GetMaxHitPoints() const
		{ return _max_hit_points; }

	uint32 GetActiveMaxHitPoints() const
		{ return _active_max_hit_points; }

	uint32 GetHitPointFatigue() const
		{ return _hit_point_fatigue; }

	uint32 GetSkillPoints() const
		{ return _skill_points; }

	uint32 GetMaxSkillPoints() const
		{ return _max_skill_points; }

	uint32 GetActiveMaxSkillPoints() const
		{ return _active_max_skill_points; }

	uint32 GetSkillPointFatigue() const
		{ return _skill_point_fatigue; }

	uint32 GetExperienceLevel() const
		{ return _experience_level; }

	uint32 GetExperiencePoints() const
		{ return _experience_points; }

	uint32 GetStrength() const
		{ return _strength; }

	uint32 GetVigor() const
		{ return _vigor; }

	uint32 GetFortitude() const
		{ return _fortitude; }

	uint32 GetProtection() const
		{ return _protection; }

	uint32 GetStamina() const
		{ return _stamina; }

	uint32 GetResilience() const
		{ return _resilience; }

	uint32 GetAgility() const
		{ return _agility; }

	float GetEvade() const
		{ return _evade; }

	uint32 GetTotalPhysicalAttack() const
		{ return _total_physical_attack; }

	uint32 GetTotalEtherealAttack() const
		{ return _total_ethereal_attack; }

	uint32 GetTotalPhysicalDefense() const
		{ return _total_physical_defense; }

	uint32 GetTotalEtherealDefense() const
		{ return _total_ethereal_defense; }

	GlobalWeapon* GetWeaponEquipped() const
		{ return _weapon_equipped; }

	const std::vector<GlobalArmor*>& GetArmorEquipped()
		{ return _armor_equipped; }

	GlobalArmor* GetArmorEquipped(uint32 index) const;

	std::vector<GlobalSkill*>* GetSkills()
		{ return &_skills; }

	/** \brief Retrieves a pointer to a skill in the _skills container
	*** \param skill_id The unique ID of the skill to find and return
	*** \return A pointer to the skill if it is found, or nullptr if the skill was not found
	**/
	GlobalSkill* GetSkill(uint32 skill_id) const;

	//! \brief An alternative GetSkill call that takes a skill pointer as an argument
	GlobalSkill* GetSkill(const GlobalSkill* skill) const;

	// TODO: elemental and status effects not yet available in game
// 	std::vector<GlobalElementalEffect*>& GetElementalAttackBonuses()
// 		{ return _elemental_attack_bonuses; }
//
// 	std::vector<std::pair<float, GlobalStatusEffect*> >& GetStatusAttackBonuses()
// 		{ return _status_attack_bonuses; }
//
// 	std::vector<GlobalElementalEffect*>& GetElementalDefenseBonuses()
// 		{ return _elemental_defense_bonuses; }
//
// 	std::vector<std::pair<float, GlobalStatusEffect*> >& GetStatusDefenseBonuses()
// 		{ return _status_defense_bonuses; }
	//@}

	/** \name Class member set functions
	*** These methods are primarily used when loading saved data for the character. Changes to these stats should
	*** normally use the add/subtract methods corresponding to the desired attribute. Active maximum HP/SP, attack,
	*** defense, and evade ratings are re-calculated when an appropriately related stat is changed.
	***
	*** \note Use caution when modifying HP/SP, maximums, and fatigues because setting these in certain orders during a
	*** character data load can cause problems with the values due to the requirements that HP/SP never exceed the
	*** active maximum and the active maximum should always equal to max HP/SP - HP/SP fatigue. You should load the
	*** maximum HP/SP first, then fatigue, and then finally the current HP/SP value.
	**/
	//@{
	void SetExperienceLevel(uint32 value)
		{ _experience_level = value; }

	void SetExperiencePoints(uint32 value)
		{ _experience_points = value; }

	void SetHitPoints(uint32 value)
		{ if (value > _active_max_hit_points) _hit_points = _active_max_hit_points; else _hit_points = value; }

	void SetMaxHitPoints(uint32 value)
		{ _max_hit_points = value; _active_max_hit_points = _max_hit_points - _hit_point_fatigue;
			if (_hit_points > _active_max_hit_points) _hit_points = _active_max_hit_points;
		}

	void SetHitPointFatigue(uint32 value)
		{ _hit_point_fatigue = value; _active_max_hit_points = _max_hit_points - _hit_point_fatigue;
			if (_hit_points > _active_max_hit_points) _hit_points = _active_max_hit_points;
		}

	void SetSkillPoints(uint32 value)
		{ if (value > _active_max_skill_points) _skill_points = _active_max_skill_points; else _skill_points = value; }

	void SetMaxSkillPoints(uint32 value)
		{ _max_skill_points = value; _active_max_skill_points = _max_skill_points - _skill_point_fatigue;
			if (_skill_points > _active_max_skill_points) _skill_points = _active_max_skill_points;
		}

	void SetSkillPointFatigue(uint32 value)
		{ _skill_point_fatigue = value; _active_max_skill_points = _max_skill_points - _skill_point_fatigue;
			if (_skill_points > _active_max_skill_points) _skill_points = _active_max_skill_points;
		}

	void SetStrength(uint32 value)
		{ _strength = value; _CalculateAttackRatings(); }

	void SetVigor(uint32 value)
		{ _vigor = value; _CalculateAttackRatings(); }

	void SetFortitude(uint32 value)
		{ _fortitude = value; _CalculateDefenseRatings(); }

	void SetProtection(uint32 value)
		{ _protection = value; _CalculateDefenseRatings(); }

	//! \note Stamina can not be set to 0 because it is used as a divisor in HP fatigue calculations
	void SetStamina(uint32 value)
		{ if (value == 0) _stamina = 1; else _stamina = value; }

	//! \note Resilience can not be set to 0 because it is used as a divisor in SP fatigue calculations
	void SetResilience(uint32 value)
		{ if (value == 0) _resilience = 1; else _resilience = value; }

	void SetAgility(uint32 ag)
		{ _agility = ag; }

	void SetEvade(float value)
		{ _evade = value; }
	//@}

	/** \name Class member add and subtract functions
	*** These methods provide a means to easily add or subtract amounts off of certain stats, such
	*** as hit points or stength. Total attack, defense, or evade ratings are re-calculated when
	*** an appropriately related stat is changed. Corner cases are checked to prevent overflow conditions
	*** and other invalid values, such as current hit points exceeding active maximum hit points.
	***
	*** \note When making changes to the maximum hit points or skill points, you should also consider
	*** making the same addition or subtraction to the current hit points / skill points. Modifying the
	*** maximum values will not modify the current values unless the change causes the new maximum to
	*** exceed the current values.
	***
	*** \note There are no methods for modifying the active maximum HP/SP, because these values should always be
	*** equal to the maximum HP/SP minus the HP/SP fatigue.
	**/
	//@{
	void AddHitPoints(uint32 amount);

	//! \note This will *not* increase hit point fatigue. Fatigue effects should be calculated separately
	void SubtractHitPoints(uint32 amount);

	//! \note This will also will increase the active max HP and the current HP
	void AddMaxHitPoints(uint32 amount);

	//! \note The number of hit points will be decreased if they are greater than the new maximum
	void SubtractMaxHitPoints(uint32 amount);

	//! \note This will also modify the active max HP
	void AddHitPointFatigue(uint32 amount);

	//! \note This will also modify the active max HP as well as add HP equal to the amount of fatigue removed
	void SubtractHitPointFatigue(uint32 amount);

	void AddSkillPoints(uint32 amount);

	//! \note This will *not* increase skill point fatigue. Fatigue effects should be calculated separately
	void SubtractSkillPoints(uint32 amount);

	//! \note This will also will increase the active max SP and the current SP
	void AddMaxSkillPoints(uint32 amount);

	//! \note The number of skill points will be decreased if they are greater than the new maximum
	void SubtractMaxSkillPoints(uint32 amount);

	//! \note This will also modify the active max SP
	void AddSkillPointFatigue(uint32 amount);

	//! \note This will also modify the active max SP as well as add SP equal to the amount of fatigue removed
	void SubtractSkillPointFatigue(uint32 amount);

	void AddStrength(uint32 amount);

	void SubtractStrength(uint32 amount);

	void AddVigor(uint32 amount);

	void SubtractVigor(uint32 amount);

	void AddFortitude(uint32 amount);

	void SubtractFortitude(uint32 amount);

	void AddProtection(uint32 amount);

	void SubtractProtection(uint32 amount);

	void AddStamina(uint32 amount);

	void SubtractStamina(uint32 amount);

	void AddResilience(uint32 amount);

	void SubtractResilience(uint32 amount);

	void AddAgility(uint32 amount);

	void SubtractAgility(uint32 amount);

	void AddEvade(float amount);

	void SubtractEvade(float amount);

	//! \note This does not remove hit point fatigue
	void RestoreAllHitPoints()
		{ _hit_points = _active_max_hit_points; }

	//! \note This does not remove skill point fatigue
	void RestoreAllSkillPoints()
		{ _skill_points = _active_max_skill_points; }

	/** \note Removing fatigue increases the actor's HP by the same amount. It does not necessarily
	*** restore the character to maximum HP.
	**/
	void RemoveAllHitPointFatigue()
		{ _active_max_hit_points = _max_hit_points; _hit_points += _hit_point_fatigue; _hit_point_fatigue = 0; }

	/** \note Removing fatigue increases the actor's SP by the same amount. It does not necessarily
	*** restore the character to maximum SP.
	**/
	void RemoveAllSkillPointFatigue()
		{ _active_max_skill_points = _max_skill_points; _skill_points += _skill_point_fatigue; _skill_point_fatigue = 0; }
	//@}

protected:
	//! \brief An identification number to represent the actor
	uint32 _id;

	//! \brief The name of the actor as it will be displayed on the screen
	hoa_utils::ustring _name;

	//! \brief The filename base used to look up an actors image files and other data
	std::string _filename;

	//! \name Base Actor Statistics
	//@{
	//! \brief The current experience level of the actor
	uint32 _experience_level;

	//! \brief The number of experience points the actor has earned
	uint32 _experience_points;

	//! \brief The current number of hit points that the actor has
	uint32 _hit_points;

	//! \brief The maximum number of hit points that the actor may have
	uint32 _max_hit_points;

	//! \brief The maximum hit points that the actor may currently have (equal to _max_hit_points - _hit_point_fatigue)
	uint32 _active_max_hit_points;

	//! \brief The amount of health fatigue the actor has accumulated, which reduces their active max HP
	uint32 _hit_point_fatigue;

	//! \brief The current number of skill points that the actor has
	uint32 _skill_points;

	//! \brief The maximum number of skill points that the actor may have
	uint32 _max_skill_points;

	//! \brief The maximum skill points that the actor may currently have (equal to _max_skill_points - _skill_point_fatigue)
	uint32 _active_max_skill_points;

	//! \brief The amount of skill fatigue the actor has accumulated, which reduces their active max SP
	uint32 _skill_point_fatigue;

	//! \brief Used to determine the actor's physical attack rating
	uint32 _strength;

	//! \brief Used to determine the actor's ethereal attack rating
	uint32 _vigor;

	//! \brief Used to determine the actor's physical defense rating
	uint32 _fortitude;

	//! \brief Used to determine the actor's ethereal defense rating
	uint32 _protection;

	//! \brief Used to translate HP damage into health fatigue (10 stamina means every 10 HP lost produces 1 fatigue)
	uint32 _stamina;

	//! \brief Used to translate SP consumption into skill fatigue (10 resilience means every 10 SP used produces 1 fatigue)
	uint32 _resilience;

	//! \brief Used to calculate the time the actor spends in the idle state in battles
	uint32 _agility;

	//! \brief The attack evade percentage of the actor, ranged from 0.0 to 1.0
	float _evade;
	//@}

	//! \brief The sum of the character's strength and their weapon's physical attack
	uint32 _total_physical_attack;

	//! \brief The sum of the character's vigor and their weapon's ethereal attack
	uint32 _total_ethereal_attack;

	//! \brief The sum of the character's fortitude and all of their armor's physical defense
	uint32 _total_physical_defense;

	//! \brief The sum of the characters protection and all of their armor's ethereal defense
	uint32 _total_ethereal_defense;

	/** \brief The weapon that the actor has equipped
	*** \note If no weapon is equipped, this member will be equal to nullptr.
	***
	*** Actors are not required to have weapons equipped, and indeed most enemies will probably not have any
	*** weapons explicitly equipped. The various bonuses to attack ratings, elemental attacks, and status
	*** attacks are automatically added to the appropriate members of this class when the weapon is equipped,
	*** and likewise those bonuses are removed when the weapon is unequipped.
	**/
	GlobalWeapon* _weapon_equipped;

	/** \brief The various armors that the actor has equipped
	***
	*** Actors are not required to have armor of any sort equipped. Equipped armor applies its defense, elemental,
	*** and status bonuses to the whole actor.
	**/
	std::vector<GlobalArmor*> _armor_equipped;

	/** \brief An ordered vector containing all skills that the actor can use
	*** For characters, a player may rearrange the skills in this container. An enemy must have <b>at least</b> one
	*** skill in order to do anything useful in battle.
	**/
	std::vector<GlobalSkill*> _skills;

	/** \brief The elemental effects added to the actor's attack
	*** Actors may carry various elemental attack bonuses, or they may carry none. These bonuses include
	*** those that are brought upon by the weapon that the character may have equipped.
	**/
// 	std::vector<GlobalElementalEffect*> _elemental_attack_bonuses;

	/** \brief The status effects added to the actor's attack
	*** Actors may carry various status attack bonuses, or they may carry none. These bonuses include
	*** those that are brought upon by the weapon that the character may have equipped. The first member
	*** in the pair is the likelihood (between 0.0 and 1.0) that the actor has of inflicting that status
	*** effect upon a targeted foe.
	**/
// 	std::vector<std::pair<float, GlobalStatusEffect*> > _status_attack_bonuses;

	/** \brief The elemental effects added to the actor's defense
	*** Actors may carry various elemental defense bonuses, or they may carry none. These bonuses include
	*** those that are brought upon by all of the armors that the character may have equipped.
	**/
// 	std::vector<GlobalElementalEffect*> _elemental_defense_bonuses;

	/** \brief The status effects added to the actor's defense
	*** Actors may carry various status defense bonuses, or they may carry none. These bonuses include
	*** those that are brought upon by the armors that the character may have equipped. The first member
	*** in the pair is the reduction in the likelihood (between 0.0 and 1.0) that the actor has of
	*** repelling an attack with a status effect.
	**/
// 	std::vector<std::pair<float, GlobalStatusEffect*> > _status_defense_bonuses;

	// ---------- Private methods

	/** \brief Calculates an actor's physical and ethereal attack ratings
	*** This function sums the actor's strength/vigor with their weapon's attack ratings
	*** and places the result in total physical/ethereal attack members
	**/
	void _CalculateAttackRatings();

	/** \brief Calculates an actor's physical and ethereal defense ratings
	*** This function sums the actor's fortitude/protection with all armor defense ratings
	*** and places the result in total physical/ethereal defense members
	**/
	void _CalculateDefenseRatings();
}; // class GlobalActor


/** ****************************************************************************
*** \brief Represents a playable game character
***
*** This class represents playable game characters that join the party and can
*** participate in battles. It does not cover NPCs or any other form of character.
*** This class retains references to loaded images of the character in various
*** formats such as sprites and portraits that are used across the different game modes.
***
*** Whenever a character gains additional experience points, there is a possibility that
*** growth may occur. Growth can occur even when the character has not reached a new experience
*** level, as the code allows for a gradual growth over time. A significant amount of growth should
*** always occur after achieving a new experience level.
***
*** The advised procedure for processing character growth is as follows.
*** -# Call AddExperiencePoints() to give the character additional XP.
*** -# If this method returns false, no further action is needed. Otherwise, growth has occurred and needs to be processed.
*** -# Call ReachedNewExperienceLevel() to determine whether the type growth is gradual or due to a
***    new experience level being reached.
*** -# If the growth type is gradual, call the various Get[STAT]Growth() methods and
***    report any non-zero values to the player. Then call AcknoledgeGrowth().
*** -# Otherwise if the growth type is a new level, report growth plus any skills
***    learned and call AcknoledgeGrowth() (*see note)
***
*** \note When an experience level is gained, after the call to AcknowledgeGrowth()
*** there may be new growth available (because the character gained multiple
*** experience levels or met the requirements for additional gradual growth for
*** the new experience level to gain). It is recommended practice to call AcknowledgeGrowth()
*** continuously until the fuction returns a false value, which indicates that no additional
*** growth is available.
***
*** \todo This class needs a better organized set of containers for its images.
*** The current containers and accessor methods are considered temporary.
*** ***************************************************************************/
class GlobalCharacter : public GlobalActor {
	/** \name Armor Equipment Types
	*** \brief Integers that represent the index location of the four armor types for characters
	**/
	//@{
	const uint32 ARMOR_TYPE_HEAD  = 0;
	const uint32 ARMOR_TYPE_TORSO = 1;
	const uint32 ARMOR_TYPE_ARMS  = 2;
	const uint32 ARMOR_TYPE_LEGS  = 3;
	//@}

	friend void hoa_defs::BindCommonCode();

	// TODO: investigate whether we can replace declaring the entire GameGlobal class as a friend with declaring
	// the GameGlobal::_SaveCharacter and GameGlobal::_LoadCharacter methods instead.
	friend class GameGlobal;
// 	friend void GameGlobal::_SaveCharacter(hoa_script::WriteScriptDescriptor &file, GlobalCharacter *character, bool last);
// 	friend void GameGlobal::_LoadCharacter(hoa_script::ReadScriptDescriptor &file, uint32 id);

public:
	/** \brief Constructs a new character from its definition in a script file
	*** \param id The integer ID of the character to create
	*** \param initial If true, the character's stats, equipment, and skills are set
	*** to the character's initial status
	*** \note If initial is set to false, the character's stats, equipment, and skills
	*** must be set by external code, otherwise they will remain 0/nullptr/empty.
	**/
	GlobalCharacter(uint32 id, bool initial = true);

	virtual ~GlobalCharacter()
		{}

	GlobalArmor* EquipHeadArmor(GlobalArmor* armor)
		{ return EquipArmor(armor, ARMOR_TYPE_HEAD); }

	GlobalArmor* EquipTorsoArmor(GlobalArmor* armor)
		{ return EquipArmor(armor, ARMOR_TYPE_TORSO); }

	GlobalArmor* EquipArmArmor(GlobalArmor* armor)
		{ return EquipArmor(armor, ARMOR_TYPE_ARMS); }

	GlobalArmor* EquipLegArmor(GlobalArmor* armor)
		{ return EquipArmor(armor, ARMOR_TYPE_LEGS); }

	/** \brief Adds experience points to the character
	*** \param xp The amount of experience points to add
	*** \return True if the new experience points triggered character growth
	**/
	bool AddExperiencePoints(uint32 xp);

	//! \brief Adds a new skill to the character, inherited from GlobalActor
	void AddSkill(uint32 skill_id);

	/** \brief Adds a new skill for the character to learn once the next experience level is gained
	*** \param skill_id The ID number of the skill to add
	*** \note This function is bound to Lua and used whenever a character gains a level.
	***
	*** The difference between this method and AddSkill() is that the skill added is also copied to the
	*** _new_skills_learned container. This allows external code to easily know what skill or skills have
	*** been added to the character.
	**/
	void AddNewSkillLearned(uint32 skill_id);

	//! \brief Returns true if the character has reached a new experience level
	bool ReachedNewExperienceLevel() const
		{ return _experience_for_next_level <= 0; }

	//! \brief Returns true if the character has outstanding growth that has not been acknowledged
	bool HasUnacknowledgedGrowth() const;

	/** \brief Adds any growth that has occured by modifying the character's stats
	*** \return True if additional growth is detected and requires another AcknowledgeGrowth() call.
	***
	*** If an experience level is gained, this function will open up the script file that contains
	*** the character's definition and get new growth stats for the next experience level. Often this
	*** requires another call to this function to process growth that has occurred after the level
	*** was gained. It is a good idea to put calls to this function in a loop to process all growth
	*** (ie, while (AcknowledgeGrowth() != false)).
	***
	*** \note If multiple experience levels were gained as a result of adding a large amount of XP, this
	*** function will only increment the experience level by one. In the case where multiple levels are
	*** gained, this function will need to be called once for each level up.
	**/
	bool AcknowledgeGrowth();

	//! \name Public Member Access Functions
	//@{
	/** \note When a character gains a level and needs to have this member updated, you should use this
	*** "Add" method instead of the "Set" method. The reason why is so that any additional experience that
	*** was earned above the amount that was needed to achieve the new level will  be factored in to reducing
	*** the amount of experience required for the next level. This is possible because the _experience_for_next_level
	*** member is allowed to become a negative value.
	**/
	void AddExperienceForNextLevel(uint32 xp) {
		_experience_for_next_level += xp;
	}

	int32 GetExperienceForNextLevel() const
		{ return _experience_for_next_level; }

	void SetExperienceForNextLevel(int32 xp)
		{ _experience_for_next_level = xp; }

	GlobalArmor* GetHeadArmorEquipped()
		{ return _armor_equipped[ARMOR_TYPE_HEAD]; }

	GlobalArmor* GetTorsoArmorEquipped()
		{ return _armor_equipped[ARMOR_TYPE_TORSO]; }

	GlobalArmor* GetArmArmorEquipped()
		{ return _armor_equipped[ARMOR_TYPE_ARMS]; }

	GlobalArmor* GetLegArmorEquipped()
		{ return _armor_equipped[ARMOR_TYPE_LEGS]; }

	std::vector<GlobalSkill*>* GetAttackSkills()
		{ return &_attack_skills; }

	std::vector<GlobalSkill*>* GetDefenseSkills()
		{ return &_defense_skills; }

	std::vector<GlobalSkill*>* GetSupportSkills()
		{ return &_support_skills; }

	uint32 GetHitPointsGrowth() const {
		return _hit_points_growth;
	}

	uint32 GetSkillPointsGrowth() const {
		return _skill_points_growth;
	}

	uint32 GetStrengthGrowth() const {
		return _strength_growth;
	}

	uint32 GetVigorGrowth() const {
		return _vigor_growth;
	}

	uint32 GetFortitudeGrowth() const {
		return _fortitude_growth;
	}

	uint32 GetProtectionGrowth() const {
		return _protection_growth;
	}

	uint32 GetStaminaGrowth() const {
		return _stamina_growth;
	}

	uint32 GetResilienceGrowth() const {
		return _resilience_growth;
	}

	uint32 GetAgilityGrowth() const {
		return _agility_growth;
	}

	float GetEvadeGrowth() const {
		return _evade_growth;
	}

	std::vector<GlobalSkill*>* GetNewSkillsLearned() {
		return &_new_skills_learned;
	}
	//@}

	// TEMP: image accessor functions
	//@{
	std::vector<hoa_video::StillImage>* GetStandardSpriteFrames()
		{ return &_map_frames_standard; }

	void AddBattleAnimation(const std::string & name, hoa_video::AnimatedImage anim)
		{ _battle_animation[name] = anim; }

	hoa_video::AnimatedImage* RetrieveBattleAnimation(const std::string & name)
		{ return &_battle_animation[name]; }

	std::vector<hoa_video::StillImage>* GetBattlePortraits()
		{ return &_battle_portraits; }
	//@}

protected:
	/** \brief Sortable skill containers
	*** Skills are divided into three types: attack, defense, and support. There is really no functional
	*** distinguishment between the various skill types, they just serve an organizational means and are
	*** used to identify a skill's general purpose/use. Characters keep their skills in these seperate
	*** containers because they are presented in this way to the player.
	**/
	//@{
	std::vector<GlobalSkill*> _attack_skills;
	std::vector<GlobalSkill*> _defense_skills;
	std::vector<GlobalSkill*> _support_skills;
	//@}

	/** \name Character Images
	*** \note Although many of the names of these members would imply that they are only used in one particular
	*** mode of operation (map, battle, etc.), these members may be freely used by different game modes for
	*** which they were not specifically designed for. The names are simply meant to indicate the primary game
	*** mode where the images are intended to be used.
	**/
	//@{
	/** \brief The standard frame images for the character's map sprite.
	*** This container holds the standard frames for the character's map sprite, which include standing and
	*** walking frames. This set includes 24 frames in total, 6 each for the down, up, left, and right
	*** orientations.
	**/
	std::vector<hoa_video::StillImage> _map_frames_standard;

	/** \brief The character's standard map portrait image
	*** The standard map portrait is ususally used in dialogues, but may also be used in other modes where
	*** appropriate. The size of the map portrait is 200x200 pixels.
	**/
	hoa_video::StillImage _map_portrait_standard;

	/** \brief The frame images for the character's battle sprite.
	*** This map container contains various animated images for the character's battle sprites. The key to the
	*** map is a simple string which describes the animation, such as "idle".
	**/
	std::map<std::string, hoa_video::AnimatedImage> _battle_animation;

	/** \brief The frame images for the character's battle portrait
	*** Each character has 5 battle portraits which represent the character's health with damage levels of 0%,
	*** 25%, 50%, 75%, and 100% (this is also the order in which the frames are stored, starting with the 0%
	*** frame at index 0). Thus, the size of this vector is always five elements. Each portait is 100x100
	*** pixels in size.
	**/
	std::vector<hoa_video::StillImage> _battle_portraits;

	/** \brief The character's full-body portrait image for use in menu mode
	*** This image is a detailed, full-scale portait of the character and is intended for use in menu mode.
	*** The size of the image is 150x350 pixels.
	**/
	hoa_video::StillImage _menu_portrait;
	//@}

private:
	/** \brief The remaining experience points required to reach the next experience level
	***
	*** As a character earns experience points, the value held in this member decreases by the amount of XP learned. Whenever the value stored member
	*** zero or a negative number, the character has gained a new level. When a new level is achieved, the amount of experience required to then
	*** proceed another level is added to this member.
	***
	*** \note The reason this member is a signed integer and not unsigned is so that it may be allowed to go negative. This makes the logic for processing
	*** experience level growth easier.
	***
	**/
	int32 _experience_for_next_level;

	/** \brief The amount of growth that should be added to each of the character's stats
	*** These members are incremented by the _ProcessPeriodicGrowth() function, which detects when a character
	*** has enough experience points to meet a growth requirement. They are all cleared to zero after
	*** a call to AcknowledgeGrowth().
	***
	*** \note These members are given read/write access in Lua so that Lua may use them to hold new
	*** growth amounts when a character reaches a new level. Refer to the function DetermineNextLevelGrowth(character)
	*** defined in lua/data/actors/characters.lua
	**/
	//@{
	uint32 _hit_points_growth;
	uint32 _skill_points_growth;
	uint32 _strength_growth;
	uint32 _vigor_growth;
	uint32 _fortitude_growth;
	uint32 _protection_growth;
	uint32 _stamina_growth;
	uint32 _resilience_growth;
	uint32 _agility_growth;
	float _evade_growth;
	//@}

	/** \brief The periodic growth of the stats as a function of experience points
	*** The purpose of these containers is to support the gradual growth of characters.
	*** The first member in each pair is the experience points required for that growth
	*** to occur, while the second member is the value of the growth. Each entry in the
	*** deques are ordered from lowest (front) to highest (back) XP requirements. The
	*** final entry in each deque should be the growth for when the next experience
	*** level is reached. Note that these structures do not need to contain any entries
	*** (ie, a stat does not need to grow on every level).
	***
	*** These containers are emptied when a new experience level occurs, and are also
	*** re-constructed after the experience level gain has been acknowledged.
	**/
	//@{
	std::deque<std::pair<uint32, uint32> > _hit_points_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _skill_points_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _strength_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _vigor_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _fortitude_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _protection_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _stamina_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _resilience_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _agility_periodic_growth;
	std::deque<std::pair<uint32, float> > _evade_periodic_growth;
	//@}

	/** \brief Contains pointers to all skills that were learned by achieving the current experience level
	***
	*** This container will not contain skills learned if the character was constructed using their initial stats.
	*** The skills listed within this container have already been added to the character's active usable skill set.
	*** This container is cleared and reset after every level up. The most common use for this container is for
	*** external code to be able to show the player what skills have been learned upon their character reaching a
	*** new experience level.
	***
	*** \note The pointers in this container are copies of the pointers contained within the _skills container. No
	*** memory management needs to be performed by this vector.
	***
	*** \note An issue that needs to be considered is that if the character has an existing skill removed
	*** and that skill is also referenced by this container, the container will then point to an invalid memory
	*** location (assuming the GlobalSkill object that was removed was also deleted). Therefore, any skills that
	*** are removed from a character should also be removed from this container if they exist.
	**/
	std::vector<GlobalSkill*> _new_skills_learned;

	/** \brief Examines if any growth has occured as a result of the character's experience points
	*** \return True if any amount of growth has occurred, false if no growth has occurred.
	***
	*** This is called by GlobalCharacter whenever the character's experience points change. If any growth is
	*** detected, _ProcessPeriodicGrowth() is called and the various growth members of the class are incremented
	*** by the growth amount.
	**/
	bool _CheckForGrowth();

	/** \brief Removes acquired growth from the periodic growth containers and accumulating it in the growth members
	***
	*** This method should be called whenever any amount of growth in any stat has been detected. There is no harm in
	*** invoking this method when no growth has occurred, however it will do nothing but waste time in this case.
	**/
	void _ProcessPeriodicGrowth();

	/** \brief Constructs the numerous periodic growth deques when growth stats for a new level are loaded in
	*** After new growth stats have been loaded in for a level, this function takes those values, breaks them
	*** apart, and spreads out their growth periodically. 50% of the growth is saved for when the character
	*** reaches a new level, while the other 50% are rewarded as the character's experience grows to values
	*** in between the previous experience level marker and the next.
	***
	*** \note The growth members should contain the total growth stats when this function is called. These
	*** members will be set back to zero before the function returns as their values will be split up
	*** and placed across numerous entries in the periodic_growth containers. All periodic_growth deques should be
	*** empty when this function is called.
	**/
	void _ConstructPeriodicGrowth();
}; // class GlobalCharacter : public GlobalActor


/** ****************************************************************************
*** \brief Representation of enemies that fight in battles
***
*** Allacrost handles enemies a little different than most RPGs. Instead of an
*** enemy always having the same statistics for health, strength, etc., enemy
*** stats are randomized so that the same type of enemy does not always have
*** the exact same stats. Guassian random values are applied to each enemy's
*** "base" stats before the player begins battle with that enemy, making
*** the enemy tougher or weaker than the base level depending on the outcome. Some
*** enemies (notably bosses) do not have this randomization applied to their stats
*** in order to make sure that bosses are challenging, but not overwhemlingly strong
*** or surprisingly weak.
***
*** Enemies have one to several different skills that they may use in battle. An enemy
*** has to have at least one skill defined for it, otherwise they would not be able to
*** perform any action in battle. Enemy's may also carry a small chance of dropping an
*** item or other object after they are defeated.
*** ***************************************************************************/
class GlobalEnemy : public GlobalActor {
public:
	GlobalEnemy(uint32 id);

	virtual ~GlobalEnemy()
		{}

	/** \brief Initializes the enemy and prepares it for battle
	***
	*** This function sets the enemy's experience level, modifies its stats using Guassian
	*** random values, and constructs the skills that the enemy is capable of using. Call this
	*** function once only, because after the enemy has skills enabled it will not be able to
	*** re-initialize. If you need to initialize the enemy once more, you'll have to create a
	*** brand new GlobalEnemy object and initialize that instead.
	***
	*** \note Certain enemies will skip the stat modification step.
	**/
	void Initialize();

	/** \brief Enables the enemy to be able to use a specific skill
	*** \param skill_id The integer ID of the skill to add to the enemy
	***
	*** This method should be called only <b>after</b> the Initialize() method has been invoked. The
	*** purpose of this method is to allow non-standard skills to be used by enemies under certain
	*** circumstances. For example, in scripted battle sequences where an enemy may become stronger
	*** and gain access to new skills after certain criteria are met. Normally you would want to define
	*** any skills that you wish an enemy to be able to use within their Lua definition file.
	**/
	void AddSkill(uint32 skill_id);

	/** \brief Uses random variables to calculate which objects, if any, the enemy dropped
	*** \param objects A reference to a vector to hold the GlobalObject pointers
	***
	*** The objects vector is cleared immediately once this function is called so make sure
	*** that it does not hold anything meaningful. Any objects which are added to this
	*** vector are created with new GlobalObject() and it becomes the callee's repsonsibility
	*** to manage this memory and delete those objects when they are no longer needed.
	**/
	void DetermineDroppedObjects(std::vector<GlobalObject*>& objects);

	//! \name Class member access functions
	//@{
	uint32 GetDrunesDropped() const
		{ return _drunes_dropped; }

	uint32 GetSpriteWidth() const
		{ return _sprite_width; }

	uint32 GetSpriteHeight() const
		{ return _sprite_height; }

	std::vector<hoa_video::StillImage>* GetBattleSpriteFrames()
		{ return &_battle_sprite_frames; }
	//@}

protected:
	//! \brief If set to true, when initialized the enemy will not randomize its statistic values
	bool _no_stat_randomization;

	//! \brief The dimensions of the enemy's battle sprite in pixels
	uint32 _sprite_width, _sprite_height;

	//! \brief The amount of drunes that the enemy will drop
	uint32 _drunes_dropped;

	/** \brief Dropped object containers
	*** These two vectors are of the same size. _dropped_objects contains the IDs of the objects that the enemy
	*** may drop. _dropped_chance contains a value from 0.0f to 1.0f that determines the probability of the
	*** enemy dropping that object.
	**/
	//@{
	std::vector<uint32> _dropped_objects;
	std::vector<float> _dropped_chance;
	//@}

	/** \brief Contains all of the possible skills that the enemy may possess
	*** This container holds the IDs of all skills that the enemy may execute in battle.
	*** The Initialize() function uses this data to populates the GlobalActor _skills container.
	**/
	std::vector<uint32> _skill_set;

	/** \brief The battle sprite frame images for the enemy
	*** Each enemy has four frames representing damage levels of 0%, 33%, 66%, and 100%. This vector thus
	*** always has a size of four holding each of these image frames. The first element contains the 0%
	*** damage frame, the second element contains the 33% damage frame, and so on.
	**/
	std::vector<hoa_video::StillImage> _battle_sprite_frames;
}; // class GlobalEnemy : public GlobalActor


/** ****************************************************************************
*** \brief Represents a party of actors
***
*** This class is a container for a group or "party" of actors. A party is a type
*** of target for items and skills. The GameGlobal class also organizes characters
*** into parties for convienence. Note that an actor may be either an enemy or
*** a character, but you should avoid creating parties that contain both characters
*** and enemies, as it can lead to conflicts. For example, a character and enemy which
*** enemy which have the same ID value).
***
*** Parties may or may not allow duplicate actors (a duplicate actor is defined
*** as an actor that has the same _id member as another actor in the party).
*** This property is determined in the GlobalParty constructor
***
*** \note When this class is destroyed, the actors contained within the class are
*** <i>not</i> destroyed. Only the references to those actors through this class object
*** are lost.
***
*** \note All methods which perform an operation by using an actor ID are
*** <b>only</b> valid to use if the party does not allow duplicates.
*** ***************************************************************************/
class GlobalParty {
public:
	//! \param allow_duplicates Determines whether or not the party allows duplicate actors to be added (default value == false)
	GlobalParty(bool allow_duplicates = false) :
		_allow_duplicates(allow_duplicates) {}

	~GlobalParty()
		{}

	// ---------- Actor addition, removal, and retrieval methods

	/** \brief Adds an actor to the party
	*** \param actor A pointer to the actor to add to the party
	*** \param index The index where the actor should be inserted. If negative, actor is added to the end
	*** \note The actor will not be added if it is already in the party and duplicates are not allowed
	**/
	void AddActor(GlobalActor* actor, int32 index = -1);

	/** \brief Removes an actor from the party
	*** \param index The index of the actor in the party to remove
	*** \return A pointer to the actor that was removed, or nullptr if the index provided was invalid
	**/
	GlobalActor* RemoveActorAtIndex(uint32 index);

	/** \brief Removes an actor from the party
	*** \param id The id value of the actor to remove
	*** \return A pointer to the actor that was removed, or nullptr if the actor was not found in the party
	**/
	GlobalActor* RemoveActorByID(uint32 id);

	/** \brief Clears the party of all actors
	*** \note This function does not return the actor pointers, so if you wish to get the
	*** GlobalActors make sure you do so prior to invoking this call.
	**/
	void RemoveAllActors()
		{ _actors.clear(); }

	/** \brief Retrieves a poitner to the actor in the party at a specified index
	*** \param index The index where the actor may be found in the party
	*** \return A pointer to the actor at the specified index, or nullptr if the index argument was invalid
	**/
	GlobalActor* GetActorAtIndex(uint32 index) const;

	/** \brief Retrieves a poitner to the actor in the party with the spefified id
	*** \param id The id of the actor to return
	*** \return A pointer to the actor with the requested ID, or nullptr if the actor was not found
	**/
	GlobalActor* GetActorByID(uint32 id) const;

	// ---------- Actor swap and replacement methods

	/** \brief Swaps the location of two actors in the party by their indeces
	*** \param first_index The index of the first actor to swap
	*** \param second_index The index of the second actor to swap
	**/
	void SwapActorsByIndex(uint32 first_index, uint32 second_index);

	/** \brief Swaps the location of two actors in the party by looking up their IDs
	*** \param first_id The id of the first actor to swap
	*** \param second_id The id of the second actor to swap
	**/
	void SwapActorsByID(uint32 first_id, uint32 second_id);

	/** \brief Replaces an actor in the party at a specified index with a new actor
	*** \param index The index of the actor to be replaced
	*** \param new_actor A pointer to the actor that will replace the existing actor
	*** \return A pointer to the replaced actor, or nullptr if the operation did not take place
	**/
	GlobalActor* ReplaceActorByIndex(uint32 index, GlobalActor* new_actor);

	/** \brief Replaces an actor in the party with the specified id with a new actor
	*** \param id The id of the actor to be replaced
	*** \param new_actor A pointer to the actor that will replace the existing actor
	*** \return A pointer to the replaced actor, or nullptr if the operation did not take place
	**/
	GlobalActor* ReplaceActorByID(uint32 id, GlobalActor* new_actor);

	// ---------- Other methods

	/** \brief Computes the average experience level of all actors in the party
	*** \return A float representing the average experience level (0.0f if party is empty)
	**/
	float AverageExperienceLevel() const;

	/** \brief Adds a certain number of hit points to all actors in the party
	*** \param hp The number of hit points to add
	**/
	void AddHitPoints(uint32 hp);

	/** \brief Adds a certain number of skill points to all actors in the party
	*** \param sp The number of skill points to add
	**/
	void AddSkillPoints(uint32 sp);

	//! \name Class member accessor methods
	//@{
	bool IsAllowDuplicates() const
		{ return _allow_duplicates; }

	bool IsPartyEmpty() const
		{ return (_actors.size() == 0); }

	uint32 GetPartySize() const
		{ return _actors.size(); }

	const std::vector<GlobalActor*>& GetAllActors() const
		{ return _actors; }
	//@}

private:
	/** \brief Actors are allowed to be inserted into the party multiple times when this member is true
	*** \note The value of this member is set in the class constructor and can not be changed at a later time
	**/
	bool _allow_duplicates;

	/** \brief A container of actors that are in this party
	*** The GlobalActor objects pointed to by the elements in this vector are not managed by this class. Therefore
	*** one needs to be careful that if any of the GlobalActor objects are destroyed outside the context of this
	*** class, the actor should be removed from this container immediately to avoid a possible segmentation fault.
	**/
	std::vector<GlobalActor*> _actors;
}; // class GlobalParty

} // namespace hoa_global
