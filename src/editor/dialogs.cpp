///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    dialogs.cpp
*** \author  Philip Vorsilak (gorzuate)
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for all of the editor's dialog windows
*** **************************************************************************/

#include <QDebug>

#include "editor.h"
#include "map_data.h"
#include "dialogs.h"

namespace hoa_editor {

///////////////////////////////////////////////////////////////////////////////
// NewMapDialog class
///////////////////////////////////////////////////////////////////////////////

NewMapDialog::NewMapDialog(QWidget* parent, MapData* data) :
	QDialog(parent),
	_map_data(data),
	_height_spinbox(nullptr),
	_length_spinbox(nullptr),
	_height_title(nullptr),
	_length_title(nullptr),
	_tileset_tree(nullptr),
	_ok_button(nullptr),
	_cancel_button(nullptr),
	_grid_layout(nullptr)
{
	setWindowTitle("Create New Map");

	if (_map_data == nullptr) {
		qDebug() << "ERROR: NewMapDialog constructor received a nullptr map data pointer";
	}
	else if (_map_data->IsInitialized() == true) {
		qDebug() << "ERROR: NewMapDialog constructor received a pointer to map data that was already initialized";
	}

	_height_spinbox = new QSpinBox(this);
	_height_spinbox->setMinimum(MINIMUM_MAP_HEIGHT);
	_height_spinbox->setMaximum(MAXIMUM_MAP_HEIGHT);
	_height_spinbox->setValue(MINIMUM_MAP_HEIGHT);
	_length_spinbox = new QSpinBox(this);
	_length_spinbox->setMinimum(MINIMUM_MAP_LENGTH);
	_length_spinbox->setMaximum(MAXIMUM_MAP_LENGTH);
	_length_spinbox->setValue(MINIMUM_MAP_LENGTH);

	_height_title = new QLabel("Map Height: ", this);
	_length_title = new QLabel("Map Length: ", this);

	// Set up the list of selectable tilesets by grabbing each file in the tileset directory (lua/data/tilesets)
	QDir tileset_dir("lua/data/tilesets");
	_tileset_tree = new QTreeWidget(this);
	_tileset_tree->setColumnCount(1);
	_tileset_tree->setHeaderLabels(QStringList("Tilesets"));
	connect(_tileset_tree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(_EnableOKButton()));

	// Start the loop at 2 to skip over the present and parent working directories ("." and "..")
	QList<QTreeWidgetItem*> tilesets;
	for (uint32 i = 2; i < tileset_dir.count(); i++) {
		// Exclude the file autotiling.lua, as it's not a tileset defition files
		if (tileset_dir[i] == QString("autotiling.lua"))
			continue;

		tilesets.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(tileset_dir[i].remove(".lua"))));
		tilesets.back()->setCheckState(0, Qt::Unchecked); // enables checkboxes
	}
	_tileset_tree->insertTopLevelItems(0, tilesets);

	_ok_button = new QPushButton("OK", this);
	_ok_button->setEnabled(false); // At construction no tilesets are checked, so the ok button is initially disabled
	connect(_ok_button, SIGNAL(released()), this, SLOT(_CreateMapData()));
	_cancel_button = new QPushButton("Cancel", this);
	_cancel_button->setDefault(true);
	connect(_cancel_button, SIGNAL(released()), this, SLOT(reject()));

	// Add all of the aforementioned widgets into a nice-looking grid layout
	_grid_layout = new QGridLayout(this);
	_grid_layout->addWidget(_height_title, 0, 0);
	_grid_layout->addWidget(_height_spinbox, 1, 0);
	_grid_layout->addWidget(_length_title, 2, 0);
	_grid_layout->addWidget(_length_spinbox, 3, 0);
	_grid_layout->addWidget(_tileset_tree, 0, 1, 5, -1);
	_grid_layout->addWidget(_ok_button, 6, 0);
	_grid_layout->addWidget(_cancel_button, 6, 1);
} // NewMapDialog::NewMapDialog(QWidget* parent, const QString& name, bool prop)



NewMapDialog::~NewMapDialog() {
	delete _height_spinbox;
	delete _length_spinbox;
	delete _height_title;
	delete _length_title;
	delete _tileset_tree;
	delete _ok_button;
	delete _cancel_button;
	delete _grid_layout;
}



