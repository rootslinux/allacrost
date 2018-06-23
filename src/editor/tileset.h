///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    tileset.h
*** \author  Philip Vorsilak (gorzuate)
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for tileset data and display code
*** **************************************************************************/

#pragma once

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QString>
#include <QTabWidget>
#include <QTableWidget>

#include "editor_utils.h"
#include <boost/concept_check.hpp>

namespace hoa_editor {

class MapData;

/** ***************************************************************************
*** \brief Represents one frame of an animated tile
***
*** Animated tiles are created from two or more tile images in the same tileset.
*** Each tile in the animation sequence has a display time indicatin how long the
*** frame should remain visible for.
*** **************************************************************************/
class AnimatedTileData {
public:
	AnimatedTileData(uint32 tile, uint32 frame_time) :
		tile_id(tile), time(frame_time) {}

	//! \brief Index into tileset that represents tile which will be part of the animation sequence.
	uint32 tile_id;

	//! \brief Time in milliseconds to display this particular tile.
	uint32 time;
};


/** ***************************************************************************
*** \brief Retains a tileset's image and other properties
***
*** Tileset data comes from two different files:
***     - The tileset image file (.png), located in img/tilesets
***     - The tileset data file (.lua), located in lua/data/tilesets
***
*** Every tileset image file (TIF) has the same dimensions of 512x512 pixels. This means
*** that they can all hold 256 32x32 pixel tiles in 16 rows and 16 columns. Although
*** not every tile area within a tileset image is guaranteed to contain image data
*** (some tile areas may simply be empty/transparent space).
***
*** The tileset definition file (TDF) defineds the properties of the tileset file.
*** The name of the TDF matches the name of the TIF. So for example, a TIF named
*** "desert_landscape.png" would have a corresponding TDF named "desert_landscape.lua".
*** The TDF contains a user-friendly name of the tileset (as it will be seen in the editor),
*** collision information about which quadrants of each tile may not be moved over by
*** sprites on a map, any animations built from tiles in the tileset, and autotile data
*** that is used to improve map editing.
***
*** This class is responsible for the loading of data from the TDF and TIF and saving
*** modified data back to the TDF. The TIF is never modified by this class.
***
*** \note It is possible for multiple TDFs to map to a single TIF, although it would be
*** highly uncommon. One reason might be if  you wanted certain tiles to have different
*** collision properties on one map versus another. In this case, obviously you couldn't
*** match the name of both TDFs to the TIF file. This is why the TDF contains the name of
*** the TIF it uses.
*** **************************************************************************/
class Tileset {
public:
	Tileset();

	virtual ~Tileset()
		{ _ClearData(); }

	//! \brief Class member accessor functions
	//@{
	bool IsInitialized() const
		{ return _initialized; }

	const QString& GetTilesetName() const
		{ return _tileset_name; }

	const QString& GetTilesetImageFilename() const
		{ return _tileset_image_filename; }

	const QString& GetTilesetDefinitionFilename() const
		{ return _tileset_definition_filename; }

	const std::vector<QPixmap>& GetTileImages() const
		{ return _tile_images; }

	const std::vector<uint32>& GetTileCollisions() const
		{ return _tile_collisions; }
	//@}

	/** \brief Creates a new tileset object using only a tileset image
	*** \param img_filename The path and name of the image file to use for the new tileset
	*** \param single_image If true, the tiles vector will contain a single image for the entire tileset
	*** \return True if the tileset image was loaded successfully
	***
	*** \note There will be no tileset definition filename until the SaveAs() function is called. Calling
	*** the Save() function will result in an error until SaveAs() returns successfully.
	**/
	virtual bool New(const QString& img_filename, bool single_image = false);

	/** \brief Loads a tileset definition file and image file and populates the data containers for the object
	*** \param def_filename The name of the tileset definition file to load the data from
	*** \param single_image If true, the tiles vector will contain a single image for the entire tileset
	*** \return True if all tileset data was loaded successfully
	*** \note This function will clear the previously loaded contents when it is called
	*** \note If the load operation fails, any and all existing data will remain cleared
	**/
	virtual bool Load(const QString& def_filename, bool single_image = false);

	/** \brief Saves the tileset data to its tileset definition file
	*** \return True if the save operation was successful
	**/
	bool Save();

