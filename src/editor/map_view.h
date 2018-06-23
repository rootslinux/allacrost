///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    map_view.h
*** \author  Philip Vorsilak (gorzuate)
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for the map view widget
*** **************************************************************************/

#pragma once

#include <QGraphicsScene>
#include <QStringList>
#include <QTreeWidgetItem>

#include "map_data.h"

namespace hoa_editor {

/** ***************************************************************************
*** \brief The GUI component where map tiles are drawn and edited
***
*** This class draws all of the tiles that compose the map to the editor's main window screen.
*** All of the data for the map is stored in the MapData object that the class maintains a pointer
*** to. Some editor properties, such as whether or not the tile grid is visible, are stored here.
*** **************************************************************************/
class MapView : public QGraphicsScene {
	Q_OBJECT // Macro needed to use QT's slots and signals

	/** \brief The different modes that the select area tool can operate in
	*** - NORMAL: Only the most recent selected area will be active and previous selections will be cleared
	*** - ADDITIVE: Add the current area being selected to the total selected area
	*** - SUBTRACTIVE: If the current area overlaps an already selected area, unselect the tiles that intersect the two
	**/
	enum SELECTION_MODE { NORMAL, ADDITIVE, SUBTRACTIVE };

public:
	/** \param parent The parent widget, which should be the main editor window
	*** \param data A pointer to the map data to manipulate and draw
	**/
	MapView(QWidget* parent, MapData* data);

	~MapView();

	//! \name Class member accessor functions
	//@{
	QGraphicsView* GetGraphicsView() const
		{ return _graphics_view; }

	EDIT_MODE GetEditMode() const
		{ return _edit_mode; }

	void SetGridVisible(bool value)
		{ _grid_visible = value; DrawMap(); }

	void SetMissingOverlayVisible(bool value)
		{ _missing_overlay_visible = value; DrawMap(); }

	void SetInheritedOverlayVisible(bool value)
		{ _inherited_overlay_visible = value; DrawMap(); }

	void SetCollisionOverlayVisible(bool value)
		{ _collision_overlay_visible = value; DrawMap(); }

	bool ToggleGridVisible()
		{ _grid_visible = !_grid_visible; DrawMap(); return _grid_visible; }

	bool ToggleMissingOverlayVisible()
		{ _missing_overlay_visible = !_missing_overlay_visible; DrawMap(); return _missing_overlay_visible; }

	bool ToggleInheritedOverlayVisible()
		{ _inherited_overlay_visible = !_inherited_overlay_visible; DrawMap(); return _inherited_overlay_visible; }

	bool ToggleCollisionOverlayVisible()
		{ _collision_overlay_visible = !_collision_overlay_visible; DrawMap(); return _collision_overlay_visible; }

	void SetEditMode(EDIT_MODE new_mode);
	//@}

	//! \brief Clears all data from _selection_area by filling it with MISSING_TILE
	void SelectNoTiles();

	//! \brief Selects the entire map in the selection area
	void SelectAllTiles()
		{ _selection_area.FillLayer(SELECTED_TILE); _selection_area_active = true; }

	/** \brief This method should be called whenever the map size is modified so that the _selection_area can be resized accordingly
	*** \note Calling this function will clear the selection area as well.
	**/
	void UpdateAreaSizes();

	/** \brief Clears and re-creates all menu actions that take effect on a specific tile layer
	*** This method should be called whenever the number, name, or ordering of tile layers change.
	**/
	void UpdateLayerActions();

	/** \brief Clears and re-creates all menu actions that take effect on a specific tile context
	*** This method should be called whenever the number, name, or ordering of tile contexts change.
	**/
	void UpdateContextActions();

	//! \brief Draws all visible tile layers from the active context as well as overlays and other visual elements
    void DrawMap();

protected:
	/** \name User Input Event Processing Functions
	*** \brief Functions to process mouse and keyboard events on the map
	*** \param event A pointer to the type of QEvent that was generated
	**/
	//{@
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
	//@}

private slots:
	/** \name Right-click menu functions
	*** \brief Functions available from right-clicking on the map view.
	*** \param action The action that caused this slot function to be called
	*** \note The location of the click may effect where or how the following operations take place
	**/
	//{@
	void _InsertSingleTileRow();
	void _InsertMultipleTileRows();
	void _InsertSingleTileColumn();
	void _InsertMultipleTileColumns();
	void _DeleteSingleTileRow();
	void _DeleteMultipleTileRows();
	void _DeleteSingleTileColumn();
	void _DeleteMultipleTileColumns();
	void _MoveSelectionToLayer(QAction* action);
	void _CopySelectionToLayer(QAction* action);
	void _MoveSelectionToContext(QAction* action);
	void _CopySelectionToContext(QAction* action);
	//@}

private:
	//! \brief A pointer to the underlying map data to read and manipulate
	MapData* _map_data;

