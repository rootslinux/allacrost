///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_treasure.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for map mode treasures.
*** *****************************************************************************/

#pragma once

// Allacrost utilities
#include "utils.h"
#include "defs.h"

// Allacrost engines
#include "video.h"

// Allacrost common code
#include "gui.h"

// Local map mode headers
#include "map_utils.h"
#include "map_objects.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief A container class for treasures procured by the player
***
*** Treasures may contain multiple quantities and types of items, weapons, armor,
*** or any other type of global object. They may additionally contain any amount
*** of drunes (money). As one would expect, the contents of a treasure should only
*** be retrieved by the player one time. This class holds a member for tracking whether
*** the treasure has been taken or not, but it is not responsible for determining
*** if a treasure was taken in the past (by a previous visit to the map or from the
*** saved game file).
*** ***************************************************************************/
class TreasureContainer {
	friend class TreasureSupervisor;

public:
	TreasureContainer();

	~TreasureContainer();

	/** \brief Adds a number of drunes to be the chest's contents
	*** \note The overflow condition is not checked here: we just assume it will never occur
	**/
	void AddDrunes(uint32 amount)
		{ _drunes += amount; }

	/** \brief Adds an object to the contents of the TreasureContainer
	*** \param id The id of the GlobalObject to add
	*** \param quantity The number of the object to add (default == 1)
	*** \return True if the object was added succesfully
	**/
	bool AddObject(uint32 id, uint32 quantity = 1);

	//! \name Class member access methods
	//@{
	bool IsTaken() const
		{ return _taken; }

	void SetTaken(bool taken)
		{ _taken = taken; }
	//@}

private:
	//! \brief Set to true when the contents of the treasure have been added to the player's inventory
	bool _taken;

	//! \brief The number of drunes contained in the chest
	uint32 _drunes;

	//! \brief The list of objects given to the player upon opening the treasure
	std::vector<hoa_global::GlobalObject*> _objects_list;
}; // class TreasureContainer


/** ****************************************************************************
*** \brief Represents an obtainable treasure on the map which the player may access
***
*** A treasure is a specific type of physical object, usually in the form of a
*** treasure chest. When the player accesses these treasures, the chest animates as
*** it is being opened and the treasure supervisor is initialized once the opening
*** animation is complete. Each treasure object on a map has a global event associated
*** with it to determine whether the treasure contents have already been retrieved by
*** the player.
***
*** Image files for treasures are single row multi images where the frame ordering
*** goes from closed, to opening, to open. This means each map treasure has exactly
*** three animations. The closed and open animations are usually single frame images.
***
*** To add contents to the treasure for this object, you will need to retreive the
*** pointer to the TreasureContainer object via the GetTreasure() method, then add drunes
*** and/or objects (items/equipment/etc) to the TreasureContainer.
***
*** \todo Add support for more treasure features, such as locked chests, chests which
*** trigger a battle, etc.
*** ***************************************************************************/
class MapTreasure : public PhysicalObject {
	//! \brief Constants representing the three types of animations for the treasure
	enum {
		TREASURE_CLOSED_ANIM   = 0,
		TREASURE_OPENING_ANIM  = 1,
		TREASURE_OPEN_ANIM     = 2
	};

public:
	/** \param image_file The name of the multi image file to load for the treasure
	*** \param num_total_frames The total number of frame images in the multi image file
	*** \param num_closed_frames The number of frames to use as the closed animation (default value == 1)
	*** \param num_open_frames The number of frames to use as the open animation (default value == 1)
	*** \note The opening animation will be created based on the total number of frames in the image file
	*** subtracted by the number of closed and open frames. If this value is zero, then the opening animation
	*** will simply be the same as the open animation
	**/
	MapTreasure(std::string image_file, uint8 num_total_frames, uint8 num_closed_frames = 1, uint8 num_open_frames = 1);

	~MapTreasure()
		{}

	//! \brief Returns the string that should be used in the record to determine if the treasure was acquired
	std::string GetRecordName() const
		{ return ("treasure_" + hoa_utils::NumberToString(GetObjectID())); }

	//! \brief Retrieves a pointer to the TreasureContainer object holding the treasure. Always guanateed to be non-nullptr.
	TreasureContainer* GetTreasureContainer()
		{ return &_treasure_container; }

	//! \brief Loads the state of the chest from the global event corresponding to the current map
	void LoadState();

	//! \brief Opens the treasure, which changes the active animation and initializes the treasure supervisor when the opening animation finishes.
	void Open();

	//! \brief Changes the current animation if it has finished looping
	void Update();

private:
	//! \brief Stores the contents of the treasure which will be processed by the treasure supervisor
	TreasureContainer _treasure_container;
}; // class MapTreasure : public PhysicalObject