	/** \brief Saves the tileset data into a new tileset definition file
	*** \param def_filename The name and path of the new definition file to save the data to
	*** \return True if the save operation is successful
	*** \note The _tileset_definition_filename will be updated if and only if the function returns true
	*** \note If the file with the givien filename already exists, it will be overwritten without warning
	**/
	bool SaveAs(const QString& def_filename);

	/** \brief Returns the tile image corresponding to a specific index
	*** \param index The index of the image to retrieve, should be between 0 and (TILESET_NUM_ROWS * TILESET_NUM_COLS - 1)
	*** \return A pointer to the image, or nullptr if the index was out-of-bounds
	**/
	const QPixmap* GetTileImage(uint32 index) const
		{ if (index >= _tile_images.size()) return nullptr; else return &_tile_images[index]; }

	/** \brief Returns the tile image corresponding to a specific x and y location on the tileset
	*** \param x The x coordinator of the tile, equivalent to the tile column
	*** \param y The y coordinator of the tile, equivalent to the tile row
	*** \return A pointer to the image, or nullptr if the coordinates were out-of-bounds
	**/
	const QPixmap* GetTileImage(uint32 x, uint32 y) const
		{ return GetTileImage((y * TILESET_NUM_ROWS) + x); }

	/** \brief Retrieves a pointer to the entire tileset image
	*** \note This method is only valid if the tileset was loaded with the single_image argument of New() or Load()
	*** set to true. Otherwise it will just return the first tile in the tileset image
	**/
	const QPixmap* GetTilesetImage() const
		{ return &_tile_images[0]; }

	/** \brief Used to retrieve the collision data for a specific quadrant
	*** \param index The index of the data into _tile_collisions to retrieve
	*** \return The value of the collision quadrant. Returns 0 if the index was invalid
	**/
	uint32 GetQuadrantCollision(uint32 index) const
		{ if (index >= _tile_collisions.size()) return 0; else return _tile_collisions[index]; }

	/** \brief Used to set the collision data for a specific quadrant
	*** \param index The index of the data into _tile_collisions to set
	*** \param value The value to set the collision data (should be a 0 or 1)
	**/
	void SetQuadrantCollision(uint32 index, uint32 value)
		{ if (index >= _tile_collisions.size()) return; else _tile_collisions[index] = value; }

protected:
	//! \brief True when the class is holding loaded tileset data.
	bool _initialized;

	//! \brief The name of the tileset that will be seen in the editor
	QString _tileset_name;

	//! \brief The name and path of the tileset image file
	QString _tileset_image_filename;

	//! \brief The name and path of the tileset definition file
	QString _tileset_definition_filename;

	/** \brief Contains the QPixmap image for each tile in the tileset
	*** \note The QPixmap class is optimized to show images on screen, but QImage is used for image data loading
	**/
	std::vector<QPixmap> _tile_images;

	/** \brief Holds the collision data for each quadrant of every tile
	***
	*** The size of this container will always be four times the size of the image_tiles container. Every entry in
	*** is either a 1 or a 0, where a 1 indicates that quadrant has a collision. Every consecutive four entries corresponds
	*** to the data for one tile. For a set of entries {A, B, C, D} for tile at index X, the entries correspond to:
	***   - A: Northwest quadrant
	***   - B: Northeast quadrant
	***   - C: Southwest quadrant
	***   - D: Southeast quadrant
	**/
	std::vector<uint32> _tile_collisions;

	//! \brief Contains all information for any animated tile
	std::vector<std::vector<AnimatedTileData> > _tile_animations;

	/** \brief Contains all of the information for every autotileable tile
	*** \TODO What are the integer key and string value for?
	**/
	std::map<uint32, std::string> _tile_autotiles;

	//! \brief Clears all data and sets the _initialized member to false
	void _ClearData();

	/** \brief Loads the tileset image and populates the _tile_images container with each tile image
	*** \param single_image When true, the tileset image will be loaded into a single entry in _tile_images
	*** \return True if the image data was loaded successfully
	***
	*** This is a helper function to New() and Load(). It loads the data from the file described by
	*** _tileset_image_filename, so this member must be set prior to calling this function.
	**/
	bool _LoadImageData(bool single_image);

