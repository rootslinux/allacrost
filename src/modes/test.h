///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    test.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for test game mode
*** **************************************************************************/

#pragma once

#include "defs.h"
#include "utils.h"

#include "mode_manager.h"
#include "gui.h"

/** \brief Namespace containing code used only for testing purposes
*** \note Normally no other code should need to use this namespace.
**/
namespace hoa_test {

//! \brief Determines whether the code in the hoa_test namespace should print debug statements or not.
extern bool TEST_DEBUG;



namespace private_test {

//! \brief Used to define an invalid test identifier
const uint32 INVALID_TEST = 0;

//! \brief The path and name of the Lua file where the test directory list is stored
const std::string TEST_MAIN_FILENAME = "lua/test/test_main.lua";

/** ****************************************************************************
*** \brief A container class to hold data about a related set of tests
***
*** This container is populated with data read from two Lua files. The main test
*** file contains the test category name, category description, min/max test IDs,
*** and the test filename. The file for the test is then used to read the
*** test ids, test names, and test descriptions.
*** ***************************************************************************/
class TestData {
public:
	//! \brief The name of the category that will displayed in the test menu
	hoa_utils::ustring category_name;

	//! \brief The text that describes the category
	hoa_utils::ustring category_description;

	//! \brief Defines the range of possible test ID numbers
	uint32 minimum_test_id, maximum_test_id;

	//! \brief The name of the Lua file that contains the code to execute all of the tests
	std::string test_filename;

	//! \brief Holds all of the IDs for the test in question
	std::vector<uint32> test_ids;

	//! \brief The names of all tests contained within this category
	std::vector<hoa_utils::ustring> test_names;

	//! \brief The descriptions for all tests contained within this category
	std::vector<hoa_utils::ustring> test_descriptions;
}; // class TestData

} // namespace private_test


/** ****************************************************************************
*** \brief A game mode used for debugging and testing purposes
***
*** This is a game mode that players will not encounter during the game. The mode manages
*** a simple GUI interface that lists all of the available tests that can be run and allows
*** the user to select from among those tests. The available tests are defined in TEST_MAIN_FILENAME.
***
*** The way to activate test mode is either through running the program executable with the -t/--test option,
*** or through the Ctrl+T meta key when the game is in BootMode. When starting TestMode via the command-line,
*** the user may optionally include a test ID number to immediately begin running a specific test. Whenever an
*** instance of TestMode exists on the game stack, the Ctrl+T command will clear the game stack of any other modes
*** and return TestMode to be the active game mode. Ctrl+T will otherwise be ignored if the active game mode is
*** not BootMode or no TestMode instance is found on the game stack.
***
*** Once in TestMode, the GUI will display three windows. The vertical window on the left side lists all of the
*** test categories. The vertical window on the right side lists all of the available tests for the selected category.
*** And the horizontal window on the bottom of the screen is used to display information text about the selected
*** category or test.
*** ***************************************************************************/
class TestMode : public hoa_mode_manager::GameMode {
public:
	TestMode() : TestMode(private_test::INVALID_TEST) {}

	/** \brief Creates a TestMode instance and immediately begins the specified test
	*** \param test_id The id number of the test to begin executing immediately once this instance becomes the active game mode
	*** \note If the test_number is invalid, a warning will be printed and TestMode will run as normal
	**/
	TestMode(uint32 test_number);

	~TestMode();

	//! \brief Sets the descriptions of the possible test command inputs
	void SetCommandDescriptions();

	//! \brief Resets appropriate class members. Called whenever TestMode is made the active game mode
	void Reset();

	//! \brief Updates the GUI objects and processes user input
	void Update();

	//! \brief Draws the GUI objects to the screen
	void Draw();

