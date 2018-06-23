///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    editor_utils.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for map editor utility code
*** *****************************************************************************/

#pragma once

#include <QString>

#include "utils.h"

namespace hoa_editor {

//! \brief Indicates that no tile is placed at a particular location
const int32 MISSING_TILE = -1;

//! \brief Indicates that the tile at this location is inherited from another context
const int32 INHERITED_TILE = -2;

//! \brief Indicates that a tile is part of a user-selected area
const int32 SELECTED_TILE = -10;

//! \brief Used to indicate a non-existing or invalid tile context ID
const int32 INVALID_CONTEXT = -1;

//! \brief The maximum number of contexts allowed on a map
const uint32 MAX_CONTEXTS = 32;

//! \brief The limits to the size dimensions of a map, in number of tiles
//@{
const int32 MINIMUM_MAP_LENGTH = 32;
const int32 MAXIMUM_MAP_LENGTH = 1000;
const int32 MINIMUM_MAP_HEIGHT = 24;
const int32 MAXIMUM_MAP_HEIGHT = 1000;
//@}

//! \brief The tile dimension sizes in number of pixels
//@{
const uint32 TILE_LENGTH = 32;
const uint32 TILE_HEIGHT = 32;
//@}

//! \brief The dimensions of a tileset image file in number of pixels
//@{
const uint32 TILESET_LENGTH = 512;
const uint32 TILESET_HEIGHT = 512;
//@}

//! \brief The dimensions of a tileset image file in number of tiles
//@{
const uint32 TILESET_NUM_COLS = 16;
const uint32 TILESET_NUM_ROWS = 16;
//@}

//! \brief The number of tiles that a tileset holds (TILESET_NUM_COLS * TILESET_NUM_ROWS)
const uint32 TILESET_NUM_TILES = 256;

//! \brief The dimensions of a tile's collision quadrant, in pixels
//@{
const uint32 TILE_QUADRANT_LENGTH = TILE_LENGTH / 2;
const uint32 TILE_QUADRANT_HEIGHT = TILE_HEIGHT / 2;
//@}

//! \brief The number of collision quadrants in a single tile
const uint32 TILE_NUM_QUADRANTS = 4;

//! \brief Various modes for map tile editing
enum EDIT_MODE {
	INVALID_MODE        = -1,
	PAINT_MODE          =  0,
	SWAP_MODE           =  1,
	ERASE_MODE          =  2,
	INHERIT_MODE        =  3,
	SELECT_AREA_MODE    =  4,
	FILL_AREA_MODE      =  5,
	CLEAR_AREA_MODE     =  6,
	INHERIT_AREA_MODE   =  7,
	TOTAL_MODE          =  8
};

//! \brief Represents different types of transition patterns for autotileable tiles.
enum TRANSITION_PATTERN_TYPE {
	INVALID_PATTERN     = -1,
	NW_BORDER_PATTERN   =  0,
	N_BORDER_PATTERN    =  1,
	NE_BORDER_PATTERN   =  2,
	E_BORDER_PATTERN    =  3,
	SE_BORDER_PATTERN   =  4,
	S_BORDER_PATTERN    =  5,
	SW_BORDER_PATTERN   =  6,
	W_BORDER_PATTERN    =  7,
	NW_CORNER_PATTERN   =  8,
	NE_CORNER_PATTERN   =  9,
	SE_CORNER_PATTERN   =  10,
	SW_CORNER_PATTERN   =  11,
	TOTAL_PATTERN       =  12
};

//! \brief The name of the editor application
const QString APP_NAME("Allacrost Map Editor");

//! \brief Pathname where editor icon files are stored
const QString ICON_PATH("img/misc/editor_tools/");

} // namespace hoa_editor
