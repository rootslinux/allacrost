///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    menu_status.h
*** \author  Daniel Steuernol (Steu)
*** \author  Andy Gardner (ChopperDave)
*** \brief   Header file for status menu.
***
*** This code handles the character statistics menu.
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
*** \brief Represents the Status window, displaying all the information about the character.
***
*** This window display all the attributes of the character.
*** You can scroll through them all as well, to view all the different characters.
*** ***************************************************************************/
class StatusWindow : public hoa_gui::MenuWindow {
private:
    //! char portraits
    std::vector<hoa_video::StillImage> _full_portraits;

    //! if the window is active or not
    bool _char_select_active;

    //! character selection option box
    hoa_gui::OptionBox _char_select;

    /*!
    * \brief initialize character selection option box
    */
    void _InitCharSelect();

public:

    StatusWindow();
    ~StatusWindow();

    /*!
    * \brief render this window to the screen
    * \return success/failure
    */
    void Draw();

    /*!
    * \brief update function handles input to the window
    */
    void Update();

    /*!
    * \brief Check if status window is active
    * \return true if the window is active, false if it's not
    */
    inline bool IsActive() { return _char_select_active; }

    /*!
    * \brief Active this window
    * \param new_value true to activate window, false to deactivate window
    */
    void Activate(bool new_value);

}; // class StatusWindow : public hoa_video::MenuWindow

} // private_menu

} // hoa_menu
