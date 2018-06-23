///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    map_data.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for map data class
*** **************************************************************************/

#include "script.h"

#include "editor_utils.h"
#include "map_data.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_script;

namespace hoa_editor {

///////////////////////////////////////////////////////////////////////////////
// MapData class -- General Functions
///////////////////////////////////////////////////////////////////////////////

MapData::MapData() :
	_map_filename(""),
	_map_name(""),
	_map_designers(""),
	_map_description(""),
	_map_length(0),
	_map_height(0),
	_map_modified(false),
	_tile_layer_count(0),
	_tile_context_count(0),
	_selected_tile_context(nullptr),
	_selected_tile_layer(nullptr),
	_selected_tile_layer_properties(nullptr),
	_all_tile_contexts(MAX_CONTEXTS, nullptr),
	_error_message("")
{}



bool MapData::CreateData(uint32 map_length, uint32 map_height) {
	if (IsInitialized() == true) {
		return false;
	}

	_map_length = map_length;
	_map_height = map_height;
	_empty_tile_layer.ResizeLayer(map_length, map_height);
	_empty_tile_layer.FillLayer(MISSING_TILE);

	// Create three tile layers, the last of which has no collision enabled initially
	_tile_layer_properties.push_back(TileLayerProperties(QString("Ground"), true, true));
	_tile_layer_properties.push_back(TileLayerProperties(QString("Middle"), true, true));
	_tile_layer_properties.push_back(TileLayerProperties(QString("Sky"), true, false));
	_tile_layer_count = 3;

	// Create a single TileContext called "Base"
	TileContext* new_context = new TileContext(1, "Base"); // Given an ID of 1 since no other contexts exist yet
	new_context->_AddTileLayer(_empty_tile_layer);
	new_context->_AddTileLayer(_empty_tile_layer);
	new_context->_AddTileLayer(_empty_tile_layer);
	_all_tile_contexts[0] = new_context;
	_tile_context_count = 1;

	_selected_tile_context = new_context;
	_selected_tile_layer = new_context->GetTileLayer(0);
	_selected_tile_layer_properties = &_tile_layer_properties[0];
	SetMapModified(true);
	return true;
}



void MapData::DestroyData() {
	_map_filename = "";
	_map_name = "";
	_map_length = 0;
	_map_height = 0;

	for (uint32 i = 0; i < _tilesets.size(); ++i) {
		delete _tilesets[i];
	}
	_tilesets.clear();

	for (uint32 i = 0; i < _all_tile_contexts.size(); ++i) {
		if (_all_tile_contexts[i] == nullptr)
			break;

		delete _all_tile_contexts[i];
		_all_tile_contexts[i] = nullptr;
	}

	_tile_context_count = 0;
	_tile_layer_count = 0;
	_tile_layer_properties.clear();
	_selected_tile_context = nullptr;
	_selected_tile_layer = nullptr;
	_selected_tile_layer_properties = nullptr;

	_error_message = "";
	SetMapModified(false);
}



bool MapData::LoadData(QString filename) {
	if (IsInitialized() == true) {
		return false;
	}

	// ---------- (1): Open the file and open the tablespace table, then clear any existing data before reading begins
	ReadScriptDescriptor data_file;
	if (data_file.OpenFile(string(filename.toLatin1()), true) == false) {
		_error_message = "Could not open file " + filename + " for reading.";
		return false;
	}

	string tablespace = DetermineLuaFileTablespaceName(filename.toStdString());
	if (data_file.DoesTableExist(tablespace) == false) {
		_error_message = "Map file " + filename.toLatin1() + " did not have the expected namespace table " + tablespace.c_str();
		return false;
	}

	data_file.OpenTable(tablespace);

	DestroyData();
	_map_filename = filename;

	// ---------- (2): Read the basic map data properties
	_map_name = QString::fromStdString(data_file.ReadString("map_name"));
	_map_designers = QString::fromStdString(data_file.ReadString("map_designers"));
	_map_description = QString::fromStdString(data_file.ReadString("map_description"));
	_map_length = data_file.ReadUInt("map_length");
	_map_height = data_file.ReadUInt("map_height");
 	uint32 number_tilesets = data_file.ReadUInt("number_tilesets");
	_tile_layer_count = data_file.ReadUInt("number_tile_layers");
	_tile_context_count = data_file.ReadUInt("number_map_contexts");
	_empty_tile_layer.ResizeLayer(_map_length, _map_height);
	_empty_tile_layer.FillLayer(MISSING_TILE);

	if (_map_length < MINIMUM_MAP_LENGTH) {
		_error_message = QString("Error when loading map file. Map was smaller (%1) than the minimum length.").arg(_map_length);
		data_file.CloseTable();
		data_file.CloseFile();
		DestroyData();
		return false;
	}
	if (_map_height < MINIMUM_MAP_HEIGHT) {
		_error_message = QString("Error when loading map file. Map was smaller (%1) than the minimum height.").arg(_map_height);
		data_file.CloseTable();
		data_file.CloseFile();
		DestroyData();
		return false;
	}
	if (_tile_layer_count == 0) {
		_error_message = QString("Error when loading map file. Map did not have any tile layers.");
		data_file.CloseTable();
		data_file.CloseFile();
		DestroyData();
		return false;
	}
	if (_tile_context_count == 0) {
		_error_message = QString("Error when loading map file. Map did not have any contexts.");
		data_file.CloseTable();
		data_file.CloseFile();
		DestroyData();
		return false;
	}

	// ---------- (3): Construct each tileset object for the map
	vector<string> tileset_filenames;
	data_file.ReadStringVector("tileset_filenames", tileset_filenames);
	if (tileset_filenames.empty() == true) {
		_error_message = QString("Error when loading map file. Map did use any tile contexts.");
		data_file.CloseTable();
		data_file.CloseFile();
		DestroyData();
		return false;
	}

	for (uint32 i = 0; i < number_tilesets; ++i) {
		Tileset* tileset = new Tileset();
		QString tileset_qname = QString::fromStdString(tileset_filenames[i].c_str());
		if (tileset->Load(tileset_qname) == false) {
			_error_message = QString("Failed to load tileset file ") + tileset_qname + QString(" during loading of map file ") + _map_filename;
			delete tileset;
			return false;
		}
		AddTileset(tileset);
	}

	// ---------- (4): Read in the properties of tile layers and tile contexts
	vector<string> tile_layer_names;
	vector<bool> tile_layer_collision_enabled;
	data_file.ReadStringVector("tile_layer_names", tile_layer_names);
	data_file.ReadBoolVector("tile_layer_collision_enabled", tile_layer_collision_enabled);

	vector<string> tile_context_names;
	vector<int32> tile_context_inheritance;
	data_file.ReadStringVector("map_context_names", tile_context_names);
	data_file.ReadIntVector("map_context_inheritance", tile_context_inheritance);

	// ---------- (5): Construct each tile context and layer and initialize it with empty data
	for (uint32 i = 0; i < _tile_layer_count; ++i) {
		_tile_layer_properties.push_back(TileLayerProperties(QString::fromStdString(tile_layer_names[i]), true, tile_layer_collision_enabled[i]));
	}

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		TileContext* new_context = new TileContext(i + 1, QString::fromStdString(tile_context_names[i]));
		if (tile_context_inheritance[i] != INVALID_CONTEXT) {
			new_context->_SetInheritingContext(tile_context_inheritance[i]);
		}
		for (uint32 j = 0; j < _tile_layer_count; ++j) {
			new_context->_AddTileLayer(_empty_tile_layer);
		}

		_all_tile_contexts[i] = new_context;
	}