	//! \brief When true, an area of the map is selected by the user
	bool _selection_area_active;

	//! \brief Holds the type of selection mode that is active when using the SELECT_AREA edit mode
	SELECTION_MODE _selection_mode;

	//! \brief Set to true when the selection area is active and the tile that most recently registerd a mouse press is in that selection
	bool _selection_area_press;

	//! \brief When true, a series of grid lines between tiles are drawn
	bool _grid_visible;

	//! \brief When true, an overlay is drawn over each missing tile (MISSING_TILE) for the selected tile layer
	bool _missing_overlay_visible;

	//! \brief When true, an overlay is drawn over each inherited tile (INHERITED_TILE) for the selected tile layer
	bool _inherited_overlay_visible;

	//! \brief When true, an overlay is drawn over each active collision grid element
	bool _collision_overlay_visible;

	//! \brief The coordinates of the tile that the mouse cursor is currently pointing to
	//@{
	int32 _cursor_tile_x;
	int32 _cursor_tile_y;
	//@}

	//! \brief The coordinates of the tile where the last mouse press event occurred
	//@{
	int32 _press_tile_x;
	int32 _press_tile_y;
	//@}

	//! \brief The current tile editing tool that is active
	EDIT_MODE _edit_mode;

	/** \brief A layer used to show where the selected tileset tiles will be painted to
	***
	**/
	TileLayer _preview_layer;

	/** \brief A tile layer used for indicating a selected area of the map
	*** The values of tiles in this layer should only be MISSING_TILE or SELECTED_TILE. If all values are
	*** MISSING_TILE, _area_selected should be set to false. Otherwise _area_selected should be set to true.
	***
	*** \note Any map resize operations will clear the selection area before resizing it.
	**/
	TileLayer _selection_area;

	/** \brief Tracks the boundaries of the current selection area
	*** Selection areas are not always rectangular, so these members store the left-most, top-most, etc. tile
	*** that is currently selected across the entire map.
	***
	*** When no selection area is active, left and top are set to the map length and height respectively, while
	*** right and bottom are set to zero.
	**/
	uint32 _selection_area_left, _selection_area_right, _selection_area_top, _selection_area_bottom;

	//! \brief Menus for right-clicks events on the map
	//@{
    QMenu* _right_click_menu;
	QMenu* _insert_menu;
	QMenu* _delete_menu;
	QMenu* _selection_menu;
	QMenu* _selection_move_to_layer_menu;
	QMenu* _selection_copy_to_layer_menu;
	QMenu* _selection_move_to_context_menu;
	QMenu* _selection_copy_to_context_menu;
	//@}

	/** \name Right-Click Menu Actions
	*** \brief Correspond to the private slots functions for user actions
	**/
	//{@
	QAction* _insert_single_row_action;
	QAction* _insert_multiple_rows_action;
	QAction* _insert_single_column_action;
	QAction* _insert_multiple_columns_action;

	QAction* _delete_single_row_action;
	QAction* _delete_multiple_rows_action;
	QAction* _delete_single_column_action;
	QAction* _delete_multiple_columns_action;

	std::vector<QAction*> _selection_move_to_layer_actions;
	std::vector<QAction*> _selection_copy_to_layer_actions;
	std::vector<QAction*> _selection_move_to_context_actions;
	std::vector<QAction*> _selection_copy_to_context_actions;
	//@}

	//! \brief A one-tile sized square used to highlight tiles in the preview layer (colored green at 20% opacity)
	QPixmap _preview_tile;

	//! \brief A one-tile sized square used to highlight multi-tile selections (colored blue at 20% opacity)
	QPixmap _selection_tile;

	//! \brief A one-tile sized square used to highlight missing tiles (colored orange at 20% opacity)
	QPixmap _missing_tile;

	//! \brief A one-tile sized square used to highlight inherited tiles (colored yellow at 20% opacity)
	QPixmap _inherited_tile;

	//! \brief A one-quarter tile sized square used to indicate which areas of the collision grid are active (colored red at 20% opacity)
	QPixmap _collision_element;

	//! \brief Used to display the graphics widgets
	QGraphicsView* _graphics_view;

	/** \brief Retrieves the value of the currently selected tile from the selected tileset
	***
	*** If more than one tile is selected from the tileset, then the top-left tile of the selection
	*** is returned.
	**/
	int32 _RetrieveCurrentTileValue() const;

	/** \brief Given a tile, retrieves a pointer to the QPixmap image that the value represents
	*** \param tile_value The value of the tile to retrieve
	*** \return A pointer to the QPixmap of the tile, or nullptr if no such image existed for this value
	**/
	const QPixmap* _RetrieveTileImage(int32 tile_value) const;

