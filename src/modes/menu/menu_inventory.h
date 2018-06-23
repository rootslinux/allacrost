///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    menu_inventory.h
*** \author  Daniel Steuernol (Steu)
*** \author  Andy Gardner (ChopperDave)
*** \brief   Header file for inventory menu.
***
*** This code handles the inventory menu.
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
*** \brief Represents the inventory window to browse the party's inventory
***
*** This handles item use.  You can also view all items by category.
*** ***************************************************************************/
class InventoryWindow : public hoa_gui::MenuWindow {
    friend class hoa_menu::MenuMode;

public:
    InventoryWindow();

    ~InventoryWindow();

    /** \brief Toggles the inventory window being in the active context for the player
    *** \param new_status Activates the inventory window when true, de-activates it when false
    **/
    void Activate(bool new_status);

    /** \brief Indicates whether the inventory window is in the active context
    *** \return True if the inventory window is in the active context
    **/
    bool IsActive()
    {
        return _active_box;
    }

    //! If the inventory window is ready to cancel out, or cancel out a sub-window
    //bool CanCancel();

    /*!
    * \brief Updates the inventory window.  Handles key presses, switches window context, etc.
    */
    void Update();

    /*!
    * \brief Draw the inventory window
    * \return success/failure
    */
    void Draw();

private:
    //! Used for char portraits in bottom menu
    std::vector<hoa_video::StillImage> _portraits;

    //! Used for the current dungeon
    hoa_video::StillImage _location_graphic;

    //! Flag to specify the active option box
    uint32 _active_box;

    //! OptionBox to display all of the items
    hoa_gui::OptionBox _inventory_items;

    //! OptionBox to choose character
    hoa_gui::OptionBox _char_select;

    //! OptionBox to choose item category
    hoa_gui::OptionBox _item_categories;

    //! TextBox that holds the selected object's description
    hoa_gui::TextBox _description;

    //! Vector of GlobalObjects that corresponds to _inventory_items
    std::vector< hoa_global::GlobalObject* > _item_objects;

    /*!
    * \brief Updates the item text in the inventory items
    */
    void _UpdateItemText();

    /*!
    * \brief Initializes inventory items option box
    */
    void _InitInventoryItems();

    /*!
    * \brief Initializes char select option box
    */
    void _InitCharSelect();

    /*!
    * \brief Initializes item category select option box
    */
    void _InitCategory();

    template <class T> std::vector<hoa_global::GlobalObject*> _GetItemVector(std::vector<T*>* inv);
}; // class InventoryWindow : public hoa_video::MenuWindow

/*!
* \brief Converts a vector of GlobalItem*, etc. to a vector of GlobalObjects*
* \return the same vector, with elements of type GlobalObject*
*/
template <class T> std::vector<hoa_global::GlobalObject*> InventoryWindow::_GetItemVector(std::vector<T*>* inv) {
    std::vector<hoa_global::GlobalObject*> obj_vector;

    for (typename std::vector<T*>::iterator i = inv->begin(); i != inv->end(); i++) {
        obj_vector.push_back(*i);
    }

    return obj_vector;
}

} // end private_menu

} // end hoa_menu
