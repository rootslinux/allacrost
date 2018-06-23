///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    tileset.cpp
*** \author  Philip Vorsilak (gorzuate)
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for tileset data and display code
*** **************************************************************************/

#include <QFile>
#include <QImage>
#include <QVariant>
#include <QHeaderView>

#include "script.h"

#include "dialogs.h"
#include "editor.h"
#include "map_data.h"
#include "tileset.h"

using namespace std;
using namespace hoa_script;

namespace hoa_editor {

/** \name Tile Collision Quadrant Bitmasks
***
*** In the TDF, the collision information for each tile is stored in a single number.
*** These bitmasks are used to extact the collision data for each quadrant of a tile.
**/
//@{
uint32 NORTHWEST_TILE_QUADRANT = 0x08;
uint32 NORTHEAST_TILE_QUADRANT = 0x04;
uint32 SOUTHWEST_TILE_QUADRANT = 0x02;
uint32 SOUTHEAST_TILE_QUADRANT = 0x01;
//@}

////////////////////////////////////////////////////////////////////////////////
// Tileset class
////////////////////////////////////////////////////////////////////////////////

Tileset::Tileset() :
	_initialized(false),
	_tileset_name(""),
	_tileset_image_filename(""),
	_tileset_definition_filename(""),
	_tile_images(TILESET_NUM_ROWS * TILESET_NUM_COLS, QPixmap()),
	_tile_collisions(TILESET_NUM_ROWS * TILESET_NUM_COLS * TILE_NUM_QUADRANTS, 0)
{}



bool Tileset::New(const QString& img_filename, bool single_image) {
	if (img_filename.isEmpty()) {
		qDebug("Missing image filename");
		return false;
	}

	_ClearData();

	_tileset_image_filename = img_filename;
	_CreateTilesetNameFromFilename(img_filename);
	if (_LoadImageData(single_image) == false) {
		_ClearData();
		return false;
	}
	// Assign the default filename to use in the case that the user saves their changes
	_tileset_definition_filename = "lua/data/tilesets/" + _tileset_name + ".lua";

	_initialized = true;
	return true;
}



bool Tileset::Load(const QString& def_filename, bool single_image) {
	_ClearData();

	if (def_filename.isEmpty())
		return false;

	// ---------- 1) Open the tileset definition file and tablespace
	ReadScriptDescriptor read_file;
	_tileset_definition_filename = def_filename;
	if (read_file.OpenFile(_tileset_definition_filename.toStdString()) == false) {
		_ClearData();
		return false;
	}

	string tablespace = DetermineLuaFileTablespaceName(_tileset_definition_filename.toStdString());
	if (read_file.DoesTableExist(tablespace) == false) {
		_ClearData();
		return false;
	}
	read_file.OpenTable(tablespace);

	// ---------- 2) Load the tileset name and image data
	if (read_file.DoesStringExist("tileset_name") == true) {
		_tileset_name = QString::fromStdString(read_file.ReadString("tileset_name"));
	}
	else {
		_CreateTilesetNameFromFilename(def_filename);
	}

	_tileset_image_filename = QString::fromStdString(read_file.ReadString("image"));

	if (_LoadImageData(single_image) == false) {
		_ClearData();
		return false;
	}

	// ---------- 3) Load in the collision data
	if (read_file.DoesTableExist("collisions") == false) {
		_ClearData();
		return false;
	}

	read_file.OpenTable("collisions");
	for (uint32 i = 0; i < TILESET_NUM_ROWS; ++i) {
		vector<uint32> collision_row;  // Temporarily stores one tile row worth of collision data
		read_file.ReadUIntVector(i, collision_row);
		// Ensure that the row is the correct size
		if (collision_row.size() != TILESET_NUM_COLS) {
			_ClearData();
			return false;
		}

		// Every entry in collision_row contains the collision data for all four tile quadrants. We
		// apply some simple bit arithmetic to extract the collision state for each quadrant
		for (uint32 j = 0; j < collision_row.size(); ++j) {
			// Determine the starting index for the next four quadrants of data
			uint32 index = (i * (TILESET_NUM_COLS * TILE_NUM_QUADRANTS)) + (j * TILE_NUM_QUADRANTS);

			_tile_collisions[index] = (collision_row[j] & NORTHWEST_TILE_QUADRANT) ? 1 : 0;
			_tile_collisions[index + 1] = (collision_row[j] & NORTHEAST_TILE_QUADRANT) ? 1 : 0;
			_tile_collisions[index + 2] = (collision_row[j] & SOUTHWEST_TILE_QUADRANT) ? 1 : 0;
			_tile_collisions[index + 3] = (collision_row[j] & SOUTHEAST_TILE_QUADRANT) ? 1 : 0;
		}
	}
	read_file.CloseTable();

	// ---------- 4) Read in animation data if the TDF contains animations
	if (read_file.DoesTableExist("animations") == true) {
		uint32 table_size = read_file.GetTableSize("animations");
		read_file.OpenTable("animations");

		for (uint32 i = 0; i < table_size; ++i) {
			// Determine how many entries (tile + time) are in this animations
			vector<uint32> animation_data;
			read_file.ReadUIntVector(i, animation_data);
			_tile_animations.push_back(vector<AnimatedTileData>());

			for (uint32 j = 0; j < animation_data.size(); j += 2) {
				_tile_animations.back().push_back(AnimatedTileData(animation_data[j], animation_data[j + 1]));
			}
		}
		read_file.CloseTable();
	}

	// ---------- 5) Read in autotiling data if the TDF contains autotiling
	if (read_file.DoesTableExist("autotiling") == true) {
		// Read in the set of autotiling keys followed by their string values
		vector<uint32> keys;
		read_file.OpenTable("autotiling");
		read_file.ReadTableKeys(keys);

		for (uint32 i = 0; i < keys.size(); ++i) {
			_tile_autotiles[keys[i]] = read_file.ReadString(keys[i]);
		}
		read_file.CloseTable();
	}

	read_file.CloseTable();
	read_file.CloseFile();
	_initialized = true;
	return true;
} // bool Tileset::Load(const QString& def_filename, bool single_image)



bool Tileset::Save() {
	// We can't save the data if we don't have a file specified
	if (_tileset_definition_filename.isEmpty() == true) {
		return false;
	}

	WriteScriptDescriptor write_file;
	if (write_file.OpenFile(_tileset_definition_filename.toStdString()) == false) {
		return false;
	}

	// Write the tablespace header to the TDF, using a transformation of the TDF filename as the tablespace name
	string tablespace_name = DetermineLuaFileTablespaceName(_tileset_definition_filename.toStdString());
	write_file.WriteNamespace(tablespace_name);
	write_file.InsertNewLine();

	// Write the basic properties: the tileset name and image filename
	write_file.WriteString("tileset_name", _tileset_name.toStdString());
	write_file.WriteString("image", _tileset_image_filename.toStdString());
	write_file.InsertNewLine();

	// Write the collision data one row at a time
	write_file.BeginTable("collisions");
	for (uint32 row = 0; row < TILESET_NUM_ROWS; row++) {
		// The row of data needs to be transformed so all four tile quadrants fit in one number
		vector<uint32> combined_row(TILESET_NUM_COLS, 0);
		for (uint32 i = 0; i < combined_row.size(); ++i) {
			// Determine the starting index for the next four quadrants of data in _tile_collisions
			uint32 index = row * (TILESET_NUM_COLS * TILE_NUM_QUADRANTS) + (i * TILE_NUM_QUADRANTS);

			if (_tile_collisions[index] != 0)
				combined_row[i] |= NORTHWEST_TILE_QUADRANT;
			if (_tile_collisions[index + 1] != 0)
				combined_row[i] |= NORTHEAST_TILE_QUADRANT;
			if (_tile_collisions[index + 2] != 0)
				combined_row[i] |= SOUTHWEST_TILE_QUADRANT;
			if (_tile_collisions[index + 3] != 0)
				combined_row[i] |= SOUTHEAST_TILE_QUADRANT;
		}
		write_file.WriteUIntVector(row, combined_row);
	}
	write_file.EndTable();
	write_file.InsertNewLine();

	// Write the animation data, if the tileset has animations
	if (_tile_animations.empty() == false) {
 		write_file.BeginTable("animations");

		for (uint32 i = 0; i < _tile_animations.size(); ++i) {
			vector<uint32> animation_data;
			for (uint32 j = 0; j < _tile_animations[i].size(); ++j) {
				animation_data.push_back(_tile_animations[i][j].tile_id);
				animation_data.push_back(_tile_animations[i][j].time);
			}
			write_file.WriteUIntVector(i, animation_data);
		}

 		write_file.EndTable();
		write_file.InsertNewLine();
	}

	// Write the autotile data, if the tileset has autotiling
	if (_tile_autotiles.empty() == false) {
	 	write_file.BeginTable("autotiling");
	 	for (map<uint32, string>::iterator i = _tile_autotiles.begin(); i != _tile_autotiles.end(); ++i)
 			write_file.WriteString((*i).first, (*i).second);
 		write_file.EndTable();
		write_file.InsertNewLine();
	}

	if (write_file.IsErrorDetected() == true) {
		cerr << "Errors were detected when saving tileset file. The errors include:\n" << write_file.GetErrorMessages() << endl;
		write_file.CloseFile();
		return false;
	}

	write_file.CloseFile();
	return true;
} // bool Tileset::Save()



bool Tileset::SaveAs(const QString& def_filename) {
	QString old_def_filename = _tileset_definition_filename;
	_tileset_definition_filename = def_filename;

	if (Save() == false) {
		_tileset_definition_filename = old_def_filename;
		return false;
	}

	return true;
}



void Tileset::_ClearData() {
	_initialized = false;
	_tileset_name = "";
	_tileset_image_filename = "";
	_tileset_definition_filename = "";

	for (uint32 i = 0; i < _tile_images.size(); ++i)
		_tile_images[i] = QPixmap();
	for (uint32 i = 0; i < _tile_collisions.size(); ++i)
		_tile_collisions[i] = 0;
	_tile_animations.clear();
	_tile_autotiles.clear();
}



bool Tileset::_LoadImageData(bool single_image) {
	QRect rectangle;
	QImage entire_tileset;
	if (entire_tileset.load(_tileset_image_filename, "png") == false) {
		qDebug("Failed to load tileset image: %s", _tileset_image_filename.toStdString().c_str());
		_ClearData();
		return false;
	}

	if (single_image == true) {
		_tile_images[0].convertFromImage(entire_tileset);
	}
	else {
		for (uint32 row = 0; row < TILESET_NUM_ROWS; ++row) {
			for (uint32 col = 0; col < TILESET_NUM_COLS; ++col) {
				rectangle.setRect(col * TILE_LENGTH, row * TILE_HEIGHT, TILE_LENGTH, TILE_HEIGHT);
				QImage tile = entire_tileset.copy(rectangle);
				if (tile.isNull()) {
					qDebug("Image loading error!");
				} else {
					uint32 index = (row * TILESET_NUM_ROWS) + col;
					_tile_images[index].convertFromImage(tile);
				}
			}
		}
	}

	return true;
}



void Tileset::_CreateTilesetNameFromFilename(const QString &filename) {
	_tileset_name = filename;
	// Remove everything up to and including the final '/' character
	_tileset_name.remove(0, _tileset_name.lastIndexOf("/") + 1);
	// Chop off the appended four characters (the filename extension)
	_tileset_name.chop(4);
}

///////////////////////////////////////////////////////////////////////////////
// TilesetTable class
///////////////////////////////////////////////////////////////////////////////

TilesetTable::TilesetTable() :
	QTableWidget(TILESET_NUM_ROWS, TILESET_NUM_COLS),
	_tileset(nullptr)
{
	// Set the table properties
	setShowGrid(false);
	setSelectionMode(QTableWidget::ContiguousSelection);
	setEditTriggers(QTableWidget::NoEditTriggers);
	setContentsMargins(0, 0, 0, 0);
	setDragEnabled(false);
	setAcceptDrops(false);
	setHorizontalHeaderLabels(QStringList());
	setVerticalHeaderLabels(QStringList());
	verticalHeader()->hide();
	verticalHeader()->setContentsMargins(0, 0, 0, 0);
	horizontalHeader()->hide();
	horizontalHeader()->setContentsMargins(0, 0, 0, 0);
// 	setStyleSheet("QTableView::item { border-bottom: 1px solid white; border-right: 1px solid white; }");
// 	setStyleSheet("QTableView::item { border: 0px; padding: 1px; }");

	for (uint32 i = 0; i < TILESET_NUM_ROWS; ++i)
		setRowHeight(i, TILE_HEIGHT);
	for (uint32 i = 0; i < TILESET_NUM_COLS; ++i)
		setColumnWidth(i, TILE_LENGTH);
}



TilesetTable::TilesetTable(Tileset* tileset) :
	TilesetTable()
{
	_tileset = tileset;
	// While using a nullptr argument won't produce any problems, it is not the expected behavior for this constructor
	if (tileset == nullptr) {
		qDebug("WARNING: TilesetTable constructor was passed a nullptr Tileset pointer");
	}
	else {
		Load(_tileset);
	}
}



bool TilesetTable::Load(Tileset* tileset) {
	if (tileset == nullptr) {
		qDebug("WARNING: TilesetTable::Load() was passed a nullptr pointer. Call Clear() instead.");
		return false;
	}

	if (_tileset->IsInitialized() == false) {
		qDebug("WARNING: TilesetTable::Load() was passed a pointer to an uninitialized Tileset object");
	}

	Clear();
	_tileset = tileset;

	// TODO: Instead of reloading the image here, see if we can use the existing image data in the Tileset object

	// Load the tileset image into a single file, then make a copy of the individual tiles within it and place them in the pixmap
	QImage entire_tileset;
	if (entire_tileset.load(_tileset->GetTilesetImageFilename()) == false) {
		qDebug("ERROR: failed to load tileset image file %s", _tileset->GetTilesetImageFilename().toStdString().c_str());
		return false;
	}

	// Go through every tile location in the tileset image, copy out the image data for that tile,
	// and place it in the table's pixmap
	QRect rectangle;
	QVariant variant;
	for (uint32 row = 0; row < TILESET_NUM_ROWS; ++row) {
		for (uint32 col = 0; col < TILESET_NUM_COLS; ++col) {
			rectangle.setRect(col * TILE_LENGTH, row * TILE_HEIGHT, TILE_LENGTH, TILE_HEIGHT);
			variant = entire_tileset.copy(rectangle);
			if (variant.isNull() == true) {
				qDebug("Error occurred while copying a tileset image tile");
			}
			else {
				QTableWidgetItem* item = new QTableWidgetItem(QTableWidgetItem::UserType);
				item->setData(Qt::DecorationRole, variant);
				item->setFlags(item->flags() &~ Qt::ItemIsEditable);
				setItem(row, col, item);
			}
		}
	}

	// Select the top left item
	setCurrentCell(0, 0);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// TilesetView class
///////////////////////////////////////////////////////////////////////////////

TilesetView::TilesetView(QWidget* parent, MapData* data) :
	QTabWidget(parent),
	_map_data(data),
	_current_tileset_table(nullptr),
	_current_tileset_index(-1),
	_right_click_menu(nullptr),
	_add_tileset_action(nullptr),
	_remove_tileset_action(nullptr)
{
	setTabPosition(QTabWidget::North);
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(_CurrentTabChanged()));

	// Right-click menu action creation
	_add_tileset_action = new QAction("Add Tilesets...", this);
	_add_tileset_action->setStatusTip("Opens a dialog window to select one or more tilesets to add to the map");
	connect(_add_tileset_action, SIGNAL(triggered()), this, SLOT(_OpenAddTilesetDialog()));
	_remove_tileset_action = new QAction("Remove Current Tileset", this);
	_remove_tileset_action->setStatusTip("Removes the tileset that is currently visible on the widget");
	connect(_remove_tileset_action, SIGNAL(triggered()), this, SLOT(_RemoveCurrentTileset()));

	_right_click_menu = new QMenu(this);
	_right_click_menu->addAction(_add_tileset_action);
	_right_click_menu->addAction(_remove_tileset_action);
}



TilesetView::~TilesetView() {
	ClearData();
	delete _right_click_menu;
	delete _add_tileset_action;
	delete _remove_tileset_action;
}



void TilesetView::ClearData() {
	for (uint32 i = 0; i < static_cast<uint32>(count()); ++i) {
		delete widget(i);
	}
	clear();

	_current_tileset_table = nullptr;
	_current_tileset_index = -1;
}



void TilesetView::RefreshView() {
	ClearData();

	vector<Tileset*>& tilesets = _map_data->GetTilesets();
	for (vector<Tileset*>::iterator i = tilesets.begin(); i != tilesets.end(); ++i) {
		TilesetTable* table = new TilesetTable(*i);
		addTab(table, (*i)->GetTilesetName());
	}
}



void TilesetView::contextMenuEvent(QContextMenuEvent* event) {
	(void)event;

	if (_map_data->IsInitialized() == false) {
		_add_tileset_action->setEnabled(false);
		_remove_tileset_action->setEnabled(false);
	}
	else if (_current_tileset_table == nullptr) { // If true, no tilesets are loaded and thus can not be removed
		_add_tileset_action->setEnabled(true);
		_remove_tileset_action->setEnabled(false);
	}
	else {
		_add_tileset_action->setEnabled(true);
		_remove_tileset_action->setEnabled(true);
	}

	_right_click_menu->exec(QCursor::pos());
}



void TilesetView::_CurrentTabChanged() {
	_current_tileset_index = currentIndex();
	if (_current_tileset_index == -1) {
		_current_tileset_table = nullptr;
	}
	else {
		_current_tileset_table = static_cast<TilesetTable*>(currentWidget());
	}
}



void TilesetView::_OpenAddTilesetDialog() {
	AddTilesetsDialog add_dialog(this, _map_data);
	if (add_dialog.exec() != QDialog::Accepted) {
		return;
	}

	uint32 tilesets_added = add_dialog.AddTilesetsToMapData();
	static_cast<Editor*>(topLevelWidget())->statusBar()->showMessage(QString("Added %1 tilesets to the map data").arg(tilesets_added), 5000);

	vector<Tileset*>& tilesets = _map_data->GetTilesets();
	uint32 first_index = tilesets.size() - tilesets_added;
	for (uint32 i = 0; i < tilesets_added; ++i) {
		TilesetTable* table = new TilesetTable(tilesets[first_index + i]);
		addTab(table, tilesets[first_index + i]->GetTilesetName());
	}
}



void TilesetView::_RemoveCurrentTileset() {
	QString tileset_name = _current_tileset_table->GetTileset()->GetTilesetName();
	QString warning_text = "Deleting a tileset from the map will nullify all drawn tiles from that tileset.";
	warning_text = warning_text.append(" Are you sure that you wish to proceed with the deletion of the tileset '%1'?");
	warning_text = warning_text.arg(tileset_name);
	switch (QMessageBox::warning(this, "Remove Tileset Confirmation", warning_text, "&Confirm", "C&ancel", 0, 1))
	{
		case 0: // Selected Confirm
			break;
		case 1: // Selected Cancel
		default:
			return;
	}

	_map_data->RemoveTileset(_current_tileset_index);

 	delete currentWidget();
	Editor* editor = static_cast<Editor*>(topLevelWidget());
	editor->DrawMapView();
	editor->statusBar()->showMessage(QString("Removed tileset '%1'").arg(tileset_name), 5000);
}

} // namespace hoa_editor