void NewMapDialog::_EnableOKButton() {
	// Disable the ok button if no tilesets are checked, otherwise enable it
	for (uint32 i = 0; i < static_cast<uint32>(_tileset_tree->topLevelItemCount()); i++) {
		// At least one tileset must be checked in order to enable push button
		if (_tileset_tree->topLevelItem(i)->checkState(0) == Qt::Checked) {
			_ok_button->setEnabled(true);
			return;
		}
	}

	// If this point is reached, no tilesets are checked.
	_ok_button->setEnabled(false);
}



void NewMapDialog::_CreateMapData() {
	Editor* editor = static_cast<Editor*>(parent());

	// Initialize the map data
	_map_data->CreateData(_length_spinbox->value(), _height_spinbox->value());
	editor->MapSizeModified();
	editor->MapLayersModified();
	editor->MapContextsModified();

	int32 num_tileset_items = _tileset_tree->topLevelItemCount();
	int32 num_checked_items = 0;
	for (int32 i = 0; i < num_tileset_items; ++i) {
		if (_tileset_tree->topLevelItem(i)->checkState(0) == Qt::Checked)
			num_checked_items++;
	}

	// Used to show the progress of tilesets that have been loaded.
	QProgressDialog load_tileset_progress("Loading tilesets...", nullptr, 0, num_checked_items, editor, Qt::Widget | Qt::FramelessWindowHint | Qt::WindowTitleHint);
	load_tileset_progress.setWindowTitle("Creating Map...");

	// Set the location of the progress dialog and show it
	load_tileset_progress.move(editor->pos().x() +  editor->width() / 2  - load_tileset_progress.width() / 2,
		 editor->pos().y() + editor->height() / 2 - load_tileset_progress.height() / 2);
	load_tileset_progress.show();

	// Load each tileset object into the map data
	num_checked_items = 0;
	for (int32 i = 0; i < num_tileset_items; i++) {
		if (_tileset_tree->topLevelItem(i)->checkState(0) != Qt::Checked) {
			continue;
		}

		// Increment the progress dialog counter
		load_tileset_progress.setValue(num_checked_items++);

		// Load the tileset data from the definition file and add it to the map data
		Tileset* tileset = new Tileset();
		QString filename = QString("lua/data/tilesets/") + _tileset_tree->topLevelItem(i)->text(0) + (".lua");
		if (tileset->Load(filename) == false) {
			QMessageBox::critical(this, APP_NAME, "Failed to load tileset: " + filename);
			delete tileset;
		}

		if (_map_data->AddTileset(tileset) == false) {
			QMessageBox::critical(this, APP_NAME, "Failed to add tileset to map data: " + _map_data->GetErrorMessage());
			delete tileset;
		}
	}

	load_tileset_progress.hide();
	accept(); // Hides the dialog and sets the result code
}

///////////////////////////////////////////////////////////////////////////////
// MapResizeDialog class
///////////////////////////////////////////////////////////////////////////////

