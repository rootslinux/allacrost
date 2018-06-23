///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    tileset_editor.h
*** \author  Bar�� Soner U�akl (Black Knight)
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for editor's tileset editor dialog
*******************************************************************************/

#pragma once

#include <QGraphicsScene>
#include <QDialog>
#include <QMessageBox>
#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QFileDialog>

#include "tileset.h"

namespace hoa_editor {

/** ****************************************************************************
*** \brief Widget display of a tileset image within the tileset editor
***
*** This class represents the display of an open tileset within the tileset editor.
*** It is through this class that the user interacts with the tileset image,
*** viewing and modifying its properties. This class is used by the TilesetEditor
*** class, which is responsible for initializing the _tileset_data
*** ***************************************************************************/
class TilesetDisplay : public QGraphicsScene {
public:
	TilesetDisplay();

	~TilesetDisplay()
		{ delete _tileset_data; }

	//! \brief Returns a pointer to the tileset data, guaranteed to never be nullptr
	Tileset* GetTilesetData() const
		{ return _tileset_data; }

	//! \brief Draws the tileset images and collision grid to the screen
	void DrawTileset();

protected:
	//! \brief Called when the display area is resized
	void resizeScene(int length, int height);

	/** \name Mouse Event Functions
	*** \brief Process mouse events that occur on the display, inherited from QGraphicsScene
	*** \param event A pointer to the QGraphicsSceneMouseEvent that was generated
	**/
	//{@
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	//@}

private:
	/** \brief Holds the value to set quadrants when the user clicks or drags the mouse over them
	*** This is to allow the user to paint multiple collision data changes with one click and drag operation.
	*** On a mouse click, it toggles the state of that quadrant and sets this member to the new value. If the
	*** user then keeps the left mouse button held down and drags the cursor over other quadrants, all quadrants
	*** will be set to the sate of the first quadrant.
	***
	*** \note The value of this member will only ever be 0 or 1. It is not a boolean because the Tileset class
	*** uses uint32 to store its collision data.
	**/
	uint32 _set_collision_state;

	/** \brief The coordinates of the most recent edited collision grid tile
	*** This data is used to improve performance by not processing mouse events unless the selected tile quadrant has changed.
	**/
	//@{
	int32 _last_x;
	int32 _last_y;
	//@}

	//! \brief The current tileset that is being edited.
	Tileset* _tileset_data;

	//! \brief A red, translucent square that is one quarter the size of a tile
	QPixmap _red_square;

	//! \brief Determines if the collision quadrant corresponding to _last_x and _last_y is enabled
	bool _IsCollisionQuadrantEnabled();

	//! \brief Updates the collision quadrant pointed to by _last_x and _last_y according to _set_collision_state
	void _UpdateCollisionQuadrant();

	/** \brief Finds the index into the tileset's collision data where a mouse event occured
	*** \return uint32 The index of the quadrant where the event took place
	**/
	uint32 _DetermineCollisionQuadrantIndex();

	//! \brief A helper function to DrawTileset that draws the tile and tile quadrant grids over the tileset image
	void _DrawGridLines();
}; // class TilesetDisplay : public QGraphicsScene


/** ****************************************************************************
*** \brief Primary class for the tileset editor
***
*** This class contains an instance of a TilesetDisplay and several buttons to
*** enact different file operations.
***
*** \todo Add a SaveFileAs() method and button
*** ***************************************************************************/
class TilesetEditor : public QDialog {
	//! \brief Macro needed to use Qt's slots and signals.
	Q_OBJECT

public:
	//! \param parent The widget from which this dialog was invoked.
	TilesetEditor(QWidget* parent);

	~TilesetEditor();

private slots:
	//! \brief Creates a new tileset definition file by loading a tileset image
	void _NewFile();

	//! \brief Loads a tileset definition file and all relevant data
	void _OpenFile();

	//! \brief Saves the modified data to the tileset definition file
	void _SaveFile();

private:
	//! \brief The tileset display, also containing the tileset data itself
	TilesetDisplay _tileset_display;

	//! \brief  The view widget for the tileset display
	QGraphicsView* _tileset_view;

	//! \brief A push button for creating a new tileset
	QPushButton* _new_button;

	//! \brief A push button for opening an existing tileset
	QPushButton* _open_button;

	//! \brief A push button for saving the current tileset
	QPushButton* _save_button;

	//! \brief A push button for exiting out of the tileset editor
	QPushButton* _exit_button;

	//! \brief A layout to manage all the labels, spinboxes, and listviews.
	QGridLayout* _grid_layout;
}; // class TilesetEditor : public QDialog

} // namespace hoa_editor
