////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global_actors.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for global game actors
*** ***************************************************************************/

#include "video.h"
#include "global_actors.h"
#include "global_objects.h"
#include "global_effects.h"
#include "global_skills.h"
#include "global_utils.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_script;
using namespace hoa_global::private_global;

namespace hoa_global {

////////////////////////////////////////////////////////////////////////////////
// GlobalActor class
////////////////////////////////////////////////////////////////////////////////

GlobalActor::GlobalActor() :
	_id(0),
	_experience_level(0),
	_experience_points(0),
	_hit_points(0),
	_max_hit_points(0),
	_active_max_hit_points(0),
	_hit_point_fatigue(0),
	_skill_points(0),
	_max_skill_points(0),
	_active_max_skill_points(0),
	_skill_point_fatigue(0),
	_strength(0),
	_vigor(0),
	_fortitude(0),
	_protection(0),
	_stamina(0),
	_resilience(0),
	_agility(0),
	_evade(0.0f),
	_total_physical_attack(0),
	_total_ethereal_attack(0),
	_total_physical_defense(0),
	_total_ethereal_defense(0),
	_weapon_equipped(nullptr)
{}



GlobalActor::~GlobalActor() {
	// Delete all equipment
	if (_weapon_equipped != nullptr)
		delete _weapon_equipped;
	for (uint32 i = 0; i < _armor_equipped.size(); i++) {
		if (_armor_equipped[i] != nullptr)
			delete _armor_equipped[i];
	}
	_armor_equipped.clear();

	// Delete all skills
	for (uint32 i = 0; i < _skills.size(); ++i) {
		delete _skills[i];
	}
	_skills.clear();
}



GlobalActor::GlobalActor(const GlobalActor& copy) {
	_id = copy._id;
	_name = copy._name;
	_filename = copy._filename;
	_experience_level = copy._experience_level;
	_experience_points = copy._experience_points;
	_hit_points = copy._hit_points;
	_max_hit_points = copy._max_hit_points;
	_active_max_hit_points = copy._active_max_hit_points;
	_hit_point_fatigue = copy._hit_point_fatigue;
	_skill_points = copy._skill_points;
	_max_skill_points = copy._max_skill_points;
	_active_max_skill_points = copy._active_max_skill_points;
	_skill_point_fatigue = copy._skill_point_fatigue;
	_strength = copy._strength;
	_vigor = copy._vigor;
	_fortitude = copy._fortitude;
	_protection = copy._protection;
	_stamina = copy._stamina;
	_resilience = copy._resilience;
	_agility = copy._agility;
	_evade = copy._evade;
	_total_physical_attack = copy._total_physical_attack;
	_total_ethereal_attack = copy._total_ethereal_attack;
	_total_physical_defense = copy._total_physical_defense;
	_total_ethereal_defense = copy._total_ethereal_defense;

	// Copy all equipment
	if (copy._weapon_equipped == nullptr)
		_weapon_equipped = nullptr;
	else
		_weapon_equipped = new GlobalWeapon(*copy._weapon_equipped);

	for (uint32 i = 0; i < _armor_equipped.size(); i++) {
		if (_armor_equipped[i] == nullptr)
			_armor_equipped.push_back(nullptr);
		else
			_armor_equipped.push_back(new GlobalArmor(*copy._armor_equipped[i]));
	}

	// Copy all skills
	for (uint32 i = 0; i < copy._skills.size(); ++i) {
		_skills.push_back(new GlobalSkill(copy._skills[i]->GetID()));
	}
}



GlobalActor& GlobalActor::operator=(const GlobalActor& copy) {
	if (this == &copy) // Handle self-assignment case
		return *this;

	_id = copy._id;
	_name = copy._name;
	_filename = copy._filename;
	_experience_level = copy._experience_level;
	_experience_points = copy._experience_points;
	_hit_points = copy._hit_points;
	_max_hit_points = copy._max_hit_points;
	_active_max_hit_points = copy._active_max_hit_points;
	_hit_point_fatigue = copy._hit_point_fatigue;
	_skill_points = copy._skill_points;
	_max_skill_points = copy._max_skill_points;
	_active_max_skill_points = copy._active_max_skill_points;
	_skill_point_fatigue = copy._skill_point_fatigue;
	_strength = copy._strength;
	_vigor = copy._vigor;
	_fortitude = copy._fortitude;
	_protection = copy._protection;
	_stamina = copy._stamina;
	_resilience = copy._resilience;
	_agility = copy._agility;
	_evade = copy._evade;
	_total_physical_attack = copy._total_physical_attack;
	_total_ethereal_attack = copy._total_ethereal_attack;

	// Copy all equipment
	if (copy._weapon_equipped == nullptr)
		_weapon_equipped = nullptr;
	else
		_weapon_equipped = new GlobalWeapon(*copy._weapon_equipped);

	for (uint32 i = 0; i < _armor_equipped.size(); i++) {
		if (_armor_equipped[i] == nullptr)
			_armor_equipped.push_back(nullptr);
		else
			_armor_equipped.push_back(new GlobalArmor(*copy._armor_equipped[i]));
	}

	// Copy all skills
	for (uint32 i = 0; i < copy._skills.size(); ++i) {
		_skills.push_back(new GlobalSkill(copy._skills[i]->GetID()));
	}
	return *this;
}



GlobalWeapon* GlobalActor::EquipWeapon(GlobalWeapon* weapon) {
	GlobalWeapon* old_weapon = _weapon_equipped;
	_weapon_equipped = weapon;
	_CalculateAttackRatings();
	return old_weapon;
}



GlobalArmor* GlobalActor::EquipArmor(GlobalArmor* armor, uint32 index) {
	if (index >= _armor_equipped.size()) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "index argument exceeded number of pieces of armor equipped: " << index << endl;
		return armor;
	}

	GlobalArmor* old_armor = _armor_equipped[index];
	_armor_equipped[index] = armor;

	if (old_armor != nullptr && armor != nullptr) {
		if (old_armor->GetObjectType() != armor->GetObjectType()) {
			IF_PRINT_WARNING(GLOBAL_DEBUG) << "old armor was replaced with a different type of armor" << endl;
		}
	}

	_CalculateDefenseRatings();
	return old_armor;
}



GlobalArmor* GlobalActor::GetArmorEquipped(uint32 index) const {
	if (index >= _armor_equipped.size()) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "index argument exceeded number of pieces of armor equipped: " << index << endl;
		return nullptr;
	}

	return _armor_equipped[index];
}



GlobalSkill* GlobalActor::GetSkill(uint32 skill_id) const {
	for (uint32 i = 0; i < _skills.size(); ++i) {
		if (skill_id == _skills[i]->GetID()) {
			return _skills[i];
		}
	}

	IF_PRINT_WARNING(GLOBAL_DEBUG) << "actor did not have a skill with the requested skill_id: " << skill_id << endl;
	return nullptr;
}



