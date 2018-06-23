///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    menu_formation.h
*** \author  Daniel Steuernol (Steu)
*** \author  Andy Gardner (ChopperDave)
*** \brief   Header file for formation menu.
***
*** This code handles the formations menu.
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
*** \brief Represents the Formation window, allowing the party to change order.
***
*** This window changes party order.
*** ***************************************************************************/
class FormationWindow : public hoa_gui::MenuWindow {
    friend class hoa_menu::MenuMode;

public:
    FormationWindow();
    ~FormationWindow();
    void Update();
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

    //! The character select option box once first character has been selected
    hoa_gui::OptionBox _second_char_select;

    /*!
    * \brief initialize character selection option box
    */
    void _InitCharSelect();

}; // class FormationWindow : public hoa_video::MenuWindow

} // private_menu

} // hoa_menu