/** ****************************************************************************
*** \brief Represents hidden treasures on the map that appear occasionally as a brief glimmer
***
*** Glimmer treasures can contain the same contents as MapTreasures, but take a slightly different
*** role. Instead of an open/opening/closed image animations, a glimmer treasure has only one animation,
*** a brief "glimmer" that is displayed only occasionally, to make them harder for the player to find.
*** Once a glimmer treasure is obtained, the animation will no longer display. The animation does not
*** need to be actively displaying for the player to acquire the treasure.
***
*** All glimmer animation image files are stored in a format of 32x32 pixels for each frame, so we can
*** compute each frame automaticaly from the image size. The amount of time to wait between glimmer display
*** animations is variable based on a Gaussian curve, so that multiple visible glimmer animations never appear
*** in-sync. If the standard deviation is left unspecified, then a value of 5% of the average delay will be used.
***
*** \note If you desire custom behavior for when the glimmer occurs (stepping on a switch, for example), then you
*** should SetDisplayEnabled(false) to turn off the normal behavior, and can then call ForceDisplay() whenever you
*** determine that you want the animation to be displayed
*** ***************************************************************************/
class GlimmerTreasure : public PhysicalObject {
public:
	//! \brief Constants representing standard display delays, where more rare treasures display less frequently
	//@{
	static const uint32 GLIMMER_WAIT_COMMON;
	static const uint32 GLIMMER_WAIT_UNCOMMON;
	static const uint32 GLIMMER_WAIT_RARE;
	//@}

	//! \brief The animation image used by default if no image file is specified
	static const std::string DEFAULT_IMAGE_FILE;

	//! \brief The number of milliseconds to wait between each frame display on average
	static const uint32 DEFAULT_FRAME_TIME;

	//! \brief This multiplier is applied to the average and the result is used as the standard deviation, when one is not explicitly provided
	static const float DEFAULT_DEVIATION_MULTIPLIER;

	//! \note Uses the default glimmer animation and frame timing when no image is supplied
	GlimmerTreasure() :
		GlimmerTreasure(DEFAULT_IMAGE_FILE, DEFAULT_FRAME_TIME, GLIMMER_WAIT_COMMON) {}

	/** \param image_file The name of the multi image file to load for the glimmer animation
	*** \param frame_time The number of milliseconds to display each frame in the animation
	*** \param average_wait The number of milliseconds to wait between each display of the animation
	**/
	GlimmerTreasure(std::string image_file, uint32 frame_time, uint32 average_wait);

	~GlimmerTreasure()
		{}

	//! \brief Returns the string that should be used in the record to determine if the treasure was acquired
	std::string GetRecordName() const
		{ return ("treasure_" + hoa_utils::NumberToString(GetObjectID())); }

	//! \brief Retrieves a pointer to the TreasureContainer object holding the treasure. Always guanateed to be non-nullptr.
	TreasureContainer* GetTreasureContainer()
		{ return &_treasure_container; }

	/** \brief Sets the amount of time to wait between glimmer animations
	*** \param average The average number of milliseconds to wait
	*** \note Uses the default standard deviation of 5% of the average value
	**/
	void SetDisplayDelay(uint32 average)
		{ SetDisplayDelay(average, (average * DEFAULT_DEVIATION_MULTIPLIER)); }

	/** \brief Sets the amount of time to wait between glimmer animations
	*** \param average The average number of milliseconds to wait between animations
	*** \param standard_deviation The standard deviation to use for the gaussiance variance
	***
	*** For example, an average of 4000ms and a standard deviation of 200ms will yield a time between
	*** 3400-4600 99.7% of the time (3 standard deviations)
	**/
	void SetDisplayDelay(uint32 average, float standard_deviation);

	/** \brief Used to enable or disable any display animation.
	*** \param enable If false, the animation will not display. Otherwise the animation displays normally
	*** \note This will reset the animation timer and cease the display of the animation if it is currently active
	**/
	void SetDisplayEnabled(bool enable);

	/** \brief If called, immediately begins a single display of the animation from the start
	*** \note This will work even if the display is currently disabled
	*** \note If the animation is currently displaying when this is called, the animation will restart from the first frame
	*** \note The animation display will *not* be forced if the treasure has already been acquired
	**/
	void ForceDisplay();

	//! \brief Acquires the contents of the treasure, causing the treasure supervisor to become active and the animation to cease
	void Acquire();

	//! \brief Updates the display timer
	void Update();

	//! \brief Draws the animation once if the display timer is expired
	void Draw();

private:
	//! \brief Stores the contents of the treasure which will be processed by the treasure supervisor
	TreasureContainer _treasure_container;

	//! \brief The number of milliseconds to wait on average between displays of the glimmer animation
	uint32 _average_wait;

	//! \brief The number of milliseconds for the standard deviation of the wait time
	float _standard_deviation_wait;

	//! \brief Setting to false will effectively prevent the animation from being displayed
	bool _display_enabled;

	//! \brief Set to true while the display is being forced explicitly by the user
	bool _display_forced;

	//! \brief This timer runs in between animation displays to wait for the specified period of time
	hoa_system::SystemTimer _wait_timer;