GlobalSkill* GlobalActor::GetSkill(const GlobalSkill* skill) const {
	if (skill == nullptr) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "function received a nullptr pointer argument" << endl;
		return nullptr;
	}

	return GetSkill(skill->GetID());
}



void GlobalActor::AddHitPoints(uint32 amount) {
	if ((0xFFFFFFFF - amount) < _hit_points) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "integer overflow condition detected: " << amount << endl;
		_hit_points = 0xFFFFFFFF;
	}
	else {
		_hit_points += amount;
	}

	if (_hit_points > _active_max_hit_points)
		_hit_points = _active_max_hit_points;
}



void GlobalActor::SubtractHitPoints(uint32 amount) {
	if (amount >= _hit_points)
		_hit_points = 0;
	else
		_hit_points -= amount;
}



void GlobalActor::AddMaxHitPoints(uint32 amount) {
	if ((0xFFFFFFFF - amount) < _max_hit_points) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "integer overflow condition detected: " << amount << endl;
		_max_hit_points = 0xFFFFFFFF;
		_active_max_hit_points = 0xFFFFFFFF;
		_hit_points = 0xFFFFFFFF;
	}
	else {
		_max_hit_points += amount;
		_active_max_hit_points += amount;
		_hit_points += amount;
	}
}



void GlobalActor::SubtractMaxHitPoints(uint32 amount) {
	if (amount > _max_hit_points) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "argument value will cause max hit points to decrease to zero: " << amount << endl;
		_max_hit_points = 0;
		_hit_point_fatigue = 0;
		_active_max_hit_points = 0;
		_hit_points = 0;
	}
	else {
		_max_hit_points -= amount;

		if (_active_max_hit_points > _max_hit_points) {
			_active_max_hit_points = _max_hit_points;
			_hit_point_fatigue = 0;
		}
		if (_hit_points > _active_max_hit_points) {
			_hit_points = _active_max_hit_points;
		}
	}
}



void GlobalActor::AddHitPointFatigue(uint32 amount) {
	// Stop accumulating fatigue once the _active_max_hit_points reaches MINIMUM_FATIGUE_HIT_POINTS
	if (amount > (_active_max_hit_points - MINIMUM_FATIGUE_HIT_POINTS)) {
		amount = _active_max_hit_points - MINIMUM_FATIGUE_HIT_POINTS;
	}

	_hit_point_fatigue += amount;
	_active_max_hit_points -= amount;
	if (_hit_points > _active_max_hit_points)
		_hit_points = _active_max_hit_points;
}



void GlobalActor::SubtractHitPointFatigue(uint32 amount) {
	if (amount > _hit_point_fatigue) {
		amount = _hit_point_fatigue;
	}

	_hit_point_fatigue -= amount;
	_active_max_hit_points += amount;
	_hit_points += amount;

	if (_active_max_hit_points > _max_hit_points) {
		// This condition should never happen. If it does, there's likely a bug because
		// active_max_hit_points + _hit_point_fatigue should always be equal to _max_hit_points
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "restoring _hit_point_fatigue caused _active_max_hit_points to exceed _max_hit_points" << endl;
		_active_max_hit_points = _max_hit_points;
		_hit_point_fatigue = 0;
		if (_hit_points > _active_max_hit_points)
			_hit_points = _active_max_hit_points;
	}
}



void GlobalActor::AddSkillPoints(uint32 amount) {
	if ((0xFFFFFFFF - amount) < _skill_points) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "integer overflow condition detected: " << amount << endl;
		_skill_points = 0xFFFFFFFF;
	}
	else {
		_skill_points += amount;
	}

	if (_skill_points > _max_skill_points)
		_skill_points = _max_skill_points;
}



void GlobalActor::SubtractSkillPoints(uint32 amount) {
	if (amount >= _skill_points)
		_skill_points = 0;
	else
		_skill_points -= amount;
}



void GlobalActor::AddMaxSkillPoints(uint32 amount) {
	if ((0xFFFFFFFF - amount) < _max_skill_points) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "integer overflow condition detected: " << amount << endl;
		_max_skill_points = 0xFFFFFFFF;
		_skill_points = 0xFFFFFFFF;
	}
	else {
		_max_skill_points += amount;
		_skill_points += amount;
	}
}



void GlobalActor::SubtractMaxSkillPoints(uint32 amount) {
	if (amount > _max_skill_points) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "argument value will cause max skill points to decrease to zero: " << amount << endl;
		_max_skill_points = 0;
		_skill_point_fatigue = 0;
		_active_max_skill_points = 0;
		_skill_points = 0;
	}
	else {
		_max_skill_points -= amount;

		if (_active_max_skill_points > _max_skill_points) {
			_active_max_skill_points = _max_skill_points;
			_skill_point_fatigue = 0;
		}
		if (_skill_points > _active_max_skill_points) {
			_skill_points = _active_max_skill_points;
		}
	}
}



void GlobalActor::AddSkillPointFatigue(uint32 amount) {
	// Stop accumulating fatigue once the _active_max_skill_points reaches MINIMUM_FATIGUE_SKILL_POINTS
	if (amount > (_active_max_skill_points - MINIMUM_FATIGUE_SKILL_POINTS)) {
		amount = _active_max_skill_points - MINIMUM_FATIGUE_SKILL_POINTS;
	}

	_skill_point_fatigue += amount;
	_active_max_skill_points -= amount;
	if (_skill_points > _active_max_skill_points)
		_skill_points = _active_max_skill_points;
}



void GlobalActor::SubtractSkillPointFatigue(uint32 amount) {
	if (amount > _skill_point_fatigue) {
		amount = _skill_point_fatigue;
	}

	_skill_point_fatigue -= amount;
	_active_max_skill_points += amount;
	_skill_points += amount;

	if (_active_max_skill_points > _max_skill_points) {
		// This condition should never happen. If it does, there's likely a bug because
		// active_max_skill_points + _skill_point_fatigue should always be equal to _max_skill_points
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "restoring _skill_point_fatigue caused _active_max_skill_points to exceed _max_skill_points" << endl;
		_active_max_skill_points = _max_skill_points;
		_skill_point_fatigue = 0;
		if (_skill_points > _active_max_skill_points)
			_skill_points = _active_max_skill_points;
	}
}



void GlobalActor::AddStrength(uint32 amount) {
	if ((0xFFFFFFFF - amount) < _strength) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "integer overflow condition detected: " << amount << endl;
		_strength = 0xFFFFFFFF;
	}
	else {
		_strength += amount;
	}

	_CalculateAttackRatings();
}



void GlobalActor::SubtractStrength(uint32 amount) {
	if (amount >= _strength)
		_strength = 0;
	else
		_strength -= amount;

	_CalculateAttackRatings();
}



