///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    map_view.cpp
*** \author  Philip Vorsilak (gorzuate)
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for the map view widget
*** **************************************************************************/

#include <queue>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QScrollBar>

#ifndef QT_NO_OPENGL
	#include <QGLWidget>
#endif

#include "dialogs.h"
#include "editor.h"
#include "map_view.h"
#include "tileset.h"

using namespace std;

namespace hoa_editor {

MapView::MapView(QWidget* parent, MapData* data) :
	QGraphicsScene(parent),
	_map_data(data),
	_selection_area_active(false),
	_selection_mode(NORMAL),
	_selection_area_press(false),
	_grid_visible(false),
	_missing_overlay_visible(false),
	_inherited_overlay_visible(false),
	_collision_overlay_visible(false),
	_cursor_tile_x(-1),
	_cursor_tile_y(-1),
	_press_tile_x(-1),
	_press_tile_y(-1),
	_edit_mode(SELECT_AREA_MODE),
	_preview_layer(data->GetMapLength(), data->GetMapHeight()),
	_selection_area(data->GetMapLength(), data->GetMapHeight()),
	_selection_area_left(data->GetMapLength()),
	_selection_area_right(0),
	_selection_area_top(data->GetMapHeight()),
	_selection_area_bottom(0),
	_right_click_menu(nullptr),
	_insert_menu(nullptr),
	_delete_menu(nullptr),
	_selection_menu(nullptr),
	_selection_move_to_layer_menu(nullptr),
	_selection_copy_to_layer_menu(nullptr),
	_selection_move_to_context_menu(nullptr),
	_selection_copy_to_context_menu(nullptr),
	_insert_single_row_action(nullptr),
	_insert_multiple_rows_action(nullptr),
	_insert_single_column_action(nullptr),
	_insert_multiple_columns_action(nullptr),
	_delete_single_row_action(nullptr),
	_delete_multiple_rows_action(nullptr),
	_delete_single_column_action(nullptr),
	_delete_multiple_columns_action(nullptr)
{
	// Create the graphics view
	_graphics_view = new QGraphicsView(parent);
	_graphics_view->setRenderHints(QPainter::Antialiasing);
	_graphics_view->setBackgroundBrush(QBrush(Qt::black));
	_graphics_view->setScene(this);
	// Use OpenGL for rendering the graphics view if it is supported
	#ifndef QT_NO_OPENGL
	if (QGLFormat::hasOpenGL() && !qobject_cast<QGLWidget*>(_graphics_view->viewport())) {
		QGLFormat format = QGLFormat::defaultFormat();
		format.setDepth(false); // No depth buffer is needed for 2D surfaces
		format.setSampleBuffers(true); // Enable anti-aliasing
		_graphics_view->setViewport(new QGLWidget(format));
	}
	else {
		// Helps with rendering when not using OpenGL
		_graphics_view->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
	}
	#endif
	_graphics_view->setMouseTracking(true);
	_graphics_view->viewport()->setAttribute(Qt::WA_StaticContents); // Helps during screen resizing

	setSceneRect(0, 0, data->GetMapLength() * TILE_LENGTH, data->GetMapHeight() * TILE_HEIGHT);

	// Create the right-click menu and corresponding actions
	_insert_single_row_action = new QAction("Insert Single Row", this);
	_insert_single_row_action->setStatusTip("Inserts a single row of empty tiles at the selected location");
	connect(_insert_single_row_action, SIGNAL(triggered()), this, SLOT(_InsertSingleTileRow()));
	_insert_multiple_rows_action = new QAction("Insert Multiple Rows...", this);
	_insert_multiple_rows_action->setStatusTip("Opens a dialog window to insert one or more empty tile rows at the selected location");
	connect(_insert_multiple_rows_action, SIGNAL(triggered()), this, SLOT(_InsertMultipleTileRows()));
	_insert_single_column_action = new QAction("Insert Single Column", this);
	_insert_single_column_action->setStatusTip("Inserts a single column of empty tiles at the selected location");
	connect(_insert_single_column_action, SIGNAL(triggered()), this, SLOT(_InsertSingleTileColumn()));
	_insert_multiple_columns_action = new QAction("Insert Multiple Columns...", this);
	_insert_multiple_columns_action->setStatusTip("Opens a dialog window to insert one or more empty tile columns at the selected location");
	connect(_insert_multiple_columns_action, SIGNAL(triggered()), this, SLOT(_InsertMultipleTileColumns()));

	_delete_single_row_action = new QAction("Delete Single Row", this);
	_delete_single_row_action->setStatusTip("Deletes a single row of tiles corresponding to the selected location");
	connect(_delete_single_row_action, SIGNAL(triggered()), this, SLOT(_DeleteSingleTileRow()));
	_delete_multiple_rows_action = new QAction("Delete Multiple Rows...", this);
	_delete_multiple_rows_action->setStatusTip("Opens a dialog window to delete one or more rows of tiles at the selected location");
	connect(_delete_multiple_rows_action, SIGNAL(triggered()), this, SLOT(_DeleteMultipleTileRows()));
	_delete_single_column_action = new QAction("Delete Single Column", this);
 	_delete_single_column_action->setStatusTip("Deletes a single column of tiles corresponding to the selected location");
	connect(_delete_single_column_action, SIGNAL(triggered()), this, SLOT(_DeleteSingleTileColumn()));
	_delete_multiple_columns_action = new QAction("Delete Multiple Columns...", this);
 	_delete_multiple_columns_action->setStatusTip("Opens a dialog window to delete one or more columns of tiles at the selected location");
	connect(_delete_multiple_columns_action, SIGNAL(triggered()), this, SLOT(_DeleteMultipleTileColumns()));

	_right_click_menu = new QMenu(_graphics_view);
	_insert_menu = new QMenu("Insert", _right_click_menu);
	_delete_menu = new QMenu("Delete", _right_click_menu);
	_selection_menu = new QMenu("Selected Area", _right_click_menu);
	_selection_move_to_layer_menu = new QMenu("Move To Layer", _right_click_menu);
	connect(_selection_move_to_layer_menu, SIGNAL(triggered(QAction*)), this, SLOT(_MoveSelectionToLayer(QAction*)));
	_selection_copy_to_layer_menu = new QMenu("Copy To Layer", _right_click_menu);
	connect(_selection_copy_to_layer_menu, SIGNAL(triggered(QAction*)), this, SLOT(_CopySelectionToLayer(QAction*)));
	_selection_move_to_context_menu = new QMenu("Move To Context", _right_click_menu);
	connect(_selection_move_to_context_menu, SIGNAL(triggered(QAction*)), this, SLOT(_MoveSelectionToContext(QAction*)));
	_selection_copy_to_context_menu = new QMenu("Copy To Context", _right_click_menu);
	connect(_selection_copy_to_context_menu, SIGNAL(triggered(QAction*)), this, SLOT(_CopySelectionToContext(QAction*)));

	_right_click_menu->addMenu(_insert_menu);
	_right_click_menu->addMenu(_delete_menu);
	_right_click_menu->addMenu(_selection_menu);

	_insert_menu->addAction(_insert_single_row_action);
	_insert_menu->addAction(_insert_multiple_rows_action);
	_insert_menu->addAction(_insert_single_column_action);
	_insert_menu->addAction(_insert_multiple_columns_action);

	_delete_menu->addAction(_delete_single_row_action);
	_delete_menu->addAction(_delete_multiple_rows_action);
	_delete_menu->addAction(_delete_single_column_action);
	_delete_menu->addAction(_delete_multiple_columns_action);

	_selection_menu->addMenu(_selection_move_to_layer_menu);
	_selection_menu->addMenu(_selection_copy_to_layer_menu);
	_selection_menu->addMenu(_selection_move_to_context_menu);
	_selection_menu->addMenu(_selection_copy_to_context_menu);

	// Green tile with 20% transparency
	_preview_tile = QPixmap(TILE_LENGTH, TILE_HEIGHT);
	_preview_tile.fill(QColor(0, 255, 0, 50));
	// Blue tile with 30% transparency
	_selection_tile = QPixmap(TILE_LENGTH, TILE_HEIGHT);
	_selection_tile.fill(QColor(0, 0, 255, 75));
	// Orange tile with 20% transparency
	_missing_tile = QPixmap(TILE_LENGTH, TILE_HEIGHT);
	_missing_tile.fill(QColor(255, 128, 0, 50));
	// Yellow tile with 20% transparency
	_inherited_tile = QPixmap(TILE_LENGTH, TILE_HEIGHT);
	_inherited_tile.fill(QColor(255, 255, 0, 50));
	// Red tile quadrant with 20% transparency
	_collision_element = QPixmap(TILE_QUADRANT_LENGTH, TILE_QUADRANT_HEIGHT);
	_collision_element.fill(QColor(255, 0, 0, 50));
} // MapView::MapView(QWidget* parent, MapData* data)



MapView::~MapView() {
	delete _insert_single_row_action;
	delete _insert_multiple_rows_action;
	delete _insert_single_column_action;
	delete _insert_multiple_columns_action;
	delete _delete_single_row_action;
	delete _delete_multiple_rows_action;
	delete _delete_single_column_action;
	delete _delete_multiple_columns_action;
	delete _right_click_menu;
	delete _insert_menu;
	delete _delete_menu;
	delete _selection_menu;
	delete _selection_move_to_layer_menu;
	delete _selection_copy_to_layer_menu;
	delete _selection_move_to_context_menu;
	delete _selection_copy_to_context_menu;
	delete _graphics_view;
}



void MapView::SetEditMode(EDIT_MODE new_mode) {
	if (_edit_mode == PAINT_MODE) {
		_preview_layer.ClearLayer();
		DrawMap();
	}

	_edit_mode = new_mode;
}



void MapView::SelectNoTiles() {
	_selection_area.ClearLayer();
	_selection_area_active = false;
	_selection_area_left = _map_data->GetMapLength();
	_selection_area_right = 0;
	_selection_area_top = _map_data->GetMapHeight();
	_selection_area_bottom = 0;
}



void MapView::UpdateAreaSizes() {
	_preview_layer.ResizeLayer(_map_data->GetMapLength(), _map_data->GetMapHeight());
	_preview_layer.ClearLayer();

	_selection_area.ResizeLayer(_map_data->GetMapLength(), _map_data->GetMapHeight());
	SelectNoTiles();
	_selection_area_press = false;
}



void MapView::UpdateLayerActions() {
	vector<TileLayerProperties>& layer_properties = _map_data->GetTileLayerProperties();

	// Delete and clear out existing actions
	for (uint32 i = 0; i < _selection_move_to_layer_actions.size(); ++i) {
		delete _selection_move_to_layer_actions[i];
	}
	_selection_move_to_layer_actions.clear();
	for (uint32 i = 0; i < _selection_copy_to_layer_actions.size(); ++i) {
		delete _selection_copy_to_layer_actions[i];
	}
	_selection_copy_to_layer_actions.clear();

	// Re-construct all actions using the current layer property data
	QAction* action = nullptr;
	for (uint32 i = 0; i < layer_properties.size(); ++i) {
		action = new QAction(layer_properties[i].GetLayerName(), this);
		_selection_move_to_layer_actions.push_back(action);
		_selection_move_to_layer_menu->addAction(action);

		action = new QAction(layer_properties[i].GetLayerName(), this);
		_selection_copy_to_layer_actions.push_back(action);
		_selection_copy_to_layer_menu->addAction(action);
	}
}



void MapView::UpdateContextActions() {
	// Get an ordered list of all active tile contexts
	vector<TileContext*> contexts;
	for (uint32 i = 0; i < _map_data->GetTileContextCount(); ++i) {
		contexts.push_back(_map_data->FindTileContextByIndex(i));
	}

	// Delete and clear out existing actions
	for (uint32 i = 0; i < _selection_move_to_context_actions.size(); ++i) {
		delete _selection_move_to_context_actions[i];
	}
	_selection_move_to_context_actions.clear();
	for (uint32 i = 0; i < _selection_copy_to_context_actions.size(); ++i) {
		delete _selection_copy_to_context_actions[i];
	}
	_selection_copy_to_context_actions.clear();

	// Re-construct all actions using the current context data
	QAction* action = nullptr;
	for (uint32 i = 0; i < contexts.size(); ++i) {
		action = new QAction(contexts[i]->GetContextName(), this);
		_selection_move_to_context_actions.push_back(action);
		_selection_move_to_context_menu->addAction(action);

		action = new QAction(contexts[i]->GetContextName(), this);
		_selection_copy_to_context_actions.push_back(action);
		_selection_copy_to_context_menu->addAction(action);
	}
}



void MapView::DrawMap() {
	clear();
	if (_map_data->IsInitialized() == false) {
		return;
	}

	// Setup drawing parameters
	setSceneRect(0, 0, _map_data->GetMapLength() * TILE_LENGTH, _map_data->GetMapHeight() * TILE_HEIGHT);
	setBackgroundBrush(QBrush(Qt::gray));

	vector<TileLayer>* tile_layers = &(_map_data->GetSelectedTileContext()->GetTileLayers());
	vector<TileLayer>* inherited_tile_layers = nullptr;
	vector<TileLayerProperties>& layer_properties = _map_data->GetTileLayerProperties();

	// If this is an inheriting context, we also want to pull in the tile layers for the inherited context
	if (_map_data->GetSelectedTileContext()->IsInheritingContext() == true) {
		// Inherited context should never be nullptr in this case
		TileContext* inherited_context = _map_data->FindTileContextByID(_map_data->GetSelectedTileContext()->GetInheritedContextID());
		inherited_tile_layers = &(inherited_context->GetTileLayers());
	}

	// Start drawing each tile from the tile layers in order
	for (uint32 l = 0; l < tile_layers->size(); ++l) {
		// Holds the value of the tile from a specific X/Y coordinate on this layer
		int32 tile = MISSING_TILE;
		// Set to true whenever we are dealing with an inherited tile
		bool inherited_tile = false;
		// Set to true if this layer is the currently selected layer that the user is viewing or editing
		bool selected_layer = (_map_data->GetSelectedTileLayer() == &(*tile_layers)[l]);
		// A pointer to the tile image that should be drawn
		const QPixmap* tile_image = nullptr;
		// Coordiantes to draw the current tile to
		uint32 draw_x = 0, draw_y = 0;

		if (layer_properties[l].IsVisible() == false)
			continue;

		for (uint32 x = 0; x < _map_data->GetMapLength(); ++x) {
			for (uint32 y = 0; y < _map_data->GetMapHeight(); ++y) {
				draw_x = x * TILE_LENGTH;
				draw_y = y * TILE_HEIGHT;
				tile = (*tile_layers)[l].GetTile(x, y);
				inherited_tile = (tile == INHERITED_TILE);
				if (inherited_tile == true) {
					tile = (*inherited_tile_layers)[l].GetTile(x, y);
				}
				tile_image = _RetrieveTileImage(tile);
				if (tile_image) {
					addPixmap(*tile_image)->setPos(draw_x, draw_y);
				}

				if (selected_layer == true) {
					// Draw the missing overlay if needed
					if (inherited_tile == false && tile == MISSING_TILE && _missing_overlay_visible == true) {
						addPixmap(_missing_tile)->setPos(draw_x, draw_y);
					}
					// Draw the inherited overlay over the inherited tile
					else if (inherited_tile == true && _inherited_overlay_visible == true) {
						addPixmap(_inherited_tile)->setPos(draw_x, draw_y);
					}

					// Draw the preview layer and overlay if it contains a tile at this location
					tile = _preview_layer.GetTile(x, y);
					if (tile >= 0) {
						tile_image = _RetrieveTileImage(tile);
						addPixmap(*tile_image)->setPos(draw_x, draw_y);
						addPixmap(_preview_tile)->setPos(draw_x, draw_y);
					}
				}
			}
		}
	}

	if (_selection_area_active == true)
		_DrawSelectionArea();

	if (_grid_visible)
		_DrawGrid();

	// Finally, draw the borders of the map in a red outline
	QPen pen;
	pen.setColor(Qt::red);
	addLine(0, 0, _map_data->GetMapLength() * TILE_LENGTH, 0, pen);
	addLine(0, _map_data->GetMapHeight() * TILE_HEIGHT, _map_data->GetMapLength() * TILE_LENGTH, _map_data->GetMapHeight() * TILE_HEIGHT, pen);
	addLine(0, 0, 0, _map_data->GetMapHeight() * TILE_HEIGHT, pen);
	addLine(_map_data->GetMapLength() * TILE_LENGTH, 0, _map_data->GetMapLength() * TILE_LENGTH, _map_data->GetMapHeight() * TILE_HEIGHT, pen);
} // void MapView::DrawMap()



void MapView::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (_map_data->IsInitialized() == false)
		return;

