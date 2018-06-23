///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    tile_context.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for tile context data and view classes
***
*** This file implements the class definitions necessary to represent map contexts.
*** A map context is a collection of tile layers, and essentially each context is
*** like "a map within a map". Every map must have at least one context and can contain
*** at most MAX_CONTEXTS. Both the data model and view widget are defined here.
*** **************************************************************************/

#pragma once

#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "editor_utils.h"
#include "tile_layer.h"
#include "tileset.h"

namespace hoa_editor {

class MapData;

/** ****************************************************************************
*** \brief A collection of tile layers
***
*** A tile context is a group of TileLayer objects that together compose the makeup of a map view.
*** Every tile context corresponds to a map context, the difference between the two being that the
*** tile context only handles the tile data whereas a map context has tiles, objects, sprites, and
*** separate collision data. The editor user interface, however, does not mention the word "tile context"
*** and only uses the term "map context" to avoid confusing the user with this difference.
***
*** Every map must contain at least one TileContext, and can contain a maximum of MAX_CONTEXTS. Every
*** context has an ID that should be unique amongst any and all other contexts. Contexts can also inherit
*** from one (and only only one) other context. When a context inherits from another context, what happens is
*** that the context that was inherited from is drawn first and the inherting context is drawn on top of that.
*** The effect of this is that sections of the map can be easily replaced with other tiles without having to
*** load an entirely different map. For example, consider a small map with a single building. One context would
*** represent the outside of the building, while a second context inherits from the first and places tiles over
*** the building to show it's interior for when the player moves inside.
***
*** Due to the nature of inheriting contexts, TileContext objects must be constructed with care. Deleting a context
*** can potentially break the map data if it is not handled properly. Therefore, the constructors and several other
*** methods for this class are made private and can only be accessed by the the MapData class. This class
*** manages all instances of TileContext for the open map and ensures that there is no violation of context data.
***
*** \note All contexts are named, but the name data is not stored within this class. This is because storing all
*** context names in a single container is more efficient for returning the list of context names and performing name
*** lookups. The name data is stored in the TileDataModel class.
***
*** \note TileContext, like TileLayer, does not store any collision data information.
*** ***************************************************************************/
class TileContext {
	friend class MapData;

public:
	//! \name Class member accessor functions
	//@{
	int32 GetContextID() const
		{ return _context_id; }

	QString GetContextName() const
		{ return _context_name; }

	bool IsInheritingContext() const
		{ return (_inherited_context_id != INVALID_CONTEXT); }

	int32 GetInheritedContextID() const
		{ return _inherited_context_id; }

	std::vector<TileLayer>& GetTileLayers()
		{ return _tile_layers; }

	//! \note Returns nullptr if layer_index is invalid
	TileLayer* GetTileLayer(uint32 layer_index)
		{ if (layer_index < _tile_layers.size()) return &(_tile_layers[layer_index]); else return nullptr; }

	void SetContextName(QString name)
		{ _context_name = name; }
	//@}

private:
	/** \param id The ID to set the newly created context
	***
	*** This constructor is used when not inheriting from another context
	**/
	TileContext(int32 id, QString name) :
		_context_id(id), _context_name(name), _inherited_context_id(INVALID_CONTEXT) {}

	/** \param id The ID to set the newly created context (should be unique among all TileContext objects)
	*** \param inherited_context_id The ID of the context that this newly created context should inherit from
	***
	*** It is the caller's responsibility to ensure that the inherited_context_id is valid (ie, another TileContext
	*** exists with the provided ID). The constructor has no means to determine if there is a valid context with this
	*** ID, other than ensuring that the value provided lies within the range 1-MAX_CONTEXTS.
	**/
	TileContext(int32 id, QString name, int32 inherited_context_id) :
		_context_id(id), _context_name(name), _inherited_context_id(inherited_context_id) {}

	//! \brief The ID number of the context which has an acceptable range of [1 - MAX_CONTEXTS]
	int32 _context_id;

	//! \brief The name of the context as it will be seen by the user in the editor
	QString _context_name;

	/** \brief The ID of the context that this context inherits from
	*** If this context does not inherit from another, then this member is set to INVALID_CONTEXT.
	**/
	int32 _inherited_context_id;

	//! \brief All tile layers that belong to the context
	std::vector<TileLayer> _tile_layers;

	void _SetContextID(int32 id)
		{ _context_id = id; }

	/** \brief Removes inheriting context data, if any exists
	*** \note This not only sets the _inherited_context_id, but also changes any tiles with the value of INHERITED_TILE to MISSING_TILE
	**/
	void _ClearInheritingContext();