void GlobalActor::AddVigor(uint32 amount) {
	if ((0xFFFFFFFF - amount) < _vigor) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "integer overflow condition detected: " << amount << endl;
		_vigor = 0xFFFFFFFF;
	}
	else {
		_vigor += amount;
	}

	_CalculateAttackRatings();
}



void GlobalActor::SubtractVigor(uint32 amount) {
	if (amount >= _vigor)
		_vigor = 0;
	else
		_vigor -= amount;

	_CalculateAttackRatings();
}



void GlobalActor::AddFortitude(uint32 amount) {
	if ((0xFFFFFFFF - amount) < _fortitude) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "integer overflow condition detected: " << amount << endl;
		_fortitude = 0xFFFFFFFF;
	}
	else {
		_fortitude += amount;
	}

	_CalculateDefenseRatings();
}



void GlobalActor::SubtractFortitude(uint32 amount) {
	if (amount >= _fortitude)
		_fortitude = 0;
	else
		_fortitude -= amount;

	_CalculateDefenseRatings();
}



void GlobalActor::AddProtection(uint32 amount) {
	if ((0xFFFFFFFF - amount) < _protection) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "integer overflow condition detected: " << amount << endl;
		_protection = 0xFFFFFFFF;
	}
	else {
		_protection += amount;
	}

	_CalculateDefenseRatings();
}



void GlobalActor::SubtractProtection(uint32 amount) {
	if (amount >= _protection)
		_protection = 0;
	else
		_protection -= amount;

	_CalculateDefenseRatings();
}



void GlobalActor::AddStamina(uint32 amount)  {
	if ((0xFFFFFFFF - amount) < _stamina) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "integer overflow condition detected: " << amount << endl;
		_stamina = 0xFFFFFFFF;
	}
	else {
		_stamina += amount;
	}
}



void GlobalActor::SubtractStamina(uint32 amount) {
	if (amount >= _stamina)
		_stamina = 1; // Stamina should never be set to zero because HP fatigue damage is calculated by HP damage / stamina
	else
		_stamina -= amount;
}



void GlobalActor::AddResilience(uint32 amount)  {
	if ((0xFFFFFFFF - amount) < _stamina) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "integer overflow condition detected: " << amount << endl;
		_resilience = 0xFFFFFFFF;
	}
	else {
		_resilience += amount;
	}
}



void GlobalActor::SubtractResilience(uint32 amount) {
	if (amount >= _resilience)
		_resilience = 1; // Resilience should never be set to zero because SP fatigue damage is calculated by SP consumed / resilience
	else
		_resilience -= amount;
}



void GlobalActor::AddAgility(uint32 amount)  {
	if ((0xFFFFFFFF - amount) < _agility) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "integer overflow condition detected: " << amount << endl;
		_agility = 0xFFFFFFFF;
	}
	else {
		_agility += amount;
	}
}



void GlobalActor::SubtractAgility(uint32 amount) {
	if (amount >= _agility)
		_agility = 0;
	else
		_agility -= amount;
}



void GlobalActor::AddEvade(float amount) {
	if (amount < 0.0f) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "function received negative argument value: " << amount << endl;
		return;
	}

	float new_evade = _evade + amount;
	if (new_evade < _evade) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "floating point overflow condition detected: " << amount << endl;
		_evade = 1.0f;
	}
	else if (new_evade > 1.0f) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "evade rating increased above 1.0f: " << amount << endl;
		_evade = 1.0f;
	}
	else {
		_evade = new_evade;
	}
}



void GlobalActor::SubtractEvade(float amount) {
	if (amount > 0.0f) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "function received positive argument value: " << amount << endl;
		return;
	}

	float new_evade = _evade + amount;
	if (new_evade > _evade) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "floating point overflow condition detected: " << amount << endl;
		_evade = 0.0f;
	}
	else if (new_evade < 0.0f) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "evade rating decreased below 0.0f: " << amount << endl;
		_evade = 0.0f;
	}
	else {
		_evade = new_evade;
	}
}



void GlobalActor::_CalculateAttackRatings() {
	_total_physical_attack = _strength;
	_total_ethereal_attack = _vigor;

	if (_weapon_equipped != nullptr) {
		_total_physical_attack += _weapon_equipped->GetPhysicalAttack();
		_total_ethereal_attack += _weapon_equipped->GetEtherealAttack();
	}
}



void GlobalActor::_CalculateDefenseRatings() {
	_total_physical_defense = _fortitude;
	_total_ethereal_defense = _protection;
	
	// Add the physical and ethereal defense bonuses for all equipped armor
	for (uint32 i = 0; i < _armor_equipped.size(); i++) {
		if (_armor_equipped[i] == nullptr) {
			continue;
		}
		
		_total_physical_defense += _armor_equipped[i]->GetPhysicalDefense();
		_total_ethereal_defense += _armor_equipped[i]->GetEtherealDefense();
	}
}

////////////////////////////////////////////////////////////////////////////////
// GlobalCharacter class
////////////////////////////////////////////////////////////////////////////////