	// Don't allow edits to the selected layer if it's not visible
	if (_map_data->GetSelectedTileLayerProperties()->IsVisible() == false)
		return;

	// Takes into account the current scrolling
	int32 x = event->scenePos().x();
	int32 y = event->scenePos().y();

	// Ensure that the coordinates map to a valid tile x and y coordinate
	if (x < 0 || (x / TILE_LENGTH) >= static_cast<uint32>(_map_data->GetMapLength()) ||
		y < 0 || (y / TILE_HEIGHT) >= static_cast<uint32>(_map_data->GetMapHeight())) {
		return;
	}

	// Determine the coordinates of the tile that was clicked during the mouse press
	_press_tile_x = x / TILE_LENGTH;
	_press_tile_y = y / TILE_HEIGHT;
	_cursor_tile_x = _press_tile_x;
	_cursor_tile_y = _press_tile_y;

	if (event->button() != Qt::LeftButton)
		return;

	_selection_area_press = false;
	if (_selection_area_active == true) {
		_selection_area_press = (_selection_area.GetTile(_press_tile_x, _press_tile_y) == SELECTED_TILE);
	}

	// Process the press event according to the active edit mode
	switch (_edit_mode) {
		case PAINT_MODE:
			_PaintTiles(_cursor_tile_x, _cursor_tile_y, false);
			_map_data->SetMapModified(true);
			DrawMap();
			break;

		case SWAP_MODE:
			break;

		case ERASE_MODE:
			_SetTile(_cursor_tile_x, _cursor_tile_y, MISSING_TILE);
			_map_data->SetMapModified(true);
			DrawMap();
			break;

		case INHERIT_MODE:
			_SetTile(_cursor_tile_x, _cursor_tile_y, INHERITED_TILE);
			_map_data->SetMapModified(true);
			DrawMap();
			break;

		case SELECT_AREA_MODE: {
			if (event->modifiers() & Qt::ShiftModifier) {
				_selection_mode = ADDITIVE;
			}
			else if (event->modifiers() & Qt::ControlModifier) {
				_selection_mode = SUBTRACTIVE;
			}
			else {
				_selection_mode = NORMAL;
				SelectNoTiles();
				_selection_area.SetTile(_press_tile_x, _press_tile_y, SELECTED_TILE);
				_selection_area_active = true;
			}
			DrawMap();
			break;
		}

		case FILL_AREA_MODE:
			_FillArea(_cursor_tile_x, _cursor_tile_y, _RetrieveCurrentTileValue());
			DrawMap();
			break;

		case CLEAR_AREA_MODE:
			_FillArea(_cursor_tile_x, _cursor_tile_y, MISSING_TILE);
			DrawMap();
			break;

		case INHERIT_AREA_MODE:
			_FillArea(_cursor_tile_x, _cursor_tile_y, INHERITED_TILE);
			DrawMap();
			break;

		default:
			QMessageBox::warning(_graphics_view, "Tile editing mode", "ERROR: Invalid tile editing mode");
			break;
	}

