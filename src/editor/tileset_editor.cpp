///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    tileset_editor.cpp
*** \author  Bar�� Soner U�akl (Black Knight)
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for editor's tileset editor dialog
*******************************************************************************/

#include <vector>

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

#include "utils.h"
#include "tileset_editor.h"

using namespace std;
using namespace hoa_utils;

namespace hoa_editor {

////////////////////////////////////////////////////////////////////////////////
////////// TilesetDisplay class
////////////////////////////////////////////////////////////////////////////////

TilesetDisplay::TilesetDisplay() :
	_set_collision_state(1),
	_last_x(-1),
	_last_y(-1),
	_tileset_data(new Tileset()),
	_red_square(QPixmap(TILE_QUADRANT_LENGTH, TILE_QUADRANT_HEIGHT))
{
	// Red color with 50% transparency
	_red_square.fill(QColor(255, 0, 0, 125));

	setSceneRect(0, 0, TILESET_LENGTH, TILESET_HEIGHT);
}



void TilesetDisplay::DrawTileset() {
	if (_tileset_data->IsInitialized() == false)
		return;

	clear();
	setBackgroundBrush(QBrush(Qt::gray));

	// Draw the tileset image
	addPixmap(*_tileset_data->GetTilesetImage());

	// For each tile, draw the red square over the quadrants that have collisions enabled
	const vector<uint32>& collision_grid = _tileset_data->GetTileCollisions();
	// Counters used to track the X and Y coordinates of the current tile being evaluated
	uint32 tile_x = 0;
	uint32 tile_y = 0;
	// Holds the screen coordinates to draw the collision grid sections to for the tile
	uint32 pos_x = 0;
	uint32 pos_y = 0;
	// Iterate through the collision grid. Every four consecutive elements correspond to a single tile.
	for (uint32 i = 0; i < collision_grid.size(); i += TILE_NUM_QUADRANTS, ++tile_x) {
		if (tile_x >= TILESET_NUM_COLS) {
			tile_x = 0;
			tile_y++;
		}
		pos_x = tile_x * TILE_LENGTH;
		pos_y = tile_y * TILE_LENGTH;


		if (collision_grid[i] != 0) { // NW quadrant
			addPixmap(_red_square)->setPos(pos_x, pos_y);
		}
		if (collision_grid[i+1] != 0) { // NE quadrant
			addPixmap(_red_square)->setPos(pos_x + TILE_QUADRANT_LENGTH, pos_y);
		}
		if (collision_grid[i+2] != 0) { // SW quadrant
			addPixmap(_red_square)->setPos(pos_x, pos_y + TILE_QUADRANT_HEIGHT);
		}
		if (collision_grid[i+3] != 0) { // SE quadrant
			addPixmap(_red_square)->setPos(pos_x + TILE_QUADRANT_LENGTH, pos_y + TILE_QUADRANT_HEIGHT);
		}
	}

	_DrawGridLines();
	update();
}



void TilesetDisplay::resizeScene(int length, int height) {
	(void)length;
	(void)height;
	setSceneRect(0, 0, TILESET_LENGTH, TILESET_HEIGHT);
	DrawTileset();
}



void TilesetDisplay::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		QPointF pos = event->scenePos();
		// Don't process clicks outside of the tileset image
		if ((pos.x() < 0) || (pos.y() < 0) || pos.x() >= TILESET_LENGTH || pos.y() >= TILESET_HEIGHT)
			return;

		_last_x = pos.x() / TILE_QUADRANT_LENGTH;
		_last_y = pos.y() / TILE_QUADRANT_HEIGHT;

		// The collision value to set should be the opposite of the value for the coordinate of this click
		_set_collision_state = _IsCollisionQuadrantEnabled() ? 0 : 1;

		_UpdateCollisionQuadrant();
	}
}



void TilesetDisplay::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	// Don't deal with the event if the left button isn't pressed
	if (event->buttons() ^= Qt::LeftButton)
		return;

	QPointF pos = event->scenePos();
	// Don't process clicks outside of the tileset image
	if ((pos.x() < 0) || (pos.y() < 0) || pos.x() >= TILESET_LENGTH || pos.y() >= TILESET_HEIGHT)
		return;

	// If the mouse event occurred over a different collision coordinate, save the new location and update the collision coordinate.
	// Otherwise the move event casuse no change to occur
	int32 x = pos.x() / TILE_QUADRANT_LENGTH;
	int32 y = pos.y() / TILE_QUADRANT_HEIGHT;
	if (_last_x != x || _last_y != y) {
		_last_x = x;
		_last_y = y;
		_UpdateCollisionQuadrant();
	}
}



void TilesetDisplay::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		// Reset the last position to permit drawing again
		_last_x = -1;
		_last_y = -1;
	}
}



void TilesetDisplay::_UpdateCollisionQuadrant() {
	if (_tileset_data->IsInitialized() == false)
		return;

	_tileset_data->SetQuadrantCollision(_DetermineCollisionQuadrantIndex(), _set_collision_state);
	DrawTileset();
}



bool TilesetDisplay::_IsCollisionQuadrantEnabled() {
	if (_tileset_data->IsInitialized() == false)
		return false;

	return _tileset_data->GetQuadrantCollision(_DetermineCollisionQuadrantIndex());
}