GlobalCharacter::GlobalCharacter(uint32 id, bool initial) :
	_experience_for_next_level(0),
	_hit_points_growth(0),
	_skill_points_growth(0),
	_strength_growth(0),
	_vigor_growth(0),
	_fortitude_growth(0),
	_protection_growth(0),
	_stamina_growth(0),
	_resilience_growth(0),
	_agility_growth(0),
	_evade_growth(0.0f)
{
	_id = id;

	// ----- (1): Open the characters script file
	string filename = "lua/data/actors/characters.lua";
	ReadScriptDescriptor char_script;
	if (char_script.OpenFile(filename) == false) {
		PRINT_ERROR << "failed to open character data file: " << filename << endl;
		return;
	}

	// ----- (2): Retrieve their basic character property data
	char_script.OpenTable("characters");
	char_script.OpenTable(_id);
	_name = MakeUnicodeString(char_script.ReadString("name"));
	_filename = char_script.ReadString("filename");

	// ----- (3): Construct the character from the initial stats if necessary
	if (initial == true) {
		char_script.OpenTable("initial_stats");
		_experience_level = char_script.ReadUInt("experience_level");
		_experience_points = char_script.ReadUInt("experience_points");
		_max_hit_points = char_script.ReadUInt("max_hit_points");
		_active_max_hit_points = _max_hit_points;
		_hit_points = _max_hit_points;
		_max_skill_points = char_script.ReadUInt("max_skill_points");
		_active_max_skill_points = _max_skill_points;
		_skill_points = _max_skill_points;
		_strength = char_script.ReadUInt("strength");
		_vigor = char_script.ReadUInt("vigor");
		_fortitude = char_script.ReadUInt("fortitude");
		_protection = char_script.ReadUInt("protection");
		_stamina = char_script.ReadUInt("stamina");
		_resilience = char_script.ReadUInt("resilience");
		_agility = char_script.ReadUInt("agility");
		_evade = char_script.ReadFloat("evade");

		// Add the character's initial equipment. If any equipment ids are zero, that indicates nothing is to be equipped.
		uint32 equipment_id = 0;
		equipment_id = char_script.ReadUInt("weapon");
		if (equipment_id != 0)
			_weapon_equipped = new GlobalWeapon(equipment_id);
		else
			_weapon_equipped = nullptr;

		equipment_id = char_script.ReadUInt("head_armor");
		if (equipment_id != 0)
			_armor_equipped.push_back(new GlobalArmor(equipment_id));
		else
			_armor_equipped.push_back(nullptr);

		equipment_id = char_script.ReadUInt("torso_armor");
		if (equipment_id != 0)
			_armor_equipped.push_back(new GlobalArmor(equipment_id));
		else
			_armor_equipped.push_back(nullptr);

		equipment_id = char_script.ReadUInt("arm_armor");
		if (equipment_id != 0)
			_armor_equipped.push_back(new GlobalArmor(equipment_id));
		else
			_armor_equipped.push_back(nullptr);

		equipment_id = char_script.ReadUInt("leg_armor");
		if (equipment_id != 0)
			_armor_equipped.push_back(new GlobalArmor(equipment_id));
		else
			_armor_equipped.push_back(nullptr);

		char_script.CloseTable();
		if (char_script.IsErrorDetected()) {
			if (GLOBAL_DEBUG) {
				PRINT_WARNING << "one or more errors occurred while reading initial data - they are listed below" << endl;
				cerr << char_script.GetErrorMessages() << endl;
			}
		}
	} // if (initial == true)
	else {
		 // Make sure the _armor_equipped vector is sized appropriately
		_armor_equipped.resize(4, nullptr);
	}

	// ----- (4): Construct the character's initial skill set if necessary
	if (initial) {
		// The skills table contains key/value pairs. The key indicate the level required to learn the skill and the value is the skill's id
		vector<uint32> skill_levels;
		char_script.OpenTable("skills");
		char_script.ReadTableKeys(skill_levels);

		// We want to add the skills beginning with the first learned to the last. ReadTableKeys does not guarantee returing the keys in a sorted order,
		// so sort the skills by level before checking each one.
		sort(skill_levels.begin(), skill_levels.end());

		// Only add the skills for which the experience level requirements are met
		for (uint32 i = 0; i < skill_levels.size(); i++) {
			if (skill_levels[i] <= _experience_level) {
				AddSkill(char_script.ReadUInt(skill_levels[i]));
			}
			// Because skill_levels is sorted, all remaining skills will not have their level requirements met
			else {
				break;
			}
		}

		char_script.CloseTable();
		if (char_script.IsErrorDetected()) {
			if (GLOBAL_DEBUG) {
				PRINT_WARNING << "one or more errors occurred while reading skill data - they are listed below" << endl;
				cerr << char_script.GetErrorMessages() << endl;
			}
		}
	} // if (initial)

	char_script.CloseTable(); // "characters[id]"
	char_script.CloseTable(); // "characters"

	// ----- (5): Determine the character's initial growth if necessary
	if (initial) {
		try {
			ScriptCallFunction<void>(char_script.GetLuaState(), "DetermineNextLevelGrowth", this);
			_ConstructPeriodicGrowth();
		}
		catch (luabind::error e) {
			ScriptManager->HandleLuaError(e);
		}
		catch (luabind::cast_failed e) {
			ScriptManager->HandleCastError(e);
		}
	}

	// ----- (6): Close the script file and calculate all rating totals
	if (char_script.IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			PRINT_WARNING << "one or more errors occurred while reading final data - they are listed below" << endl;
			cerr << char_script.GetErrorMessages() << endl;
		}
	}
	char_script.CloseFile();

	_CalculateAttackRatings();
	_CalculateDefenseRatings();

	// ----- (7) Load character sprite and portrait images
	// NOTE: The code below is all TEMP and is subject to great change or removal in the future
	// TEMP: load standard map sprite walking frames
	if (ImageDescriptor::LoadMultiImageFromElementGrid(_map_frames_standard, "img/sprites/characters/" + _filename + "_walk.png", 4, 6) == false) {
		exit(1);
	}

	// TEMP: Load the character's run animation
	AnimatedImage run;
	vector<StillImage> run_frames;

	if (ImageDescriptor::LoadMultiImageFromElementGrid(run_frames, "img/sprites/characters/" + _filename + "_run.png", 4, 6) == false) {
		exit(1);
	}

	// Store only the right-facing run frames in the animated image
	for (uint32 i = 19; i < run_frames.size(); i++) {
		run.AddFrame(run_frames[i], 75);
	}
	run.SetDimensions(64, 128);
	_battle_animation["run"] = run;

	// TEMP: Load the character's idle animation
	AnimatedImage idle;
	idle.SetDimensions(128, 128);
	vector<uint32> idle_timings(4, 150);

	if (idle.LoadFromFrameGrid("img/sprites/characters/" + _filename + "_idle.png", idle_timings, 1, 4) == false) {
		exit(1);
	}
	_battle_animation["idle"] = idle;

	// TEMP: Load the character's attack animation
	AnimatedImage attack;
	attack.SetDimensions(128, 128);
	vector<uint32> attack_timings(5, 100);

	if (attack.LoadFromFrameGrid("img/sprites/characters/" + _filename + "_attack.png", attack_timings, 1, 5) == false) {
		exit(1);
	}
	_battle_animation["attack"] = attack;

	// TEMP: Load the character's battle portraits from a multi image
	_battle_portraits.assign(5, StillImage());
	for (uint32 i = 0; i < _battle_portraits.size(); i++) {
		_battle_portraits[i].SetDimensions(100.0f, 100.0f);
	}
	string portrait_filename = "img/portraits/damage/" + _filename + ".png";
	if (ImageDescriptor::LoadMultiImageFromElementGrid(_battle_portraits, portrait_filename, 1, 5) == false) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "failed to load battle portrait for character: " << portrait_filename << endl;

		// Load empty portraits
		for (uint32 i = 0; i < _battle_portraits.size(); i++) {
			_battle_portraits[i].Clear();
			_battle_portraits[i].Load("", 1.0f, 1.0f);
		}
	}
} // GlobalCharacter::GlobalCharacter(uint32 id, bool initial)



bool GlobalCharacter::AddExperiencePoints(uint32 xp) {
	_experience_points += xp;
	_experience_for_next_level -= xp;
	return _CheckForGrowth();
}



