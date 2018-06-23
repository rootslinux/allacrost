///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    tile_layer.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for tile layer data and view classes
***
*** This file contains the implementation of tile layers for maps. Each map contains one
*** or more tile layers that indicate which tiles from the list of tilesets are placed at
*** each location on the map. All map contexts share the same number of layers and the properties
*** of tile layers are consistent across each context. This file also contains the definition of a
*** widget class that lists each tile layer in the editor and allows the user to view and modify
*** those properties.
*** **************************************************************************/

#pragma once

#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "editor_utils.h"
#include "tileset.h"

namespace hoa_editor {

class MapData;

/** ****************************************************************************
*** \brief Represents a layer of tiles on the map
***
*** A tile layer is simply a 2D array of indeces that map to a specific tile among the
*** selected tilesets. Each tile context maintains its own set of tile layers that it
*** manages. This class provides public methods for setting the values of tiles within
*** the layer, but any operations that change the size of the layer are kept private and
*** are only able to be modified by the layer's containing TileContext.
***
*** \note There are additional properties about a tile layer that are not stored here. For
*** example, the layer's visibility, whether or not collision data is active, and the tileset's
*** name. This is because every map context shares the same layers, and these properties would
*** need to be duplicated for every TileLayer object. Instead, look to the class TileLayerProperties,
*** which contains a single set of these properties for every tile layer shared across all contexts.
*** ***************************************************************************/
class TileLayer {
	friend class MapData;

public:
	TileLayer()
		{}

	TileLayer(uint32 length, uint32 height)
		{ ResizeLayer(length, height); }

	//! \brief Returns the number of tiles high that the layer is
	uint32 GetHeight() const
		{ return _tiles.size(); }

	//! \brief Returns the number of tiles long that the layer is
	uint32 GetLength() const
		{ if (_tiles.empty() == true) return 0; else return _tiles[0].size(); }

	/** \brief Retrieves the tile at a specific location
	*** \param x The x (column) location of the tile
	*** \param y The y (row) location of the tile
	*** \return The index value of the tile. Returns MISSING_TILE if the coordinate arguments are invalid.
	**/
	int32 GetTile(uint32 x, uint32 y) const;

	/** \brief Sets the value of a tile at a specific location
	*** \param x The x (column) location of the tile
	*** \param y The y (row) location of the tile
	*** \param value The value to set for the title
	***
	*** If the coordinates are invalid, no change will take place.
	**/
	void SetTile(uint32 x, uint32 y, int32 value);

	/** \brief Clears the tile at a specific location by setting it's value to MISSING_TILE
	*** \param x The x (column) location of the tile
	*** \param y The y (row) location of the tile
	***
	*** If the coordinates are invalid, no change will take place.
	**/
	void ClearTile(uint32 x, uint32 y)
		{ SetTile(x, y, MISSING_TILE); }

	/** \brief Replaces all tiles that match a specified value with a new tile value
	*** \param old_tile The tile value to be replaced
	*** \param new_tile The tile value to set replaced tiles to
	**/
	void ReplaceTiles(int32 old_tile, int32 new_tile);

	/** \brief Sets every tile location in the layer to the same value
	*** \param value The value to set each tile to
	**/
	void FillLayer(int32 value);

	//! \brief Clears the layer of all data, setting each tile in the layer to MISSING_TILE
	void ClearLayer()
		{ FillLayer(MISSING_TILE); }

	/** \brief Resizes the layer to the dimensions specified
	*** \param length The new length of the layer, in number of tiles
	*** \param height The new height of the layer, in number of tiles
	*** \note Use care when calling this function. Resizing a tile layer that is managed by the map data can
	*** cause serious issues and will likely cause the editor to crash.
	***
	*** If the resize operation makes the layer smaller in either dimension, the appropriate
	*** number of rows and/or columns will be removed from the rows and columns on the ends.
	*** If the resize operations causes the layer to grow in size, the rows and columns will be
	*** added to the end with MISSING_TILE.
	**/
	void ResizeLayer(uint32 length, uint height);

	/** \brief Returns a reference to the tile container
	*** \note This method is provided for convenience for operations such as drawing functions.
	*** The caller should not attempt to resize the tiles container, as that would lead to erroneous
	*** behavior.
	**/
	std::vector<std::vector<int32> >& GetTiles()
		{ return _tiles; }

private:
    //! \brief Represents the tile indeces, where a tile at (x,y) is accessed as _tiles[y][x]
    std::vector<std::vector<int32> > _tiles;

	/** \brief Adds new rows of tiles to a specified location
	*** \param row_index The index where the rows should be added
	*** \param row_count The number of rows that should be added
	*** \param value The value to set each tile in the newly added rows
	***
	*** Specifying the row_index as the height of the layer results in appending the row to the end
	*** of the existing rows. Any value beyond this range is considered invalid and no operation will
	*** take place.
	***
	*** \note This function does not report any error or print any debug message if an error condition is found
	*** and the rows are not added. Additionally, the function does not check if the number of rows added
	*** will cause the layer to exceed the MAXIMUM_MAP_HEIGHT. It is the responsibility of the caller to
	*** ensure that the arguments passed are valid before calling this method.
	**/
	void _AddRows(uint32 row_index, uint32 row_count, int32 value);