	/** \brief Creates a default name for the tileset from a a filename
	*** \param filename The filename to use
	***
	*** This method is useful when creating a new tileset or when using a tileset
	*** definition file that does not have a tileset name defined. The name is
	*** constructed by removing the path and file extention from the filename.
	**/
	void _CreateTilesetNameFromFilename(const QString &filename);
}; // class Tileset


/** ***************************************************************************
*** \brief Visualizes a tileset as a QTableWidget
***
*** This class creates a copy of all of the image data from a tileset and populates
*** a 2D grid where each tile represents an element. The class maintains a pointer
*** to the most recent Tileset object that was used in loading the image data, but
*** it does not require that the Tileset object remain valid as the pointer is not
*** used again after the load completes. Still, you should be mindful of any
*** TilesetTable objects that may exist when deleting a Tileset object, as it may
*** not be the case that you still want to visually represent a Tileset object that
*** is no longer active.
***
*** \todo Add support for displaying and editing animated tiles
*** **************************************************************************/
class TilesetTable : public QTableWidget {
public:
	TilesetTable();

	/** \brief Constructs the object and immediately invokes the Load() method
	*** \param tileset A pointer to the tileset to pass to the Load() method
	**/
	TilesetTable(Tileset* tileset);

	~TilesetTable()
		{ Clear(); }

	Tileset* GetTileset() const
		{ return _tileset; }

	//! \brief Clears all loaded tileset and image data
	void Clear()
		{ clear(); _tileset = nullptr; }

	/** \brief Populates the images of table with the tiles from a tileset
	*** \param tileset A pointer to the tileset to use, which should already be initialized
	*** \return True if the image data was loaded into the table successfully
	**/
	bool Load(Tileset* tileset);

private:
	//! \brief A pointer to the most recent tileset object that the table loaded image data from
	Tileset* _tileset;
}; // class TilesetTable : public QTableWidget


/** ***************************************************************************
*** \brief GUI widget that displays all open tilesets in tabbed windows
***
*** This class is placed in the lower right corner of the main window. Each tileset
*** opened by the map is placed in its own tab, with the tab name corresponding to the
*** name of the tileset.
***
*** The tabs and their ordering in this widget should reflect the tilesets and their
*** ordering from the map data at all times.
***
*** \todo Allow reordering of tabs within this widget by enabling setMovable(). Any tab
*** reordering will need to reorder the tileset in the MapData object as well.
*** **************************************************************************/
class TilesetView : public QTabWidget {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	/** \param parent The parent widget of this object
	*** \param data A pointer to the map data to manipulate and draw
	**/
	TilesetView(QWidget* parent, MapData* data);

	~TilesetView();

	//! \name Class Member Accessor Methods
	//@{
	TilesetTable* GetCurrentTilesetTable()
		{ return static_cast<TilesetTable*>(currentWidget()); }

	//! \brief Returns the value that all tiles in this tileset should be multiplied by before being placed in the map data
	int32 GetCurrentTilesetIndex() const
		{ return _current_tileset_index; }
	//@}

	//! \brief Removes all tileset tabs from the widget
	void ClearData();

	//! \brief Clears all existing data and reconstructs all the tabs from the tilesets loaded in the map data
	void RefreshView();

private:
	//! \brief A pointer to the active map data that contains the tile layers
	MapData* _map_data;

	TilesetTable* _current_tileset_table;

	/** \brief Holds the index value corresponding to the current tab
	*** Set to -1 when no tabs are loaded
	**/
	int32 _current_tileset_index;

	//! \brief Menu for right-clicks events on the widget
    QMenu* _right_click_menu;

	/** \name Right-Click Menu Actions
	*** \brief Correspond to the private slots functions for user actions
	**/
	//@{
	QAction* _add_tileset_action;
	QAction* _remove_tileset_action;
	//@}

private slots:
	/** \brief Inherited slot that is called whenever the user right clicks the widget
	*** \param event A pointer to the object that contains the details about the event
	**/
	void contextMenuEvent(QContextMenuEvent* event);

	//! \brief Called whenever the selected tab is changed and sets _current_tileset_table and _current_tileset_multiplier appropriately
	void _CurrentTabChanged();

	//! \brief Opens a dialog window to allow additional tilesets to be opened and added to the widget
	void _OpenAddTilesetDialog();

	//! \brief Removes the current tileset table from the widget and the corresponding tileset from the map data
	void _RemoveCurrentTileset();
}; // class TilesetView : public QTabWidget

} // namespace hoa_editor