void GlobalCharacter::AddSkill(uint32 skill_id) {
	if (skill_id == 0) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "function received an invalid skill_id argument: " << skill_id << endl;
		return;
	}
	if (GetSkill(skill_id) != nullptr) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "failed to add skill because the character already knew this skill: " << skill_id << endl;
		return;
	}

	GlobalSkill* skill = new GlobalSkill(skill_id);
	if (skill->IsValid() == false) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "the skill to add failed to load: " << skill_id << endl;
		delete skill;
		return;
	}

	// Add a pointer to the new skill in the appropriate containers
	_skills.push_back(skill);
	switch (skill->GetType()) {
		case GLOBAL_SKILL_ATTACK:
			_attack_skills.push_back(skill);
			break;
		case GLOBAL_SKILL_DEFEND:
			_defense_skills.push_back(skill);
			break;
		case GLOBAL_SKILL_SUPPORT:
			_support_skills.push_back(skill);
			break;
		default:
			IF_PRINT_WARNING(GLOBAL_DEBUG) << "loaded a new skill with an unknown skill type: " << skill->GetType() << endl;
			break;
	}
}



void GlobalCharacter::AddNewSkillLearned(uint32 skill_id) {
	if (skill_id == 0) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "function received an invalid skill_id argument: " << skill_id << endl;
		return;
	}

	// Make sure we don't add a newly learned skill more than once
	for (vector<GlobalSkill*>::iterator i = _new_skills_learned.begin(); i != _new_skills_learned.end(); i++) {
		if (skill_id == (*i)->GetID()) {
			IF_PRINT_WARNING(GLOBAL_DEBUG) << "the skill to add was already present in the list of newly learned skills: " << skill_id << endl;
			return;
		}
	}

	AddSkill(skill_id);
	GlobalSkill* skill = GetSkill(skill_id);
	if (skill == nullptr) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "failed because the new skill was not added successfully: " << skill_id << endl;
		return;
	}

	_new_skills_learned.push_back(skill);
}



bool GlobalCharacter::HasUnacknowledgedGrowth() const {
	if (ReachedNewExperienceLevel() == true) {
		return true;
	}

	if (_hit_points_growth != 0)
		return true;
	if (_skill_points_growth != 0)
		return true;
	if (_strength_growth != 0)
		return true;
	if (_vigor_growth != 0)
		return true;
	if (_fortitude_growth != 0)
		return true;
	if (_protection_growth != 0)
		return true;
	if (_agility_growth != 0)
		return true;
	if (IsFloatEqual(_evade_growth, 0.0f) == false)
		return true;

	return false;
}



bool GlobalCharacter::AcknowledgeGrowth() {
	if (HasUnacknowledgedGrowth() == false) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "function called when no unacknowledged growth was available" << endl;
		return false;
	}

	// Add all growth stats to the character actor
	if (_hit_points_growth != 0) {
		AddMaxHitPoints(_hit_points_growth);
		if (_hit_points > 0)
			AddHitPoints(_hit_points_growth);
	}
	if (_skill_points_growth != 0) {
		AddMaxSkillPoints(_skill_points_growth);
		if (_skill_points > 0)
			AddSkillPoints(_skill_points_growth);
	}
	if (_strength_growth != 0)
		AddStrength(_strength_growth);
	if (_vigor_growth != 0)
		AddVigor(_vigor_growth);
	if (_fortitude_growth != 0)
		AddFortitude(_fortitude_growth);
	if (_protection_growth != 0)
		AddProtection(_protection_growth);
	if (_agility_growth != 0)
		AddAgility(_agility_growth);
	if (IsFloatEqual(_evade_growth, 0.0f) == false)
		AddEvade(_evade_growth);

	_hit_points_growth = 0;
	_skill_points_growth = 0;
	_strength_growth = 0;
	_vigor_growth = 0;
	_fortitude_growth = 0;
	_protection_growth = 0;
	_agility_growth = 0;
	_evade_growth = 0.0f;

	if (ReachedNewExperienceLevel() == false) {
		return false;
	}

	// A new experience level has been gained. Retrieve the growth data for the new experience level
	_experience_level += 1;

	// Retrieve the growth data for the new experience level and check for any additional growth
	bool additional_growth_detected = false;
	string filename = "lua/data/actors/characters.lua";
	ReadScriptDescriptor character_script;
	if (character_script.OpenFile(filename) == false) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "failed to open character data file: " << filename << endl;
		return false;
	}

	try {
		ScriptCallFunction<void>(character_script.GetLuaState(), "DetermineNextLevelGrowth", this);
		_ConstructPeriodicGrowth();
		additional_growth_detected = _CheckForGrowth();
	} catch(luabind::error& e) {
		ScriptManager->HandleLuaError(e);
	} catch(luabind::cast_failed& e) {
		ScriptManager->HandleCastError(e);
	}

	// Reset the skills learned container and add any skills learned at this level
	_new_skills_learned.clear();
	try {
		ScriptCallFunction<void>(character_script.GetLuaState(), "DetermineNewSkillsLearned", this);
	} catch(luabind::error& e) {
		ScriptManager->HandleLuaError(e);
	} catch(luabind::cast_failed& e) {
		ScriptManager->HandleCastError(e);
	}

	character_script.CloseFile();
	return additional_growth_detected;
} // bool GlobalCharacter::AcknowledgeGrowth()



bool GlobalCharacter::_CheckForGrowth() {
	// ----- (1): If a new experience level is gained, empty the periodic growth containers into the growth members
	if (ReachedNewExperienceLevel() == true) {
		_ProcessPeriodicGrowth();
		return true;
	}

	// ----- (2): If there is no growth detected, check all periodic growth containers
	if (_hit_points_periodic_growth.empty() == false) {
		if (_experience_for_next_level <= static_cast<int32>(_hit_points_periodic_growth.front().first)) {
			_ProcessPeriodicGrowth();
			return true;
		}
	}

	if (_skill_points_periodic_growth.empty() == false) {
		if (_experience_for_next_level <= static_cast<int32>(_skill_points_periodic_growth.front().first)) {
			_ProcessPeriodicGrowth();
			return true;
		}
	}

	if (_strength_periodic_growth.empty() == false) {
		if (_experience_for_next_level <= static_cast<int32>(_strength_periodic_growth.front().first)) {
			_ProcessPeriodicGrowth();
			return true;
		}
	}

	if (_vigor_periodic_growth.empty() == false) {
		if (_experience_for_next_level <= static_cast<int32>(_vigor_periodic_growth.front().first)) {
			_ProcessPeriodicGrowth();
			return true;
		}
	}

	if (_fortitude_periodic_growth.empty() == false) {
		if (_experience_for_next_level <= static_cast<int32>(_fortitude_periodic_growth.front().first)) {
			_ProcessPeriodicGrowth();
			return true;
		}
	}

	if (_protection_periodic_growth.empty() == false) {
		if (_experience_for_next_level <= static_cast<int32>(_protection_periodic_growth.front().first)) {
			_ProcessPeriodicGrowth();
			return true;
		}
	}

	if (_agility_periodic_growth.empty() == false) {
		if (_experience_for_next_level <= static_cast<int32>(_agility_periodic_growth.front().first)) {
			_ProcessPeriodicGrowth();
			return true;
		}
	}

	if (_evade_periodic_growth.empty() == false) {
		if (_experience_for_next_level <= static_cast<int32>(_evade_periodic_growth.front().first)) {
			_ProcessPeriodicGrowth();
			return true;
		}
	}

	return false;
} // bool GlobalCharacter::_CheckForGrowth()



