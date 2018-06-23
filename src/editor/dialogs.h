///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    dialogs.h
*** \author  Philip Vorsilak (gorzuate)
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for all of the editor's dialog windows
*** **************************************************************************/

#pragma once

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeWidget>

#include "map_data.h"

namespace hoa_editor {

/** ***************************************************************************
*** \brief A dialog window that allows the user to create a new map
***
*** The properties that may be modified through this dialog include the following:
*** - The map dimensions (in tiles)
*** - Which tilesets are used by this map
*** **************************************************************************/
class NewMapDialog : public QDialog {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	/** \param parent The widget from which this dialog was invoked
	*** \param data A pointer to the active map data
	**/
	NewMapDialog(QWidget* parent, MapData* data);

	~NewMapDialog();

	//! \name Class member accessor functions
	//@{
	QTreeWidget* GetTilesetTree() const
		{ return _tileset_tree; }
	//@}

private slots:
	//! \brief Enables or disables the OK push button of this dialog depending on whether any tilesets are checked or not.
	void _EnableOKButton();

	//! \brief Creates the new map data based on the input that the user entered
	void _CreateMapData();

private:
	//! \brief A pointer to the map data object to populate based on the user's input
	MapData* _map_data;

	//! \brief Spinboxes that allow the user to specify the dimensions of the new map
	//@{
	QSpinBox* _height_spinbox;
	QSpinBox* _length_spinbox;
	//@}

	//! \brief Labels used to identify the height or length spinboxes
	//@{
	QLabel* _height_title;
	QLabel* _length_title;
	//@}

	//! \brief A tree for showing all available tilesets
	QTreeWidget* _tileset_tree;

	//! \brief A pushbutton for okaying the new map dialog.
	QPushButton* _ok_button;

	//! \brief A pushbutton for canceling the new map dialog.
	QPushButton* _cancel_button;

	//! \brief A layout to manage all the labels, spinboxes, and listviews.
	QGridLayout* _grid_layout;
}; // class NewMapsDialog : public QDialog


/** ***************************************************************************
*** \brief Allows the user to resize the map by adding or removing rows and columns from its end
***
*** This dialog allows the user to specify the new height and length of the map. New rows and columns
*** are either added or removed from the right and bottom sides.
***
*** \todo This class needs to be enhanced in the future. Instead of the current format, the user should
*** be able to select the new height and length of the map and an x/y offset that determines where rows
*** and columns are added and removed. Refer to the Tiled map editor's "Resize Map" menu option for how
*** this should be done.
*** **************************************************************************/
class MapResizeDialog : public QDialog {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	/** \param parent The widget from which this dialog was invoked
	*** \param data A pointer to the active map data
	**/
	MapResizeDialog(QWidget* parent, MapData* data);

	~MapResizeDialog();

	//! \brief Makes the changes to the map data and redraws the map
	void ModifyMapData();

private:
	//! \brief A pointer to the active map data containing the list of open tilesets
	MapData* _map_data;

	//! \brief Spinboxes that allow the user to specify the new dimensions of the map
	//@{
	QSpinBox* _height_spinbox;
	QSpinBox* _length_spinbox;
	//@}

	//! \brief Labels used to identify the height or length segements of the dialog
	//@{
	QLabel* _height_title;
	QLabel* _length_title;
	//@}

	//! \brief Labels used to specify the number of rows or columns that will be added or deleted
	//@{
	QLabel* _height_change;
	QLabel* _length_change;
	//@}

	//! \brief A button to confirm the resize operation
	QPushButton* _ok_button;

	//! \brief A button for cancelling the resize operation
	QPushButton* _cancel_button;

	//! \brief Defines the layout of all widgets in the dialog window
	QGridLayout* _grid_layout;

private slots:
	//! \brief Processes changes in height to update the _height_change label
	void _HeightChanged();

