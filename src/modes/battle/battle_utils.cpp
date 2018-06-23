///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_utils.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for battle mode utility code
***
*** This file contains utility code that is shared among the various battle mode
*** classes.
*** ***************************************************************************/

#include "defs.h"
#include "utils.h"

#include "global.h"

#include "system.h"

#include "battle.h"
#include "battle_actors.h"
#include "battle_utils.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_system;

using namespace hoa_global;

namespace hoa_battle {

namespace private_battle {

////////////////////////////////////////////////////////////////////////////////
// Standard battle calculation functions
////////////////////////////////////////////////////////////////////////////////

bool CalculateStandardEvasion(BattleTarget* target) {
	return CalculateStandardEvasionAdder(target, 0.0f);
}



bool CalculateStandardEvasionAdder(BattleTarget* target, float add_eva) {
	if (target == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr target argument" << endl;
		return false;
	}
	if (IsTargetParty(target->GetType()) == true) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "target was a party type: " << target->GetType() << endl;
		return false;
	}

	float evasion = 0.0f;
	evasion = target->GetActor()->GetEvade();
	evasion += add_eva;

	// Check for absolute hit/miss conditions
	if (evasion <= 0.0f)
		return false;
	else if (evasion >= 100.0f)
		return true;

	if (RandomFloat(0.0f, 100.0f) <= evasion)
		return true;
	else
		return false;
} // bool CalculateStandardEvasionAdder(BattleTarget* target, float add_evade)



bool CalculateStandardEvasionMultiplier(BattleTarget* target, float mul_eva) {
	if (target == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr target argument" << endl;
		return false;
	}
	if (IsTargetActor(target->GetType()) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "target type was not actor: " << target->GetType() << endl;
		return 0;
	}
	if (mul_eva < 0.0f) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received negative multiplier argument: " << mul_eva << endl;
		mul_eva = fabs(mul_eva);
	}

	// Find the base evasion and apply the multiplier
	float evasion = 0.0f;
	evasion = target->GetActor()->GetEvade();

	evasion = evasion * mul_eva;

	// Check for absolute hit/miss conditions
	if (evasion <= 0.0f)
		return false;
	else if (evasion >= 100.0f)
		return true;

	if (RandomFloat(0.0f, 100.0f) > evasion)
		return false;
	else
		return true;
} // bool CalculateStandardEvasionMultiplier(BattleTarget* target, float mul_evade)



uint32 CalculatePhysicalDamage(BattleActor* attacker, BattleTarget* target) {
	return CalculatePhysicalDamageAdder(attacker, target, 0, 0.10f);
}



uint32 CalculatePhysicalDamage(BattleActor* attacker, BattleTarget* target, float std_dev) {
	return CalculatePhysicalDamageAdder(attacker, target, 0, std_dev);
}



uint32 CalculatePhysicalDamageAdder(BattleActor* attacker, BattleTarget* target, int32 add_atk) {
	return CalculatePhysicalDamageAdder(attacker, target, add_atk, 0.10f);
}



uint32 CalculatePhysicalDamageAdder(BattleActor* attacker, BattleTarget* target, int32 add_atk, float std_dev) {
	if (attacker == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr attacker argument" << endl;
		return 0;
	}
	if (target == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr target argument" << endl;
		return 0;
	}
	if (IsTargetActor(target->GetType()) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "target type was not actor: " << target->GetType() << endl;
		return 0;
	}
	if (std_dev < 0.0f) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received negative standard deviation argument: " << std_dev << endl;
		std_dev = fabs(std_dev);
	}

	// Holds the total physical attack of the attacker and modifier
	int32 total_phys_atk = attacker->GetTotalPhysicalAttack() + add_atk;
	if (total_phys_atk < 0)
		total_phys_atk = 0;

	// Holds the total physical defense of the target
	int32 total_phys_def = 0;
	total_phys_def = target->GetActor()->GetTotalPhysicalDefense();

	// Holds the total damage dealt
	int32 total_dmg = total_phys_atk - total_phys_def;

	// If the total damage is zero, fall back to causing a small non-zero damage value
	if (total_dmg <= 0)
		return static_cast<uint32>(RandomBoundedInteger(1, 5));

	// Holds the absolute standard deviation used in the GaussianRandomValue function
	float abs_std_dev = 0.0f;
	abs_std_dev = static_cast<float>(total_dmg) * std_dev;
	total_dmg = GaussianRandomValue(total_dmg, abs_std_dev, false);

	// If the total damage came to a value less than or equal to zero after the gaussian randomization,
	// fall back to returning a small non-zero damage value
	if (total_dmg <= 0)
		return static_cast<uint32>(RandomBoundedInteger(1, 5));

	return static_cast<uint32>(total_dmg);
} // uint32 CalculatePhysicalDamageAdder(BattleActor* attacker, BattleTarget* target, int32 add_atk, float std_dev)



