////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    editor.h
*** \author  Philip Vorsilak (gorzuate)
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for the map editor main window
*** ***************************************************************************/

#pragma once

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QErrorMessage>
#include <QFileDialog>
#include <QLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProgressDialog>
#include <QSplitter>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QUndoCommand>

#include "editor_utils.h"
#include "dialogs.h"
#include "map_data.h"
#include "map_view.h"
#include "tile_layer.h"
#include "tileset_editor.h"

//! \brief All editor code is contained within this namespace.
namespace hoa_editor {

/** ****************************************************************************
*** \brief The main window of the editor program and the top-level widget
***
*** This class is responsible for creating the application menus and toolbars and
*** processing the actions when those items are selected. As the top-level widget,
*** it is also responsible for the creation and layout of all lower-level widgets
*** as well as holding the active instance of the map data. Several of the actions
*** that a user takes that are processed by this class will make calls into the
*** appropriate sub widget to reflect the changes.
***
*** The MapView widget, which represents the viewble and editable area of the map,
*** is located on the left side of the window. The right side of the window contains
*** three widgets. From top to bottom, they are: the LayerView widget, ContextView widget,
*** and TilesetTab widget.
***
*** \todo Add save/restore state information for the QSplitter objects used by this
*** class so that the editor remembers the size that the user last left the editor
*** window at. This information should be saved to a Lua file called "editor_state.lua"
*** or something similar.
***
*** \todo In the File menu, add a "Recent Files >" action with a submenu below the Open action.
*** The submenu should contain around 5 files maximum along with a "Clear Recent Files" option.
*** Mimic the way this is done in the Tiled map editor.
*** ***************************************************************************/
class Editor : public QMainWindow {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	Editor();

	~Editor();

	//! \brief Returns a pointer to the map data
	MapData* GetMapData()
		{ return &_map_data; }

	TilesetView* GetTilesetView() const
		{ return _tileset_view; }

	/** \brief Sends a notification to the MapView widget when the map size has been modified
	*** \note Whenever the map size is modified external to the MapView class, this method must be called.
	*** The MapView widget maintains its own layer objects separate from the map data that need to reflect
	*** the new map size.
	**/
	void MapSizeModified()
		{ _map_view->UpdateAreaSizes(); }

	/** \brief Sends a notification to the MapView widget when the map layers have been modified
	*** \note This only needs to be called when the layer count, order, or names are changed. Changes to any other
	*** properties will have no effect.
	**/
	void MapLayersModified()
		{ _map_view->UpdateLayerActions(); }

	/** \brief Sends a notification to the MapView widget when the map contexts have been modified
	*** \note This only needs to be called when the context count, order, or names are changed. Changes to any other
	*** properties will have no effect.
	**/
	void MapContextsModified()
		{ _map_view->UpdateContextActions(); }

	//! \brief Used by other subwidgets to redraw the map view, tyically used when the map data has been modified
	void DrawMapView()
		{ _map_view->DrawMap(); }

	//! \brief Used by other subwidgets to inform the editor that the selected context has changed
	void UpdateSelectedContext()
		{ _CheckToolsActions(); _map_view->DrawMap(); }

private:
	//! \brief Contains all data for the open map file and methods for manipulating that data
	MapData _map_data;

	//! \brief The toolbar at the top of the window containing icons for various edit options
	QToolBar* _tiles_toolbar;

	//! \brief Splits the widget into two horizontal sections
	QSplitter* _horizontal_splitter;

	//! \brief Splits the right horizontal section into three vertical sections
	QSplitter* _right_vertical_splitter;

	//! \brief The left sub-widget containing the editable map area
	MapView* _map_view;

	//! \brief Widget used to display and edit the ordered list of all tile layers
	LayerView* _layer_view;

	//! \brief Widget used to display and edit the properties of map contexts
	ContextView* _context_view;

	//! \brief Widget used to display each tileset
	TilesetView* _tileset_view;

	//! \brief The stack that contains the undo and redo operations.
	QUndoStack* _undo_stack;

	/** \name Application Menus
	*** \brief The top-level menus found in the menu bar: File, Edit, View, Tools, Help
	**/
	//{@
	QMenu* _file_menu;
	QMenu* _edit_menu;
	QMenu* _view_menu;
	QMenu* _tools_menu;
	QMenu* _help_menu;
	//@}

	/** \name Application Menu Actions
	*** \brief Actions that are executed when particular menus are selected
	***
	*** These are Qt's way of associating the same back-end functionality to occur whether a user
	*** invokes a menu through the menu bar, a keyboard shortcut, a toolbar button, or other means.
	*** They are organized in the order in which they appear in the application menus.
	**/
	//{@
	QAction* _new_action;
	QAction* _open_action;
	QAction* _save_action;
	QAction* _save_as_action;
	QAction* _close_action;
	QAction* _quit_action;

	QAction* _undo_action;
	QAction* _redo_action;
	QAction* _cut_action;
	QAction* _copy_action;
	QAction* _paste_action;
	QAction* _tileset_properties_action;
	QAction* _map_properties_action;
	QAction* _map_resize_action;

	QAction* _view_grid_action;
	QAction* _view_missing_action;
	QAction* _view_inherited_action;
	QAction* _view_collision_action;