	_selected_tile_context = _all_tile_contexts[0];
	_selected_tile_layer = _selected_tile_context->GetTileLayer(0);
	_selected_tile_layer_properties = &_tile_layer_properties[0];

	// ---------- (6): Read in the collision grid data
	_collision_data.resize(_map_height * 2);
	for (uint32 y = 0; y < _map_height * 2; ++y) {
		_collision_data[y].reserve(_map_length * 2);
	}

	data_file.OpenTable("collision_grid");
	for (uint32 y = 0; y < _map_height * 2; ++y) {
		data_file.ReadUIntVector(y, _collision_data[y]);
	}
	data_file.CloseTable();

	// ---------- (7): Read the map tile data into the appropriate layers of each tile context
	vector<int32> tile_data; // Container used to read in all the data for a tile corresponding to one X, Y coordinate
	tile_data.reserve(_tile_context_count * _tile_layer_count);

	vector<vector<std::vector<int32> >* > layer_tiles; // Holds pointers to the tile vectors within each context and layer
	for (uint32 c = 0; c < _tile_context_count; ++c) {
		for (uint32 l = 0; l < _tile_layer_count; ++l) {
			layer_tiles.push_back(&_all_tile_contexts[c]->GetTileLayer(l)->GetTiles());
		}
	}

	data_file.OpenTable("map_tiles");
	for (uint32 y = 0; y < _map_height; ++y) {
		data_file.OpenTable(y);
		for (uint32 x = 0; x < _map_length; ++x) {
			tile_data.clear();
			data_file.ReadIntVector(x, tile_data);
			for (uint32 t = 0; t < tile_data.size(); ++t) {
				(*layer_tiles[t])[y][x] = tile_data[t];
			}
		}
		data_file.CloseTable();
	}
	data_file.CloseTable();

	if (data_file.IsErrorDetected()) {
		_error_message = QString("One or more errors were detected when reading in the map file:\n") + QString::fromStdString(data_file.GetErrorMessages());
		data_file.CloseTable();
		data_file.CloseFile();
		_map_modified = false;
		return false;
	}

	data_file.CloseTable();
	data_file.CloseFile();
	SetMapModified(false);
	return true;
} // bool MapData::LoadData(QString filename)