MapResizeDialog::MapResizeDialog(QWidget* parent, MapData* data) :
	QDialog(parent),
	_map_data(data),
	_height_spinbox(nullptr),
	_length_spinbox(nullptr),
	_height_title(nullptr),
	_length_title(nullptr),
	_height_change(nullptr),
	_length_change(nullptr),
	_ok_button(nullptr),
	_cancel_button(nullptr),
	_grid_layout(nullptr)
{
	setWindowTitle("Resize Map");

	if (_map_data == nullptr) {
		qDebug() << "ERROR: MapResizesDialog constructor received a nullptr map data pointer";
	}

	_height_spinbox = new QSpinBox(this);
	_height_spinbox->setMinimum(MINIMUM_MAP_HEIGHT);
	_height_spinbox->setMaximum(MAXIMUM_MAP_HEIGHT);
	_height_spinbox->setValue(_map_data->GetMapHeight());
	connect(_height_spinbox, SIGNAL(valueChanged(int)), this, SLOT(_HeightChanged()));
	_length_spinbox = new QSpinBox(this);
	_length_spinbox->setMinimum(MINIMUM_MAP_LENGTH);
	_length_spinbox->setMaximum(MAXIMUM_MAP_LENGTH);
	_length_spinbox->setValue(_map_data->GetMapLength());
	connect(_length_spinbox, SIGNAL(valueChanged(int)), this, SLOT(_LengthChanged()));

	_height_title = new QLabel("Map Height:", this);
	_length_title = new QLabel("Map Length:", this);
	_height_change = new QLabel("Change: 0", this);
	_length_change = new QLabel("Change: 0", this);

	_cancel_button = new QPushButton("Cancel", this);
	_cancel_button->setDefault(true);
	connect(_cancel_button, SIGNAL(released()), this, SLOT(reject()));
	_ok_button = new QPushButton("OK", this);
	connect(_ok_button, SIGNAL(released()), this, SLOT(accept()));

	_grid_layout = new QGridLayout(this);
	_grid_layout->addWidget(_height_title, 0, 0);
	_grid_layout->addWidget(_height_spinbox, 0, 1);
	_grid_layout->addWidget(_height_change, 0, 2);
	_grid_layout->addWidget(_length_title, 1, 0);
	_grid_layout->addWidget(_length_spinbox, 1, 1);
	_grid_layout->addWidget(_length_change, 1, 2);
	_grid_layout->addWidget(_ok_button, 2, 1);
	_grid_layout->addWidget(_cancel_button, 2, 2);
}



MapResizeDialog::~MapResizeDialog() {
	delete _height_spinbox;
	delete _length_spinbox;
	delete _height_title;
	delete _length_title;
	delete _height_change;
	delete _length_change;
	delete _ok_button;
	delete _cancel_button;
	delete _grid_layout;
}



void MapResizeDialog::ModifyMapData() {
	uint32 new_height = static_cast<uint32>(_height_spinbox->value());
	uint32 new_length = static_cast<uint32>(_length_spinbox->value());
	Editor* editor = static_cast<Editor*>(parent());

	if ((new_height == _map_data->GetMapHeight()) && (new_length == _map_data->GetMapLength())) {
		editor->statusBar()->showMessage("Map size was not changed", 5000);
		return;
	}

	_map_data->ResizeMap(new_length, new_height);
	editor->MapSizeModified();
	editor->statusBar()->showMessage("Map resized", 5000);
}



void MapResizeDialog::_HeightChanged() {
	int32 change = _height_spinbox->value() - static_cast<int32>(_map_data->GetMapHeight());
	if (change <= 0) {
		_height_change->setText("Change: " + QString::number(change));
	}
	else {
		_height_change->setText("Change: +" + QString::number(change));
	}
}



void MapResizeDialog::_LengthChanged() {
	int32 change = _length_spinbox->value() - static_cast<int32>(_map_data->GetMapLength());
	if (change <= 0) {
		_length_change->setText("Change: " + QString::number(change));
	}
	else {
		_length_change->setText("Change: +" + QString::number(change));
	}
}

///////////////////////////////////////////////////////////////////////////////
// MapResizeInternalDialog class
///////////////////////////////////////////////////////////////////////////////

