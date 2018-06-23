///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    tile_layer.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for tile layer data and view classes
*** **************************************************************************/

#include <QDebug>
#include <QMouseEvent>

#include "editor_utils.h"
#include "editor.h"
#include "map_data.h"
#include "tile_layer.h"

using namespace std;

namespace hoa_editor {

///////////////////////////////////////////////////////////////////////////////
// TileLayer class
///////////////////////////////////////////////////////////////////////////////

int32 TileLayer::GetTile(uint32 x, uint32 y) const {
	// Make sure that there is at least one column and one row
	if (_tiles.empty() == true)
		return MISSING_TILE;
	if (_tiles[0].empty() == true)
		return MISSING_TILE;

	// Check that the coordinates do not exceed the bounds of the layer
	if (y >= _tiles.size())
		return MISSING_TILE;
	if (x >= _tiles[0].size())
		return MISSING_TILE;

	return _tiles[y][x];
}



void TileLayer::SetTile(uint32 x, uint32 y, int32 value) {
	// Make sure that there is at least one column and one row
	if (_tiles.empty() == true)
		return;
	if (_tiles[0].empty() == true)
		return;

	// Check that the coordinates do not exceed the bounds of the layer
	if (y >= _tiles.size())
		return;
	if (x >= _tiles[0].size())
		return;

	_tiles[y][x] = value;
}



void TileLayer::ReplaceTiles(int32 old_tile, int32 new_tile) {
	for (uint32 y = 0; y < GetHeight(); ++y) {
		for (uint32 x = 0; x < GetLength(); ++x) {
			if (_tiles[y][x] == old_tile) {
				_tiles[y][x] = new_tile;
			}
		}
	}
}



void TileLayer::FillLayer(int32 value) {
	for (uint32 y = 0; y < GetHeight(); ++y) {
		for (uint32 x = 0; x < GetLength(); ++x) {
			_tiles[y][x] = value;
		}
	}
}



void TileLayer::ResizeLayer(uint32 length, uint height) {
	_tiles.resize(height, vector<int32>(length, MISSING_TILE));
	for (uint32 y = 0; y < height; ++y) {
		_tiles[y].resize(length, MISSING_TILE);
	}
}



void TileLayer::_AddRows(uint32 row_index, uint32 row_count, int32 value) {
	if (row_count == 0) {
		return;
	}
	if (row_index > GetHeight()) {
		return;
	}

	// Add new rows to the bottom of the layer to allocate the necessary space
	for (uint32 i = 0; i < row_count; ++i) {
		_tiles.push_back(vector<int32>(GetLength()));
	}
	// Shift existing rows down the appropriate amount
	for (uint32 i = GetHeight() - 1; i >= row_index + row_count; --i) {
		_tiles[i] = _tiles[i - row_count];
	}
	// Now set the value of the rows in their new positions
	for (uint32 i = row_index; i < row_index + row_count; ++i) {
		_tiles[i].assign(GetLength(), value);
	}
}



void TileLayer::_AddColumns(uint32 col_index, uint32 col_count, int32 value) {
	if (col_count == 0) {
		return;
	}
	if (col_index > GetLength()) {
		return;
	}

	for (uint32 i = 0; i < GetHeight(); ++i) {
		// Add new columns to the right of the layer to allocate the necessary space
		for (uint32 j = 0; j < col_count; ++j) {
			_tiles[i].push_back(value);
		}
		// Shift existing rows right the appropriate amount
		for (uint32 j = GetLength() - 1; j >= col_index + col_count; --j) {
			_tiles[i][j] = _tiles[i][j - col_count];
		}
		// Now set the value of the columns in their new positions
		for (uint32 j = col_index; j < col_index + col_count; ++j) {
			_tiles[i][j] = value;
		}
	}
}



void TileLayer::_DeleteRows(uint32 row_index, uint32 row_count) {
	if (row_count == 0) {
		return;
	}
	if (row_index + row_count > GetHeight()) {
		return;
	}

	// Move up all rows beyond row_index + row_count
	for (uint32 i = row_index + row_count; i < GetHeight(); ++i) {
		_tiles[i - row_count] = _tiles[i];
	}

	// Now remove the rows from the end
	for (uint32 i = 0; i < row_count; ++i) {
		_tiles.pop_back();
	}
}



void TileLayer::_DeleteColumns(uint32 col_index, uint32 col_count) {
	if (col_count == 0) {
		return;
	}
	if (col_index + col_count > GetLength()) {
		return;
	}

	uint32 original_length = GetLength();
	for (uint32 i = 0; i < GetHeight(); ++i) {
		// Move left all columns beyond col_index + col_count
		for (uint32 j = col_index + col_count; j < original_length; ++j) {
			_tiles[i][j - col_count] = _tiles[i][j];
		}

		// Now remove the columns from the end
		for (uint32 j = 0; j < col_count; ++j) {
			_tiles[i].pop_back();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// LayerView class
///////////////////////////////////////////////////////////////////////////////

const uint32 LayerView::ID_COLUMN;
const uint32 LayerView::VISIBLE_COLUMN;
const uint32 LayerView::NAME_COLUMN;
const uint32 LayerView::COLLISION_COLUMN;

LayerView::LayerView(MapData* data) :
	QTreeWidget(),
	_map_data(data),
	_original_layer_name(),
	_visibility_icon(QString("img/misc/editor_tools/eye.png")),
	_right_click_item(nullptr),
	_right_click_menu(nullptr),
	_add_layer_action(nullptr),
	_clone_layer_action(nullptr),
	_rename_layer_action(nullptr),
	_delete_layer_action(nullptr)
{
	if (data == nullptr) {
		qDebug() << "constructor received nullptr map data argument" << endl;
		return;
	}

	// Enable settings so that layers can be dragged and reordered
	setSelectionMode(QAbstractItemView::SingleSelection);
	setDragEnabled(true);
	viewport()->setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(QAbstractItemView::InternalMove);

	// Create column dimensions, headers, and properties
    setColumnCount(4);
	hideColumn(ID_COLUMN); // Hide the ID column as we only use it internally
	setColumnWidth(VISIBLE_COLUMN, 25); // Make this column small as it only contains the eye icon
	setColumnWidth(NAME_COLUMN, 200);
	QStringList layer_headers;
	layer_headers << "ID" << "" << "Layer" << "Collisions";
	setHeaderLabels(layer_headers);
	setIndentation(0);

	// Setup actions for the right click menu
	_add_layer_action = new QAction("Add New Layer", this);
	_add_layer_action->setStatusTip("Adds a new empty tile layer to the end of the layer list");
	_clone_layer_action = new QAction("Clone Layer", this);
	_clone_layer_action->setStatusTip("Adds a new layer that clones the data and properties of an existing layer");
	_rename_layer_action = new QAction("Rename Layer", this);
	_rename_layer_action->setStatusTip("Renames the selected layer (can also be activated by double-clicking the layer's name)");
	_delete_layer_action = new QAction("Delete Tile Layer", this);
	_delete_layer_action->setStatusTip("Deletes the selected layer");

	_right_click_menu = new QMenu(this);
	_right_click_menu->addAction(_add_layer_action);
	_right_click_menu->addAction(_clone_layer_action);
	_right_click_menu->addAction(_rename_layer_action);
	_right_click_menu->addAction(_delete_layer_action);

	// Connect all signals and slots
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(_ChangeSelectedLayer()));
	connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(_SetTileLayerName(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(_ChangeLayerProperties(QTreeWidgetItem*, int)));
	connect(_add_layer_action, SIGNAL(triggered()), this, SLOT(_AddTileLayer()));
	connect(_clone_layer_action, SIGNAL(triggered()), this, SLOT(_CloneTileLayer()));
	connect(_rename_layer_action, SIGNAL(triggered()), this, SLOT(_RenameTileLayer()));
	connect(_delete_layer_action, SIGNAL(triggered()), this, SLOT(_DeleteTileLayer()));
}



LayerView::~LayerView() {
	delete _right_click_menu;
	delete _add_layer_action;
	delete _clone_layer_action;
	delete _rename_layer_action;
	delete _delete_layer_action;
}



void LayerView::mousePressEvent(QMouseEvent* event) {
	// Handle left clicks the standard way. Right clicks bring up the layer action menu
	if (event->button() == Qt::LeftButton) {
		QTreeWidget::mousePressEvent(event);
		return;
	}

	if (_map_data->IsInitialized() == false) {
		_add_layer_action->setEnabled(false);
		_clone_layer_action->setEnabled(false);
		_rename_layer_action->setEnabled(false);
		_delete_layer_action->setEnabled(false);
	}
	else {
		// Determine which QTreeWidgetItem was selected, if any. Enable/disable menu actions appropriately
		_right_click_item = itemAt(event->pos());
		_add_layer_action->setEnabled(true);
		if (_right_click_item != nullptr) {
			_clone_layer_action->setEnabled(true);
			_rename_layer_action->setEnabled(true);
			_delete_layer_action->setEnabled(true);
		}
		else {
			// Clicked a space in the widget that did not point to any item
			_clone_layer_action->setEnabled(false);
			_rename_layer_action->setEnabled(false);
			_delete_layer_action->setEnabled(false);
		}
	}

	_right_click_menu->exec(QCursor::pos());
}



void LayerView::dropEvent(QDropEvent* event) {
	QTreeWidget::dropEvent(event);
	vector<uint32> layer_order; // Holds the new layer positions

	// Update the IDs for each tile layer to correspond to the new layer order
	QTreeWidgetItem* root = invisibleRootItem();
	for (uint32 i = 0; i < static_cast<uint32>(root->childCount()); ++i) {
		QTreeWidgetItem* child = root->child(i);
		layer_order.push_back(child->text(ID_COLUMN).toUInt());
		child->setText(ID_COLUMN, QString::number(i));
	}

	// Make the appropriate changes corresponding to the layer order in the map data
	for (uint32 i = 0; i < layer_order.size(); ++i) {
		// Skip over layers that haven't been affected by the reordering
		if (layer_order[i] == i) {
			continue;
		}

		// Find the new location of this layer and swap it with other layer
		for (uint32 j = 0; j < _map_data->GetTileLayerCount(); ++j) {
			if (layer_order[j] == i) {
				uint32 temp = layer_order[i];
				layer_order[i] = layer_order[j];
				layer_order[j] = temp;
				_map_data->SwapTileLayers(i, j);
				break;
			}
		}
	}

	Editor* editor = static_cast<Editor*>(topLevelWidget());
	editor->MapLayersModified();
	editor->DrawMapView();
}



void LayerView::RefreshView() {
	clear();

	// Add all tile layers from the map data
	vector<TileLayerProperties>& layer_properties = _map_data->GetTileLayerProperties();

	for (uint32 i = 0; i < layer_properties.size(); ++i) {
		QTreeWidgetItem* item = new QTreeWidgetItem(this);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
		item->setText(ID_COLUMN, QString::number(i));
		if (layer_properties[i].IsVisible() == true)
			item->setIcon(VISIBLE_COLUMN, _visibility_icon);
		else
			item->setIcon(VISIBLE_COLUMN, QIcon());
		item->setText(NAME_COLUMN, layer_properties[i].GetLayerName());
		item->setText(COLLISION_COLUMN, layer_properties[i].IsCollisionEnabled() ? QString("Enabled") : QString("Disabled"));
	}

	setCurrentItem(itemAt(0, 0));
}



void LayerView::_ChangeSelectedLayer() {
	// We only allow one selected layer at a time. The size of selected items should only ever be 0 or 1.
	QList<QTreeWidgetItem*> selected_items = selectedItems();
	if (selected_items.size() != 1) {
		return;
	}

	QTreeWidgetItem* selection = selected_items.first();
	uint32 layer_id = selection->text(ID_COLUMN).toUInt();
	if (_map_data->ChangeSelectedTileLayer(layer_id) == nullptr) {
		QMessageBox::warning(this, "Layer Selection Failure", _map_data->GetErrorMessage());
	}

	// Certain map overlays change depending on which layer is selected, which is why we have to update the map view here
	static_cast<Editor*>(topLevelWidget())->DrawMapView();
}



void LayerView::_ChangeLayerProperties(QTreeWidgetItem* item, int column) {
	if (item == nullptr)
		return;

	uint32 layer_id = item->text(ID_COLUMN).toUInt();
	vector<TileLayerProperties>& layer_properties = _map_data->GetTileLayerProperties();

	if (column == VISIBLE_COLUMN) {
		_map_data->ToggleTileLayerVisibility(layer_id);
		if (layer_properties[layer_id].IsVisible() == true)
			item->setIcon(VISIBLE_COLUMN, _visibility_icon);
		else
			item->setIcon(VISIBLE_COLUMN, QIcon());

		static_cast<Editor*>(topLevelWidget())->DrawMapView();
	}
	else if (column == NAME_COLUMN) {
		// While technically this was not a right-click event, this allows us to use the same code path for performing rename operations
		_right_click_item = item;
		_RenameTileLayer();
	}
	else if (column == COLLISION_COLUMN) {
		_map_data->ToggleTileLayerCollision(layer_id);
		item->setText(COLLISION_COLUMN, layer_properties[layer_id].IsCollisionEnabled() ? QString("Enabled") : QString("Disabled"));
	}
	else {
		QMessageBox::warning(this, "Layer Property Change Failure", "Invalid column clicked");
	}
}



void LayerView::_SetTileLayerName(QTreeWidgetItem* item, int column) {
	if ((item != _right_click_item) || (column != NAME_COLUMN) || (_original_layer_name.isEmpty() == true))
		return;

	closePersistentEditor(item, column);
	if (_map_data->RenameTileLayer(item->text(ID_COLUMN).toUInt(), item->text(NAME_COLUMN)) == false) {
		// To prevent an infinite recursion loop, we must nullify _right_click_item before restoring the layer's name
		_right_click_item = nullptr;
		item->setText(NAME_COLUMN, _original_layer_name);
		_original_layer_name.clear();
		QMessageBox::warning(this, "Layer Rename Failure", _map_data->GetErrorMessage());
		return;
	}

	Editor* editor = static_cast<Editor*>(topLevelWidget());
	editor->MapLayersModified();
	_original_layer_name.clear();
}



void LayerView::_AddTileLayer() {
	static uint32 new_layer_number = 1; // Used so that each new tile layer added is written as "New Layer (#)"

	// Add the new layer to the map data. If it fails, increment the number to use a different layer name and try again
	QString layer_name;
	while (true) {
		layer_name.clear();
		layer_name = "New Layer (" + QString::number(new_layer_number) + QString(")");

		if (_map_data->AddTileLayer(layer_name, true) == true) {
			_map_data->SetMapModified(true);
			break;
		}
		else {
			new_layer_number++;
		}
	}

	// Add the new item to the view. All new tile layers will have vision and collisions enableed
	QTreeWidgetItem* item = new QTreeWidgetItem(this);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
	item->setText(ID_COLUMN, QString::number(_map_data->GetTileLayerCount() - 1));
	item->setIcon(VISIBLE_COLUMN, _visibility_icon);
	item->setText(NAME_COLUMN, layer_name);
	item->setText(COLLISION_COLUMN, "Enabled");

	Editor* editor = static_cast<Editor*>(topLevelWidget());
	editor->MapLayersModified();
	setCurrentItem(item); // Select the newly added item
	new_layer_number++;
}



void LayerView::_CloneTileLayer() {
	if (_right_click_item == nullptr)
		return;

	// Clone the layer data
	uint32 layer_id = _right_click_item->text(ID_COLUMN).toUInt();
	_map_data->CloneTileLayer(layer_id);

	// Retrieve the properties of the most recently added layer and construct a new widget item with them
	TileLayerProperties* clone_properties = _map_data->GetTileLayerProperties(_map_data->GetTileLayerCount() - 1);
	QTreeWidgetItem* item = new QTreeWidgetItem(this);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
	item->setText(ID_COLUMN, QString::number(_map_data->GetTileLayerCount() - 1));
	if (clone_properties->IsVisible() == true)
		item->setIcon(VISIBLE_COLUMN, _visibility_icon);
	else
		item->setIcon(VISIBLE_COLUMN, QIcon());
	item->setText(NAME_COLUMN, clone_properties->GetLayerName());
	item->setText(COLLISION_COLUMN, clone_properties->IsCollisionEnabled() ? QString("Enabled") : QString("Disabled"));

	Editor* editor = static_cast<Editor*>(topLevelWidget());
	editor->MapLayersModified();
	setCurrentItem(item);
}



void LayerView::_RenameTileLayer() {
	if (_right_click_item == nullptr)
		return;

	_original_layer_name = _right_click_item->text(NAME_COLUMN);
	openPersistentEditor(_right_click_item, NAME_COLUMN);
}



void LayerView::_DeleteTileLayer() {
	if (_right_click_item == nullptr)
		return;

	if (_map_data->GetTileLayerCount() == 1) {
		QMessageBox::warning(this, "Layer Deletion Failure", "You may not delete the last remaining layer for a map.");
		return;
	}

	uint32 layer_index = static_cast<uint32>(indexOfTopLevelItem(_right_click_item));
	QString layer_name = _map_data->GetTileLayerProperties().at(layer_index).GetLayerName();
	QString warning_text = "Deleting a tile layer from the map will delete the layer from all map contexts.";
	warning_text = warning_text.append(" Are you sure that you wish to proceed with the deletion of the tile layer '%1'?");
	warning_text = warning_text.arg(layer_name);
	switch (QMessageBox::warning(this, "Delete Layer Confirmation", warning_text, "&Confirm", "C&ancel", 0, 1))
	{
		case 0: // Selected Confirm
			break;
		case 1: // Selected Cancel
		default:
			return;
	}

	// Delete the layer from the map data first and make sure that it was successful
	if (_map_data->DeleteTileLayer(layer_index) == false) {
		QMessageBox::warning(this, "Layer Deletion Failure", _map_data->GetErrorMessage());
		return;
	}

	// If the item being deleted is the selected item, change the selction to the item before it (or after if its the first item)
	if (currentItem() == _right_click_item) {
		QTreeWidgetItem* new_selection = itemAbove(_right_click_item);
		if (new_selection == nullptr)
			new_selection = itemBelow(_right_click_item);
		setCurrentItem(new_selection);
	}

	// Deleting the item directly also removes it from the QTreeWidget automatically
	delete _right_click_item;
	_right_click_item = nullptr;

	// Update the IDs of the remaining layers
	QTreeWidgetItem* root = invisibleRootItem();
	for (uint32 i = 0; i < static_cast<uint32>(root->childCount()); ++i) {
		root->child(i)->setText(ID_COLUMN, QString::number(i));
	}

	// Redraw the map view now that the layer is removed
	Editor* editor = static_cast<Editor*>(topLevelWidget());
	editor->MapLayersModified();
	editor->DrawMapView();
	editor->statusBar()->showMessage(QString("Deleted tile layer '%1'").arg(layer_name), 5000);
}

} // namespace hoa_editor