	/** \brief Specifies a test to immediately begin the next time that TestMode is made the active game mode (ie when the Reset() method is invoked)
	*** \param id The ID of the test to execute
	***
	*** If the id does not point to an invalid test, a warning message will be issued and no test will be executed. This method is bound to Lua to make
	*** it easy to chain tests together. When a test makes this call, the next test will be issued whenever TestMode becomes active again.
	***
	*** \note Exercise caution when calling this function in your tests. If you create a loop of tests (A -> B, B -> C, C -> A, ...) then you will be unable
	*** to return to the TestMode interface and you will have to quit the application to get out of the infinite loop.
	**/
	void SetImmediateTestID(uint32 id)
		{ _immediate_test_id = id; }

private:
	//! \brief Defines the places where the user input may be focused
	typedef enum {
		SELECTING_CATEGORY,
		SELECTING_TEST
	} UserFocus;

	//! \brief Holds the ID number of the test to execute immediately if TestMode was created and told to immediately run a specific test
	uint32 _immediate_test_id;

	//! \brief Where the user focus is currently at, used to update the mode state appropriately
	UserFocus _user_focus;

	//! \brief Contains all of the data that will be displayed in the TestMode GUI. Each element represents one category of test data
	std::vector<private_test::TestData> _test_data;

	// ---------- GUI Objects

	//! \brief Used to display information in the test window when a test category contains no tests
	hoa_video::TextImage _missing_tests_text;

	//! \brief Vertical window on the left side of the screen. Used to display the _category_list OptionBox
	hoa_gui::MenuWindow _category_window;

	//! \brief Vertical window on the right side of the screen. Used to display the _test_list OptionBox
	hoa_gui::MenuWindow _test_window;

	//! \brief Horizontal window on the bottom of the screen. Used to display the _description_text TextBox
	hoa_gui::MenuWindow _description_window;

	//! \brief The list of selectabled quit options presented to the user while the mode is in the quit state
	hoa_gui::OptionBox _category_list;

	//! \brief The lists of available tests for each test category
	std::vector<hoa_gui::OptionBox*> _all_test_lists;

	//! \brief A pointer to the list inside _all_test_lists that represent the selected category
	hoa_gui::OptionBox* _test_list;

	//! \brief Holds the descriptive text of the highlighted test category or test
	hoa_gui::TextBox _description_text;

	// ---------- Methods

	//! \brief Defines the static properties of the various GUI objects
	void _Initialize();

	//! \brief Clears out and reloads all test data
	void _ReloadTestData();

	/** \brief Checks each test ID and test ID range for any potential problems
	***
	*** This is called at the end of _ReloadTestData as a means to ensure the integrity of that data. The function checks for three things.
	*** First, it ensures that the test ID ranges for each test category do not overlap. Second, it checks to see that each defined test
	*** ID lies within the valid range of it's category. And finally, it makes sure that all test IDs are unique among the entire set of
	*** tests. Warning messages will be printed to the console if these checks detect any issues. But if TEST_DEBUG is not set to true,
	*** the function will do nothing since it's only output is printing debug messages.
	**/
	void _CheckForInvalidTestID();

	/** \brief Runs the Lua function to execute the test that is identified by _test_number
	*** \param request_id Optional argument to specify the ID number of a specific test to run. By default this is set to an invalid test number
	***
	*** When a test nuumber is defined, this function finds the appropriate test data and updates the GUI lists so that the specified test
	*** is selected. Otherwise, the currently selected category and test are used to begin executing a test. Depending on the test, a new
	*** game mode may be pushed on to the stack, removing TestMode as the active game mode. Usually a test will not destroy the TestMode
	*** instance by popping it off the stack, however, making it simple to return to.
	***
	*** \note All global game data is cleared prior to beginning a test to help ensure that the test behavior remains repeatable
	**/
	void _ExecuteTest(uint32 request_id = private_test::INVALID_TEST);

	//! \brief Clears and updates the description text to reflect the currently selected test or test category
	void _SetDescriptionText();
}; // class TestMode : public hoa_mode_manager::GameMode

} // namespace hoa_test