	QAction* _tool_paint_action;
	QAction* _tool_swap_action;
	QAction* _tool_erase_action;
	QAction* _tool_inherit_action;
	QAction* _tool_area_select_action;
	QAction* _tool_area_fill_action;
	QAction* _tool_area_clear_action;
	QAction* _tool_area_inherit_action;
	QAction* _tool_select_clear_action;
	QAction* _tool_select_all_action;

	QAction* _help_action;
	QAction* _about_action;
	QAction* _about_qt_action;
	//@}

	//! \brief Used to group the various edit tools together so that only one may be active at a given time
	QActionGroup* _tool_action_group;

	//! \brief Handles close and quit events
	void closeEvent(QCloseEvent* event)
		{ (void)event; _FileQuit(); }

	//! \brief Creates actions for use by menus, toolbars, and keyboard shortcuts
	void _CreateActions();

	//! \brief Creates the main menus
	void _CreateMenus();

	//! \brief Creates the main toolbar
	void _CreateToolbars();

	/** \brief Sets the editor to its default state for editing mode, checkboxes, and so on
	*** This is called whenever the application starts, a new map is created, or an existing map is loaded
	**/
	void _ClearEditorState();

	/** \brief Called whenever an operation occurs that could discard unsaved map data
	*** \return False if the user cancelled the operation that would cause the data to be discarded
	***
	*** The options that are present to the user include: save the map data, discard the map data, or cancel the operation
	*** that caused this dialog to be invoked. A return value of true means that the user either saved or intentionally
	*** discarded the data.
	**/
	bool _UnsavedDataPrompt();

private slots:
	/** \name Action Setup Slots
	*** \brief Used to enable or disable actions in the menus and toolbars based on the current state of the editor
	**/
	//{@
	void _CheckFileActions();
	void _CheckEditActions();
	void _CheckViewActions();
	void _CheckToolsActions();
	//@}

	/** \name Action Execution Slots
	*** \brief Processes the various actions commanded by the user
	**/
	//{@
	void _FileNew();
	void _FileOpen();
	void _FileSave();
	void _FileSaveAs();
	void _FileClose();
	void _FileQuit();

	void _CutSelection();
	void _CopySelection();
	void _PasteSelection();
	void _EditTilesetProperties();
	void _EditMapProperties();
	void _EditMapResize();

	void _ViewTileGrid()
		{ _view_grid_action->setChecked(_map_view->ToggleGridVisible()); }
	void _ViewMissingTiles()
		{ _view_missing_action->setChecked(_map_view->ToggleMissingOverlayVisible()); }
	void _ViewInheritedTiles()
		{ _view_inherited_action->setChecked(_map_view->ToggleInheritedOverlayVisible()); }
	void _ViewCollisionData()
		{ _view_collision_action->setChecked(_map_view->ToggleCollisionOverlayVisible()); }

	void _SelectPaintTool()
		{ _map_view->SetEditMode(PAINT_MODE); }
	void _SelectSwapTool()
		{ _map_view->SetEditMode(SWAP_MODE); }
	void _SelectEraseTool()
		{ _map_view->SetEditMode(ERASE_MODE); }
	void _SelectInheritTool()
		{ _map_view->SetEditMode(INHERIT_MODE); }
	void _SelectAreaSelectTool()
		{ _map_view->SetEditMode(SELECT_AREA_MODE); }
	void _SelectAreaFillTool()
		{ _map_view->SetEditMode(FILL_AREA_MODE); }
	void _SelectAreaClearTool()
		{ _map_view->SetEditMode(CLEAR_AREA_MODE); }
	void _SelectAreaInheritTool()
		{ _map_view->SetEditMode(INHERIT_AREA_MODE); }
	void _SelectSelectionClearTool()
		{ _map_view->SelectNoTiles(); _map_view->DrawMap(); }
	void _SelectSelectionAllTool()
		{ _map_view->SelectAllTiles(); _map_view->DrawMap(); }

	void _HelpMessage();
	void _AboutMessage();
	void _AboutQtMessage();
	//@}
}; // class Editor : public QMainWindow


/** ****************************************************************************
*** \brief Holds the previous state of the map data, which is used to undo or redo past actions
***
*** Whenever map tiles are modified by the user, a new instance of this class gets created so that those
*** changes can be rolled back or reapplied via undo/redo requests by the user. This class handles both
*** map resizing events (adding or removing columns and/or rows) and changes to the values of the map tiles
***
*** \todo This entire class needs to be implemented so that it can handle operations such as map resizing,
*** multi layer changes, and multi context changes.
***
*** \todo There needs to be more QUndoCommand-derived classes for the other types of map edits: map properties,
*** layers, contexts, and tilesets.
*** ***************************************************************************/
class EditTileCommand : public QUndoCommand {
public:
	/** \param action_text The type of action that occured (eg "paint", "erase", "resize")
	*** \param parent THe parent command of this new command
	**/
	EditTileCommand(const QString& action_text, QUndoCommand* parent = 0);

	/** \name Undo Functions
	*** \brief Takes the actions necessary to undo or redo the command
	**/
	//{@
	void undo();
	void redo();
	//@}

private:
}; // class EditTileCommand : public QUndoCommand

} // namespace hoa_editor