uint32 CalculatePhysicalDamageMultiplier(BattleActor* attacker, BattleTarget* target, float mul_atk) {
	return CalculatePhysicalDamageMultiplier(attacker, target, mul_atk, 0.10f);
}



uint32 CalculatePhysicalDamageMultiplier(BattleActor* attacker, BattleTarget* target, float mul_atk, float std_dev) {
	if (attacker == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr attacker argument" << endl;
		return 0;
	}
	if (target == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr target argument" << endl;
		return 0;
	}
	if (IsTargetActor(target->GetType()) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "target type was not actor: " << target->GetType() << endl;
		return 0;
	}
	if (mul_atk < 0.0f) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received negative multiplier arument: " << mul_atk << endl;
		mul_atk = fabs(mul_atk);
	}
	if (std_dev < 0.0f) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received negative standard deviation argument: " << std_dev << endl;
		std_dev = fabs(std_dev);
	}

	// Retrieve the total physical attack of the attacker and apply the modifier
	int32 total_phys_atk = attacker->GetTotalPhysicalAttack();
	total_phys_atk = static_cast<int32>(static_cast<float>(total_phys_atk) * mul_atk);

	if (total_phys_atk < 0)
		total_phys_atk = 0;

	// Holds the total physical defense of the target
	int32 total_phys_def = 0;
	total_phys_def = target->GetActor()->GetTotalPhysicalDefense();

	// Holds the total damage dealt
	int32 total_dmg = total_phys_atk - total_phys_def;

	// If the total damage is zero, fall back to causing a small non-zero damage value
	if (total_dmg <= 0)
		return static_cast<uint32>(RandomBoundedInteger(1, 5));

	// Holds the absolute standard deviation used in the GaussianRandomValue function
	float abs_std_dev = 0.0f;
	// A value of "0.075f" means the standard deviation should be 7.5% of the mean (the total damage)
	abs_std_dev = static_cast<float>(total_dmg) * std_dev;
	total_dmg = GaussianRandomValue(total_dmg, abs_std_dev, false);

	// If the total damage came to a value less than or equal to zero after the gaussian randomization,
	// fall back to returning a small non-zero damage value
	if (total_dmg <= 0)
		return static_cast<uint32>(RandomBoundedInteger(1, 5));

	return static_cast<uint32>(total_dmg);
} // uint32 CalculatePhysicalDamageMultiplier(BattleActor* attacker, BattleTarget* target, float mul_phys, float std_dev)



uint32 CalculateEtherealDamage(BattleActor* attacker, BattleTarget* target) {
	return CalculateEtherealDamageAdder(attacker, target, 0, 0.10f);
}



uint32 CalculateEtherealDamage(BattleActor* attacker, BattleTarget* target, float std_dev) {
	return CalculateEtherealDamageAdder(attacker, target, 0, std_dev);
}



uint32 CalculateEtherealDamageAdder(BattleActor* attacker, BattleTarget* target, int32 add_atk) {
	return CalculateEtherealDamageAdder(attacker, target, add_atk, 0.10f);
}