bool MapData::SaveData(QString filename) {
	if (IsInitialized() == false) {
		return false;
	}

	// ---------- (1): Open the file and write the tablespace header and map header information
	WriteScriptDescriptor data_file;
	if (data_file.OpenFile(filename.toStdString()) == false) {
		_error_message = "Could not open file for writing: " + filename;
		return false;
	}

	_map_filename = filename;
	data_file.WriteNamespace(DetermineLuaFileTablespaceName(filename.toStdString()));
	data_file.InsertNewLine();

	data_file.WriteString("map_name", _map_name.toStdString());
	data_file.WriteString("map_designers", _map_designers.toStdString());
	data_file.WriteString("map_description", _map_description.toStdString());
	data_file.InsertNewLine();

	// ---------- (2): Write the basic map data properties
	data_file.WriteUInt("map_length", _map_length);
	data_file.WriteUInt("map_height", _map_height);
 	data_file.WriteUInt("number_tilesets", _tilesets.size());
	data_file.WriteUInt("number_tile_layers", _tile_layer_count);
	data_file.WriteUInt("number_map_contexts", _tile_context_count);
	data_file.InsertNewLine();

	// ---------- (3): Write properties of tilesets, tile layers, and map contexts
	data_file.BeginTable("tileset_filenames");
	for (uint32 i = 0; i < _tilesets.size(); ++i) {
		data_file.WriteString((i+1), _tilesets[i]->GetTilesetDefinitionFilename().toStdString());
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	data_file.BeginTable("tile_layer_names");
	QStringList layer_names = GetTileLayerNames();
	for (int32 i = 0; i < layer_names.size(); ++i) {
		data_file.WriteString((i+1), layer_names[i].toStdString());
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	data_file.BeginTable("tile_layer_collision_enabled");
	for (uint32 i = 0; i < _tile_layer_properties.size(); ++i) {
		data_file.WriteBool((i+1), _tile_layer_properties[i].IsCollisionEnabled());
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	data_file.BeginTable("map_context_names");
	QStringList context_names = GetTileContextNames();
	for (int32 i = 0; i < context_names.size(); ++i) {
		data_file.WriteString((i+1), context_names[i].toStdString());
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	data_file.BeginTable("map_context_inheritance");
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		data_file.WriteInt((i+1), _all_tile_contexts[i]->GetInheritedContextID());
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	// ---------- (4): Write collision grid data
	data_file.BeginTable("collision_grid");
	_ComputeCollisionData();
	for (uint32 i = 0; i < _collision_data.size(); ++i) {
		data_file.WriteUIntVector(i, _collision_data[i]);
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	// ---------- (5): For each tile, write the tile value for each layer and each context
	vector<int32> tiles(_tile_context_count * _tile_layer_count, MISSING_TILE);
	data_file.BeginTable("map_tiles");
	for (uint32 y = 0; y < _map_height; ++y) {
		data_file.DeclareTable(y);
	}
	for (uint32 y = 0; y < _map_height; ++y) {
		data_file.OpenTable(y);
		for (uint32 x = 0; x < _map_length; ++x) {
			for (uint32 c = 0; c < _tile_context_count; ++c) {
				for (uint32 l = 0; l < _tile_layer_count; ++l) {
					tiles[(c * _tile_layer_count) + l] = _all_tile_contexts[c]->GetTileLayer(l)->GetTile(x, y);
				}
			}
			data_file.WriteIntVector(x, tiles);
		}
		data_file.EndTable();
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	if (data_file.IsErrorDetected()) {
		_error_message = "One or more errors occurred when writing map file:\n" + QString::fromStdString(data_file.GetErrorMessages());
		data_file.CloseFile();
		_map_modified = false;
		return false;
	}

	data_file.CloseFile();
	SetMapModified(false);
	return true;
} // bool MapData::SaveData(QString filename)



void MapData::ResizeMap(uint32 map_length, uint32 map_height) {
	// If the dimensions of the map will not change, return with no notice as this is a harmless operation
	if ((map_length == _map_length) && (map_height == _map_height)) {
		return;
	}

	// Each tile layer in every context must be resized along with the empty tile layer
	_empty_tile_layer.ResizeLayer(map_length, map_height);
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		vector<TileLayer>& layers = _all_tile_contexts[i]->GetTileLayers();
		for (uint32 j = 0; j < layers.size(); ++j) {
			layers[j].ResizeLayer(map_length, map_height);
		}
	}

	_map_length = map_length;
	_map_height = map_height;
	SetMapModified(true);
}

///////////////////////////////////////////////////////////////////////////////
// MapData class -- Tileset Functions
///////////////////////////////////////////////////////////////////////////////

QStringList MapData::GetTilesetFilenames() const {
	QStringList tileset_filenames;
	for (uint32 i = 0; i < _tilesets.size(); ++i) {
		tileset_filenames.append(_tilesets[i]->GetTilesetDefinitionFilename());
	}

	return tileset_filenames;
}



bool MapData::AddTileset(Tileset* new_tileset) {
	if (new_tileset == nullptr) {
		_error_message = "ERROR: function received nullptr pointer argument";
		return false;
	}

	if (new_tileset->IsInitialized() == false) {
		_error_message = "ERROR: function received uninitialized tileset object";
		return false;
	}

	for (uint32 i = 0; i < _tilesets.size(); ++i) {
		if (_tilesets[i] == new_tileset) {
			_error_message = "ERROR: tileset was already added to map data";
			return false;
		}

		if (_tilesets[i]->GetTilesetDefinitionFilename() == new_tileset->GetTilesetDefinitionFilename()) {
			_error_message = "ERROR: a tileset with the same definition file already exists within the map data";
			return false;
		}
	}

	_tilesets.push_back(new_tileset);
	SetMapModified(true);
	return true;
}



void MapData::RemoveTileset(uint32 tileset_index) {
	if (tileset_index >= _tilesets.size()) {
		_error_message = "ERROR: no tileset exists at index " + QString(NumberToString(tileset_index).c_str());
		return;
	}

	delete _tilesets[tileset_index];

	// Shift all remaining tilesets over, then remove the last entry in the list
	for (uint32 i = tileset_index + 1; i < _tilesets.size(); ++i) {
		_tilesets[i-1] = _tilesets[i];
	}
	_tilesets.pop_back();

	// When a tileset is removed, two things need to happen to the map data. First, any tiles from the removed tileset need to be nullified (set to MISSING_TILE).
	// Second, the values for any tile from a tileset that was ordered after the removed tileset need to be updated to reflect the new tileset indexes. In other
	// words, TILESET_NUM_TILES must be subtracted from each of these tilesets.
	int32 tile_null_start = tileset_index * TILESET_NUM_TILES; // The starting value for all tiles that need to be nullified
	int32 tile_update_start = tile_null_start + TILESET_NUM_TILES; // The starting value for all tiles that need to be updated

	for (uint32 c = 0; c < _tile_context_count; ++c) {
		vector<TileLayer>& layers = _all_tile_contexts[c]->GetTileLayers();
		for (uint32 l = 0; l < _tile_layer_count; ++l) {
			vector<vector<int32> >& tiles = layers[l].GetTiles();
			for (uint32 y = 0; y < _map_height; ++y) {
				for (uint32 x = 0; x < _map_length; ++x) {
					if (tiles[y][x] >= tile_null_start) {
						if (tiles[y][x] < tile_update_start) {
							tiles[y][x] = MISSING_TILE;
						}
						else {
							tiles[y][x] -= TILESET_NUM_TILES;
						}
					}
				}
			}
		}
	}

	SetMapModified(true);
}



void MapData::MoveTilesetUp(uint32 tileset_index) {
	if (tileset_index >= _tilesets.size()) {
		_error_message = "ERROR: no tileset exists at index " + QString(NumberToString(tileset_index).c_str());
		return;
	}

	if (tileset_index == 0) {
		_error_message = "WARN: tileset could not be moved further down at index " + QString(NumberToString(tileset_index).c_str());
		return;
	}

	Tileset* temp_tileset = _tilesets[tileset_index - 1];
	_tilesets[tileset_index - 1] = _tilesets[tileset_index];
	_tilesets[tileset_index] = temp_tileset;

	// TODO: update tile values on the map
	SetMapModified(true);
}



void MapData::MoveTilesetDown(uint32 tileset_index) {
	if (tileset_index >= _tilesets.size()) {
		_error_message = "ERROR: no tileset exists at index " + QString(NumberToString(tileset_index).c_str());
		return;
	}

	if (tileset_index == _tilesets.size() - 1) {
		_error_message = "WARN: tileset could not be moved further up at index " + QString(NumberToString(tileset_index).c_str());
		return;
	}

	Tileset* temp_tileset = _tilesets[tileset_index + 1];
	_tilesets[tileset_index + 1] = _tilesets[tileset_index];
	_tilesets[tileset_index] = temp_tileset;

	// TODO: update tile values on the map
	SetMapModified(true);
}

///////////////////////////////////////////////////////////////////////////////
// MapData class -- Tile Layer Functions
///////////////////////////////////////////////////////////////////////////////

TileLayer* MapData::ChangeSelectedTileLayer(uint32 layer_index) {
	if (layer_index >= _tile_layer_count) {
		_error_message = "WARN: could not change selected tile layer because no layer existed with this index";
		return nullptr;
	}

	_selected_tile_layer = _selected_tile_context->GetTileLayer(layer_index);
	_selected_tile_layer_properties = &_tile_layer_properties[layer_index];
	return _selected_tile_layer;
}



QStringList MapData::GetTileLayerNames() const {
	QStringList layer_names;
	for (uint32 i = 0; i < _tile_layer_count; ++i) {
		layer_names.append(_tile_layer_properties[i].GetLayerName());
	}

	return layer_names;
}



void MapData::ToggleTileLayerVisibility(uint32 layer_index) {
	if (layer_index > _tile_layer_count)
		return;

	bool visible = _tile_layer_properties[layer_index].IsVisible();
	_tile_layer_properties[layer_index].SetVisible(!visible);
}



void MapData::ToggleTileLayerCollision(uint32 layer_index) {
	if (layer_index > _tile_layer_count)
		return;

	bool collisions = _tile_layer_properties[layer_index].IsCollisionEnabled();
	_tile_layer_properties[layer_index].SetCollisionEnabled(!collisions);
	SetMapModified(true);
}



bool MapData::AddTileLayer(QString name, bool collision_on) {
	// Check that the name will be unique among all existing tile layers before adding
	QStringList layer_names = GetTileLayerNames();
	if (layer_names.indexOf(name) != -1) {
		_error_message = "ERROR: a tile layer with this name already exists";
		return false;
	}

	_tile_layer_count += 1;
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		_all_tile_contexts[i]->_AddTileLayer(_empty_tile_layer);
	}
	_tile_layer_properties.push_back(TileLayerProperties(name, true, collision_on));

	SetMapModified(true);
	return true;
}



bool MapData::DeleteTileLayer(uint32 layer_index) {
	if (layer_index >= _tile_layer_count) {
		_error_message = "ERROR: no tile layer exists at this index";
		return false;
	}

	// Delete the layer from each context
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		_all_tile_contexts[i]->_RemoveTileLayer(layer_index);
	}

	// Remove the corresponding entry from the layer properties
	for (uint32 i = layer_index; i < _tile_layer_count - 1; ++i) {
		_tile_layer_properties[i] = _tile_layer_properties[i+1];
	}
	_tile_layer_properties.pop_back();

	_tile_layer_count--;
	SetMapModified(true);
	return true;
}



bool MapData::CloneTileLayer(uint32 layer_index) {
	if (layer_index >= _tile_layer_count) {
		_error_message = "ERROR: no tile layer exists at this index";
		return false;
	}

	// First clone the properties of the layer. Layers can't share the same name, so generate a name for the clone layer
	TileLayerProperties clone_properties = _tile_layer_properties[layer_index];
	clone_properties.SetLayerName(_CreateCloneName(clone_properties.GetLayerName(), GetTileLayerNames()));
	_tile_layer_properties.push_back(clone_properties);

	// Go through each tile context and clone the appropriate layer data
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		_all_tile_contexts[i]->_CloneTileLayer(layer_index);
	}

	_tile_layer_count++;
	return true;
}



bool MapData::RenameTileLayer(uint32 layer_index, QString new_name) {
	if (layer_index > _tile_layer_count) {
		_error_message = "ERROR: no tile layer exists at this index";
		return false;
	}

	if (new_name.isEmpty() == true) {
		_error_message = "ERROR: can not set layer name to an empty string";
		return false;
	}

	// Check for the case where the name doesn't actually change
	if (_tile_layer_properties[layer_index].GetLayerName() == new_name) {
		return true;
	}

	// Check that the name will be unique among all existing tile layers before renaming
	QStringList layer_names = GetTileLayerNames();
	if (layer_names.indexOf(new_name) != -1) {
		_error_message = "ERROR: a tile layer with this name already exists";
		return false;
	}

	_tile_layer_properties[layer_index].SetLayerName(new_name);
	SetMapModified(true);
	return true;
}



bool MapData::SwapTileLayers(uint32 index_one, uint32 index_two) {
	if (index_one == index_two) {
		_error_message = "WARN: tried to use same index to swap two tile layers";
		return false;
	}
	if (index_one >= _tile_layer_count) {
		_error_message = "ERROR: no tile layer exists at first layer index";
		return false;
	}
	if (index_two >= _tile_layer_count) {
		_error_message = "ERROR: no tile layer exists at second layer index";
		return false;
	}

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		_all_tile_contexts[i]->_SwapTileLayers(index_one, index_two);
	}

	// Move the layer properties up
	TileLayerProperties swap = _tile_layer_properties[index_two];
	_tile_layer_properties[index_two] = _tile_layer_properties[index_one];
	_tile_layer_properties[index_one] = _tile_layer_properties[index_two];

	SetMapModified(true);
	return true;
}



void MapData::InsertTileLayerRows(uint32 row_index, uint32 row_count) {
	if (row_count == 0) {
		return;
	}
	if (row_index >= _map_height) {
		return;
	}
	if (_map_height + row_count > MAXIMUM_MAP_HEIGHT) {
		return;
	}

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		vector<TileLayer>& layers = _all_tile_contexts[i]->GetTileLayers();
		for (uint32 j = 0; j < _tile_layer_count; ++j) {
			layers[j]._AddRows(row_index, row_count, MISSING_TILE);
		}
	}
	_empty_tile_layer._AddRows(row_index, row_count, MISSING_TILE);

	_map_height = _map_height + row_count;
	SetMapModified(true);
}



