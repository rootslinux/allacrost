////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_effects.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for battle actor effects.
***
*** This file contains the code that manages effects that influence an actor's
*** behavior and properties.
*** ***************************************************************************/

#pragma once

#include "defs.h"
#include "utils.h"

#include "script.h"
#include "system.h"

#include "global_effects.h"

namespace hoa_battle {

namespace private_battle {

/** ****************************************************************************
*** \brief Abstract base class representing an effect that is active on an actor
***
*** Battle effects change the dynamic of battles. There are two different types of
*** effects, both which inherit from this base class. Actor effects change the
*** state of common battle operations, such as an actor being able to defend an
*** ally from receiving damage for a short while. These effects may be the result
*** of using a skill,
***
*** Status effects change the state of a single actor and have a visible indicator
*** of the status on the screen. Status effects include things like attribute modifiers,
*** paralsysis, and so on.
*** ***************************************************************************/
class BattleEffect {
public:
	//! \param actor A pointer to the actor that the effect is active on
	BattleEffect(BattleActor* actor);

	virtual ~BattleEffect()
		{}

	//! \brief Updates the state of the effect as necessary
	virtual void Update() = 0;

	BattleActor* GetEffectActor() const
		{ return _effect_actor; }

	const hoa_utils::ustring& GetEffectName() const
		{ return _effect_name; }

protected:
	//! \brief Holds the translated name of the effect, if available
	hoa_utils::ustring _effect_name;

	//! \brief The actor that this effect is active upon
	BattleActor* _effect_actor;
};


/** ****************************************************************************
*** \brief Manages all data related to a single status effect in battle
***
*** A status effect is a special type of battle effect that has an intensity,
*** visual indicator, and gradually dissipates over time. An object of this class
*** represents an active status on a single actor. Status effects never apply to
*** more than one actor, although multiple actors may each have the same type of
*** status effect active.
***
*** Status effects only have positive intensity values and will naturally decrease
*** in intensity over time until they reach the neutral intensity level, upon which
*** they are removed. Some types of status effects have an opposite type. For example,
*** one status effect may increase the actor's strength while another decreases strength.
*** We do not allow these two statuses to co-exist on the same actor, thus the two have a
*** cancelation effect on each other and the stronger (more intense) effect will remain.
***
*** This class is abstract and thus can not be instantiated. It contains all common data and
*** functionality that other types of status effects utilize. In the Lua definition for the
*** status effect, there are three functions which may be optionally defined: Apply, Update,
*** and Remove. These functions are called during different points of the status effect's life
*** cycle.
*** ***************************************************************************/
class StatusEffect : public BattleEffect {
public:
	/** \param type The status type that this class object should represent
	*** \param intensity The intensity of the status
	*** \param actor A pointer to the actor affected by the status
	**/
	StatusEffect(hoa_global::GLOBAL_STATUS type, hoa_global::GLOBAL_INTENSITY intensity, BattleActor* actor);

	virtual ~StatusEffect();

	/** \brief Loads all common data for this status effect
	*** \param script_file A reference to the script containing the status effect data, already opened to the table for the effect
	*** \return True if all data was successfully loaded
	***
	*** The script does not close the open status table nor the file when it is finished. It is the responsibility of the caller
	*** to open the file and the appropriate table, then close the table and file after the call to this function returns.
	**/
	virtual bool Load(hoa_script::ReadScriptDescriptor& script_file);

	virtual void Update() = 0;

	/** \brief Increments the status effect intensity by a positive amount
	*** \param amount The number of intensity levels to increase the status effect by
	*** \return True if the intensity level was modified
	**/
	bool IncrementIntensity(uint8 amount);

	/** \brief Decrements the status effect intensity by a negative amount
	*** \param amount The number of intensity levels to decrement the status effect by
	*** \return True if the intensity level was modified
	*** \note Intensity will not be decremented below GLOBAL_INTENSITY_NEUTRAL
	**/
	bool DecrementIntensity(uint8 amount);

	//! \note This will cause the duration timer to reset
	void SetIntensity(hoa_global::GLOBAL_INTENSITY intensity);

	void ResetIntensityChanged()
		{ _intensity_changed = false; }

	//! \brief Returns true if the effect is no longer active because it has a neutral or invalid intensity
	bool IsEffectFinished() const
		{ return (_status_effect.GetIntensity() == hoa_global::GLOBAL_INTENSITY_NEUTRAL) || (_status_effect.GetIntensity() == hoa_global::GLOBAL_INTENSITY_INVALID); }

	/** \brief Calls the corresponding script function for the status effect
	*** \note If there is no corresponding function, no call will be made
	**/
	//@{
	void CallApplyFunction() const;

	void CallUpdateFunction() const;

	void CallRemoveFunction() const;
	//@}

	//! \brief Class Member Access Functions
	//@{
	hoa_global::GLOBAL_STATUS GetStatusType() const
		{ return _status_effect.GetType(); }

