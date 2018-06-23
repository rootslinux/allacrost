///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_tiles.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for map mode tile management.
*** ***************************************************************************/

// Allacrost engines
#include "script.h"
#include "video.h"

// Local map mode headers
#include "map.h"
#include "map_tiles.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_script;
using namespace hoa_video;

namespace hoa_map {

namespace private_map {

void TileLayer::Draw(MAP_CONTEXT context) const {
	MapMode::CurrentInstance()->GetTileSupervisor()->DrawTileLayer(_tile_layer_id, context);
}



TileSupervisor::TileSupervisor() :
	_row_count(0),
	_column_count(0)
{}



TileSupervisor::~TileSupervisor() {
	// Delete all objects in _tile_images but *not* _animated_tile_images.
	// This is because _animated_tile_images is a subset of _tile_images.
	for (uint32 i = 0; i < _tile_images.size(); i++)
		delete(_tile_images[i]);

	_tile_grid.clear();
	_tile_images.clear();
	_animated_tile_images.clear();
}



MAP_CONTEXT TileSupervisor::GetInheritedContext(MAP_CONTEXT context) {
	map<MAP_CONTEXT, MAP_CONTEXT>::iterator context_mapping = _inherited_contexts.find(context);
	if (context_mapping != _inherited_contexts.end()) {
		return context_mapping->second;
	}
	else {
		IF_PRINT_WARNING(MAP_DEBUG) << "no context with the requested ID exists: " << context << endl;
		return MAP_CONTEXT_NONE;
	}
}



void TileSupervisor::Load(ReadScriptDescriptor& map_file) {
	// TODO: Add some more error checking in this function (such as checking for script errors after reading blocks of data from the map file)

	// ---------- (1) Load the map properties and do some basic sanity checks
	_row_count = map_file.ReadUInt("map_height");
	_column_count = map_file.ReadUInt("map_length");
	uint32 tileset_count = map_file.ReadUInt("number_tilesets");
	uint32 tile_layer_count = map_file.ReadUInt("number_tile_layers");
	uint32 map_context_count = map_file.ReadUInt("number_map_contexts");

	if (map_file.GetTableSize("tileset_filenames") != tileset_count) {
		PRINT_ERROR << "the number of tilesets declared does not match the size of the tileset_filenames table" << endl;
		return;
	}

	if (map_file.GetTableSize("tile_layer_names") != tile_layer_count) {
		PRINT_ERROR << "the number of tile layers declared does not match the size of the tile_layer_names table" << endl;
		return;
	}

	if (map_file.GetTableSize("map_context_inheritance") != map_context_count) {
		PRINT_ERROR << "the number of map contexts declared does not match the size of the map_context_inheritance table" << endl;
		return;
	}

	// For collision_grid and map_tiles tables, we only check that the number of rows are correct and not columns in the interest of reducing load time
	if (map_file.GetTableSize("collision_grid") != _row_count * 2) {
		PRINT_ERROR << "the collision_grid table size is incorrect" << endl;
		return;
	}


	if (map_file.GetTableSize("map_tiles") != _row_count) {
		PRINT_ERROR << "the map_tiles table size was not equal to the number of tile rows specified by the map" << endl;
		return;
	}

	// ---------- (2) Construct the tile layer and map context containers
	for (uint32 i = 0; i < tile_layer_count; ++i)
		_tile_layers.push_back(TileLayer(i));

	vector<MAP_CONTEXT> map_contexts;
	vector<int32> context_inheritance;
	map_file.ReadIntVector("map_context_inheritance", context_inheritance);

	// For each context, populate the map_context vector and the _inherited_contexts map
	for (uint32 i = 0; i < map_context_count; ++i) {
		context_inheritance[i] -= 1; // The map file enumerates contexts from 1..n, so we decrement this value to the range 0..n-1

		MAP_CONTEXT context = static_cast<MAP_CONTEXT>(1 << (i));
		MAP_CONTEXT inherited_context = MAP_CONTEXT_NONE;
		// Check if this context inherits or not. If so, translate the integer value into the context ID
		if (context_inheritance[i] >= 0) {
			inherited_context = static_cast<MAP_CONTEXT>(1 << (context_inheritance[i]));
		}

		map_contexts.push_back(context);
		_inherited_contexts.insert(pair<MAP_CONTEXT, MAP_CONTEXT>(context, inherited_context));
	}

	// ---------- (3) Load all of the tileset images that are used by this map
	// Contains all of the definition filenames used for each tileset
	vector<string> tileset_definition_filenames;
	// The image filename corresponding to each tileset definition
	vector<string> image_filenames;
	// Temporarily retains all tile images loaded for each tileset. Each inner vector contains 256 StillImage objects
	vector<vector<StillImage> > tileset_images;

	// First we have to load the definition file for each tileset and retrieve the corresponding image filename in it
	ReadScriptDescriptor definition_file;
	map_file.ReadStringVector("tileset_filenames", tileset_definition_filenames);
	for (uint32 i = 0; i < tileset_count; ++i) {
		if (definition_file.OpenFile(tileset_definition_filenames[i]) == false) {
			PRINT_ERROR << "failed to load tileset definition file: " << tileset_definition_filenames[i] << endl;
			exit(1);
		}
		definition_file.OpenTable(DetermineLuaFileTablespaceName(tileset_definition_filenames[i]));
		image_filenames.push_back(definition_file.ReadString("image"));
		definition_file.CloseFile();
	}

	// Prepare the container to hold the tile image files and load each tileset into them
	for (uint32 i = 0; i < tileset_count; i++) {
		tileset_images.push_back(vector<StillImage>(TILES_PER_TILESET));
		// The map mode coordinate system used corresponds to a tile size of (2.0, 2.0)
		for (uint32 j = 0; j < TILES_PER_TILESET; j++) {
			tileset_images[i][j].SetDimensions(2.0f, 2.0f);
		}

		// Each tileset image is 512x512 pixels, yielding 16 * 16 (== 256) tiles of 32x32 pixels each
		if (ImageDescriptor::LoadMultiImageFromElementGrid(tileset_images[i], image_filenames[i], 16, 16) == false) {
			PRINT_ERROR << "failed to load tileset image: " << image_filenames[i] << endl;
			exit(1);
		}
	}

	// ---------- (4) Read in the map tile data for all layers and all contexts
	// Tilesets contain a total of 256 tiles each, so 0-255 correspond to the first tileset, 256-511 the second, etc. The tile location
	// within the tileset is also determined by the value, where the first 16 indeces in the tileset range are the tiles of the first row
	// (left to right), and so on.

	// First allocate all of the tile objects needed for each context before reading in the tile data
	MapTile blank_tile(tile_layer_count); // Used to size each MapTile in the _tile_grid appropriately
	for (uint32 i = 0; i < map_context_count; ++i) {
		_tile_grid.insert(make_pair(map_contexts[i], vector<vector<MapTile> >(_row_count)));
		for (uint32 r = 0; r < _row_count; r++) {
			_tile_grid[map_contexts[i]][r].resize(_column_count, blank_tile);
		}
	}

	// Now read in all of the tile data and write it to the correct location in the _tile_grid
	vector<int32> tile_data;
	map_file.OpenTable("map_tiles");
	for (uint32 y = 0; y < _row_count; ++y) {
		map_file.OpenTable(y);
		for (uint32 x = 0; x < _column_count; ++x) {
			tile_data.clear();
			map_file.ReadIntVector(x, tile_data);
			for (uint32 c = 0; c < map_context_count; ++c) {
				MAP_CONTEXT context = map_contexts[c];
				for (uint32 l = 0, data_index = c * tile_layer_count; l < tile_layer_count; ++l, ++data_index) {
					_tile_grid[context][y][x].tile_layers[l] = tile_data[data_index];
				}
			}
		}
		map_file.CloseTable();
	}
	map_file.CloseTable();

	// ---------- (5) Determine which tiles in each tileset are referenced in this map
	// Used to determine whether each tile is used by the map or not. An entry of UNREFERENCED_TILE indicates that particular tile is not used
	vector<int16> tile_references;
	// Set size to be equal to the total number of tiles and initialize all entries to unrefereced
	tile_references.assign(tileset_count * TILES_PER_TILESET, UNREFERENCED_TILE);

	for (map<MAP_CONTEXT, vector<vector<MapTile> > >::iterator i = _tile_grid.begin(); i != _tile_grid.end(); i++) {
		for (uint32 r = 0; r < _row_count; r++) {
			for (uint32 c = 0; c < _column_count; c++) {
				for (uint32 l = 0; l < tile_layer_count; l++) {
					if ((i->second)[r][c].tile_layers[l] >= 0)
						tile_references[(i->second)[r][c].tile_layers[l]] = 0;
				}
			}
		}
	}

	// ---------- (6) Translate the tileset tile indeces into indeces for the vector of tile images
	// Here, we have to convert the original tile indeces defined in the map file into a new form. The original index
	// indicates the tileset where the tile is used and its location in that tileset. We need to convert those indeces
	// so that they serve as an index to the MapMode::_tile_images vector, where the tile images will soon be stored.

	// Keeps track of the next translated index number to assign
	uint32 next_index = 0;

	for (uint32 i = 0; i < tile_references.size(); i++) {
		if (tile_references[i] >= 0) {
			tile_references[i] = next_index;
			next_index++;
		}
	}

	// Now, go back and re-assign all tile layer indeces with the translated indeces
	for (map<MAP_CONTEXT, vector<vector<MapTile> > >::iterator i = _tile_grid.begin(); i != _tile_grid.end(); i++) {
		for (uint32 r = 0; r < _row_count; r++) {
			for (uint32 c = 0; c < _column_count; c++) {
				for (uint32 l = 0; l < tile_layer_count; l++) {
					if ((i->second)[r][c].tile_layers[l] >= 0)
						(i->second)[r][c].tile_layers[l] = tile_references[(i->second)[r][c].tile_layers[l]];
				}
			}
		}
	}

	// ---------- (7) Parse all of the tileset definition files and create any animated tile images that will be used
	// Temporarily retains the animation data (every two elements corresponds to a pair of tile frame index and display time)
	vector<uint32> animation_info;
	// Temporarily holds all animated tile images. The map key is the value of the tile index, before reference translation is done in the next step
	map<uint32, AnimatedImage*> tile_animations;

	for (uint32 i = 0; i < tileset_definition_filenames.size(); i++) {
		if (definition_file.OpenFile(tileset_definition_filenames[i]) == false) {
			PRINT_ERROR << "map failed to load because it could not open a tileset definition file: " << tileset_definition_filenames[i] << endl;
			exit(1);
		}
		definition_file.OpenTable(DetermineLuaFileTablespaceName(tileset_definition_filenames[i]));

		if (definition_file.DoesTableExist("animations") == true) {
			definition_file.OpenTable("animations");
			for (uint32 j = 1; j <= definition_file.GetTableSize(); j++) {
				animation_info.clear();
				definition_file.ReadUIntVector(j, animation_info);

				// The index of the first frame in the animation. (i * TILES_PER_TILESET) factors in which tileset the frame comes from
				uint32 first_frame_index = animation_info[0] + (i * TILES_PER_TILESET);

				// If the first tile frame index of this animation was not referenced anywhere in the map, then the animation is unused and
				// we can safely skip over it and move on to the next one. Otherwise if it is referenced, we have to construct the animated image
				if (tile_references[first_frame_index] == UNREFERENCED_TILE) {
					continue;
				}

				AnimatedImage* new_animation = new AnimatedImage();
				new_animation->SetDimensions(2.0f, 2.0f);

				// Each pair of entries in the animation info indicate the tile frame index (k) and the time (k+1)
				for (uint32 k = 0; k < animation_info.size(); k += 2) {
					new_animation->AddFrame(tileset_images[i][animation_info[k]], animation_info[k+1]);
				}
				tile_animations.insert(make_pair(first_frame_index, new_animation));
			}
			definition_file.CloseTable();
		}

		definition_file.CloseTable();
		definition_file.CloseFile();
	}

	// ---------- (8) Add all referenced tiles to the _tile_images vector, in the proper order
	for (uint32 i = 0; i < tileset_images.size(); i++) {
		for (uint32 j = 0; j < TILES_PER_TILESET; j++) {
			uint32 reference = (i * TILES_PER_TILESET) + j;

			if (tile_references[reference] >= 0) {
				// Add the tile as a StillImage
				if (tile_animations.find(reference) == tile_animations.end()) {
					_tile_images.push_back(new StillImage(tileset_images[i][j]));
				}

				// Add the tile as an AnimatedImage
				else {
					_tile_images.push_back(tile_animations[reference]);
					_animated_tile_images.push_back(tile_animations[reference]);
					tile_animations.erase(reference);
				}
			}
		}
	}

	if (tile_animations.empty() == false) {
		IF_PRINT_WARNING(MAP_DEBUG) << "one or more tile animations that were created were not added into the map -- this is a memory leak" << endl;
	}

	// Remove all tileset images. Any tiles which were not added to _tile_images will no longer exist in memory
	tileset_images.clear();
} // void TileSupervisor::Load(ReadScriptDescriptor& map_file)



void TileSupervisor::Update() {
	for (uint32 i = 0; i < _animated_tile_images.size(); i++) {
		_animated_tile_images[i]->Update();
	}
}



void TileSupervisor::DrawTileLayer(uint16 layer_index, MAP_CONTEXT context) {
	if (layer_index >= _tile_layers.size()) {
		PRINT_ERROR << "tried to draw a tile layer at an invalid index: " << layer_index << endl;
		return;
	}
	if (context == MAP_CONTEXT_NONE || context == MAP_CONTEXT_ALL) {
		PRINT_ERROR << "invalid context argument: " << context << endl;
		return;
	}

	const MapFrame& frame = MapMode::CurrentInstance()->GetMapFrame();
	MAP_CONTEXT inherited_context = GetInheritedContext(context);

	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->Move(frame.tile_x_start, frame.tile_y_start);
	for (uint32 r = static_cast<uint32>(frame.starting_row); r < static_cast<uint32>(frame.starting_row + frame.num_draw_rows); ++r)	{
		for (uint32 c = static_cast<uint32>(frame.starting_col); c < static_cast<uint32>(frame.starting_col + frame.num_draw_cols); ++c)	{
			// Draw a tile image if it exists at this location
			if (_tile_grid[context][r][c].tile_layers[layer_index] >= 0) {
				_tile_images[_tile_grid[context][r][c].tile_layers[layer_index]]->Draw();
			}
			else if (_tile_grid[context][r][c].tile_layers[layer_index] == INHERITED_TILE) {
				if (_tile_grid[inherited_context][r][c].tile_layers[layer_index] >= 0) {
					_tile_images[_tile_grid[inherited_context][r][c].tile_layers[layer_index]]->Draw();
				}
			}
			VideoManager->MoveRelative(2.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(frame.num_draw_cols * 2), 2.0f);
	}
}

} // namespace private_map

} // namespace hoa_map