	//! \brief Called to reset the wait timer and determine a new wait time. This will instantly end the display of any non-forced animation
	void _ResetWaitTimer();
}; // class GlimmerTreasure : public PhysicalObject


/** ***************************************************************************************
*** \brief Displays the contents of a discovered treasure in a menu window
***
*** Upon opening a treasure chest or other treasure-containing map object, this menu
*** will appear and list the amount of drunes found (if any), a list of the icon and name of
*** each GlobalObject found (items, equipment, etc), and a list of player options.
*** The player may select to view detailed information about a particular entry, go to menu mode,
*** and possibly other actions in the future.
***
*** The treasure menu is composed of three sets of windows. The action window is a small window
*** at the top of the menu that displays the action options in a horizontal list. The list window
*** displays the contents of the treasure in a larger window below the action window. This object
*** list is formatted vertically. The detail window shares the same area as the list window and
*** displays textual and visual detail about an object selected by the user from the list window.
***
*** Proper utilization of this class entails the following steps:
***
*** -# Call the Initialize method to show the menu with the treasure that has been obtained
*** -# Call the Update method to process user input and update the menu's appearance
*** -# Call the Draw method to draw the menu to the screen
*** -# Call the Finish method to hide the menu and add the treasure's contents to the player's
***    inventory
***
*** \todo Allow the player to use or equip selected treasure objects directly from the
*** action menu.
***
*** \todo Add visual scissoring to the list window so that the option list or detail text does
*** not exceed the window boundary when the text or list is exceedingly long.
***
*** \todo Instead of forcing the detail window to share the list window, maybe it would look
*** better if there was a separate detail window which "popped out" of the other two windows
*** and could be placed over them when it was visible? I think this would be much more visually
*** pleasing than the current implementation.
*** **************************************************************************************/
class TreasureSupervisor {
public:
	//! \brief The possible sub-windows that are selected, used for determining how to process user input
	enum SELECTION {
		ACTION_SELECTED = 0, //!< the list of actions a user may take in the treasure menu
		LIST_SELECTED = 1,   //!< active when the user is browsing the list of treasures
		DETAIL_SELECTED = 2  //!< set when the user is viewing details about a particular treasure
	};

	TreasureSupervisor();

	~TreasureSupervisor();

	/** \brief Displays the menu window and initializes it to display the contents of a new treasure
	*** \param map_object A pointer to the object on the map holding the treasure to procure
	**/
	void Initialize(MapTreasure* map_object);

	/** \brief Displays the menu window and initializes it to display the contents of a new treasure
	*** \param treasure A pointer to the treasure to display the contents of
	**/
	void Initialize(TreasureContainer* treasure);

	//! \brief Processes input events from the user and updates the showing/hiding progress of the window
	void Update();

	/** \brief Draws the window to the screen
	*** \note If the Initialize method has not been called with a valid treasure pointer beforehand, this
	*** method will print a warning and it will not draw anything to the screen.
	**/
	void Draw();

	//! \brief Hides the window and adds the treasure's contents to the player's inventory
	void Finish();

	//! \brief Returns true if the treasure menu is active
	bool IsActive() const
		{ return (_treasure != nullptr); }

private:
	//! \brief A pointer to the treasure object to display the contents of
	TreasureContainer* _treasure;

	//! \brief The currently selected sub-window for processing user input
	SELECTION _selection;

	//! \brief A vector containing pointers to objects which should be deleted upon finishing with the current treasure
	std::vector<hoa_global::GlobalObject*> _objects_to_delete;

	//! \brief Contains options for viewing, using, or equipping inventory, or for exiting the menu
	hoa_gui::MenuWindow _action_window;

	//! \brief Lists all of the drunes and inventory objects contained in the treasure
	hoa_gui::MenuWindow _list_window;

	//! \brief The available actions that a user can currently take. Displayed in the _action_window.
	hoa_gui::OptionBox _action_options;

	//! \brief The name + quantity of all drunes and inventory objects earned. Displayed in the _list_window
	hoa_gui::OptionBox _list_options;

	//! \brief A textbox that displays the detailed description about a selected treasure
	hoa_gui::TextBox _detail_textbox;

	//! \brief A rendering of the name for the treasure window
	hoa_video::TextImage _window_title;

	//! \brief The name of the selected list item
	hoa_video::TextImage _selection_name;

	//! \brief A pointer to the image of the selected list item
	hoa_video::StillImage* _selection_icon;

	//! \brief Holds the icon image that represent drunes
	hoa_video::StillImage _drunes_icon;

	// ---------- Private methods

	//! \brief Processes user input when the action sub-window is selected
	void _UpdateAction();

	//! \brief Processes user input when the list sub-window is selected
	void _UpdateList();

	//! \brief Processes user input when the detailed view of a treasure object is selected
	void _UpdateDetail();
}; // class TreasureSupervisor

} // namespace private_map

} // namespace hoa_map