uint32 TilesetDisplay::_DetermineCollisionQuadrantIndex() {
	// Determine which tile that the last location took place over
	uint32 tile_x = _last_x / 2;
	uint32 tile_y = _last_y / 2;
	bool x_offset = IsOddNumber(_last_x);
	bool y_offset = IsOddNumber(_last_y);

	// Now determine which quadrant of that tile was clicked.
	uint32 quadrant_index = ((tile_y * TILESET_NUM_COLS) + tile_x) * TILE_NUM_QUADRANTS;
	if ((x_offset == false) && (y_offset == false))  // NW quadrant
		quadrant_index += 0;
	else if ((x_offset == true) && (y_offset == false)) // NE quadrant
		quadrant_index += 1;
	else if ((x_offset == false) && (y_offset == true)) // SW quadrant
		quadrant_index += 2;
	else if ((x_offset == true) && (y_offset == true)) // SE quadrant
		quadrant_index += 3;

 	return quadrant_index;
}



void TilesetDisplay::_DrawGridLines() {
	// Draw dashed lines outlining each tile collision quadrant
	for (uint32 x = 0; x < TILESET_LENGTH; x += TILE_QUADRANT_LENGTH) {
		for (uint32 y = 0; y < TILESET_HEIGHT; y += TILE_QUADRANT_HEIGHT) {
			addLine(x, 0, x, TILESET_HEIGHT, QPen(Qt::DashLine));
			addLine(0, y, TILESET_LENGTH, y, QPen(Qt::DashLine));
		}
	}

	// Draw solid lines outlining each tile
	for (uint32 x = 0; x < TILESET_LENGTH; x += TILE_LENGTH) {
		for (uint32 y = 0; y < TILESET_HEIGHT; y += TILE_HEIGHT) {
			addLine(x, 0, x, TILESET_HEIGHT, QPen(Qt::SolidLine));
			addLine(0, y, TILESET_LENGTH, y, QPen(Qt::SolidLine));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
////////// TilesetEditor class
////////////////////////////////////////////////////////////////////////////////

TilesetEditor::TilesetEditor(QWidget* parent) :
	QDialog(parent),
	_tileset_display(),
	_tileset_view(nullptr),
	_new_button(nullptr),
	_open_button(nullptr),
	_save_button(nullptr),
	_exit_button(nullptr)
{
	setWindowTitle("Tileset Editor");
	setMinimumSize(TILESET_LENGTH + 80, TILESET_HEIGHT + 80); // Just enough room to fit the tileset image and buttons

	// Initialize the tileset display and view objects
	_tileset_display.setSceneRect(0, 0, TILESET_LENGTH, TILESET_HEIGHT);
	_tileset_display.setBackgroundBrush(QBrush(Qt::black));

	_tileset_view = new QGraphicsView(&_tileset_display);
	_tileset_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_tileset_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_tileset_view->setFixedSize(TILESET_LENGTH, TILESET_HEIGHT);

	// Create GUI Items
	_new_button = new QPushButton("New", this);
	_open_button = new QPushButton("Open", this);
	_save_button = new QPushButton("Save", this);
	_exit_button = new QPushButton("Exit", this);
	_exit_button->setDefault(true);

	// Cnnect button signals
	connect(_new_button, SIGNAL(clicked()), this, SLOT(_NewFile()));
	connect(_open_button, SIGNAL(clicked()), this, SLOT(_OpenFile()));
	connect(_save_button, SIGNAL(clicked()), this, SLOT(_SaveFile()));
	connect(_exit_button, SIGNAL(released()), this, SLOT(reject()));

	// Add all of the widgets into an organized grid layout
	_grid_layout = new QGridLayout(this);
	_grid_layout->addWidget(_new_button, 0, 1);
	_grid_layout->addWidget(_open_button, 1, 1);
	_grid_layout->addWidget(_save_button, 2, 1);
	_grid_layout->addWidget(_exit_button, 3, 1);
	_grid_layout->addWidget(_tileset_view, 0, 0, 3, 1);
}



TilesetEditor::~TilesetEditor() {
	delete _tileset_view;
	delete _new_button;
	delete _open_button;
	delete _save_button;
	delete _exit_button;
	delete _grid_layout;
}



void TilesetEditor::_NewFile() {
	// Get the filename to open through the OpenFileName dialog
	QString filename = QFileDialog::getOpenFileName(this, "Allacrost Map Editor -- File Open",
		"img/tilesets", "Tileset Image (*.png)");

	if (filename.isEmpty()) {
		return;
	}

	if (_tileset_display.GetTilesetData()->New(filename, true) == false) {
		QMessageBox::warning(this, "Allacrost Map Editor", "Failed to create new tileset.");
		return;
	}

	_tileset_display.DrawTileset();
}



void TilesetEditor::_OpenFile() {
	// Get the filename to open through the OpenFileName dialog
	QString filename = QFileDialog::getOpenFileName(this, "Allacrost Map Editor -- File Open",
		"lua/data/tilesets", "Tileset Definition File (*.lua)");

	if (filename.isEmpty())
		return;


	if (_tileset_display.GetTilesetData()->Load(filename, true) == false) {
		QMessageBox::warning(this, "Allacrost Map Editor", "Failed to load existing tileset.");
	}

	_tileset_display.DrawTileset();
}



void TilesetEditor::_SaveFile() {
	// Data must exist in order to save it
	if (_tileset_display.GetTilesetData()->IsInitialized() == false)
		return;

	if (_tileset_display.GetTilesetData()->Save() == false)
		QMessageBox::warning(this, "Allacrost Map Editor", "Failed to save data to tileset definition file.");
}

} // namespace hoa_editor