void MapData::RemoveTileLayerRows(uint32 row_index, uint32 row_count) {
	if (row_count == 0) {
		return;
	}
	if (_map_height < (row_index + row_count)) {
		return;
	}
	if (row_count > (_map_height - MINIMUM_MAP_HEIGHT)) {
		return;
	}

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		vector<TileLayer>& layers = _all_tile_contexts[i]->GetTileLayers();
		for (uint32 j = 0; j < _tile_layer_count; ++j) {
			layers[j]._DeleteRows(row_index, row_count);
		}
	}
	_empty_tile_layer._DeleteRows(row_index, row_count);

	_map_height = _map_height - row_count;
	SetMapModified(true);
}



void MapData::InsertTileLayerColumns(uint32 col_index, uint32 col_count) {
	if (col_count == 0) {
		return;
	}
	if (col_index >= _map_length) {
		return;
	}
	if (_map_length + col_count > MAXIMUM_MAP_LENGTH) {
		return;
	}

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		vector<TileLayer>& layers = _all_tile_contexts[i]->GetTileLayers();
		for (uint32 j = 0; j < _tile_layer_count; ++j) {
			layers[j]._AddColumns(col_index, col_count, MISSING_TILE);
		}
	}
	_empty_tile_layer._AddColumns(col_index, col_count, MISSING_TILE);

	_map_length = _map_length + col_count;
	SetMapModified(true);
}