	_UpdateStatusBar(event);
} // void MapView::mousePressEvent(QGraphicsSceneMouseEvent* event)



void MapView::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	if (_map_data->IsInitialized() == false)
		return;

	int32 mouse_x = event->scenePos().x();
	int32 mouse_y = event->scenePos().y();

	// Ensure that the coordinates map to a valid tile x and y coordinate
	if (mouse_x < 0 || (static_cast<uint32>(mouse_x) / TILE_LENGTH) >= _map_data->GetMapLength() ||
		mouse_y < 0 || (static_cast<uint32>(mouse_y) / TILE_HEIGHT) >= _map_data->GetMapHeight()) {
		return;
	}

	// Determine the tile that maps to the mouse coordinates
	int32 tile_x = mouse_x / TILE_LENGTH;
	int32 tile_y = mouse_y / TILE_HEIGHT;

	// Don't allow edits to the selected layer if it's not visible
	if (_map_data->GetSelectedTileLayerProperties()->IsVisible() == false) {
		_UpdateStatusBar(event);
	}

	// Check if the user has moved the cursor over a different tile
	if (tile_x != _cursor_tile_x || tile_y != _cursor_tile_y) {
		_cursor_tile_x = tile_x;
		_cursor_tile_y = tile_y;

		if (event->buttons() == Qt::LeftButton) {
			switch (_edit_mode) {
				case PAINT_MODE:
					_PaintTiles(_cursor_tile_x, _cursor_tile_y, false);
					DrawMap();
					break;

				case SWAP_MODE:
					break;

				case ERASE_MODE: {
					if (_IsTileEqualToPressSelection(_cursor_tile_x, _cursor_tile_y) == true) {
						_SetTile(_cursor_tile_x, _cursor_tile_y, MISSING_TILE);
						DrawMap();
					}
					break;
				}

				case INHERIT_MODE: {
					if (_IsTileEqualToPressSelection(_cursor_tile_x, _cursor_tile_y) == true) {
						_SetTile(_cursor_tile_x, _cursor_tile_y, INHERITED_TILE);
						DrawMap();
					}
					break;
				}

				case SELECT_AREA_MODE: {
					if (_selection_mode == NORMAL) {
						_SetSelectionArea(_press_tile_x, _press_tile_y, _cursor_tile_x, _cursor_tile_y);
					}
					DrawMap();
					break;
				}

				case FILL_AREA_MODE:
					_FillArea(_cursor_tile_x, _cursor_tile_y, _RetrieveCurrentTileValue());
					DrawMap();
					break;

				case CLEAR_AREA_MODE:
					_FillArea(_cursor_tile_x, _cursor_tile_y, MISSING_TILE);
					DrawMap();
					break;

				case INHERIT_AREA_MODE:
					_FillArea(_cursor_tile_x, _cursor_tile_y, INHERITED_TILE);
					DrawMap();
					break;

				default:
					break;
			}
		}
		else if (_edit_mode == PAINT_MODE) {
			// Paint the preview layer as the mouse is moved around
			_PaintTiles(_cursor_tile_x, _cursor_tile_y, true);
			DrawMap();
		}
	}

	_UpdateStatusBar(event);
} // void MapView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)



