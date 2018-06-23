///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    menu_equip.h
*** \author  Daniel Steuernol (Steu)
*** \author  Andy Gardner (ChopperDave)
*** \brief   Header file for equip menu.
***
*** This code handles the equipment menu.
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
*** \brief Represents the Equipment window, allowing the player to change equipment.
***
*** This window changes a character's equipment.
*** You can choose a piece of equipment and replace with an item from the given list.
*** ***************************************************************************/
class EquipWindow : public hoa_gui::MenuWindow {
    friend class hoa_menu::MenuMode;

public:
    EquipWindow();
    ~EquipWindow();

    /*!
    * \brief Draws window
    * \return success/failure
    */
    void Draw();

    /*!
    * \brief Performs updates
    */
    void Update();

    /*!
    * \brief Checks to see if the equipment window is active
    * \return true if the window is active, false if it's not
    */
    bool IsActive()
    {
        return _active_box;
    }

    /*!
    * \brief Activates the window
    * \param new_value true to activate window, false to deactivate window
    */
    void Activate(bool new_status);

    /*!
    * \brief Sets Remove Mode
    * \param remove_mode true to activate Remove Mode
    */
    void SetRemoveMode(bool remove_mode);

private:

    //! Character selector
    hoa_gui::OptionBox _char_select;

    //! Equipment selector
    hoa_gui::OptionBox _equip_select;

    //! Replacement selector
    hoa_gui::OptionBox _equip_list;

    //! Flag to specify the active option box
    uint32 _active_box;

    //! equipment images
    std::vector<hoa_video::StillImage> _equip_images;

    //! True if equipment should just be removed and not replaced
    bool _remove_mode;

    /*!
    * \brief Set up char selector
    */
    void _InitCharSelect();

    /*!
    * \brief Set up equipment selector
    */
    void _InitEquipmentSelect();

    /*!
    * \brief Set up replacement selector
    */
    void _InitEquipmentList();

    /*!
    * \brief Updates the equipment list
    */
    void _UpdateEquipList();

}; // class EquipWindow : public hoa_video::MenuWindow

} // private_menu

} // hoa_menu