void GlobalCharacter::_ProcessPeriodicGrowth() {
	// When a new level is gained, we can simply empty all of the periodic growth containers
	if (ReachedNewExperienceLevel() == true) {
		for (uint32 i = 0; i < _hit_points_periodic_growth.size(); i++)
			_hit_points_growth += _hit_points_periodic_growth[i].second;
		_hit_points_periodic_growth.clear();

		for (uint32 i = 0; i < _skill_points_periodic_growth.size(); i++)
			_skill_points_growth += _skill_points_periodic_growth[i].second;
		_skill_points_periodic_growth.clear();

		for (uint32 i = 0; i < _strength_periodic_growth.size(); i++)
			_strength_growth += _strength_periodic_growth[i].second;
		_strength_periodic_growth.clear();

		for (uint32 i = 0; i < _vigor_periodic_growth.size(); i++)
			_vigor_growth += _vigor_periodic_growth[i].second;
		_vigor_periodic_growth.clear();

		for (uint32 i = 0; i < _fortitude_periodic_growth.size(); i++)
			_fortitude_growth += _fortitude_periodic_growth[i].second;
		_fortitude_periodic_growth.clear();

		for (uint32 i = 0; i < _protection_periodic_growth.size(); i++)
			_protection_growth += _protection_periodic_growth[i].second;
		_protection_periodic_growth.clear();

		for (uint32 i = 0; i < _agility_periodic_growth.size(); i++)
			_agility_growth += _agility_periodic_growth[i].second;
		_agility_periodic_growth.clear();

		for (uint32 i = 0; i < _evade_periodic_growth.size(); i++)
			_evade_growth += _evade_periodic_growth[i].second;
		_evade_periodic_growth.clear();
	}
	// Otherwise if no level was gained, process each growth container to deal out any growth that has been earned
	else {
		while (_hit_points_periodic_growth.begin() != _hit_points_periodic_growth.end()) {
			if (_experience_for_next_level <= static_cast<int32>(_hit_points_periodic_growth.begin()->first)) {
				_hit_points_growth += _hit_points_periodic_growth.begin()->second;
				_hit_points_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_skill_points_periodic_growth.begin() != _skill_points_periodic_growth.end()) {
			if (_experience_for_next_level <= static_cast<int32>(_skill_points_periodic_growth.begin()->first)) {
				_skill_points_growth += _skill_points_periodic_growth.begin()->second;
				_skill_points_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_strength_periodic_growth.begin() != _strength_periodic_growth.end()) {
			if (_experience_for_next_level <= static_cast<int32>(_strength_periodic_growth.begin()->first)) {
				_strength_growth += _strength_periodic_growth.begin()->second;
				_strength_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_vigor_periodic_growth.begin() != _vigor_periodic_growth.end()) {
			if (_experience_for_next_level <= static_cast<int32>(_vigor_periodic_growth.begin()->first)) {
				_vigor_growth += _vigor_periodic_growth.begin()->second;
				_vigor_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_fortitude_periodic_growth.begin() != _fortitude_periodic_growth.end()) {
			if (_experience_for_next_level <= static_cast<int32>(_fortitude_periodic_growth.begin()->first)) {
				_fortitude_growth += _fortitude_periodic_growth.begin()->second;
				_fortitude_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_protection_periodic_growth.begin() != _protection_periodic_growth.end()) {
			if (_experience_for_next_level <= static_cast<int32>(_protection_periodic_growth.begin()->first)) {
				_protection_growth += _protection_periodic_growth.begin()->second;
				_protection_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_agility_periodic_growth.begin() != _agility_periodic_growth.end()) {
			if (_experience_for_next_level <= static_cast<int32>(_agility_periodic_growth.begin()->first)) {
				_agility_growth += _agility_periodic_growth.begin()->second;
				_agility_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_evade_periodic_growth.begin() != _evade_periodic_growth.end()) {
			if (_experience_for_next_level <= static_cast<int32>(_evade_periodic_growth.begin()->first)) {
				_evade_growth += _evade_periodic_growth.begin()->second;
				_evade_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}
	}
} // void GlobalCharacter::_ProcessPeriodicGrowth()



void GlobalCharacter::_ConstructPeriodicGrowth() {
	// TODO: Implement a gradual growth algorithm.
	// The following scheme is suggested for this algorithm:
	// 1) Divide all stat gains by 2. Half is rewarded gradually, while the other half is rewarded when the next level is reached
	// 2) If a stat does not grow by more than one point, award all growth for that stat at the next level
	// 3) Divide the gradual amount of the growth into a number of small sections (ie, +10 HP => +2 HP 5 times)
	// 4) Evenly space the growth distributed based on the range of current XP to the XP needed to reach the next level

	// TEMP: currently all growth is added when the experience level is gained
	_hit_points_periodic_growth.push_back(make_pair(0, _hit_points_growth));
	_skill_points_periodic_growth.push_back(make_pair(0, _skill_points_growth));
	_strength_periodic_growth.push_back(make_pair(0, _strength_growth));
	_vigor_periodic_growth.push_back(make_pair(0, _vigor_growth));
	_fortitude_periodic_growth.push_back(make_pair(0, _fortitude_growth));
	_protection_periodic_growth.push_back(make_pair(0, _protection_growth));
	_stamina_periodic_growth.push_back(make_pair(0, _stamina_growth));
	_resilience_periodic_growth.push_back(make_pair(0, _resilience_growth));
	_agility_periodic_growth.push_back(make_pair(0, _agility_growth));
	_evade_periodic_growth.push_back(make_pair(0, _evade_growth));

	// Reset all growth accumulators
	_hit_points_growth = 0;
	_skill_points_growth = 0;
	_strength_growth = 0;
	_vigor_growth = 0;
	_fortitude_growth = 0;
	_protection_growth = 0;
	_stamina_growth = 0;
	_resilience_growth = 0;
	_agility_growth = 0;
	_evade_growth = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////
// GlobalEnemy class
////////////////////////////////////////////////////////////////////////////////

GlobalEnemy::GlobalEnemy(uint32 id) :
	GlobalActor(),
	_no_stat_randomization(false),
	_sprite_width(0),
	_sprite_height(0),
	_drunes_dropped(0)
{
	_id = id;

	// ----- (1): Use the id member to determine the name of the data file that the enemy is defined in
	string file_ext;
	string filename;

	if (_id == 0)
		PRINT_ERROR << "invalid id for loading enemy data: " << _id << endl;
	else if ((_id > 0) && (_id <= 100))
		file_ext = "01";
	else if ((_id > 100) && (_id <= 200))
		file_ext = "02";

	filename = "lua/data/actors/enemies_set_" + file_ext + ".lua";

	// ----- (2): Open the script file and table that store the enemy data
	ReadScriptDescriptor enemy_data;
	if (enemy_data.OpenFile(filename) == false) {
		PRINT_ERROR << "failed to open enemy data file: " << filename << endl;
		return;
	}

	enemy_data.OpenTable("enemies");
	enemy_data.OpenTable(_id);

	// ----- (3): Load the enemy's name and sprite data
	_name = MakeUnicodeString(enemy_data.ReadString("name"));
	_filename = enemy_data.ReadString("filename");
	_sprite_width = enemy_data.ReadInt("sprite_width");
	_sprite_height = enemy_data.ReadInt("sprite_height");

	// ----- (4): Attempt to load the MultiImage for the sprite's frames, which should contain one row and four columns of images
	_battle_sprite_frames.assign(4, StillImage());
	string sprite_filename = "img/sprites/enemies/" + _filename + ".png";
	if (ImageDescriptor::LoadMultiImageFromElementGrid(_battle_sprite_frames, sprite_filename, 1, 4) == false) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "failed to load sprite frames for enemy: " << sprite_filename << endl;
	}

	// ----- (5): Load the enemy's base stats
	if (enemy_data.DoesBoolExist("no_stat_randomization") == true) {
		_no_stat_randomization = enemy_data.ReadBool("no_stat_randomization");
	}

	enemy_data.OpenTable("base_stats");
	_max_hit_points = enemy_data.ReadUInt("hit_points");
	_hit_points = _max_hit_points;
	_max_skill_points = enemy_data.ReadUInt("skill_points");
	_skill_points = _max_skill_points;
	_experience_points = enemy_data.ReadUInt("experience_points");
	_strength = enemy_data.ReadUInt("strength");
	_vigor = enemy_data.ReadUInt("vigor");
	_fortitude = enemy_data.ReadUInt("fortitude");
	_protection = enemy_data.ReadUInt("protection");
	_stamina = _max_hit_points;
	if (enemy_data.DoesUIntExist("stamina") == true) {
		_stamina = enemy_data.ReadUInt("stamina");
	}
	_resilience = _max_skill_points;
	if (enemy_data.DoesUIntExist("resilience") == true) {
		_resilience = enemy_data.ReadUInt("resilience");
	}
	_agility = enemy_data.ReadUInt("agility");
	_evade = enemy_data.ReadFloat("evade");
	_drunes_dropped = enemy_data.ReadUInt("drunes");
	enemy_data.CloseTable();

	// ----- (6): Add the set of skills for the enemy
	enemy_data.OpenTable("skills");
	for (uint32 i = 1; i <= enemy_data.GetTableSize(); i++) {
		_skill_set.push_back(enemy_data.ReadUInt(i));
	}
	enemy_data.CloseTable();

	// ----- (7): Load the possible items that the enemy may drop
	enemy_data.OpenTable("drop_objects");
	for (uint32 i = 1; i <= enemy_data.GetTableSize(); i++) {
		enemy_data.OpenTable(i);
		_dropped_objects.push_back(enemy_data.ReadUInt(1));
		_dropped_chance.push_back(enemy_data.ReadFloat(2));
		enemy_data.CloseTable();
	}
	enemy_data.CloseTable();

	enemy_data.CloseTable(); // enemies[_id]
	enemy_data.CloseTable(); // enemies

	if (enemy_data.IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			PRINT_WARNING << "one or more errors occurred while reading the enemy data - they are listed below" << endl;
			cerr << enemy_data.GetErrorMessages() << endl;
		}
	}

	enemy_data.CloseFile();

	_CalculateAttackRatings();
	_CalculateDefenseRatings();
} // GlobalEnemy::GlobalEnemy(uint32 id)



void GlobalEnemy::AddSkill(uint32 skill_id) {
	if (skill_id == 0) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "function received an invalid skill_id argument: " << skill_id << endl;
		return;
	}
	if (GetSkill(skill_id) != nullptr) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "failed to add skill because the enemy already knew this skill: " << skill_id << endl;
		return;
	}

	GlobalSkill* skill = new GlobalSkill(skill_id);
	if (skill->IsValid() == false) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "the skill to add failed to load: " << skill_id << endl;
		delete skill;
		return;
	}

	_skills.push_back(skill);
}



void GlobalEnemy::Initialize() {
	if (_skills.empty() == false) { // Indicates that the enemy has already been initialized
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "function was invoked for an already initialized enemy: " << _id << endl;
		return;
	}

	// TODO: we may wish to actually define XP levels for enemies in their data table, though I don't know what purpose it may serve
	_experience_level = 1;

	// ----- (1): Add all new skills that should be available at the current experience level
	for (uint32 i = 0; i < _skill_set.size(); i++) {
		AddSkill(_skill_set[i]);
	}

	if (_skills.empty()) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "no skills were added for the enemy: " << _id << endl;
	}

	// ----- (3): Randomize the stats by using a guassian random variable
	if (_no_stat_randomization == false) {
		// Use the base stats as the means and a standard deviation of 10% of the mean
		_max_hit_points     = GaussianRandomValue(_max_hit_points, _max_hit_points / 10.0f);
		_max_skill_points   = GaussianRandomValue(_max_skill_points, _max_skill_points / 10.0f);
		_experience_points  = GaussianRandomValue(_experience_points, _experience_points / 10.0f);
		_strength           = GaussianRandomValue(_strength, _strength / 10.0f);
		_vigor              = GaussianRandomValue(_strength, _strength / 10.0f);
		_fortitude          = GaussianRandomValue(_fortitude, _fortitude / 10.0f);
		_protection         = GaussianRandomValue(_protection, _protection / 10.0f);
		_stamina            = GaussianRandomValue(_stamina, _stamina / 10.0f);
		_resilience         = GaussianRandomValue(_resilience, _resilience / 10.0f);
		_agility            = GaussianRandomValue(_agility, _agility / 10.0f);
		// TODO: need a gaussian random var function that takes a float arg
		//_evade              = static_cast<float>(GaussianRandomValue(_evade, _evade / 10.0f));
		_drunes_dropped     = GaussianRandomValue(_drunes_dropped, _drunes_dropped / 10.0f);
	}

	// ----- (4): Set the current hit points and skill points to their new maximum values
	_hit_points = _max_hit_points;
	_active_max_hit_points = _max_hit_points;
	_skill_points = _max_skill_points;
	_active_max_skill_points = _max_skill_points;
	// Stamina and resilieince should never fall below one since it is used as a divisor in fatigue damage calculations
	if (_stamina < 1)
		_stamina = 1;
	if (_resilience < 1)
		_resilience = 1;
} // void GlobalEnemy::Initialize(uint32 xp_level)



void GlobalEnemy::DetermineDroppedObjects(vector<GlobalObject*>& objects) {
	objects.clear();

	for (uint32 i = 0; i < _dropped_objects.size(); i++) {
		if (RandomFloat() < _dropped_chance[i]) {
			objects.push_back(GlobalCreateNewObject(_dropped_objects[i]));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// GlobalParty class
////////////////////////////////////////////////////////////////////////////////

void GlobalParty::AddActor(GlobalActor* actor, int32 index) {
	if (actor == nullptr) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "function received a nullptr actor argument" << endl;
		return;
	}

	if (_allow_duplicates == false) {
		// Check that this actor is not already in the party
		for (uint32 i = 0; i < _actors.size(); i++) {
			if (actor->GetID() == _actors[i]->GetID()) {
				IF_PRINT_WARNING(GLOBAL_DEBUG) << "attempted to add an actor that was already in the party "
					<< "when duplicates were not allowed: " << actor->GetID() << endl;
				return;
			}
		}
	}

	// Add actor to the end of the party if index is negative
	if (index < 0) {
		_actors.push_back(actor);
		return;
	}

	// Check that the requested index does not exceed the size of the container
	if (static_cast<uint32>(index) >= _actors.size()) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "index argument exceeded the current party size: " << index << endl;
		_actors.push_back(actor); // Add the actor to the end of the party instead
		return;
	}
	else {
		vector<GlobalActor*>::iterator position = _actors.begin();
		for (int32 i = 0; i < index; i++, position++);
		_actors.insert(position, actor);
	}
}



GlobalActor* GlobalParty::RemoveActorAtIndex(uint32 index) {
	if (index >= _actors.size()) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "index argument exceeded current party size: " << index << endl;
		return nullptr;
	}

	GlobalActor* removed_actor = _actors[index];
	vector<GlobalActor*>::iterator position = _actors.begin();
	for (uint32 i = 0; i < index; i++, position++);
	_actors.erase(position);

	return removed_actor;
}



GlobalActor* GlobalParty::RemoveActorByID(uint32 id) {
	if (_allow_duplicates) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "tried to remove actor when duplicates were allowed in the party: " << id << endl;
		return nullptr;
	}

	GlobalActor* removed_actor = nullptr;
	for (vector<GlobalActor*>::iterator position = _actors.begin(); position != _actors.end(); position++) {
		if (id == (*position)->GetID()) {
			removed_actor = *position;
			_actors.erase(position);
			break;
		}
	}

	if (removed_actor == nullptr) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "failed to find an actor in the party with the requested id: " << id << endl;
	}

	return removed_actor;
}



GlobalActor* GlobalParty::GetActorAtIndex(uint32 index) const {
	if (index >= _actors.size()) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "index argument exceeded current party size: " << index << endl;
		return nullptr;
	}

	return _actors[index];
}