MapResizeInternalDialog::MapResizeInternalDialog(QWidget* parent, MapData* data, uint32 row, uint32 column, bool insert_operation, bool column_operation) :
	QDialog(parent),
	_map_data(data),
	_row_position(row),
	_column_position(column),
	_insert_operation(insert_operation),
	_column_operation(column_operation),
	_change_spinbox(nullptr),
	_operation_text(nullptr),
	_position_text(nullptr),
	_ok_button(nullptr),
	_cancel_button(nullptr),
	_grid_layout(nullptr)
{
	if (_map_data == nullptr) {
		qDebug() << "ERROR: MapResizesDialog constructor received a nullptr map data pointer";
	}

	_change_spinbox = new QSpinBox(this);
	_change_spinbox->setMinimum(0);
	_change_spinbox->setValue(1);
	connect(_change_spinbox, SIGNAL(valueChanged(int)), this, SLOT(_EnableOkButton()));

	_operation_text = new QLabel("", this);
	_position_text = new QLabel("Operation will take place at\nX/Y coordinates: [" + QString::number(_column_position) +
		", " + QString::number(_row_position) + "]", this);

	_ok_button = new QPushButton("OK", this);
	connect(_ok_button, SIGNAL(released()), this, SLOT(accept()));
	_cancel_button = new QPushButton("Cancel", this);
	_cancel_button->setDefault(true);
	connect(_cancel_button, SIGNAL(released()), this, SLOT(reject()));

	_grid_layout = new QGridLayout(this);
	_grid_layout->addWidget(_operation_text, 0, 0);
	_grid_layout->addWidget(_change_spinbox, 0, 1);
	_grid_layout->addWidget(_position_text, 1, 0);
	_grid_layout->addWidget(_ok_button, 2, 0);
	_grid_layout->addWidget(_cancel_button, 2, 1);

	// The max spinbox value, operation text, and window title all need to be set appropriately based on the
	// insert_operation and column_operation booleans
	if (_insert_operation) {
		if (_column_operation) {
			setWindowTitle("Insert Tile Columns");
			_operation_text->setText("Tile columns to insert:");
			_change_spinbox->setMaximum(MAXIMUM_MAP_LENGTH - _map_data->GetMapLength());
		}
		else {
			setWindowTitle("Insert Tile Rows");
			_operation_text->setText("Tile rows to insert:");
			_change_spinbox->setMaximum(MAXIMUM_MAP_HEIGHT - _map_data->GetMapHeight());
		}
	}
	else {
		if (_column_operation) {
			setWindowTitle("Delete Tile Columns");
			_operation_text->setText("Tile columns to delete:");
			if ((_map_data->GetMapLength() - _column_position) < (_map_data->GetMapLength() - MINIMUM_MAP_LENGTH))
				_change_spinbox->setMaximum(_map_data->GetMapLength() - _column_position);
			else
				_change_spinbox->setMaximum(_map_data->GetMapLength() - MINIMUM_MAP_LENGTH);
		}
		else {
			setWindowTitle("Delete Tile Rows");
			_operation_text->setText("Tile rows to delete:");
			if ((_map_data->GetMapHeight() - _row_position) < (_map_data->GetMapHeight() - MINIMUM_MAP_HEIGHT))
				_change_spinbox->setMaximum(_map_data->GetMapHeight() - _row_position);
			else
				_change_spinbox->setMaximum(_map_data->GetMapHeight() - MINIMUM_MAP_HEIGHT);
		}
	}
}



MapResizeInternalDialog::~MapResizeInternalDialog() {
	delete _change_spinbox;
	delete _operation_text;
	delete _position_text;
	delete _ok_button;
	delete _cancel_button;
	delete _grid_layout;
}



void MapResizeInternalDialog::ModifyMapData() {
	uint32 change_value = static_cast<uint32>(_change_spinbox->value());
	Editor* editor = static_cast<Editor*>(parent());

	// This case should never happen. If it does, the code that invoked this call needs to be corrected
	if (change_value == 0) {
		qDebug() << "Called MapResizeInternalDialog::ModifyMapData when the change value was equal to zero";
		return;
	}

	if (_insert_operation) {
		if (_column_operation) {
			_map_data->InsertTileLayerColumns(_column_position, change_value);
			editor->statusBar()->showMessage("Inserted " + QString::number(change_value) + " tile columns to map", 5000);
		}
		else {
			_map_data->InsertTileLayerRows(_row_position, change_value);
			editor->statusBar()->showMessage("Inserted " + QString::number(change_value) + " tile rows to map", 5000);
		}
	}
	else {
		if (_column_operation) {
			_map_data->RemoveTileLayerColumns(_column_position, change_value);
			editor->statusBar()->showMessage("Deleted " + QString::number(change_value) + " tile columns from map", 5000);
		}
		else {
			_map_data->RemoveTileLayerRows(_row_position, change_value);
			editor->statusBar()->showMessage("Deleted " + QString::number(change_value) + " tile rows from map", 5000);
		}
	}

	editor->MapSizeModified();
}



void MapResizeInternalDialog::_EnableOkButton() {
	if (_change_spinbox->value() > 0)
		_ok_button->setEnabled(true);
	else
		_ok_button->setEnabled(false);
}

///////////////////////////////////////////////////////////////////////////////
// AddTilesetsDialog class
///////////////////////////////////////////////////////////////////////////////

