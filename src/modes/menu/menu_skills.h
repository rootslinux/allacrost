///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    menu_skills.h
*** \author  Daniel Steuernol (Steu)
*** \author  Andy Gardner (ChopperDave)
*** \brief   Header file for skill menu.
***
*** This code handles the skills menu.
*** ***************************************************************************/

#pragma once

#include <string>
#include <vector>

#include "utils.h"
#include "defs.h"

#include "video.h"
#include "gui.h"

#include "global.h"

namespace hoa_menu {

namespace private_menu {

/** ****************************************************************************
*** \brief Represents the Skills window, displaying all the skills for the character.
***
*** This window display all the skills for a particular character.
*** You can scroll through them all, filter by category, choose one, and apply it
*** to a character.
*** ***************************************************************************/
class SkillsWindow : public hoa_gui::MenuWindow {
    friend class hoa_menu::MenuMode;

public:
    SkillsWindow();

    ~SkillsWindow()
    {}

    /*!
    * \brief Updates key presses and window states
    */
    void Update();

    /*!
    * \brief Draws the windows and option boxes
    * \return success/failure
    */
    void Draw();

    /*!
    * \brief Activates the window
    * \param new_value true to activate window, false to deactivate window
    */
    void Activate(bool new_status);

    /*!
    * \brief Checks to see if the skills window is active
    * \return true if the window is active, false if it's not
    */
    bool IsActive()
    {
        return _active_box;
    }

private:
    //! Flag to specify the active option box
    uint32 _active_box;

    //! The character select option box
    hoa_gui::OptionBox _char_select;

    //! The skills categories option box
    hoa_gui::OptionBox _skills_categories;

    //! The skills list option box
    hoa_gui::OptionBox _skills_list;

    //! The skill SP cost option box
    hoa_gui::OptionBox _skill_cost_list;

    //! TextBox that holds the selected skill's description
    hoa_gui::TextBox _description;

    //! Track which character's skillset was chosen
    int32 _char_skillset;

    /*!
    * \brief Initializes the skills category chooser
    */
    void _InitSkillsCategories();

    /*!
    * \brief Initializes the skills chooser
    */
    void _InitSkillsList();

    /*!
    * \brief Initializes the character selector
    */
    void _InitCharSelect();

    //! \brief Returns the currently selected skill
    hoa_global::GlobalSkill *_GetCurrentSkill();

    /*!
    * \brief Sets up the skills that comprise the different categories
    */
    void _UpdateSkillList();

    hoa_utils::ustring _BuildSkillListText(const hoa_global::GlobalSkill * skill);

    //! \brief parses the 3 skill lists of the global character and sorts them according to use (menu/battle)
    void _BuildMenuBattleSkillLists(std::vector<hoa_global::GlobalSkill *> *skill_list,
        std::vector<hoa_global::GlobalSkill *> *field, std::vector<hoa_global::GlobalSkill *> *battle,
        std::vector<hoa_global::GlobalSkill *> *all);

}; //class SkillsWindow : public hoa_video::MenuWindow

} // private_menu

} // hoa_menu