GlobalActor* GlobalParty::GetActorByID(uint32 id) const {
	if (_allow_duplicates) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "tried to retrieve actor when duplicates were allowed in the party: " << id << endl;
		return nullptr;
	}

	for (uint32 i = 0; i < _actors.size(); i++) {
		if (_actors[i]->GetID() == id) {
			return _actors[i];
		}
	}

	IF_PRINT_WARNING(GLOBAL_DEBUG) << "failed to find an actor in the party with the requested id: " << id << endl;
	return nullptr;
}



void GlobalParty::SwapActorsByIndex(uint32 first_index, uint32 second_index) {
	if (first_index == second_index) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "first_index and second_index arguments had the same value: " << first_index << endl;
		return;
	}
	if (first_index >= _actors.size()) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "first_index argument exceeded current party size: " << first_index << endl;
		return;
	}
	if (second_index >= _actors.size()) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "second_index argument exceeded current party size: " << second_index << endl;
		return;
	}

	GlobalActor* tmp = _actors[first_index];
	_actors[first_index] = _actors[second_index];
	_actors[second_index] = tmp;
}



void GlobalParty::SwapActorsByID(uint32 first_id, uint32 second_id) {
	if (first_id == second_id) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "first_id and second_id arguments had the same value: " << first_id << endl;
		return;
	}
	if (_allow_duplicates) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "tried to swap actors when duplicates were allowed in the party: " << first_id << endl;
		return;
	}

	vector<GlobalActor*>::iterator first_position;
	vector<GlobalActor*>::iterator second_position;
	for (first_position = _actors.begin(); first_position != _actors.end(); first_position++) {
		if ((*first_position)->GetID() == first_id)
			break;
	}
	for (second_position = _actors.begin(); second_position != _actors.end(); second_position++) {
		if ((*second_position)->GetID() == second_id)
			break;
	}

	if (first_position == _actors.end()) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "failed to find an actor in the party with the requested first_id: " << first_id << endl;
		return;
	}
	if (second_position == _actors.end()) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "failed to find an actor in the party with the requested second_id: " << second_id << endl;
		return;
	}

	GlobalActor* tmp = *first_position;
	*first_position = *second_position;
	*second_position = tmp;
}