void MapData::RemoveTileLayerColumns(uint32 col_index, uint32 col_count) {
	if (col_count == 0) {
		return;
	}
	if (_map_length < (col_index + col_count)) {
		return;
	}
	if (col_count > (_map_length - MINIMUM_MAP_LENGTH)) {
		return;
	}

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		vector<TileLayer>& layers = _all_tile_contexts[i]->GetTileLayers();
		for (uint32 j = 0; j < _tile_layer_count; ++j) {
			layers[j]._DeleteColumns(col_index, col_count);
		}
	}
	_empty_tile_layer._DeleteColumns(col_index, col_count);

	_map_length = _map_length - col_count;
	SetMapModified(true);
}

///////////////////////////////////////////////////////////////////////////////
// MapData class -- Tile Context Functions
///////////////////////////////////////////////////////////////////////////////

TileContext* MapData::ChangeSelectedTileContext(int32 context_id) {
	if (context_id <= 0) {
		return nullptr;
	}
	if (static_cast<uint32>(context_id) > _tile_context_count) {
		_error_message = "WARN: could not change selected context because no context existed with this index";
		return nullptr;
	}

	// Before changing the context, figure out the index of the selected tile layer for the current context
	uint32 layer_index = 0;
	for (uint32 i = 0; i < _tile_layer_count; ++i) {
		if (_selected_tile_context->GetTileLayer(i) == _selected_tile_layer) {
			layer_index = i;
			break;
		}
	}
	_selected_tile_context = _all_tile_contexts[context_id - 1];
	ChangeSelectedTileLayer(layer_index);
	return _selected_tile_context;
}