uint32 CalculateEtherealDamageAdder(BattleActor* attacker, BattleTarget* target, int32 add_atk, float std_dev) {
	if (attacker == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr attacker argument" << endl;
		return 0;
	}
	if (target == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr target argument" << endl;
		return 0;
	}
	if (IsTargetActor(target->GetType()) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "target type was not actor: " << target->GetType() << endl;
		return 0;
	}
	if (std_dev < 0.0f) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received negative standard deviation argument: " << std_dev << endl;
		std_dev = fabs(std_dev);
	}

	// Holds the total ethereal attack of the attacker and modifier
	int32 total_eth_atk = 0;
	total_eth_atk = attacker->GetTotalEtherealAttack() + add_atk;
	if (total_eth_atk < 0)
		total_eth_atk = 0;

	// Holds the total ethereal defense of the target
	int32 total_eth_def = 0;
	total_eth_def = target->GetActor()->GetTotalEtherealDefense();

	// Holds the total damage dealt
	int32 total_dmg = total_eth_atk - total_eth_def;
	if (total_dmg < 0)
		total_dmg = 0;

	// If the total damage is zero, fall back to causing a small non-zero damage value
	if (total_dmg <= 0)
		return static_cast<uint32>(RandomBoundedInteger(1, 5));

	// Holds the absolute standard deviation used in the GaussianRandomValue function
	float abs_std_dev = 0.0f;
	abs_std_dev = static_cast<float>(total_dmg) * std_dev;
	total_dmg = GaussianRandomValue(total_dmg, abs_std_dev, false);

	// If the total damage came to a value less than or equal to zero after the gaussian randomization,
	// fall back to returning a small non-zero damage value
	if (total_dmg <= 0)
		return static_cast<uint32>(RandomBoundedInteger(1, 5));

	return static_cast<uint32>(total_dmg);
} // uint32 CalculateEtherealDamageAdder(BattleActor* attacker, BattleTarget* target, int32 add_atk, float std_dev)



uint32 CalculateEtherealDamageMultiplier(BattleActor* attacker, BattleTarget* target, float mul_atk) {
	return CalculateEtherealDamageMultiplier(attacker, target, mul_atk, 0.10f);
}



uint32 CalculateEtherealDamageMultiplier(BattleActor* attacker, BattleTarget* target, float mul_atk, float std_dev) {
	if (attacker == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr attacker argument" << endl;
		return 0;
	}
	if (target == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr target argument" << endl;
		return 0;
	}
	if (IsTargetActor(target->GetType()) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "target type was not actor: " << target->GetType() << endl;
		return 0;
	}
	if (mul_atk < 0.0f) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received negative multiplier arument: " << mul_atk << endl;
		mul_atk = fabs(mul_atk);
	}
	if (std_dev < 0.0f) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received negative standard deviation argument: " << std_dev << endl;
		std_dev = fabs(std_dev);
	}

	// Retrieve the total ethereal attack of the attacker and apply the modifier
	int32 total_eth_atk = static_cast<int32>(static_cast<float>(attacker->GetTotalEtherealAttack()) * mul_atk);

	if (total_eth_atk < 0)
		total_eth_atk = 0;

	// Holds the total etheral defense of the target
	int32 total_eth_def = 0;
	total_eth_def = target->GetActor()->GetTotalEtherealDefense();

	// Holds the total damage dealt
	int32 total_dmg = total_eth_atk - total_eth_def;

	// If the total damage is zero, fall back to causing a small non-zero damage value
	if (total_dmg <= 0)
		return static_cast<uint32>(RandomBoundedInteger(1, 5));

	// Holds the absolute standard deviation used in the GaussianRandomValue function
	float abs_std_dev = 0.0f;
	// A value of "0.075f" means the standard deviation should be 7.5% of the mean (the total damage)
	abs_std_dev = static_cast<float>(total_dmg) * std_dev;
	total_dmg = GaussianRandomValue(total_dmg, abs_std_dev, false);

	// If the total damage came to a value less than or equal to zero after the gaussian randomization,
	// fall back to returning a small non-zero damage value
	if (total_dmg <= 0)
		return static_cast<uint32>(RandomBoundedInteger(1, 5));

	return static_cast<uint32>(total_dmg);
} // uint32 CalculateEtherealDamageMultiplier(BattleActor* attacker, BattleTarget* target, float mul_phys, float std_dev)

////////////////////////////////////////////////////////////////////////////////
// BattleTimer class
////////////////////////////////////////////////////////////////////////////////

BattleTimer::BattleTimer() :
	SystemTimer(),
	_stun_time(0),
	_visible_time_expired(0),
	_multiplier_active(false),
	_multiplier_factor(0.0f),
	_multiplier_fraction_accumulator(0.0f)
{}