	/** \brief Adds new rows of tiles to a specified location in the table
	*** \note This method calls the three argument version and uses a value of MISSING_TILE for the third argument
	**/
	void _AddRows(uint32 row_index, uint32 row_count)
		{ _AddRows(row_index, row_count, MISSING_TILE); }

	/** \brief Adds new columns of tiles to a specified location
	*** \param col_index The index where the columns should be added
	*** \param col_count The number of columns that should be added
	*** \param value The value to set each tile in the newly added columns
	***
	*** Specifying the col_index as the length of the layer results in appending the column to the end
	*** of the existing columns. Any value beyond this range is considered invalid and no operation will
	*** take place.
	***
	*** \note This function does not report any error or print any debug message if an error condition is found
	*** and the columnss are not added. Additionally, the function does not check if the number of columns added
	*** will cause the layer to exceed the MAXIMUM_MAP_LENGTH. It is the responsibility of the caller to
	*** ensure that the arguments passed are valid before calling this method.
	**/
	void _AddColumns(uint32 col_index, uint32 col_count, int32 value);

	/** \brief Adds new columns of tiles to a specified location in the table
	*** \note This method calls the three argument version and uses a value of MISSING_TILE for the third argument
	**/
	void _AddColumns(uint32 col_index, uint32 col_count)
		{ _AddColumns(col_index, col_count, MISSING_TILE); }

	/** \brief Deletes rows from the tile layer at a specified location
	*** \param row_index The starting index of the rows to delete
	*** \param row_count The number of rows that should be deleted
	***
	*** All tile rows will be shifted over to accomodate the deleted rows. If there are not enough rows between
	*** the row_index and the maximum row, no operation will take place.
	***
	*** \note This function does not report any error or print any debug message if an error condition is found
	*** and the rows are not deleted. Additionally, the function does not check if the number of rows deleted
	*** will cause the layer to go under the MINIMUM_MAP_HEIGHT. It is the responsibility of the caller to
	*** ensure that the arguments passed are valid before calling this method.
	**/
	void _DeleteRows(uint32 row_index, uint32 row_count);

	/** \brief Deletes columns from the tile layer at a specified location
	*** \param col_index The starting index of the columns to delete
	*** \param col_count The number of columns that should be deleted
	***
	*** All tile columns will be shifted over to accomodate the deleted columns. If there are not enough columns between
	*** the col_index and the maximum column, no operation will take place.
	***
	*** \note This function does not report any error or print any debug message if an error condition is found
	*** and the columns are not deleted. Additionally, the function does not check if the number of columns deleted
	*** will cause the layer to go under the MINIMUM_MAP_LENGTH. It is the responsibility of the caller to
	*** ensure that the arguments passed are valid before calling this method.
	**/
	void _DeleteColumns(uint32 col_index, uint32 col_count);
}; // class TileLayer


/** ****************************************************************************
*** \brief A container class holding properties of tile layers that are shared across contexts
***
*** This simple class retains properties of a tile layer that must remain the same for the layer
*** across all map contexts. This includes the layer's name, whether or not it is visible, and whether
*** or not it's collision data is active.
*** ***************************************************************************/
class TileLayerProperties {
public:
	TileLayerProperties() :
		_layer_name(QString("")), _visible(true), _collision_enabled(true) {}

	TileLayerProperties(QString name) :
		_layer_name(name), _visible(true), _collision_enabled(true) {}

	TileLayerProperties(QString name, bool visible, bool collisions) :
		_layer_name(name), _visible(visible), _collision_enabled(collisions) {}

	//! \name Class member accessor functions
	//@{
	QString GetLayerName() const
		{ return _layer_name; }

	bool IsVisible() const
		{ return _visible; }

	bool IsCollisionEnabled() const
		{ return _collision_enabled; }

	void SetLayerName(QString name)
		{ _layer_name = name; }

	void SetVisible(bool visible)
		{ _visible = visible; }

	void SetCollisionEnabled(bool collisions)
		{ _collision_enabled = collisions; }

	void ToggleVisible()
		{ _visible = !_visible; }

	void ToggleCollisionEnabled()
		{ _collision_enabled = !_collision_enabled; }
	//@}

private:
	/** \brief The name of the layer as it will be seen by the user of the editor
	*** \note Although this data is saved to the map file, it is used only by the editor and not the game.
	**/
	QString _layer_name;

	/** \brief Indicates whether or not the layer is visible in the editor
	*** \note This data is not saved to the map file. Any newly created or loaded tile layer will be visible by default.
	**/
	bool _visible;