QStringList MapData::GetTileContextNames() const {
	QStringList context_names;
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		context_names.append(_all_tile_contexts[i]->GetContextName());
	}

	return context_names;
}



QStringList MapData::GetInheritedTileContextNames() const {
	QStringList name_list;

	// Contexts that do not inherit have an empty string placed in the name list
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		if (_all_tile_contexts[i]->IsInheritingContext() == false) {
			name_list.append("");
		}
		else {
			uint32 inherit_index = _all_tile_contexts[i]->GetInheritedContextID();
			name_list.append(_all_tile_contexts[inherit_index]->GetContextName());
		}
	}

	return name_list;
}



TileContext* MapData::AddTileContext(QString name, int32 inheriting_context_id) {
	// Check all conditions where we would not be able to create the new context

	if (_tile_context_count == MAX_CONTEXTS) {
		_error_message = "ERROR: could not add new context as the maximum number of contexts has been reached";
		return nullptr;
	}
	if (name.isEmpty() == true) {
		_error_message = "ERROR: tile context must have a name";
		return nullptr;
	}
	if (FindTileContextByName(name) != nullptr) {
		_error_message = "ERROR: a context with this name already exists";
		return nullptr;
	}
	if (inheriting_context_id != INVALID_CONTEXT) {
		if (inheriting_context_id <= 0 || static_cast<uint32>(inheriting_context_id) > MAX_CONTEXTS) {
			_error_message = "ERROR: invalid value for inhertiting context ID";
			return nullptr;
		}
		if (_all_tile_contexts[inheriting_context_id - 1] == nullptr) {
			_error_message = "ERROR: no context exists for the requested inheriting context ID";
			return nullptr;
		}
	}

	// Create the new context and add it to the bottom of the context list
	uint32 new_id = _tile_context_count + 1;
	TileContext* new_context = new TileContext(new_id, name, inheriting_context_id);
	for (uint32 i = 0; i < _tile_layer_count; ++i) {
		new_context->_AddTileLayer(_empty_tile_layer);
	}
	_all_tile_contexts[_tile_context_count] = new_context;
	_tile_context_count++;

	SetMapModified(true);
	return new_context;
}



bool MapData::DeleteTileContext(int32 context_id) {
	TileContext* context = FindTileContextByID(context_id);
	// Check all conditions where we would not be able to delete the context
	if (context == nullptr) {
		_error_message = "ERROR: received invalid context ID";
		return false;
	}
	if (_tile_context_count <= 1) {
		_error_message = "ERROR: can not delete the last remaining context for the map";
		return false;
	}
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		if (context_id == _all_tile_contexts[i]->GetInheritedContextID()) {
			_error_message = "ERROR: could not delete context as it is being inherited by one or more additional contexts";
			return false;
		}
	}

	// Move the context to delete to the end of the list
	for (uint32 i = static_cast<uint32>(context_id); i < _tile_context_count; ++i) {
		SwapTileContexts(i, i + 1);
	}

	delete context;
	_all_tile_contexts[_tile_context_count - 1] = nullptr;
	_tile_context_count--;

	SetMapModified(true);
	return true;
}