	//! \brief Processes changes in height to update the _height_change label
	void _LengthChanged();
}; // class MapResizeDialog : public QDialog


/** ***************************************************************************
*** \brief A dialog window that allows the user to insert or delete multiple rows or columns of tiles from a chosen location
***
*** The values passed to the constructor of this class determine whether the object instance will be manipulating rows or columns,
*** and whether it will be inserting or deleting them. The user can not change the operation from insert to delete or from rows to
*** columns once this class object is constructed. The position from where the insert/delete operation takes place also can not be
*** changed.
***
*** When rows or columns are inserted, the number to insert are all placed at the selected position. This means that the existing rows
*** at and past this position (right for columns, down for rows) are "pushed out" to allow space for the new columns and rows to be placed.
*** All rows and columns inserted are initialized to MISSING_TILE. When a delete operation takes place, the first row or column deleted is that
*** corresponding to the selected location. Every additional column or row to delete are taken from the right (for columns) or bottom (for
*** rows).
*** **************************************************************************/
class MapResizeInternalDialog : public QDialog {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	/** \param parent The widget from which this dialog was invoked
	*** \param data A pointer to the active map data
	*** \param row The starting tile row coordinate for the operation
	*** \param column The starting tile column coordinate for the operation
	*** \param insert_operation If true, this widget should be inserting rows or columns. If false, it will be deleting them
	*** \param column_operation If true, this widget should be manipulating columns. If false, it will be manipulating rows
	**/
	MapResizeInternalDialog(QWidget* parent, MapData* data, uint32 row, uint32 column, bool insert_operation, bool column_operation);

	~MapResizeInternalDialog();

	//! \brief Makes the changes to the map data and redraws the map
	void ModifyMapData();

private:
	//! \brief A pointer to the active map data containing the list of open tilesets
	MapData* _map_data;

	//! \brief The map coordinates where the operation will take place
	//@{
	uint32 _row_position;
	uint32 _column_position;
	//@}

	//! \brief If true, this class is performing an insert operation. False indicates a delete operation.
	bool _insert_operation;

	//! \brief If true, this class is manipulating tile columns. False indicates manipulation of tile rows.
	bool _column_operation;

	//! \brief A spinbox for specifying the number of columns or rows to insert or delete
	QSpinBox* _change_spinbox;

	//! \brief Describes the operation about to take place (insert or delete)
	QLabel* _operation_text;

	//! \brief Tells the user the map position where the operation will take place
	QLabel* _position_text;

	//! \brief A button to confirm the insert/delete operation
	QPushButton* _ok_button;

	//! \brief A button for cancelling the insert/delete operation
	QPushButton* _cancel_button;

	//! \brief Defines the layout of all widgets in the dialog window
	QGridLayout* _grid_layout;

private slots:
	//! \brief Enables the Ok button so long as a non-zero value is entered in the change spinbox
	void _EnableOkButton();
}; // class MapResizeInternalDialog : public QDialog


/** ***************************************************************************
*** \brief A dialog window that allows the user to add additional tilesets to a map
***
*** This presents the user with a list of all available tilesets that can be added to
*** the map. Tilesets which are already loaded and in use by the map are also shown, but
*** they are greyed out and the user can not interact with them. The user can add more than
*** one tileset to the map at a time with this widget.
*** **************************************************************************/
class AddTilesetsDialog : public QDialog {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	/** \param parent The widget from which this dialog was invoked
	*** \param data A pointer to the active map data
	**/
	AddTilesetsDialog(QWidget* parent, MapData* data);

	~AddTilesetsDialog();

	/** \brief Adds the tilesets selected by the user to the map data
	*** \return The number of tilesets that were added
	***
	*** This should be called only after the user clicks the "Add" button. It may generate error message dialogs to the user if any
	*** of the tilesets failed to load.
	**/
	uint32 AddTilesetsToMapData();

private:
	//! \brief A pointer to the active map data containing the list of open tilesets
	MapData* _map_data;

	//! \brief A tree for showing all available tilesets
	QTreeWidget* _tileset_tree;

	//! \brief A button to confirm adding the new tilesets
	QPushButton* _add_button;

	//! \brief A button for cancelling the add operation
	QPushButton* _cancel_button;

	//! \brief Defines the layout of all widgets in the dialog window
	QGridLayout* _grid_layout;

private slots:
	//! \brief Enables or disables the add push button of this dialog depending on whether any tilesets are selected
	void _EnableAddButton();
}; // class AddTilesetsDialog : public QDialog

} // namespace hoa_editor