BattleTimer::BattleTimer(uint32 duration, int32 loops) :
	SystemTimer(duration, loops),
	_stun_time(0),
	_visible_time_expired(0),
	_multiplier_active(false),
	_multiplier_factor(0.0f),
	_multiplier_fraction_accumulator(0.0f)
{}



void BattleTimer::Update() {
	Update(SystemManager->GetUpdateTime());
}



void BattleTimer::Update(uint32 time) {
	if (_auto_update == true) {
		IF_PRINT_WARNING(SYSTEM_DEBUG) << "update failed because timer is in automatic update mode" << endl;
		return;
	}
	if (IsRunning() == false) {
		return;
	}

	if (_multiplier_active == true) {
		time = _ApplyMultiplier(time);
	}
	time = _ApplyStun(time);
	SystemTimer::_UpdateTimer(time);
	_UpdateVisibleTimeExpired(time);
}



void BattleTimer::Reset() {
	SystemTimer::Reset();
	_multiplier_fraction_accumulator = 0.0f;
}



void BattleTimer::SetTimeExpired(uint32 time) {
	_time_expired = time;

	if ((_time_expired == 0) && (_times_completed == 0)) {
		_state = SYSTEM_TIMER_INITIAL;
	}

	else if (_time_expired >= _duration) {
		_time_expired = 0;
		_times_completed++;

		// Check if the timer has finished the final loop and should reach the finished state
		if ((_number_loops >= 0) && (_times_completed >= static_cast<uint32>(_number_loops))) {
			_state = SYSTEM_TIMER_FINISHED;
		}
	}
}



void BattleTimer::ActivateMultiplier(bool activate, float multiplier) {
	_multiplier_active = activate;

	if (activate == true) {
		if (multiplier < 0.0f) {
			IF_PRINT_WARNING(BATTLE_DEBUG) << "attempted to activate a negative multiplier factor: " << multiplier << endl;
			_multiplier_active = false;
		}
		else {
			_multiplier_factor = multiplier;
			// Note: the fraction accumulator is not reset in this case. This allows the multiplier factor to
			// change without reseting the fraction. Fractions are only reset when the multiplier is deactivated
		}
	}
	else {
		_multiplier_factor = 0.0f;
		_multiplier_fraction_accumulator = 0.0f;
	}
}



void BattleTimer::_AutoUpdate() {
	if (_auto_update == false) {
		IF_PRINT_WARNING(SYSTEM_DEBUG) << "tried to automatically update a timer that does not have auto updates enabled" << endl;
		return;
	}
	if (IsRunning() == false) {
		return;
	}

	uint32 time = SystemManager->GetUpdateTime();
	if (_multiplier_active == true) {
		time = _ApplyMultiplier(time);
	}
	time = _ApplyStun(time);
	SystemTimer::_UpdateTimer(time);
	_UpdateVisibleTimeExpired(time);
}



uint32 BattleTimer::_ApplyStun(uint32 time) {
	if (_stun_time > 0) {
		if (_stun_time >= time) {
			_stun_time -= time;
			time = 0;
		}
		else {
			_stun_time = 0;
			time -= _stun_time;
		}
	}

	return time;
}



uint32 BattleTimer::_ApplyMultiplier(uint32 time) {
	float update_time = _multiplier_factor * static_cast<float>(time);
	uint32 return_time = static_cast<uint32>(_multiplier_factor * time);

	_multiplier_fraction_accumulator += (update_time - return_time);

	if (_multiplier_fraction_accumulator >= 1.0f) {
		uint32 accumulator_overflow = static_cast<uint32>(_multiplier_fraction_accumulator);
		_multiplier_fraction_accumulator -= accumulator_overflow;
		return_time += accumulator_overflow;
	}

	return return_time;
}



void BattleTimer::_UpdateVisibleTimeExpired(uint32 time) {
	(void)time;

	// TODO: adjust the _visible_time_expired member accordingly
}

////////////////////////////////////////////////////////////////////////////////
// BattleTarget class
////////////////////////////////////////////////////////////////////////////////

BattleTarget::BattleTarget() :
	_type(GLOBAL_TARGET_INVALID),
	_actor(nullptr),
	_party(nullptr)
{}