TileContext* MapData::CloneTileContext(int32 context_id) {
	TileContext* context = FindTileContextByID(context_id);
	// Check all conditions where we would not be able to clone the context
	if (context == nullptr) {
		_error_message = "ERROR: received invalid context ID";
		return nullptr;
	}
	if (_tile_context_count == MAX_CONTEXTS) {
		_error_message = "ERROR: could not clone context as the maximum number of contexts has been reached";
		return nullptr;
	}

	TileContext* clone_context = new TileContext(*context);
	clone_context->_SetContextID(_tile_context_count + 1);
	clone_context->SetContextName(_CreateCloneName(clone_context->GetContextName(), GetTileContextNames()));

	_all_tile_contexts[_tile_context_count] = clone_context;
	_tile_context_count++;
	return clone_context;
}



bool MapData::RenameTileContext(int32 context_id, QString new_name) {
	if (context_id <= 0) {
		return false;
	}
	if (static_cast<uint32>(context_id) > _tile_context_count) {
		_error_message = "ERROR: context_id exceeds size of context list";
		return false;
	}

	if (new_name.isEmpty() == true) {
		_error_message = "ERROR: can not set context name to an empty string";
		return false;
	}

	if (_all_tile_contexts[context_id - 1]->GetContextName() == new_name) {
		return true;
	}

	QStringList context_names = GetTileContextNames();
	if (context_names.indexOf(new_name) != -1) {
		_error_message = "ERROR: a context with this name already exists";
		return false;
	}

	_all_tile_contexts[context_id - 1]->SetContextName(new_name);
	SetMapModified(true);
	return true;
}



bool MapData::ChangeInheritanceTileContext(int32 context_id, int32 inherit_id) {
	if (context_id <= 0 || static_cast<uint32>(context_id) > _tile_context_count) {
		_error_message = "ERROR: invalid context id";
		return false;
	}
	// Removing inheritance from a context is always a valid operation
	if (inherit_id == INVALID_CONTEXT) {
		_all_tile_contexts[context_id - 1]->_SetInheritingContext(inherit_id);
		return true;
	}
	// If the inheriting context ID isn't changing, do nothing and report success
	if (_all_tile_contexts[context_id - 1]->GetInheritedContextID() == inherit_id) {
		return true;
	}

	if (inherit_id <= 0 || static_cast<uint32>(inherit_id) > _tile_context_count) {
		_error_message = "ERROR: invalid inheriting context id";
		return false;
	}
	else if (_all_tile_contexts[inherit_id - 1]->GetInheritedContextID() != INVALID_CONTEXT) {
		_error_message = QString("ERROR: can not inherit from context %1 because it inherits from a context itself").arg(inherit_id);
		return false;
	}

	_all_tile_contexts[context_id - 1]->_SetInheritingContext(inherit_id);
	SetMapModified(true);
	return true;
}



bool MapData::SwapTileContexts(int32 first_id, int32 second_id) {
	if (first_id <= 0 || second_id <= 0) {
		_error_message = "ERROR: invalid context ID passed when trying to swap context positions";
		return false;
	}
	if (first_id == second_id) {
		_error_message = "ERROR: tried to swap two contexts with the same ID";
		return false;
	}
	if (static_cast<uint32>(first_id) > _tile_context_count) {
		_error_message = "ERROR: no tile context exists at first context ID";
		return false;
	}
	if (static_cast<uint32>(second_id) > _tile_layer_count) {
		_error_message = "ERROR: no tile context exists at second context ID";
		return false;
	}

	uint32 first_index = static_cast<uint32>(first_id - 1);
	uint32 second_index = static_cast<uint32>(second_id - 1);

	// Perform the swap and update each context's ID to match it's new position in the container
	TileContext* swap = _all_tile_contexts[first_index];
	_all_tile_contexts[first_index] = _all_tile_contexts[second_index];
	_all_tile_contexts[first_index]->_SetContextID(first_id);
	_all_tile_contexts[second_index] = swap;
	_all_tile_contexts[second_index]->_SetContextID(second_id);

	// Go through each context and see if it inherited from either the first or the second context. Update these values appropriately
	for (uint32 i = 0; i < _all_tile_contexts.size(); ++i) {
		if (_all_tile_contexts[i] == nullptr)
			break;

		int32 inherited_id = _all_tile_contexts[i]->GetInheritedContextID();
		if (inherited_id == first_id)
			_all_tile_contexts[i]->_SetInheritingContext(second_id);
		else if (inherited_id == second_id)
			_all_tile_contexts[i]->_SetInheritingContext(first_id);
	}

	SetMapModified(true);
	return true;
}



TileContext* MapData::FindTileContextByID(int32 context_id) const {
	if (context_id <= 0 || static_cast<uint32>(context_id) >= MAX_CONTEXTS) {
		return nullptr;
	}

	if (context_id > static_cast<int32>(_tile_context_count)) {
		return nullptr;
	}

	return _all_tile_contexts[context_id - 1];
}