AddTilesetsDialog::AddTilesetsDialog(QWidget* parent, MapData* data) :
	QDialog(parent),
	_map_data(data),
	_tileset_tree(nullptr),
	_add_button(nullptr),
	_cancel_button(nullptr),
	_grid_layout(nullptr)
{
	setWindowTitle("Add Tilesets...");

	if (_map_data == nullptr) {
		qDebug() << "ERROR: AddTilesetsDialog constructor received a nullptr map data pointer";
	}

	_add_button = new QPushButton("Add", this);
	_cancel_button = new QPushButton("Cancel", this);
	_cancel_button->setDefault(true);

	connect(_add_button, SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_button, SIGNAL(released()), this, SLOT(reject()));

	// Set up the list of selectable tilesets
	_tileset_tree = new QTreeWidget(this);
	_tileset_tree->setColumnCount(1);
	_tileset_tree->setHeaderLabels(QStringList("Tilesets"));
	connect(_tileset_tree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(_EnableAddButton()));

	// Retrieve all files found in the tileset definition directory (lua/data/tilesets).
	QList<QTreeWidgetItem*> file_items;
	QDir tileset_filenames("lua/data/tilesets");
	QStringList loaded_tileset_filenames = _map_data->GetTilesetFilenames();

	// Start the loop at 2 to skip over the present and parent working directories ("." and "..")
	for (uint32 i = 2; i < tileset_filenames.count(); i++) {
		// Exclude the file autotiling.lua, as it is not a tileset defition files
		if (tileset_filenames[i] == QString("autotiling.lua"))
			continue;

		QTreeWidgetItem* new_item = new QTreeWidgetItem(_tileset_tree, QStringList(tileset_filenames[i].remove(".lua")));
		if (loaded_tileset_filenames.contains("lua/data/tilesets/" + tileset_filenames[i]) == true) {
			new_item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
			new_item->setCheckState(0, Qt::Checked);
		}
		else {
			new_item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			new_item->setCheckState(0, Qt::Unchecked);
		}
		file_items.append(new_item);
	}

	_tileset_tree->insertTopLevelItems(0, file_items);

	_grid_layout = new QGridLayout(this);
	_grid_layout->addWidget(_tileset_tree, 0, 0, 10, -1);
	_grid_layout->addWidget(_cancel_button, 11, 0);
	_grid_layout->addWidget(_add_button, 11, 1);
}



AddTilesetsDialog::~AddTilesetsDialog() {
	delete _tileset_tree;
	delete _add_button;
	delete _cancel_button;
	delete _grid_layout;
}



uint32 AddTilesetsDialog::AddTilesetsToMapData() {
	uint32 tilesets_added = 0;

	for (int i = 0; i < _tileset_tree->topLevelItemCount(); i++) {
		QTreeWidgetItem* item = _tileset_tree->topLevelItem(i);
		// At least one tileset must be checked in order to enable push button
		if (item->checkState(0) == Qt::Checked && item->isDisabled() == false) {
			Tileset* tileset = new Tileset();
			QString filename = QString("lua/data/tilesets/") + item->text(0) + (".lua");

			if (tileset->Load(filename) == false) {
				QMessageBox::critical(this, APP_NAME, "Failed to load tileset: " + filename);
				delete tileset;
				item->setCheckState(0, Qt::Unchecked);
				continue;
			}

			if (_map_data->AddTileset(tileset) == false) {
				QMessageBox::critical(this, APP_NAME, "Failed to add tileset to map data: " + _map_data->GetErrorMessage());
				delete tileset;
				item->setCheckState(0, Qt::Unchecked);
				continue;
			}

			item->setFlags(item->flags() ^ Qt::ItemIsEnabled); // Disable this item now that it has been loaded
			tilesets_added++;
		}
	}

	return tilesets_added;
}



void AddTilesetsDialog::_EnableAddButton() {
	// Disable the add button if no tilesets are checked, otherwise enable it
	for (int i = 0; i < _tileset_tree->topLevelItemCount(); i++) {
		// At least one tileset must be checked in order to enable push button
		if (_tileset_tree->topLevelItem(i)->checkState(0) == Qt::Checked && _tileset_tree->topLevelItem(i)->isDisabled() == false) {
			_add_button->setEnabled(true);
			return;
		}
	}

	// If this point is reached, no tilesets are checked.
	_add_button->setEnabled(false);
}

} // namespace hoa_editor