	/** \brief Transforms the context into an inheriting context
	*** \param inherited_context_id The ID of the context that this context should inherit from
	**/
	void _SetInheritingContext(int32 inherited_context_id)
		{ if (inherited_context_id == INVALID_CONTEXT) _ClearInheritingContext(); else _inherited_context_id = inherited_context_id; }

	/** \brief Adds a new tile layer to the end of the layer container
	*** \param layer A reference to the pre-created TileLayer to add
	**/
	void _AddTileLayer(TileLayer& layer);

	/** \brief Removes an existing tile layer from the context
	*** \param layer_index The index of the layer to remove
	**/
	void _RemoveTileLayer(uint32 layer_index);

	/** \brief Creates a copy of an existing tile layer
	*** \param layer_index The index of the layer to clone
	**/
	void _CloneTileLayer(uint32 layer_index);

	/** \brief Swaps the position of two tile layers
	*** \param first_index The index of the first layer to swap
	*** \param second_index The index of the second layer to swap
	**/
	void _SwapTileLayers(uint32 first_index, uint32 second_index);
}; // class TileContext


/** ****************************************************************************
*** \brief Displays the sortable list of tile contexts for a map
***
*** This widget is located below the layer view widget in the right section of the main editor window.
*** The active map context is highlighted and shows each context's ID, name, and inheriting context
*** if any is active. These properties can be modified except for the ID, which is automatically
*** set according to the order of each context in the context list.
***
*** TODO: this is a skeleton class that will be fleshed out later
*** ***************************************************************************/
class ContextView : public QTreeWidget {
private:
	Q_OBJECT // Macro needed to use QT's slots and signals

	//! \name Widget Column Identifiers
	//@{
	static const uint32 ID_COLUMN = 0;
	static const uint32 NAME_COLUMN = 1;
	static const uint32 INHERITS_COLUMN = 2;
	//@}

public:
	ContextView(MapData* data);

	~ContextView();

	//! \brief Refreshes the viewable contents of the widget. Should be called whenever the map context data changes outside of this widget
	void RefreshView();

protected:
	/** \brief Reimplemented from QTreeWidget to process left and right clicks separately
	*** \param event A pointer to the mouse event which occurred
	**/
	void mousePressEvent(QMouseEvent* event);

	/** \brief Reimplemented from QTreeWidget to change map data when contexts are reordered
	*** \param event A pointer to the drop event which occurred
	**/
	void dropEvent(QDropEvent* event);

private slots:
	/** \brief Updates the selected context for editing in the map view widget
	***
	*** This function is called whenever the user single-clicks one of the context items in the widget
	**/
	void _ChangeSelectedContext();

	/** \brief Modifies one of the properties of a tile context
	*** \param item A pointer to the context where the property change will happen
	*** \param column The column number of the property which should be changed
	***
	*** This function is called whenever the user double-clicks one of the context items in the widget
	**/
	void _ChangeContextProperties(QTreeWidgetItem* item, int column);

	/** \brief Closes any open persistence editor and validates that the data which was changed is valid
	*** \param item A pointer to the item that has been modified
	*** \param column The column that has been modified
	***
	*** \note If the new data is not valid, the old data will be restored and a warning message window sent to the user
	**/
	void _ValidateChangedData(QTreeWidgetItem* item, int column);

	//! \brief Creates a new empty tile context and adds it to the end of the context list
	void _AddTileContext();

	//! \brief Creates a new tile context that clones all the data and properties of an existing context
	void _CloneTileContext();

	//! \brief Opens up an editor to rename the layer pointed to by _right_click_item
	void _RenameTileContext();

	//! \brief Deletes the context item pointed to by _right_click_item
	void _DeleteTileContext();

private:
	//! \brief A pointer to the active map data that contains the tile contexts
	MapData* _map_data;

	//! \brief While renaming a context, holds the original name in case the renaming operation is cancelled for fails
	QString _original_context_name;

	//! \brief While changing the inheritance of a context, holds the original value in case the operation fails
	QString _original_context_inheritance;

	//! \brief A pointer to the most recent item that was right clicked. Set to nullptr if no item was clicked
	QTreeWidgetItem* _right_click_item;

	//! \brief Menu for right-clicks events on the widget
    QMenu* _right_click_menu;

	/** \name Right-Click Menu Actions
	*** \brief The possible actions the user can take on the right-click menu
	**/
	//{@
	QAction* _add_context_action;
	QAction* _clone_context_action;
	QAction* _rename_context_action;
	QAction* _delete_context_action;
	//@}
}; // class ContextView : public QTreeWidget

} // namespace hoa_editor