GlobalActor* GlobalParty::ReplaceActorByIndex(uint32 index, GlobalActor* new_actor) {
	if (new_actor == nullptr) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "function received a nullptr new_actor argument" << endl;
		return nullptr;
	}
	if (index >= _actors.size()) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "index argument exceeded current party size: " << index << endl;
		return nullptr;
	}

	GlobalActor* tmp = _actors[index];
	_actors[index] = new_actor;
	return tmp;
}



GlobalActor* GlobalParty::ReplaceActorByID(uint32 id, GlobalActor* new_actor) {
	if (_allow_duplicates) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "tried to replace actor when duplicates were allowed in the party: " << id << endl;
		return nullptr;
	}
	if (new_actor == nullptr) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "function received a nullptr new_actor argument" << endl;
		return nullptr;
	}

	GlobalActor* removed_actor = nullptr;
	for (vector<GlobalActor*>::iterator position = _actors.begin(); position != _actors.end(); position++) {
		if ((*position)->GetID() == id) {
			removed_actor = *position;
			*position = new_actor;
			break;
		}
	}

	if (removed_actor == nullptr) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "failed to find an actor in the party with the requested id: " << id << endl;
	}

	return removed_actor;
}



float GlobalParty::AverageExperienceLevel() const {
	if (_actors.empty())
		return 0.0f;

	float xp_level_sum = 0.0f;
	for (uint32 i = 0; i < _actors.size(); i++)
		xp_level_sum += static_cast<float>(_actors[i]->GetExperienceLevel());
	return (xp_level_sum / static_cast<float>(_actors.size()));
}



void GlobalParty::AddHitPoints(uint32 hp) {
	for (vector<GlobalActor*>::iterator i = _actors.begin(); i != _actors.end(); i++) {
		(*i)->AddHitPoints(hp);
	}
}



void GlobalParty::AddSkillPoints(uint32 sp) {
	for (vector<GlobalActor*>::iterator i = _actors.begin(); i != _actors.end(); i++) {
		(*i)->AddSkillPoints(sp);
	}
}

} // namespace hoa_global