	hoa_global::GLOBAL_INTENSITY GetIntensity() const
		{ return _status_effect.GetIntensity(); }

	hoa_global::GLOBAL_STATUS GetOppositeStatusType() const
		{ return _opposite_status_type; }

	bool IsIntensityChanged() const
		{ return _intensity_changed; }

	//! \note Returns a pointer instead of a reference so that Lua functions can access the timer
	hoa_system::SystemTimer* GetDurationTimer()
		{ return &_duration_timer; }

	hoa_video::StillImage* GetIconImage() const
		{ return _icon_image; }
	//@}

protected:
	//! \brief The type and intensity of the status effect represented
	hoa_global::GlobalStatusEffect _status_effect;

	//! \brief The opposing status type for this effect, set to GLOBAL_STATUS_INVALID if no opposite status  exists
	hoa_global::GLOBAL_STATUS _opposite_status_type;

	//! \brief A flag set to true when the intensity value was changed and cleared when the Update method is called
	bool _intensity_changed;

	//! \brief A timer used to determine how long the status effect lasts
	hoa_system::SystemTimer _duration_timer;

	//! \brief A pointer to the icon image that represents the status. Set to nullptr if the status is invalid
	hoa_video::StillImage* _icon_image;

	//! \brief Called when the status effect is initially applied
	ScriptObject* _apply_function;

	//! \brief Called when the appropriate set of conditions occur (defined by the derived class) and require the status effect to make a change
	ScriptObject* _update_function;

	//! \brief Called when the status effect is removed
	ScriptObject* _remove_function;

	//! \brief If the timer finishes, decrements the intensity and processes changes accordingly
	void _UpdateDurationTimer();
}; // class StatusEffect : public BattleEffect


/** ****************************************************************************
*** \brief A status effect that makes a static change when it is applied or changes intensity
***
*** Static effects are the most basic type of status effects. Typically these status effects change
*** one or more attributes on an actor and changes in intensity change the amount that the attribute
*** is modified. However this effect has more uses as well, such as setting a temporary state for an
*** actor like paralysis.
***
*** Whenever the intensity changes for this type of effect, the _update_function is called.
*** ***************************************************************************/
class StaticStatusEffect : public StatusEffect {
public:
	/** \param type The status type that this class object should represent
	*** \param intensity The intensity of the status
	*** \param actor A pointer to the actor affected by the status
	**/
	StaticStatusEffect(hoa_global::GLOBAL_STATUS type, hoa_global::GLOBAL_INTENSITY intensity, BattleActor* actor) :
		StatusEffect(type, intensity, actor) {}

	~StaticStatusEffect()
		{}

	//! \brief Calls the _update_function when the intensity has been changed
	void Update();
};


/** ****************************************************************************
*** \brief A status effect that makes changes periodically while it is active
***
*** Periodic effects make a change to the actor repeatedly over time. Every periodic effect
*** has a separate timer with it that determines the amount of time in between updates that
*** are made. For example, a poison effect reduces the actor's HP by some amount every few
*** seconds.
***
*** In the Lua script, a period time is defined that is used to determine how often the
*** _update_function is invoked. This time should always be less than the _duration_timer,
*** otherwise the effect may never make any change while it is active.
*** ***************************************************************************/
class PeriodicStatusEffect : public StatusEffect {
public:
	/** \param type The status type that this class object should represent
	*** \param intensity The intensity of the status
	*** \param actor A pointer to the actor affected by the status
	**/
	PeriodicStatusEffect(hoa_global::GLOBAL_STATUS type, hoa_global::GLOBAL_INTENSITY intensity, BattleActor* actor) :
		StatusEffect(type, intensity, actor), _period_timer(0) {}

	~PeriodicStatusEffect()
		{}

	//! \brief Calls the function in the base class and additionally reads a period time from the script data
	bool Load(hoa_script::ReadScriptDescriptor& script_file);

	//! \brief Calls the _update_function when the intensity has been changed
	void Update();

protected:
	//! \brief Used to determine when the _update_function is called
	hoa_system::SystemTimer _period_timer;
};


/** ****************************************************************************
*** \brief Manages all elemental and status elements for an actor
***
*** The class contains all of the active effects on an actor. These effects are
*** updated regularly by this class and are removed when their timers expire or their
*** intensity status is nullified by an external call. This class performs all the
*** calls to the Lua script functions (Apply/Update/Remove) for each status effect at
*** the appropriate time. The class also contains a draw function which will display
*** icons for all the active status effects of an actor to the screen.
***
*** \todo The Draw function probably should be renamed to something more specific
*** and should check whether or not the actor is a character. Its intended to be
*** used only for character actors to draw on the bottom menu. There should also
*** probably be another draw function for drawing the status of an actor to the
*** command window.
***
*** \todo Elemental effects are not yet implemented or supported by this class.
*** They should be added when elemental effects in battle are ready to be used.
*** ***************************************************************************/
class EffectsSupervisor {
public:
	//! \param actor A valid pointer to the actor object that this class is responsible for
	EffectsSupervisor(BattleActor* actor);