	/** \brief Sets the value of a single tile on the map for the selected tile layer
	*** \param x The x coordinate of the tile to set
	*** \param y The y coordinate of the tile to set
	*** \param value The value to set the tile to
	***
	*** \note This should not be used for the paint tool as it does not modify the _preview_layer
	**/
	void _SetTile(int32 x, int32 y, int32 value);

	/** \brief Paints the currently selected tiles from the tileset to a location on the map
	*** \param x The x coordinate of the tile to paint to
	*** \param y The y coordinate of the tile to paint to
	*** \param preview If true, tiles should be painted to the preview layer. Otherwise tiles will be painted to the selected tile layer
	***
	*** When the selection area is active, a paint operation that begins in a non-selected area will not paint any tiles to the selection area,
	*** and vice-versa if the paint operation begins in a selected area.
	**/
	void _PaintTiles(uint32 x, uint32 y, bool preview);

	/** \brief Swaps the location of one or more tiles on more tiles on the map
	*** \param x1 The x coordinate of the first swap position
	*** \param y1 The y coordinate of the first swap position
	*** \param x2 The x coordinate of the second swap position
	*** \param y2 The y coordinate of the second swap position
	***
	*** Swaps are handled differently if a selection area is active and the first position was inside the selection area. Otherwise only
	*** a single tile will be swapped. When swapping multiple tiles, if the swap location for any given tile is outside the bounds of the
	*** map then that tile will not be swapped, but other tiles with a valid destination will swap.
	**/
	void _SwapTiles(uint32 x1, uint32 y1, uint32 x2, uint32 y2);

	/** \brief Sets a tile to a specific value as well as all neighboring tiles that share the tile's original value
	*** \param start_x The x coordinate to start the fill operation from
	*** \param start_y The y coordinate to start the fill operation from
	*** \param value The value to fill tiles in the area with
	***
	*** This function behaves differently if a selection area is active on the map. If _selection_area_active is true and the tile corresponding
	*** to the x/y coordinates is selected, then the operation applies to all neighboring tiles that are also selected.
	**/
	void _FillArea(uint32 start_x, uint32 start_y, int32 value);

	/** \brief Takes two coordinates representing a rectangle and selects the tiles inside that shape
	*** \param x1 The x coordinate of the first rectangle corner
	*** \param y1 The y coordinate of the first rectangle corner
	*** \param x2 The x coordinate of the second rectangle corner
	*** \param y2 The y coordinate of the second rectangle corner
	***
	*** \note The corners of the rectangle should be opposite of one another.
	*** \note Calling this function will clear the existing selected area before setting the newly selected tiles.
	**/
	void _SetSelectionArea(uint32 x1, uint32 y1, uint32 x2, uint32 y2);

	/** \brief Compares a tile's selection status with that of the tile that was most recently pressed
	*** \param x The x coordinate of the tile to check
	*** \param y The y coordinate of the tile to check
	*** \return True if the selection area is active or inactive for both the tile and the press tile
	***
	*** True will also be returned if no selection area is active. This function is used by the editing operations to ensure that if we started
	*** an operation in a tile that was not selected, the operation will not take place in any selected tiles (or vice versa).
	**/
	bool _IsTileEqualToPressSelection(uint32 x, uint32 y);

	/** \brief Moves or copies the selected tiles to a different tile layer
	*** \param uint32 layer_id The ID of the layer to move or copy the selected tiles to
	*** \param copy_or_move If true, copy the tiles to the new layer. Otherwise move them there and then delete the tiles from the original layer.
	**/
	void _SelectionToLayer(uint32 layer_id, bool copy_or_move);

	/** \brief Moves or copies the selected tiles to a different tile context
	*** \param uint32 context_index The index of the context to move or copy the selected tiles to
	*** \param copy_or_move If true, copy the tiles to the new context. Otherwise move them there and then delete the tiles from the original context.
	***
	*** \note If the source context is an inheriting context, the selection contains inherited tiles, and the destination context is not an inheriting context,
	*** then all INHERITED_TILE tiles will be converted to MISSING_TILE. A warning message also pops up to inform the user about this condition when it happens.
	**/
	void _SelectionToContext(uint32 context_index, bool copy_or_move);

	/** \brief Sets the text in the editor window status bar. Should be called whenever a mouse event occurs
	*** \param event A pointer to the mouse event that occured. Used to grab the current coorindates of the mouse
	**/
	void _UpdateStatusBar(QGraphicsSceneMouseEvent* event);

	//! \brief A helper function to DrawMap() that draws the current selection area when the edit mode is SELECT_AREA
	void _DrawSelectionArea();

	//! \brief A helper function to DrawMap() that draws the tile grid over the tiles
	void _DrawGrid();
}; // class MapView : public QGraphicsScene

} // namespace hoa_editor