void MapView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	if (_map_data->IsInitialized() == false)
		return;

	// Don't allow edits to the selected layer if it's not visible
	if (_map_data->GetSelectedTileLayerProperties()->IsVisible() == false)
		return;

	int32 mouse_x = event->scenePos().x();
	int32 mouse_y = event->scenePos().y();
	_cursor_tile_x = mouse_x / TILE_LENGTH;
	_cursor_tile_y = mouse_y / TILE_HEIGHT;

	switch (_edit_mode) {
		case PAINT_MODE: {
			_preview_layer.ClearLayer();
			DrawMap();
			break;
		}

		case SWAP_MODE: {
			_SwapTiles(_press_tile_x, _press_tile_y, _cursor_tile_x, _cursor_tile_y);
			DrawMap();
			break;
		}

		case ERASE_MODE: {
			break;
		}

		case INHERIT_MODE: {
			break;
		}

		case SELECT_AREA_MODE: {
			// If only a single tile was selected in normal mode, deselect the area
			if (_selection_mode == NORMAL && _cursor_tile_x == _press_tile_x && _cursor_tile_y == _press_tile_y) {
				SelectNoTiles();
				DrawMap();
			}
			else {
				_SetSelectionArea(_press_tile_x, _press_tile_y, _cursor_tile_x, _cursor_tile_y);
			}
			break;
		}

		case FILL_AREA_MODE:
			break;

		case CLEAR_AREA_MODE:
			break;

		case INHERIT_AREA_MODE:
			break;

		default:
			QMessageBox::warning(_graphics_view, "Tile editing mode", "ERROR: Invalid tile editing mode!");
	} // switch (_edit_mode)

	_UpdateStatusBar(event);
} // void MapView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)



void MapView::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	if (_map_data->IsInitialized() == false) {
		// Show the menu, but disable all options
		_insert_single_row_action->setEnabled(false);
		_insert_multiple_rows_action->setEnabled(false);
		_insert_single_column_action->setEnabled(false);
		_insert_multiple_columns_action->setEnabled(false);
		_delete_single_row_action->setEnabled(false);
		_delete_multiple_rows_action->setEnabled(false);
		_delete_single_column_action->setEnabled(false);
		_delete_multiple_columns_action->setEnabled(false);
		_selection_move_to_layer_menu->setEnabled(false);
		_selection_copy_to_layer_menu->setEnabled(false);
		_selection_move_to_context_menu->setEnabled(false);
		_selection_copy_to_context_menu->setEnabled(false);
		_right_click_menu->exec(QCursor::pos());
		return;
	}

	// We could check the map size here to see if an insert or delete operation is possible or not. We leave these options enabled because
	// we don't want to confuse the user as to why these options would suddenly be disabled. Instead, the slot functions for these actions
	// do the check and if they find it to be invalid, they'll present a warning dialog window to the user.
	_insert_single_row_action->setEnabled(true);
	_insert_multiple_rows_action->setEnabled(true);
	_insert_single_column_action->setEnabled(true);
	_insert_multiple_columns_action->setEnabled(true);
	_delete_single_row_action->setEnabled(true);
	_delete_multiple_rows_action->setEnabled(true);
	_delete_single_column_action->setEnabled(true);
	_delete_multiple_columns_action->setEnabled(true);
	if (_selection_area_active == false) {
		_selection_move_to_layer_menu->setEnabled(false);
		_selection_copy_to_layer_menu->setEnabled(false);
		_selection_move_to_context_menu->setEnabled(false);
		_selection_copy_to_context_menu->setEnabled(false);
	}
	else {
		_selection_move_to_layer_menu->setEnabled(true);
		_selection_copy_to_layer_menu->setEnabled(true);
		_selection_move_to_context_menu->setEnabled(true);
		_selection_copy_to_context_menu->setEnabled(true);

		// Go through all layer and context actions. The action corresponding to the selected layer or context needs to be disabled
		TileContext* selected_context = _map_data->GetSelectedTileContext();
		TileLayer* selected_layer = _map_data->GetSelectedTileLayer();
		for (uint32 i = 0; i < _map_data->GetTileLayerCount(); ++i) {
			if (selected_layer == selected_context->GetTileLayer(i)) {
				_selection_move_to_layer_actions[i]->setEnabled(false);
				_selection_copy_to_layer_actions[i]->setEnabled(false);
			}
			else {
				_selection_move_to_layer_actions[i]->setEnabled(true);
				_selection_copy_to_layer_actions[i]->setEnabled(true);
			}
		}
		for (uint32 i = 0; i < _map_data->GetTileContextCount(); ++i) {
			if (selected_context == _map_data->FindTileContextByIndex(i)) {
				_selection_move_to_context_actions[i]->setEnabled(false);
				_selection_copy_to_context_actions[i]->setEnabled(false);
			}
			else {
				_selection_move_to_context_actions[i]->setEnabled(true);
				_selection_copy_to_context_actions[i]->setEnabled(true);
			}
		}
	}

	// Retrieve the coordinates of the right click event and translate them into tile coordinates
	int32 mouse_x = event->scenePos().x();
	int32 mouse_y = event->scenePos().y();

	// Ensure that the coordinates map to a valid tile x and y coordinate
	if (mouse_x < 0 || (static_cast<uint32>(mouse_x) / TILE_LENGTH) >= _map_data->GetMapLength() ||
		mouse_y < 0 || (static_cast<uint32>(mouse_y) / TILE_HEIGHT) >= _map_data->GetMapHeight()) {
		return;
	}

	_cursor_tile_x = mouse_x / TILE_LENGTH;
	_cursor_tile_y = mouse_y / TILE_HEIGHT;
	_right_click_menu->exec(QCursor::pos());
}



void MapView::_InsertSingleTileRow() {
	if (_map_data->GetMapHeight() == MAXIMUM_MAP_HEIGHT) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Insert Tile Row Failure",
			"Could not insert additional tile rows as the map height is currently at its maximum limit.");
		return;
	}

	_map_data->InsertTileLayerRows(_cursor_tile_y);
	UpdateAreaSizes();
	DrawMap();
}