void BattleTarget::InvalidateTarget() {
	_type = GLOBAL_TARGET_INVALID;
	_actor = nullptr;
	_party = nullptr;
}



void BattleTarget::SetInitialTarget(BattleActor* user, GLOBAL_TARGET type) {
	InvalidateTarget();

	if (user == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr argument" << endl;
		return;
	}
	if ((type <= GLOBAL_TARGET_INVALID) || (type >= GLOBAL_TARGET_TOTAL)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid target type argument: " << type << endl;
		return;
	}

	// Determine what party the initial target will exist in
	deque<BattleActor*>* target_party;
	if ((type == GLOBAL_TARGET_ALLY) || (type == GLOBAL_TARGET_ALL_ALLIES)) {
		if (user->IsEnemy() == false)
			target_party = &BattleMode::CurrentInstance()->GetCharacterParty();
		else
			target_party = &BattleMode::CurrentInstance()->GetEnemyParty();
	}
	else if ((type == GLOBAL_TARGET_FOE) || (type == GLOBAL_TARGET_ALL_FOES)) {
		if (user->IsEnemy() == false)
			target_party = &BattleMode::CurrentInstance()->GetEnemyParty();
		else
			target_party = &BattleMode::CurrentInstance()->GetCharacterParty();
	}
	else {
		target_party = nullptr;
	}

	// Set the actor/party according to the target type
	switch (type) {
		case GLOBAL_TARGET_SELF:
			_actor = user;
			break;
		case GLOBAL_TARGET_ALLY:
		case GLOBAL_TARGET_FOE:
			_actor = target_party->at(0);
			break;
		case GLOBAL_TARGET_ALL_ALLIES:
		case GLOBAL_TARGET_ALL_FOES:
			_party = target_party;
			break;
		default:
			IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid type: " << type << endl;
			return;
	}

	_type = type;

	// If the target is not a party and not the user themselves, select the first valid actor
	if ((_actor != nullptr) && (_actor != user)) {
		if (IsValid() == false) {
			if (SelectNextActor(user, true, true) == false)
				IF_PRINT_WARNING(BATTLE_DEBUG) << "could not find an initial actor that was a valid target" << endl;
		}
	}
}



void BattleTarget::SetActorTarget(GLOBAL_TARGET type, BattleActor* actor) {
	if (IsTargetActor(type) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received invalid type argument: " << type << endl;
		return;
	}
	if (actor == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr argument" << endl;
		return;
	}

	_type = type;
	_actor = actor;
	_party = nullptr;
}



void BattleTarget::SetPartyTarget(GLOBAL_TARGET type, deque<BattleActor*>* party) {
	if (IsTargetParty(type) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received invalid type argument: " << type << endl;
		return;
	}
	if (party == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr argument" << endl;
		return;
	}

	_type = type;
	_actor = nullptr;
	_party = party;
}



bool BattleTarget::IsValid() {
	if (IsTargetActor(_type) == true) {
		if (_actor == nullptr)
			return false;
		else if (_actor->IsAlive() == false)
			return false;
		else
			return true;
	}
	else if (IsTargetParty(_type) == true) {
		if (_party == nullptr)
			return false;
		else
			return true;
	}
	else {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid target type: " << _type << endl;
		return false;
	}
}



