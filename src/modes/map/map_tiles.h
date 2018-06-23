///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_tiles.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for map mode tile management.
***
*** This code encapsulates everything related to tiles and tile management in
*** map mode.
*** ***************************************************************************/

#pragma once

// Allacrost utilities
#include "defs.h"
#include "utils.h"

// Local map mode headers
#include "map_utils.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief Holds the indeces to the images used for a particular tile on the map
***
*** The images that a tile uses are not stored within this class. This class
*** only holds indices to the container class holding those images. This class
*** also does not contain any information about the collision grid. That information
*** is maintained in the map object manager.
***
*** \note The reason that tiles do not contain collision information is that
*** each tile is 32x32 pixels, but collision is defined on a 16x16 granularity,
*** meaning that there are four collision sections to each tile. Certain code
*** such as pathfinding is more simple if all collision information is kept in
*** in another form of container.
*** ***************************************************************************/
class MapTile {
public:
	/** \name Tile Layer Indeces
	*** \brief Indeces to the tile image container, where each element corresponds to a different tile layer
	*** \note A negative value means that no image is registered to that tile layer
	**/
	std::vector<int16> tile_layers;

	MapTile()
		{}

	MapTile(uint32 layer_count) :
		tile_layers(layer_count, -1) { tile_layers.shrink_to_fit(); }
}; // class MapTile


/** ****************************************************************************
*** \brief Represents a layer of tiles on a map independently of any map context
***
*** This class does little more than enable layers of tiles to be drawn interspersed with
*** layers of objects and sprites. All of the heavy lifting is done by the TileSupervisor
*** class.
*** ***************************************************************************/
class TileLayer : public MapLayer {
public:
	TileLayer(uint32 id) :
		MapLayer(), _tile_layer_id(id) {}

	uint32 GetTileLayerID() const
		{ return _tile_layer_id; }

	//! \brief Does nothing. Animated tiles are updated by the TileSupervisor class across all layers
	void Update()
		{}

	/** \brief Makes the appropriate call to TileSupervisor::DrawTileLayer()
	*** \param context The context of the layer that should be drawn
	**/
	void Draw(MAP_CONTEXT context) const;

private:
	//! \brief Holds the unique ID of this tile layer. The first layer created for a map should use the value DEFAULT_LAYER_ID
	uint32 _tile_layer_id;
}; // class TileLayer : public MapLayer


/** ****************************************************************************
*** \brief A helper class to MapMode responsible for all tile data and operations
***
*** This class is responsible for loading, updating, and drawing all tile images
*** and managing the tile grid. The TileSupervisor does not manage the map
*** collision grid, which is used by map objects and sprites.
***
*** Maps have a minimum size of 24 rows and 32 columns of tiles. There is no hard
*** limit there is no upper limit on map size.
*** ***************************************************************************/
class TileSupervisor {
	friend class hoa_map::MapMode;

public:
	TileSupervisor();

	~TileSupervisor();

	//! \name Class Member Accessor Methods
	//@{
	uint16 GetRowCount() const
		{ return _row_count; }

	uint16 GetColumnCount() const
		{ return _column_count; }

	uint32 GetTileLayerCount() const
		{ return _tile_layers.size(); }

	/** \brief Retrieves a pointer to a layer object with a specified ID
	*** \param layer_id The ID of the layer requested
	*** \return A pointer to the layer object, or nullptr if no layer existed with that layer ID
	**/
	TileLayer* GetTileLayer(uint32 layer_id)
		{ if (layer_id >= _tile_layers.size()) return nullptr; else return &_tile_layers[layer_id]; }

	/** \brief Retrieves the inherting context for the given context
	*** \param context The context to retrieve the inheriting context for
	*** \return The inherited context ID. If the context does not inherit or does not exist, returns MAP_CONTEXT_NONE
	**/
	MAP_CONTEXT GetInheritedContext(MAP_CONTEXT context);
	//@}

	/** \brief Handles all operations on loading tilesets and tile images from the map data file
	*** \param map_file A reference to the Lua file containing the map data
	*** \note The map file should already be opened with no Lua tables open
	**/
	void Load(hoa_script::ReadScriptDescriptor& map_file);

	//! \brief Updates all animated tile images
	void Update();

	/** \brief Draws a tile layer to the screen
	*** \param layer_index The index of the layer that should be drawn
	*** \param context The context of the tile layer that should be drawn
	***
	*** \note This function does not reset the coordinate system and hence require that the proper coordinate system is
	*** already set prior to this function call (0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f). These functions do make
	*** modifications to the blending draw flag and the draw cursor position which are not restored by the function upon
	*** its return, so take measures to retain this information before calling these functions if necessary.
	**/
	void DrawTileLayer(uint16 layer_index, MAP_CONTEXT context);

private:
	/** \brief The number of rows of tiles in the map.
	*** This number must be greater than or equal to 24 for the map to be valid.
	**/
	uint16 _row_count;

	/** \brief The number of columns of tiles in the map.
	*** This number must be greater than or equal to 32 for the map to be valid.
	**/
	uint16 _column_count;

	//! \brief Holds a TileLayer object for each tile layer loaded from the map
	std::vector<TileLayer> _tile_layers;

	//! \brief A mapping of each context to the context that it inherits from. Set to MAP_CONTEXT_NONE for a context that does not inherit
	std::map<MAP_CONTEXT, MAP_CONTEXT> _inherited_contexts;

	/** \brief A map of 2D vectors that contains all of the map's tile objects.
	*** Each key-value pair in the std::map represents a map context, thus the size of the std::map is equal to
	*** number of contexts in the game map (up to 32). The 2D vector represents the rows and columns of tiles,
	*** respectively, for the given map context.
	**/
	std::map<MAP_CONTEXT, std::vector<std::vector<MapTile> > > _tile_grid;

	//! \brief Contains the image objects for all map tiles, both still and animated.
	std::vector<hoa_video::ImageDescriptor*> _tile_images;

	/** \brief Contains all of the animated tile images used on the map.
	*** The purpose of this vector is to easily update all tile animations without stepping through the
	*** _tile_images vector, which contains both still and animated images.
	**/
	std::vector<hoa_video::AnimatedImage*> _animated_tile_images;
}; // class TileSupervisor

} // namespace private_map

} // namespace hoa_map