void MapView::_InsertMultipleTileRows() {
	if (_map_data->GetMapHeight() == MAXIMUM_MAP_HEIGHT) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Insert Tile Row Failure",
			"Could not insert additional tile rows as the map height is currently at its maximum limit.");
		return;
	}

	MapResizeInternalDialog resize_dialog(_graphics_view->topLevelWidget(), _map_data, _cursor_tile_y, _cursor_tile_x, true, false);
	if (resize_dialog.exec() == QDialog::Accepted) {
		resize_dialog.ModifyMapData();
		DrawMap();
	}
}



void MapView::_InsertSingleTileColumn() {
	if (_map_data->GetMapHeight() == MAXIMUM_MAP_LENGTH) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Insert Tile Column Failure",
			"Could not insert additional tile columns as the map length is currently at its maximum limit.");
		return;
	}

	_map_data->InsertTileLayerColumns(_cursor_tile_x);
	UpdateAreaSizes();
	DrawMap();
}



void MapView::_InsertMultipleTileColumns() {
	if (_map_data->GetMapHeight() == MAXIMUM_MAP_LENGTH) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Insert Tile Column Failure",
			"Could not insert additional tile columns as the map length is currently at its maximum limit.");
		return;
	}

	MapResizeInternalDialog resize_dialog(_graphics_view->topLevelWidget(), _map_data, _cursor_tile_y, _cursor_tile_x, true, true);
	if (resize_dialog.exec() == QDialog::Accepted) {
		resize_dialog.ModifyMapData();
		DrawMap();
	}
}



void MapView::_DeleteSingleTileRow() {
	if (_map_data->GetMapHeight() == MINIMUM_MAP_HEIGHT) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Delete Tile Row Failure",
			"Could not delete any tile rows as the map height is currently at its minimum limit.");
		return;
	}

	_map_data->RemoveTileLayerRows(_cursor_tile_y);
	UpdateAreaSizes();
	DrawMap();
}



void MapView::_DeleteMultipleTileRows() {
	if (_map_data->GetMapHeight() == MINIMUM_MAP_HEIGHT) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Delete Tile Row Failure",
			"Could not delete any tile rows as the map height is currently at its minimum limit.");
		return;
	}

	MapResizeInternalDialog resize_dialog(_graphics_view->topLevelWidget(), _map_data, _cursor_tile_y, _cursor_tile_x, false, false);
	if (resize_dialog.exec() == QDialog::Accepted) {
		resize_dialog.ModifyMapData();
		DrawMap();
	}
}



void MapView::_DeleteSingleTileColumn() {
	if (_map_data->GetMapLength() == MINIMUM_MAP_LENGTH) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Delete Tile Column Failure",
			"Could not delete any tile columns as the map length is currently at its minimum limit.");
		return;
	}

	_map_data->RemoveTileLayerColumns(_cursor_tile_x);
	UpdateAreaSizes();
	DrawMap();
}



void MapView::_DeleteMultipleTileColumns() {
	if (_map_data->GetMapLength() == MINIMUM_MAP_LENGTH) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Delete Tile Column Failure",
			"Could not delete any tile columns as the map length is currently at its minimum limit.");
		return;
	}

	MapResizeInternalDialog resize_dialog(_graphics_view->topLevelWidget(), _map_data, _cursor_tile_y, _cursor_tile_x, false, true);
	if (resize_dialog.exec() == QDialog::Accepted) {
		resize_dialog.ModifyMapData();
		DrawMap();
	}
}



void MapView::_MoveSelectionToLayer(QAction* action) {
	for (uint32 i = 0; i < _selection_move_to_layer_actions.size(); ++i) {
		if (action == _selection_move_to_layer_actions[i]) {
			_SelectionToLayer(i, false);
			return;
		}
	}

	// If this box ever appears, it is almost certainly a bug in the editor code and not the result of user error
	QMessageBox::warning(_graphics_view->topLevelWidget(), "Selection Move to Layer Failure",
		"Failed to move selected tiles to requested layer.");
}



void MapView::_CopySelectionToLayer(QAction* action) {
	for (uint32 i = 0; i < _selection_copy_to_layer_actions.size(); ++i) {
		if (action == _selection_copy_to_layer_actions[i]) {
			_SelectionToLayer(i, true);
			return;
		}
	}

	// If this box ever appears, it is almost certainly a bug in the editor code and not the result of user error
	QMessageBox::warning(_graphics_view->topLevelWidget(), "Selection Copy to Layer Failure",
		"Failed to copy selected tiles to requested layer.");
}



void MapView::_MoveSelectionToContext(QAction* action) {
	for (uint32 i = 0; i < _selection_move_to_context_actions.size(); ++i) {
		if (action == _selection_move_to_context_actions[i]) {
			_SelectionToContext(i, false);
			return;
		}
	}

	// If this box ever appears, it is almost certainly a bug in the editor code and not the result of user error
	QMessageBox::warning(_graphics_view->topLevelWidget(), "Selection Move to Context Failure",
		"Failed to move selected tiles to requested context.");
}



void MapView::_CopySelectionToContext(QAction* action) {
	for (uint32 i = 0; i < _selection_copy_to_context_actions.size(); ++i) {
		if (action == _selection_copy_to_context_actions[i]) {
			_SelectionToContext(i, true);
			return;
		}
	}

	// If this box ever appears, it is almost certainly a bug in the editor code and not the result of user error
	QMessageBox::warning(_graphics_view->topLevelWidget(), "Selection Copy to Context Failure",
		"Failed to copy selected tiles to requested context.");
}



int32 MapView::_RetrieveCurrentTileValue() const {
	// Get a reference to the current tileset
	Editor* editor = static_cast<Editor*>(_graphics_view->topLevelWidget());
	TilesetTable* tileset_table = editor->GetTilesetView()->GetCurrentTilesetTable();

	// Detect the first selection range and use to paint an area
	QList<QTableWidgetSelectionRange> selections = tileset_table->selectedRanges();
	QTableWidgetSelectionRange selection;
	if (selections.size() > 0)
		selection = selections.at(0);

	// Determine the index of the current tileset in the tileset list to determine its multiplier for calculating the image index
	vector<Tileset*> all_tilesets = _map_data->GetTilesets();
	int32 multiplier = editor->GetTilesetView()->GetCurrentTilesetIndex();
	if (multiplier < 0) {
		qDebug() << "ERROR: failed to retrieve current tile value because there was no valid tileset data" << endl;
		return MISSING_TILE;
	}
	multiplier *= TILESET_NUM_TILES;

	int32 tileset_index = 0;
	if (selections.size() > 0 && (selection.columnCount() * selection.rowCount() > 1)) { // Multiple tiles are selected
		tileset_index = (selection.topRow() * 16) + selection.leftColumn();
	}
	else { // A single tile is selected
		tileset_index = tileset_table->currentRow() * TILESET_NUM_COLS + tileset_table->currentColumn();
	}
	return (tileset_index + multiplier);
}