TileContext* MapData::FindTileContextByName(QString context_name) const {
	for (uint32 i = 0; i < _all_tile_contexts.size(); ++i) {
		if (_all_tile_contexts[i] == nullptr)
			break;

		if (_all_tile_contexts[i]->GetContextName() == context_name)
			return _all_tile_contexts[i];
	}

	return nullptr;
}



TileContext* MapData::FindTileContextByIndex(uint32 context_index) const {
	if (context_index >= _all_tile_contexts.size())
		return nullptr;

	return _all_tile_contexts[context_index];
}



QString MapData::_CreateCloneName(const QString& name, const QStringList& taken_names) const {
	uint32 clone_id = 1;
	QString clone_name = name + " (Clone)";

	while (taken_names.contains(clone_name) == true) {
		clone_name = name + " (Clone #" + QString::number(clone_id) + ")";
		clone_id++;
	}
	return clone_name;
}



void MapData::_ComputeCollisionData() {
	// Resize the container to hold each grid element that will be computed and reset each value to 0
	_collision_data.resize(_map_height * 2);
	for (uint32 i = 0; i < _map_height * 2; ++i) {
		_collision_data[i].assign(_map_length * 2, 0);
	}

	// Holds the indexes of only the tile layers that have their collision data enabled
	vector<uint32> collision_layers;
	for (uint32 i = 0; i < _tile_layer_properties.size(); ++i) {
		if (_tile_layer_properties[i].IsCollisionEnabled() == true)
			collision_layers.push_back(i);
	}

	// A bit-mask used to put the collision data into the proper bit based on the context's ID
	uint32 context_mask = 0;
	// The context being processed
	TileContext* context = nullptr;
	// The context that the processing context inherits from (if it is non-inheriting, will be set to nullptr)
	TileContext* inherited_context = nullptr;

	// The value of the current tile being processed
	int32 tile = 0;
	// The indexes to the proper tileset and that tileset's collision data for the tile being processed
	uint32 tileset_index = 0;
	uint32 tileset_collision_index = 0;
	// Keeps track of whether there are no tiles for a given X/Y location
	bool no_tiles_at_coordinates = true;

	// Holds the indexes to the _collision_data for each of the four collision quadrants of a single tile
	uint32 north_index = 0;
	uint32 south_index = 0;
	uint32 west_index = 0;
	uint32 east_index = 0;
	for (uint32 c = 0; c < _tile_context_count; ++c) {
		// This mask is used to set the appropriate bit for this context
		context_mask = 0x00000001 << c;
		context = _all_tile_contexts[c];
		if (context->IsInheritingContext() == true) {
			inherited_context = FindTileContextByID(context->GetInheritedContextID());
		}
		else {
			inherited_context = nullptr;
		}

		// Iterate through each tile in the map and extract the collision data from each
		for (uint32 y = 0; y < _map_height; ++y) {
			north_index = y * 2;
			south_index = north_index + 1;
			for (uint32 x = 0; x < _map_length; ++x) {
				west_index = x * 2;
				east_index = west_index + 1;
				no_tiles_at_coordinates = true;
				for (uint32 l = 0; l < collision_layers.size(); ++l) {
					tile = context->GetTileLayer(collision_layers[l])->GetTile(x, y);
					if (tile == MISSING_TILE) {
						// If no tile is at this location, all collision data in this area is set to true
						continue;
					}
					else if (tile == INHERITED_TILE) {
						tile = inherited_context->GetTileLayer(collision_layers[l])->GetTile(x, y);
						if (tile == MISSING_TILE) {
							// If no tile is at this location, all collision data in this area is set to true

							continue;
						}
					}

					no_tiles_at_coordinates = false;

					// Determine the tileset that this tile belongs to and the location of the tile within that set
					tileset_index = tile / TILESET_NUM_TILES;
					tile = tile % TILESET_NUM_TILES;
					tileset_collision_index = tile * TILE_NUM_QUADRANTS;

					if (_tilesets[tileset_index]->GetQuadrantCollision(tileset_collision_index) != 0)
						_collision_data[north_index][west_index] |= context_mask;
					if (_tilesets[tileset_index]->GetQuadrantCollision(tileset_collision_index + 1) != 0)
						_collision_data[north_index][east_index] |= context_mask;
					if (_tilesets[tileset_index]->GetQuadrantCollision(tileset_collision_index + 2) != 0)
						_collision_data[south_index][west_index] |= context_mask;
					if (_tilesets[tileset_index]->GetQuadrantCollision(tileset_collision_index + 3) != 0)
						_collision_data[south_index][east_index] |= context_mask;
				}

				// When all tile layers that take collision properties of tiles into account contained no tile at a
				// given location, we want to enable collision data for that tile location. This is because we don't
				// want any sprites to be able to walk into dark pits, through walls, etc.
				if (no_tiles_at_coordinates == true) {
					_collision_data[north_index][west_index] |= context_mask;
					_collision_data[north_index][east_index] |= context_mask;
					_collision_data[south_index][west_index] |= context_mask;
					_collision_data[south_index][east_index] |= context_mask;
				}
			}
		}
	}
} // void MapData::_ComputeCollisionData()

} // namespace hoa_editor