bool BattleTarget::SelectNextActor(BattleActor* user, bool direction, bool valid_criteria) {
	if (user == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received nullptr argument" << endl;
		return false;
	}
	if (IsTargetActor(_type) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid target type: " << _type << endl;
		return false;
	}
	if (_actor == nullptr) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "no valid actor target" << endl;
		return false;
	}

	// ----- (1): Retrieve the proper party container that contains the actors we would like to select from
	deque<BattleActor*>* target_party = nullptr;
	if (_type == GLOBAL_TARGET_SELF) {
		return false; // Self type targets do not have multiple actors to select from
	}
	else if (_type == GLOBAL_TARGET_ALLY) {
		if (user->IsEnemy() == false)
			target_party = &BattleMode::CurrentInstance()->GetCharacterParty();
		else
			target_party = &BattleMode::CurrentInstance()->GetEnemyParty();
	}
	else if (_type == GLOBAL_TARGET_FOE) {
		if (user->IsEnemy() == false)
			target_party = &BattleMode::CurrentInstance()->GetEnemyParty();
		else
			target_party = &BattleMode::CurrentInstance()->GetCharacterParty();
	}
	else {
		// This should never be reached because the target type was already determined to be a point or actor above
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid target type: " << _type << endl;
		return false;
	}

	// ----- (2): Check the target party for early exit conditions
	if (target_party->empty() == true) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "actor target's party was empty" << endl;
		return false;
	}
	if (target_party->size() == 1) {
		return false; // No more actors to select from in the party
	}

	// ----- (3): Determine the index of the current actor in the target party
	uint32 original_target_index = 0xFFFFFFFF; // Initially set to an impossibly high index for error checking
	for (uint32 i = 0; i < target_party->size(); i++) {
		if (target_party->at(i) == _actor) {
			original_target_index = i;
			break;
		}
	}
	if (original_target_index == 0xFFFFFFFF) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "actor target was not found in party" << endl;
		return false;
	}

	// ----- (4): Starting from the index of the original actor, select the next available actor
	BattleActor* original_actor = _actor;
	uint32 new_target_index = original_target_index;
	while (true) {
		// Increment or decrement the target index based on the direction argument
		if (direction == true) {
			new_target_index = (new_target_index >= target_party->size() - 1) ? 0 : new_target_index + 1;
		}
		else {
			new_target_index = (new_target_index == 0) ? target_party->size() - 1 : new_target_index - 1;
		}

		// If we've reached the original target index then we were unable to select another actor target
		if (new_target_index == original_target_index) {
			_actor = original_actor;
			return false;
		}

		// Set the new actor target and if required, ascertain the new target's validity. If the new target
		// must be valid and this new actor is not, the loop will continue and will try again with the next actor
		_actor = target_party->at(new_target_index);
		if (valid_criteria == false) {
			return true;
		}
		else if (IsValid() == true){
			return true;
		}
	}
} // bool BattleTarget::SelectNextActor(BattleActor* user, bool direction, bool valid_criteria)



BattleActor* BattleTarget::GetPartyActor(uint32 index) {
	if (_party == nullptr) {
		return nullptr;
	}

	if (index >= _party->size()) {
		return nullptr;
	}

	return _party->at(index);
}



ustring BattleTarget::GetName() {
	if ((_type == GLOBAL_TARGET_SELF) || (_type == GLOBAL_TARGET_ALLY) || (_type == GLOBAL_TARGET_FOE)) {
		return _actor->GetName();
	}
	else if (_type == GLOBAL_TARGET_ALL_ALLIES) {
		return UTranslate("All Allies");
	}
	else if (_type == GLOBAL_TARGET_ALL_FOES) {
		return UTranslate("All Enemies");
	}

	return ustring();
}

////////////////////////////////////////////////////////////////////////////////
// BattleItem class
////////////////////////////////////////////////////////////////////////////////

BattleItem::BattleItem(hoa_global::GlobalItem item) :
	_item(item),
	_available_count(item.GetCount())
{
	if (item.GetID() == 0)
		IF_PRINT_WARNING(BATTLE_DEBUG) << "constructor received invalid item argument" << endl;
}



BattleItem::~BattleItem() {
	if (_available_count != _item.GetCount())
		IF_PRINT_WARNING(BATTLE_DEBUG) << "actual count was not equal to available count upon destruction" << endl;
}



void BattleItem::IncrementAvailableCount() {
	_available_count++;
	if (_available_count > _item.GetCount()) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "attempted to increment available count above actual count: " << _available_count << endl;
		_available_count--;
	}
}



void BattleItem::DecrementAvailableCount() {
	if (_available_count == 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "attempted to decrement available count below zero" << endl;
		return;
	}
	_available_count--;
}



void BattleItem::IncrementCount() {
	_item.IncrementCount();
	_available_count++;
}



void BattleItem::DecrementCount() {
	if (_item.GetCount() == 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "item count was zero when function was called" << endl;
		return;
	}

	_item.DecrementCount();

	if (_available_count > _item.GetCount()) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "available count was greater than actual count: " << _available_count  << endl;
		_available_count = _item.GetCount();
	}
}

} // namespace private_shop

} // namespace hoa_shop