const QPixmap* MapView::_RetrieveTileImage(int32 tile_value) const {
	if (tile_value < 0)
		return nullptr;

	vector<Tileset*>& tilesets = _map_data->GetTilesets();

	// Convert the tile_value into the index of the tileset where the image can be found and the index of the tile within that tileset
	int32 tileset_index = tile_value / TILESET_NUM_TILES;
	int32 tile_index = tile_value;
	if (tileset_index != 0) // Don't divide by zero
		tile_index = tile_value % (tileset_index * TILESET_NUM_TILES);

	return tilesets[tileset_index]->GetTileImage(tile_index);
}



void MapView::_SetTile(int32 x, int32 y, int32 value) {
	// TODO: Record information for undo/redo stack
	_map_data->GetSelectedTileLayer()->SetTile(x, y, value);
}



void MapView::_PaintTiles(uint32 x, uint32 y, bool preview) {
	// Get a reference to the current tileset
	Editor* editor = static_cast<Editor*>(_graphics_view->topLevelWidget());
	TilesetTable* tileset_table = editor->GetTilesetView()->GetCurrentTilesetTable();

	// Detect the first selection range and use to paint an area
	QList<QTableWidgetSelectionRange> selections = tileset_table->selectedRanges();
	QTableWidgetSelectionRange selection;
	if (selections.size() > 0)
		selection = selections.at(0);

	// Determine the index of the current tileset in the tileset list to determine its multiplier for calculating the image index
	vector<Tileset*> all_tilesets = _map_data->GetTilesets();
	int32 multiplier = editor->GetTilesetView()->GetCurrentTilesetIndex();
	if (multiplier < 0) {
		qDebug() << "could not paint tile at location [" << x << ", " << y << "] "
			<< "because there was no tileset data that matched the tileset in the tileset table." << endl;
		return;
	}
	multiplier *= TILESET_NUM_TILES;

	TileLayer* destination_layer;
	if (preview == false) {
		destination_layer = _map_data->GetSelectedTileLayer();
	}
	else {
		// For the preview layer, we always clear any existing tiles in the layer before painting.
		destination_layer = &_preview_layer;
		destination_layer->ClearLayer();
	}

	int32 start_tile = MISSING_TILE;
	if (_selection_area_active == true) {
		start_tile = _selection_area.GetTile(x, y);
	}

	if (selections.size() > 0 && (selection.columnCount() * selection.rowCount() > 1)) { // Multiple tiles are selected
		// Draw tiles from tileset selection onto map, one tile at a time.
		for (int32 i = 0; i < selection.rowCount() && y + i < _map_data->GetMapHeight(); i++) {
			for (int32 j = 0; j < selection.columnCount() && x + j < _map_data->GetMapLength(); j++) {
				// Skip over tiles that do not match the selection status of the first tile
				if (_selection_area_active == true && _selection_area.GetTile(x + j, y + i) != start_tile) {
					continue;
				}
				int32 tileset_index = (selection.topRow() + i) * 16 + (selection.leftColumn() + j);
				// TODO: Perform randomization for autotiles
				// _AutotileRandomize(multiplier, tileset_index);
				// TODO: Record information for undo/redo stack
				destination_layer->SetTile(x + j, y + i, tileset_index + multiplier);
			} // iterate through columns of selection
		} // iterate through rows of selection
	}
	else { // A single tile is selected
		// put selected tile from tileset into tile array at correct position
		int32 tileset_index = tileset_table->currentRow() * TILESET_NUM_COLS + tileset_table->currentColumn();

		// TODO: Perform randomization for autotiles
		// _AutotileRandomize(multiplier, tileset_index);

		// TODO: Record information for undo/redo stack

		destination_layer->SetTile(x, y, tileset_index + multiplier);
	}
}



void MapView::_SwapTiles(uint32 x1, uint32 y1, uint32 x2, uint32 y2) {
	if (x1 == x2 && y1 == y2) {
		return;
	}

	int32 xdiff = x2 - x1;
	int32 ydiff = y2 - y1;
	vector<vector<int32> >& layer = _map_data->GetSelectedTileLayer()->GetTiles();

	bool swap_multiple_tiles = (_selection_area_active == true && _selection_area.GetTile(x1, y1) == SELECTED_TILE);
	if (swap_multiple_tiles == false) {
		// TODO: Record information for undo/redo stack
		int32 temp = layer[_cursor_tile_y][_cursor_tile_x];
		layer[_cursor_tile_y][_cursor_tile_x] = layer[_press_tile_y][_press_tile_x];
		layer[_press_tile_y][_press_tile_x] = temp;
	}
	else {
		// The x and y order in which we update the tiles needs to coincide with the direction of the swap movement.
		// If the swap operation moves left, then the tiles that need to be swapped should start with the left. Otherwise
		// we will end up swapping a single tile location multiple times and not get the result that we want.
		uint32 xstart, ystart;
		uint32 xlast, ylast;
		uint32 xincrement, yincrement;
		if (xdiff <= 0) {
			xstart = 0;
			xlast = _map_data->GetMapLength() - 1;
			xincrement = 1;
		}
		else {
			xstart = _map_data->GetMapLength() - 1;
			xlast = 0;
			xincrement = -1;
		}
		if (ydiff <= 0) {
			ystart = 0;
			ylast = _map_data->GetMapHeight() - 1;
			yincrement = 1;
		}
		else {
			ystart = _map_data->GetMapHeight() - 1;
			ylast = 0;
			yincrement = -1;
		}

		vector<vector<int32> >& select_layer = _selection_area.GetTiles();
		uint32 y = ystart;
		uint32 x = xstart;
		while (true) {
			while (true) {
				if (select_layer[y][x] != MISSING_TILE) {
					int32 swapx = x + static_cast<int32>(xdiff);
					int32 swapy = y + static_cast<int32>(ydiff);
					// Make sure we're not going to access any tile locations that are beyond the bounds of the map
					if (swapy < 0 || swapy >= static_cast<int32>(_map_data->GetMapHeight()) || swapx < 0 || swapx >= static_cast<int32>(_map_data->GetMapLength())) {
						continue;
					}

					// TODO: Record information for undo/redo stack
					int32 temp = layer[swapy][swapx];
					layer[swapy][swapx] = layer[y][x];
					layer[y][x] = temp;
				}

				x += xincrement;
				if (x == xlast)
					break;
			}

			y += yincrement;
			if (y == ylast) {
				break;
			}

			x = xstart;
		}
	}

	_map_data->SetMapModified(true);
} // void MapView::_SwapTiles(uint32 x1, uint32 y1, uint32 x2, uint32 y2)



