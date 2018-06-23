///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    menu_character.h
*** \author  Daniel Steuernol (Steu)
*** \author  Andy Gardner (ChopperDave)
*** \brief   Header file for character menu.
***
*** This code handles the character menu.
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
*** \brief Represents an individual character window
***
*** There should be one of these windows for each character in the game.
*** It will contain all the information of the character and handle its draw
*** placement.
*** ***************************************************************************/
class CharacterWindow : public hoa_gui::MenuWindow {
private:
    //! The name of the character that this window corresponds) to
    uint32 _char_id;

    //! The image of the character
    hoa_video::StillImage _portrait;

public:
    CharacterWindow();

    ~CharacterWindow();

    /** \brief Set the character for this window
    *** \param character the character to associate with this window
    **/
    void SetCharacter(hoa_global::GlobalCharacter *character);

    /** \brief render this window to the screen
    *** \return success/failure
    **/
    void Draw();
}; // class CharacterWindow : public hoa_video::MenuWindow

} // private_menu

} // hoa_menu