	~EffectsSupervisor();

	//! \brief Updates the timers and state of any effects
	void Update();

	//! \brief Draws the element and status effect icons to the bottom status menu
	void Draw();

	/** \brief Returns true if the requested status is active on the managed actor
	*** \param status The type of status to check for
	**/
	bool IsStatusActive(hoa_global::GLOBAL_STATUS status)
		{ return (_active_status_effects.find(status) != _active_status_effects.end()); }

	/** \brief Reurns true if the opposite status to that of the argument is active
	*** \param status The type of opposite status to check for
	**/
	bool IsOppositeStatusActive(hoa_global::GLOBAL_STATUS status);

	/** \brief Populates the argument vector with all status effects that are active on the actor
	*** \param all_status_effects A reference to the data vector to populate
	**/
	void GetAllStatusEffects(std::vector<hoa_global::GLOBAL_STATUS>& all_status_effects);

	/** \brief Immediately removes all active status effects from the actor
	*** \note This function is typically used in the case of an actor's death. Because it returns no value, indicator icons
	*** illustrating the removal of status effects can not be shown, as the indicators need to know which status effects were
	*** active and at what intensity before they were removed. If you wish to remove all status while displaying indicators,
	*** use a combination of GetActiveStatusEffects() and repeated calls to ChangeStatus() for each effect.
	**/
	void RemoveAllStatus();

	/** \brief Changes the intensity level of a status effect
	*** \param status The status effect type to change
	*** \param intensity The amount of intensity to increase or decrease the status effect by
	*** \param old_status A reference to hold the previous status type as a result of this operation
	*** \param old_intensity A reference to hold the previous intensity of the previous status
	*** \param new_status A reference to the new status as a result of the change
	*** \param new_intensity A reference to new intensity fo the new status
	*** \return True if a change in status took place
	***
	*** Primary function for performing status changes on an actor. Depending upon the current state of the actor and
	*** the first two status and intensity arguments, this function may add new status effects, remove existing effects,
	*** or modify the intensity of existing effects. This function also takes into account status effects which have an
	*** opposite type (e.g., strength gain status versus strength depletion status) and change the state of both effects
	*** accordingly. So, for example, a single call to this function could remove an old effect -and- add a new effect, if
	*** the effect to be added has an opposite effect that is currently active.
	***
	*** \note The old/new status/intensity arguments are used to store additional return values, so don't pass references to
	*** variables that have data you wish to retain. If the function returns false, the value of these members is meaningless
	*** and should be disregarded.
	***
	*** \note To be absolutely certain that a particular status effect is removed from the actor regardless of its current
	*** intensity, use the value GLOBAL_INTENSITY_NEG_EXTREME for the intensity argument.
	***
	*** \note This function only changes the state of the status and does <i>not</i> display any visual or other indicator
	*** to the player that the status was modified. Typically you should invoke BattleActor::RegisterStatusChange(...)
	*** when you want to change the status of the actor. That method will call this one as well as activating the proper
	*** indicator based on the return values from this function
	**/
	bool ChangeStatus(hoa_global::GLOBAL_STATUS status, hoa_global::GLOBAL_INTENSITY intensity,
		hoa_global::GLOBAL_STATUS& previous_status, hoa_global::GLOBAL_INTENSITY& previous_intensity,
		hoa_global::GLOBAL_STATUS& new_status, hoa_global::GLOBAL_INTENSITY& new_intensity
	);

private:
	//! \brief A pointer to the actor that this class supervises effects for
	BattleActor* _actor;

	// TODO: support for elemental effects may be added here at a later time
//	//! \brief Contains all active element effects
// 	std::map<hoa_global::GLOBAL_ELEMENTAL, BattleElementEffect*> _element_effects;

	//! \brief Contains all active status effects
	std::map<hoa_global::GLOBAL_STATUS, StatusEffect*> _active_status_effects;

	/** \brief Creates a new status effect and applies it to the actor
	*** \param status The type of the status to create
	*** \param intensity The intensity level that the effect should be initialized at
	***
	*** \note This method does not check if the requested status effect already exists or not in the map of active effects.
	*** Do not call this method unless you are certain that the given status is not already active on the actor, otherwise
	*** memory leaks and other problems may arise.
	**/
	void _CreateNewStatus(hoa_global::GLOBAL_STATUS status, hoa_global::GLOBAL_INTENSITY intensity);

	/** \brief Removes an existing status effect from the actor
	*** \param status_effect A pointer to the status effect to be removed
	*** \note After this function completes, if it was successful, the object pointed to by the status_effect argument will
	*** be invalid and should not be used. It is good practice for the caller to set the pointer passed in to this function to
	*** nullptr immediately after the function call returns.
	**/
	void _RemoveStatus(StatusEffect* status_effect);
}; // class EffectsSupervisor

} // namespace private_battle

} // namespace hoa_battle