void MapView::_FillArea(uint32 start_x, uint32 start_y, int32 value) {
	if (start_x >= _map_data->GetMapLength() || start_y >= _map_data->GetMapHeight()) {
		return;
	}

	// A pointer to the layer that the fill operation will take effect on
	TileLayer* layer = _map_data->GetSelectedTileLayer();
	// The coordinates of the node actively being processed
	uint32 x, y;
	// The left and right nodes that end the current segement
	uint32 x_left_end, x_right_end;
	// Queue that holds the nodes that need to be checked (x, y cooridnate pairs)
	queue<pair<uint32, uint32> > nodes;
	nodes.push(make_pair(start_x, start_y));

	// This function is an implementation of a flood fill algorithm. Generally speaking, the algorithm does the following:
	//   1) Maintain a queue of nodes (x,y coordinates) that need to be examined
	//   2) For each node in the queue, if it still needs to be set to the new value then
	//      3) Find the left and right ends of the segment of all tiles that match the old value
	//      4) For each point in the segement, set it to the new value and examine the top and bottom points
	//      5) If the top or bottom point matches the old value, add it to the nodes queue
	//   6) Repeat this process until the queue is empty
	if (_selection_area.GetTile(start_x, start_y) == MISSING_TILE) {
		int32 original_value = layer->GetTile(start_x, start_y);
		if (original_value == value) {
			return;
		}

		while (nodes.empty() == false) {
			x = nodes.front().first;
			y = nodes.front().second;
			nodes.pop();
			// In this case, the node has already been set to the new value previously so we can skip over it
			if (layer->GetTile(x, y) != original_value || _selection_area.GetTile(x, y) == SELECTED_TILE) {
				continue;
			}

			// Find the left and right ends of the current line segment in row y
			x_left_end = x;
			while ((x_left_end > 0) && (layer->GetTile(x_left_end - 1, y) == original_value) &&
				(_selection_area.GetTile(x_left_end - 1, y) != SELECTED_TILE))
			{
				x_left_end--;
			}
			x_right_end = x;
			while ((x_right_end < _map_data->GetMapLength() - 1) && (layer->GetTile(x_right_end + 1, y) == original_value) &&
				(_selection_area.GetTile(x_right_end + 1, y) != SELECTED_TILE))
			{
				x_right_end++;
			}
			// Go through the segment and set the values of each node, adding the element to the top and bottom to the nodes queue if necessary
			for (uint32 i = x_left_end; i <= x_right_end; ++i) {
				layer->SetTile(i, y, value);
				if ((y > 0) && (layer->GetTile(i, y - 1) == original_value)) {
					nodes.push(make_pair(i, y - 1));
				}
				if ((y < _map_data->GetMapHeight() - 1) && (layer->GetTile(i, y + 1) == original_value)) {
					nodes.push(make_pair(i, y + 1));
				}
			}
		}
	}
	// In this case, the fill operation needs to take place within the selection area. All tiles within the area should be set regardless
	// of their value. To mark nodes as set, we create a copy of the selection area and set visited nodes to MISSING_TILE. Otherwise this is
	// precisely the same algorithm as the other case.
	else {
		TileLayer fill_area = _selection_area;
		while (nodes.empty() == false) {
			x = nodes.front().first;
			y = nodes.front().second;
			nodes.pop();
			// In this case, the node has either already been visited or is not a part of the selection area
			if (fill_area.GetTile(x, y) != SELECTED_TILE) {
				continue;
			}

			// Find the left and right ends of the current line segment in row y
			x_left_end = x;
			while ((x_left_end > 0) && (fill_area.GetTile(x_left_end - 1, y) == SELECTED_TILE)) {
				x_left_end--;
			}
			x_right_end = x;
			while ((x_right_end < _map_data->GetMapLength() - 1) && (fill_area.GetTile(x_right_end + 1, y) == SELECTED_TILE)) {
				x_right_end++;
			}
			// Go through the segment and set the values of each node, adding the element to the top and bottom to the nodes queue if necessary
			for (uint32 i = x_left_end; i <= x_right_end; ++i) {
				layer->SetTile(i, y, value);
				fill_area.SetTile(i, y, MISSING_TILE);
				if ((y > 0) && (fill_area.GetTile(i, y - 1) == SELECTED_TILE)) {
					nodes.push(make_pair(i, y - 1));
				}
				if ((y < _map_data->GetMapHeight() - 1) && (fill_area.GetTile(i, y + 1) == SELECTED_TILE)) {
					nodes.push(make_pair(i, y + 1));
				}
			}
		}
	}

	_map_data->SetMapModified(true);
} // void MapView::_FillArea(uint32 start_x, uint32 start_y, int32 value)



void MapView::_SetSelectionArea(uint32 x1, uint32 y1, uint32 x2, uint32 y2) {
	if (x1 >= _map_data->GetMapLength() || x2 >= _map_data->GetMapLength() || y1 >= _map_data->GetMapHeight() || y2 >= _map_data->GetMapHeight()) {
		qDebug("ERROR: Called MapView::_SetSelectionArea with coordinates that exceeded map boundaries");
		return;
	}

	// We ignore subtractive selections when there are already no selected tiles
	if (_selection_mode == SUBTRACTIVE && _selection_area_active == false) {
		return;
	}

	uint32 xmin = x1;
	uint32 xmax = x2;
	uint32 ymin = y1;
	uint32 ymax = y2;
	if (x2 < x1) {
		xmin = x2;
		xmax = x1;
	}
	if (y2 < y1) {
		ymin = y2;
		ymax = y1;
	}

	if (_selection_mode == NORMAL)
		SelectNoTiles();

	for (uint32 x = xmin; x <= xmax; ++x) {
		for (uint32 y = ymin; y <= ymax; ++y) {
			if (_selection_mode == SUBTRACTIVE)
				_selection_area.SetTile(x, y, MISSING_TILE);
			else
				_selection_area.SetTile(x, y, SELECTED_TILE);
		}
	}
	_selection_area_active = true;

	// When SUBTRACTIVE selection mode is active, we need to examine the entire area to make sure that at least one tile is still selected
	// We also want to check if we need to update the selection area boundaries, so set them to the ends of the map
	if (_selection_mode == SUBTRACTIVE) {
		_selection_area_active = false;
		_selection_area_left = _selection_area.GetLength();
		_selection_area_right = 0;
		_selection_area_top = _selection_area.GetHeight();
		_selection_area_bottom = 0;

		for (uint32 x = 0; x < _selection_area.GetLength(); ++x) {
			for (uint32 y = 0; y < _selection_area.GetHeight(); ++y) {
				if (_selection_area.GetTile(x, y) == SELECTED_TILE) {
					_selection_area_active = true;
					if (x < _selection_area_left)
						_selection_area_left = x;
					if (x > _selection_area_right)
						_selection_area_right = x;
					if (y < _selection_area_top)
						_selection_area_top = y;
					if (y > _selection_area_bottom)
						_selection_area_bottom = y;
				}
			}
		}
	}
	// Otherwise this is a normal or additive selection mode, and we can simply check if the coordinate arguments are the new min/max values
	else {
		if (x1 < _selection_area_left)
			_selection_area_left = x1;
		if (x2 < _selection_area_left)
			_selection_area_left = x2;
		if (x1 > _selection_area_right)
			_selection_area_right = x1;
		if (x2 > _selection_area_right)
			_selection_area_right = x2;

		if (y1 < _selection_area_top)
			_selection_area_top = y1;
		if (y2 < _selection_area_top)
			_selection_area_top = y2;
		if (y1 > _selection_area_bottom)
			_selection_area_bottom = y1;
		if (y2 > _selection_area_bottom)
			_selection_area_bottom = y2;
	}
}



bool MapView::_IsTileEqualToPressSelection(uint32 x, uint32 y) {
	if (_selection_area_active == false)
		return true;

	if (_selection_area_press == true && _selection_area.GetTile(x, y) == SELECTED_TILE)
		return true;
	else if (_selection_area_press == false && _selection_area.GetTile(x, y) != SELECTED_TILE)
		return true;
	else
		return false;
}