	/** \brief Indicates whether the collision properties of the tile in this layer should take effect
	***
	*** This member is best set to true for layers that comprise the ground or floor of a tileset. Layers which constitute
	*** the higher part of ceilings, the tops of trees, and other unwalkable locations usually should have this property disabled.
	**/
	bool _collision_enabled;
}; // class TileLayerProperties


/** ****************************************************************************
*** \brief Displays the sortable list of tile layers on the map
***
*** This widget is located in the top right section of the main editor window.
*** The user can see the order of tile layers and some of the properties of those
*** layers. The user interacts with this widget to query information about a layer,
*** change the order of the layer, or change the active property of a layer. The
*** layer properties include: visibility, name, and collisions enabled
***
*** Single clicking a layer row will select that layer. Double-clicking a layer's column
*** will either toggle the property for that column, or open up a persistent editor for
*** the user to input new information (ie, layer renaming). A right click anywhere in the
*** widget brings up a menu allowing the user to add a new layer, or delete or rename an
*** existing layer if the click occurred over an existing layer in the widget.
***
*** The widget places no restrictions on the number of layers that can be added. The user
*** is not permitted to delete a layer if it is the only remaining layer for the map. All layer
*** names must be unique, so a rename operation will fail if the user tries to rename a layer
*** and uses a name that already exists.
***
*** \todo Renaming tile layers can result in weird behavior if the user changes the focus to
*** another widget before finishing the renaming. Also pressing the ESC button does not cancel
*** the rename operation, and starting a rename without making any changes and selecting a different
*** layer behaves poorly also. The rename functionality needs to be improved to eliminate all of these
*** undesirable behaviors.
*** ***************************************************************************/
class LayerView : public QTreeWidget {
private:
	Q_OBJECT // Macro needed to use QT's slots and signals

	//! \name Widget Column Identifiers
	//@{
	static const uint32 ID_COLUMN = 0;
	static const uint32 VISIBLE_COLUMN = 1;
	static const uint32 NAME_COLUMN = 2;
	static const uint32 COLLISION_COLUMN = 3;
	//@}

public:
	LayerView(MapData* data);

	~LayerView();

	//! \brief Refreshes the viewable contents of the widget. Should be called whenever the map layer data changes outside of this widget
	void RefreshView();

protected:
	/** \brief Reimplemented from QTreeWidget to process left and right clicks separately
	*** \param event A pointer to the mouse event which occurred
	**/
	void mousePressEvent(QMouseEvent* event);

	/** \brief Reimplemented from QTreeWidget to change map data when layers are reordered
	*** \param event A pointer to the drop event which occurred
	**/
	void dropEvent(QDropEvent* event);

private slots:
	/** \brief Updates the selected layer for editing in the map view widget
	***
	*** This function is called whenever the user single-clicks one of the layer items in the widget
	**/
	void _ChangeSelectedLayer();

	/** \brief Modifies one of the properties of a tile layer
	*** \param item A pointer to the layer where the property change will happen
	*** \param column The column number of the property which should be changed
	***
	*** This function is called whenever the user double-clicks one of the layer items in the widget
	**/
	void _ChangeLayerProperties(QTreeWidgetItem* item, int column);

	/** \brief Closes the persistent editor that was opened by the _RenameTileLayer method and changes the layer name in the map data
	*** \param item A pointer to the item with the modified name
	*** \param column The column that is being edited (should be NAME_COLUMN)
	***
	*** This function is connected to the signal that is sent whenever any item in the widget is changed, not just
	*** when the name of a layer has been renamed. Any change event other than a rename event will be ignored.
	**/
	void _SetTileLayerName(QTreeWidgetItem* item, int column);

	//! \brief Creates a new empty tile layer and adds it to the end of the layer list
	void _AddTileLayer();

	//! \brief Creates a new tile layer that clones all the data and properties of an existing layer
	void _CloneTileLayer();

	//! \brief Opens up an editor to rename the layer pointed to by _right_click_item
	void _RenameTileLayer();

	//! \brief Deletes the layer item pointed to by _right_click_item
	void _DeleteTileLayer();

private:
	//! \brief A pointer to the active map data that contains the tile layers
	MapData* _map_data;

	//! \brief While renaming a layer, holds the original name in case the renaming operation is cancelled for fails
	QString _original_layer_name;

	//! \brief An icon used to indicate the visibility property of a tile layer
	QIcon _visibility_icon;

	//! \brief A pointer to the most recent item that was right clicked. Set to nullptr if no item was clicked
	QTreeWidgetItem* _right_click_item;

	//! \brief Menu for right-clicks events on the widget
    QMenu* _right_click_menu;

	/** \name Right-Click Menu Actions
	*** \brief The possible actions the user can take on the right-click menu
	**/
	//{@
	QAction* _add_layer_action;
	QAction* _clone_layer_action;
	QAction* _rename_layer_action;
	QAction* _delete_layer_action;
	//@}
}; // class LayerView : public QTreeWidget

} // namespace hoa_editor