void MapView::_SelectionToLayer(uint32 layer_id, bool copy_or_move) {
	TileContext* context = _map_data->GetSelectedTileContext();
	TileLayer* source_layer = _map_data->GetSelectedTileLayer();
	TileLayer* destination_layer = context->GetTileLayer(layer_id);

	for (uint32 x = 0; x < _map_data->GetMapLength(); ++x) {
		for (uint32 y = 0; y < _map_data->GetMapHeight(); ++y) {
			if (_selection_area.GetTile(x, y) == SELECTED_TILE) {
				destination_layer->SetTile(x, y, source_layer->GetTile(x, y));

				if (copy_or_move == false) {
					source_layer->SetTile(x, y, MISSING_TILE);
				}
			}
		}
	}

	DrawMap();
}



void MapView::_SelectionToContext(uint32 layer_id, bool copy_or_move) {
	TileContext* source_context = _map_data->GetSelectedTileContext();
	TileContext* destination_context = _map_data->FindTileContextByIndex(layer_id);
	TileLayer* source_layer = _map_data->GetSelectedTileLayer();
	TileLayer* destination_layer = nullptr;

	// Find the destination_layer by determining the index of the source layer in the source context
	for (uint32 i = 0; i < _map_data->GetTileLayerCount(); ++i) {
		if (source_context->GetTileLayer(i) == source_layer) {
			destination_layer = destination_context->GetTileLayer(i);
			break;
		}
	}

	// Used to determine if we're moving any INHERITED_TILE tiles to a non-inheriting context
	bool inherited_tiles_nullified = false;

	for (uint32 x = 0; x < _map_data->GetMapLength(); ++x) {
		for (uint32 y = 0; y < _map_data->GetMapHeight(); ++y) {
			if (_selection_area.GetTile(x, y) == SELECTED_TILE) {
				if (source_layer->GetTile(x, y) == INHERITED_TILE && destination_context->IsInheritingContext() == false) {
					destination_layer->SetTile(x, y, MISSING_TILE);
					inherited_tiles_nullified = true;
				}
				else {
					destination_layer->SetTile(x, y, source_layer->GetTile(x, y));
				}


				if (copy_or_move == false) {
					source_layer->SetTile(x, y, MISSING_TILE);
				}
			}
		}
	}

	if (inherited_tiles_nullified == true) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Inherited Tiles Not Supported",
			QString("The destination context is not an inheriting context and the selected tiles contained inherited tiles. ") +
			QString("These tiles were set to no tile in the destination context."));
	}

	DrawMap();
}



void MapView::_UpdateStatusBar(QGraphicsSceneMouseEvent* event) {
	Editor* editor = static_cast<Editor*>(_graphics_view->topLevelWidget());

	int32 mouse_x = event->scenePos().x();
	int32 mouse_y = event->scenePos().y();

	// Determine the tile that maps to the mouse coordinates
	int32 tile_x = mouse_x / TILE_LENGTH;
	int32 tile_y = mouse_y / TILE_HEIGHT;

	// Display the mouse position coordinates and the tile that the position corresponds to.
	// Note that the position coordinates are in units of the collision grid, not the tile grid.
	QString text = QString("Tile: [%1,  %2]").arg(tile_x).arg(tile_y);
	text.append(QString(" -- Position: [%1,  %2]").arg(event->scenePos().x() * 2 / TILE_LENGTH, 0, 'f', 2)
		.arg(event->scenePos().y() * 2 / TILE_HEIGHT, 0, 'f', 2));
	// If an area of the map is selected, display those dimensions as well
	if (_selection_area_active == true) {
		// Multiply the bounds by two to convert the coordinates from the tile grid to the collision grid
		text.append(QString(" -- Selection: [%1/%2, %3/%4]").arg(_selection_area_left * 2).arg(_selection_area_right * 2 + 2)
			.arg(_selection_area_top * 2).arg(_selection_area_bottom * 2 + 2));
	}
	editor->statusBar()->showMessage(text);
}



void MapView::_DrawSelectionArea() {
	// Start by determining the bounds of the area currently being selected by the user. This information is
	// necessary for ADDITIVE or SUBTRACTIVE selection modes, but not for normal mode.
	uint32 xmin = _press_tile_x;
	uint32 ymin = _press_tile_y;
	uint32 xmax = _cursor_tile_x;
	uint32 ymax = _cursor_tile_y;
	if (xmax < xmin) {
		xmin = _cursor_tile_x;
		xmax = _press_tile_x;
	}
	if (ymax < ymin) {
		ymin = _cursor_tile_y;
		ymax = _press_tile_y;
	}

	// In these two cases, we can simply draw the set selected area and return
	if (_edit_mode != SELECT_AREA_MODE || (_edit_mode == SELECT_AREA_MODE && _selection_mode == NORMAL)) {
		for (uint32 x = 0; x < _map_data->GetMapLength(); ++x) {
			for (uint32 y = 0; y < _map_data->GetMapHeight(); ++y) {
				if (_selection_area.GetTile(x, y) == SELECTED_TILE) {
					addPixmap(_selection_tile)->setPos(x * TILE_LENGTH, y * TILE_HEIGHT);
				}
			}
		}
		return;
	}
	// Additive mode requires us to potentially draw more selection tiles than what is already set in the selection area
	else if (_selection_mode == ADDITIVE) {
		for (uint32 x = 0; x < _map_data->GetMapLength(); ++x) {
			for (uint32 y = 0; y < _map_data->GetMapHeight(); ++y) {
				if (_selection_area.GetTile(x, y) == SELECTED_TILE) {
					addPixmap(_selection_tile)->setPos(x * TILE_LENGTH, y * TILE_HEIGHT);
				}
				// Determine if this tile is within the current additive selection area
				else if (x >= xmin && x <= xmax && y >= ymin && y <= ymax) {
					addPixmap(_selection_tile)->setPos(x * TILE_LENGTH, y * TILE_HEIGHT);
				}
			}
		}
	}
	// Subtractive mode forces us to ignore tiles set in the selection area if they overlap with the active selection box
	else if (_selection_mode == SUBTRACTIVE) {
		for (uint32 x = 0; x < _map_data->GetMapLength(); ++x) {
			for (uint32 y = 0; y < _map_data->GetMapHeight(); ++y) {
				// Skip this tile if it is within the current subtractive selection area
				if (x >= xmin && x <= xmax && y >= ymin && y <= ymax) {
					continue;
				}
				else if (_selection_area.GetTile(x, y) == SELECTED_TILE) {
					addPixmap(_selection_tile)->setPos(x * TILE_LENGTH, y * TILE_HEIGHT);
				}
			}
		}
	}
	else {
		qDebug("ERROR: unknown case reached in _DrawSelectionArea()");
	}
} // void MapView::_DrawSelectionArea()



void MapView::_DrawGrid() {
	QPen grid_pen(Qt::black);

	for (uint32 x = 0; x < (_map_data->GetMapLength() * TILE_LENGTH); x+= TILE_LENGTH) {
		for (uint32 y = 0; y < (_map_data->GetMapHeight() * TILE_HEIGHT); y+= TILE_HEIGHT) {
			addLine(0, y, _map_data->GetMapLength() * TILE_LENGTH, y, grid_pen);
			addLine(x, 0, x, _map_data->GetMapHeight() * TILE_HEIGHT, grid_pen);
		}
	}
}

} // namespace hoa_editor
