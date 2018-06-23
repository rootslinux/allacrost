///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    editor.cpp
*** \author  Philip Vorsilak (gorzuate)
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for the map editor main window
*** ***************************************************************************/

#include <QGraphicsView>

#include "script.h"
#include "editor.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_script;

namespace hoa_editor {

///////////////////////////////////////////////////////////////////////////////
// Editor class -- public functions
///////////////////////////////////////////////////////////////////////////////

Editor::Editor() :
	QMainWindow(),
	_tiles_toolbar(nullptr),
	_horizontal_splitter(nullptr),
	_right_vertical_splitter(nullptr),
	_map_view(nullptr),
	_layer_view(nullptr),
	_context_view(nullptr),
	_tileset_view(nullptr),
	_undo_stack(nullptr),
	_file_menu(nullptr),
	_edit_menu(nullptr),
	_view_menu(nullptr),
	_tools_menu(nullptr),
	_help_menu(nullptr),
	_new_action(nullptr),
	_open_action(nullptr),
	_save_action(nullptr),
	_save_as_action(nullptr),
	_close_action(nullptr),
	_quit_action(nullptr),
	_undo_action(nullptr),
	_redo_action(nullptr),
	_cut_action(nullptr),
	_copy_action(nullptr),
	_paste_action(nullptr),
	_tileset_properties_action(nullptr),
	_map_properties_action(nullptr),
	_map_resize_action(nullptr),
	_view_grid_action(nullptr),
	_view_missing_action(nullptr),
	_view_inherited_action(nullptr),
	_view_collision_action(nullptr),
	_tool_paint_action(nullptr),
	_tool_swap_action(nullptr),
	_tool_erase_action(nullptr),
	_tool_inherit_action(nullptr),
	_tool_area_select_action(nullptr),
	_tool_area_fill_action(nullptr),
	_tool_area_clear_action(nullptr),
	_tool_area_inherit_action(nullptr),
	_tool_select_clear_action(nullptr),
	_tool_select_all_action(nullptr),
	_help_action(nullptr),
	_about_action(nullptr),
	_about_qt_action(nullptr),
	_tool_action_group(nullptr)
{
	// Create and initialize the script engine that the editor code uses
	ScriptManager = ScriptEngine::SingletonCreate();
	ScriptManager->SingletonInitialize();

	// Create actions, menus, and toolbars
	_CreateActions();
	_CreateMenus();
	_CreateToolbars();

	_undo_stack = new QUndoStack();
	// TODO: undo/redo support not implemented yet
// 	connect(_undo_stack, SIGNAL(canUndoChanged(bool)), _undo_action, SLOT(setEnabled(bool)));
// 	connect(_undo_stack, SIGNAL(canRedoChanged(bool)), _redo_action, SLOT(setEnabled(bool)));

	// Create each widget that forms the main window
	_horizontal_splitter = new QSplitter(this);
	_horizontal_splitter->setOrientation(Qt::Horizontal);
	setCentralWidget(_horizontal_splitter);
	_right_vertical_splitter = new QSplitter(_horizontal_splitter);
	_right_vertical_splitter->setOrientation(Qt::Vertical);

    _map_view = new MapView(_horizontal_splitter, &_map_data);
    _layer_view = new LayerView(&_map_data);
    _context_view = new ContextView(&_map_data);
    _tileset_view = new TilesetView(_right_vertical_splitter, &_map_data);

    _horizontal_splitter->addWidget(_map_view->GetGraphicsView());
    _horizontal_splitter->addWidget(_right_vertical_splitter);
    _right_vertical_splitter->addWidget(_layer_view);
	_right_vertical_splitter->addWidget(_context_view);
    _right_vertical_splitter->addWidget(_tileset_view);

	// Size the window and each widget in it appropriately
	resize(1200, 800);
    QList<int> splitter_size;
    splitter_size << 660 << 540;
    _horizontal_splitter->setSizes(splitter_size);
	_horizontal_splitter->show();
    splitter_size.clear();
    splitter_size << 80 << 80 << 640;
    _right_vertical_splitter->setSizes(splitter_size);
	_right_vertical_splitter->show();

	setWindowTitle("Allacrost Map Editor");
	setWindowIcon(QIcon("img/logos/program_icon.ico"));
	_ClearEditorState();
}



Editor::~Editor() {
	delete _new_action;
	delete _open_action;
	delete _save_action;
	delete _save_as_action;
	delete _close_action;
	delete _quit_action;

	delete _undo_action;
	delete _redo_action;
	delete _cut_action;
	delete _copy_action;
	delete _paste_action;
	delete _tileset_properties_action;
	delete _map_properties_action;
	delete _map_resize_action;

	delete _view_grid_action;
	delete _view_missing_action;
	delete _view_inherited_action;
	delete _view_collision_action;

	delete _tool_paint_action;
	delete _tool_swap_action;
	delete _tool_erase_action;
	delete _tool_inherit_action;
	delete _tool_area_select_action;
	delete _tool_area_fill_action;
	delete _tool_area_clear_action;
	delete _tool_area_inherit_action;
	delete _tool_select_clear_action;
	delete _tool_select_all_action;
	delete _tool_action_group;

	delete _help_action;
	delete _about_action;
	delete _about_qt_action;

	delete _map_view;
	delete _layer_view;
	delete _context_view;
	delete _tileset_view;

	delete _right_vertical_splitter;
	delete _horizontal_splitter;
	delete _undo_stack;

	ScriptEngine::SingletonDestroy();
}

///////////////////////////////////////////////////////////////////////////////
// Editor class -- private functions
///////////////////////////////////////////////////////////////////////////////

void Editor::_CreateActions() {
	// Create actions found in the File menu
	_new_action = new QAction(QIcon("img/misc/editor_tools/new.png"), "&New...", this);
	_new_action->setShortcut(tr("Ctrl+N"));
	_new_action->setStatusTip("Create a new map");
	connect(_new_action, SIGNAL(triggered()), this, SLOT(_FileNew()));

	_open_action = new QAction(QIcon("img/misc/editor_tools/open.png"), "&Open...", this);
	_open_action->setShortcut(tr("Ctrl+O"));
	_open_action->setStatusTip("Open an existing map file");
	connect(_open_action, SIGNAL(triggered()), this, SLOT(_FileOpen()));

	_save_action = new QAction(QIcon("img/misc/editor_tools/save.png"), "&Save", this);
	_save_action->setShortcut(tr("Ctrl+S"));
	_save_action->setStatusTip("Save the map file");
	connect(_save_action, SIGNAL(triggered()), this, SLOT(_FileSave()));

	_save_as_action = new QAction(QIcon("img/misc/editor_tools/save_as.png"), "Save &As...", this);
	_save_as_action->setStatusTip("Save the map to a new file");
	connect(_save_as_action, SIGNAL(triggered()), this, SLOT(_FileSaveAs()));

	_close_action = new QAction(QIcon("img/misc/editor_tools/close.png"), "&Close", this);
	_close_action->setShortcut(tr("Ctrl+W"));
	_close_action->setStatusTip("Close the map");
	connect(_close_action, SIGNAL(triggered()), this, SLOT(_FileClose()));

	_quit_action = new QAction(QIcon("img/misc/editor_tools/exit.png"), "&Quit", this);
	_quit_action->setShortcut(tr("Ctrl+Q"));
	_quit_action->setStatusTip("Exit the application");
	connect(_quit_action, SIGNAL(triggered()), this, SLOT(_FileQuit()));

	// Create actions found in the Edit menu
	_undo_action = new QAction(QIcon("img/misc/editor_tools/undo.png"), "&Undo", this);
	_undo_action->setShortcut(tr("Ctrl+Z"));
	_undo_action->setStatusTip("Undo the previous command");
	// TODO: undo support not implemented yet
// 	connect(_undo_action, SIGNAL(triggered()), _undo_stack, SLOT(undo()));

	_redo_action = new QAction(QIcon("img/misc/editor_tools/redo.png"), "&Redo", this);
	_redo_action->setShortcut(tr("Ctrl+Y"));
	_redo_action->setStatusTip("Redo the next command");
	// TODO: redo support not implemented yet
// 	connect(_redo_action, SIGNAL(triggered()), _undo_stack, SLOT(redo()));

	_cut_action = new QAction(QIcon("img/misc/editor_tools/cut.png"), "Cu&t", this);
	_cut_action->setShortcut(tr("Ctrl+X"));
	_cut_action->setStatusTip("Cut the selected area");
	connect(_cut_action, SIGNAL(triggered()), this, SLOT(_CutSelection()));

	_copy_action = new QAction(QIcon("img/misc/editor_tools/copy.png"), "&Copy", this);
	_copy_action->setShortcut(tr("Ctrl+C"));
	_copy_action->setStatusTip("Copy the selected area");
	connect(_copy_action, SIGNAL(triggered()), this, SLOT(_CopySelection()));

	_paste_action = new QAction(QIcon("img/misc/editor_tools/paste.png"), "&Paste", this);
	_paste_action->setShortcut(tr("Ctrl+V"));
	_paste_action->setStatusTip("Paste the copied selection");
	connect(_paste_action, SIGNAL(triggered()), this, SLOT(_PasteSelection()));

	_tileset_properties_action = new QAction("Edit Tile&set Properties...", this);
	_tileset_properties_action->setStatusTip("Edits the properties of a chosen tileset file");
	connect(_tileset_properties_action, SIGNAL(triggered()), this, SLOT(_EditTilesetProperties()));

	_map_properties_action = new QAction("Edit &Map Properties...", this);
	_map_properties_action->setStatusTip("Modify the properties of the active map");
	connect(_map_properties_action, SIGNAL(triggered()), this, SLOT(_EditMapProperties()));

	_map_resize_action = new QAction("&Resize Map...", this);
	_map_resize_action->setStatusTip("Change the dimensions of the active map");
	connect(_map_resize_action, SIGNAL(triggered()), this, SLOT(_EditMapResize()));

	// Create actions found in the View menu
	_view_grid_action = new QAction("Tile &Grid", this);
	_view_grid_action->setStatusTip("Toggles the display of the tile grid");
	_view_grid_action->setShortcut(Qt::Key_G);
	_view_grid_action->setCheckable(true);
	connect(_view_grid_action, SIGNAL(triggered()), this, SLOT(_ViewTileGrid()));

	_view_missing_action = new QAction("&Missing Tiles", this);
	_view_missing_action->setStatusTip("Toggles the display of an overlay for all missing tiles on the selected tile layer");
	_view_missing_action->setShortcut(Qt::Key_M);
	_view_missing_action->setCheckable(true);
	connect(_view_missing_action, SIGNAL(triggered()), this, SLOT(_ViewMissingTiles()));

	_view_inherited_action = new QAction("&Inherited Tiles", this);
	_view_inherited_action->setStatusTip("Toggles the display of an overlay for all inherited tiles on the selected tile layer");
	_view_inherited_action->setShortcut(Qt::Key_I);
	_view_inherited_action->setCheckable(true);
	connect(_view_inherited_action, SIGNAL(triggered()), this, SLOT(_ViewInheritedTiles()));

	_view_collision_action = new QAction("&Collision &Data", this);
	_view_collision_action->setStatusTip("Shows which quadrants on the map have collisions enabled");
	_view_collision_action->setShortcut(Qt::Key_C);
	_view_collision_action->setCheckable(true);
	connect(_view_collision_action, SIGNAL(triggered()), this, SLOT(_ViewCollisionData()));

	// Create actions found in the Tools menu
	_tool_paint_action = new QAction(QIcon("img/misc/editor_tools/pencil.png"), "&Paint Tiles", this);
	_tool_paint_action->setShortcut(Qt::Key_1);
	_tool_paint_action->setStatusTip("Switches the edit mode to allowing painting of tiles to the map");
	_tool_paint_action->setCheckable(true);
	connect(_tool_paint_action, SIGNAL(triggered()), this, SLOT(_SelectPaintTool()));

	_tool_swap_action = new QAction(QIcon("img/misc/editor_tools/arrow.png"), "S&wap Tiles", this);
	_tool_swap_action->setShortcut(Qt::Key_2);
	_tool_swap_action->setStatusTip("Switches the edit mode to allowing swapping of tiles at different positions");
	_tool_swap_action->setCheckable(true);
	connect(_tool_swap_action, SIGNAL(triggered()), this, SLOT(_SelectSwapTool()));

	_tool_erase_action = new QAction(QIcon("img/misc/editor_tools/eraser.png"), "&Erase Tiles", this);
	_tool_erase_action->setShortcut(Qt::Key_3);
	_tool_erase_action->setStatusTip("Switches the edit mode to erase tiles from the map");
	_tool_erase_action->setCheckable(true);
	connect(_tool_erase_action, SIGNAL(triggered()), this, SLOT(_SelectEraseTool()));

	_tool_inherit_action = new QAction(QIcon("img/misc/editor_tools/inherit.png"), "&Inherit Tiles", this);
	_tool_inherit_action->setShortcut(Qt::Key_4);
	_tool_inherit_action->setStatusTip("Switches the edit mode to inherit tiles from the inherited context");
	_tool_inherit_action->setCheckable(true);
	connect(_tool_inherit_action, SIGNAL(triggered()), this, SLOT(_SelectInheritTool()));

	_tool_area_select_action = new QAction(QIcon("img/misc/editor_tools/selection_rectangle.png"), "&Select Area", this);
	_tool_area_select_action->setShortcut(Qt::Key_5);
	_tool_area_select_action->setStatusTip("Select an area of tiles on the map");
	_tool_area_select_action->setCheckable(true);
	connect(_tool_area_select_action, SIGNAL(triggered()), this, SLOT(_SelectAreaSelectTool()));

	_tool_area_fill_action = new QAction(QIcon("img/misc/editor_tools/fill.png"), "&Fill Area", this);
	_tool_area_fill_action->setShortcut(Qt::Key_6);
	_tool_area_fill_action->setStatusTip("Fills the selection area or tile area with the chosen tile(s)");
	_tool_area_fill_action->setCheckable(true);
	connect(_tool_area_fill_action, SIGNAL(triggered()), this, SLOT(_SelectAreaFillTool()));

	_tool_area_clear_action = new QAction(QIcon("img/misc/editor_tools/clear.png"), "&Clear Area", this);
	_tool_area_clear_action->setShortcut(Qt::Key_7);
	_tool_area_clear_action->setStatusTip("Clears all tiles from the selection area or tile area");
	_tool_area_clear_action->setCheckable(true);
	connect(_tool_area_clear_action, SIGNAL(triggered()), this, SLOT(_SelectAreaClearTool()));

	_tool_area_inherit_action = new QAction(QIcon("img/misc/editor_tools/inherit_area.png"), "I&nherit Area", this);
	_tool_area_inherit_action->setShortcut(Qt::Key_8);
	_tool_area_inherit_action->setStatusTip("Inherits all tiles from the selection area or tile area");
	_tool_area_inherit_action->setCheckable(true);
	connect(_tool_area_inherit_action, SIGNAL(triggered()), this, SLOT(_SelectAreaInheritTool()));

	_tool_select_clear_action = new QAction(QIcon(), "Selection C&lear", this);
	_tool_select_clear_action->setShortcut(Qt::Key_Escape);
	_tool_select_clear_action->setStatusTip("Unselects any selects areas of the map");
	connect(_tool_select_clear_action, SIGNAL(triggered()), this, SLOT(_SelectSelectionClearTool()));

	_tool_select_all_action = new QAction(QIcon(), "Selection &All", this);
	_tool_select_all_action->setShortcut(tr("Ctrl+A"));
	_tool_select_all_action->setStatusTip("Selects the entire map area");
	connect(_tool_select_all_action, SIGNAL(triggered()), this, SLOT(_SelectSelectionAllTool()));

	// The following tools represent edit modes, and only one mode may be active at any given time
	_tool_action_group = new QActionGroup(this);
	_tool_action_group->addAction(_tool_paint_action);
	_tool_action_group->addAction(_tool_swap_action);
	_tool_action_group->addAction(_tool_erase_action);
	_tool_action_group->addAction(_tool_inherit_action);
	_tool_action_group->addAction(_tool_area_select_action);
	_tool_action_group->addAction(_tool_area_fill_action);
	_tool_action_group->addAction(_tool_area_clear_action);
	_tool_action_group->addAction(_tool_area_inherit_action);
	_tool_area_select_action->setChecked(true);

	// Create actions found in the Help menu
	_help_action = new QAction("&Help", this);
	_help_action->setShortcut(Qt::Key_F1);
	_help_action->setStatusTip("Brings up help documentation for the editor");
	connect(_help_action, SIGNAL(triggered()), this, SLOT(_HelpMessage()));

	_about_action = new QAction("&About", this);
	_about_action->setStatusTip("Brings up information about the editor");
	connect(_about_action, SIGNAL(triggered()), this, SLOT(_AboutMessage()));

	_about_qt_action = new QAction("About &Qt", this);
	_about_qt_action->setStatusTip("Brings up information about Qt");
	connect(_about_qt_action, SIGNAL(triggered()), this, SLOT(_AboutQtMessage()));
} // void Editor::_CreateActions()



void Editor::_CreateMenus() {
	_file_menu = menuBar()->addMenu("&File");
	_file_menu->addAction(_new_action);
	_file_menu->addAction(_open_action);
	// TODO: add a "Recent Files" action and submenu here
	_file_menu->addSeparator();
	_file_menu->addAction(_save_action);
	_file_menu->addAction(_save_as_action);
	_file_menu->addSeparator();
	_file_menu->addAction(_close_action);
	_file_menu->addAction(_quit_action);
	connect(_file_menu, SIGNAL(aboutToShow()), this, SLOT(_CheckFileActions()));

	_edit_menu = menuBar()->addMenu("&Edit");
	_edit_menu->addAction(_undo_action);
	_edit_menu->addAction(_redo_action);
	_edit_menu->addSeparator();
	_edit_menu->addAction(_cut_action);
	_edit_menu->addAction(_copy_action);
	_edit_menu->addAction(_paste_action);
	_edit_menu->addSeparator();
	_edit_menu->addAction(_tileset_properties_action);
	_edit_menu->addAction(_map_properties_action);
	_edit_menu->addAction(_map_resize_action);
	connect(_edit_menu, SIGNAL(aboutToShow()), this, SLOT(_CheckEditActions()));

	_view_menu = menuBar()->addMenu("&View");
	_view_menu->addAction(_view_grid_action);
	_view_menu->addAction(_view_missing_action);
	_view_menu->addAction(_view_inherited_action);
	_view_menu->addAction(_view_collision_action);
	connect(_view_menu, SIGNAL(aboutToShow()), this, SLOT(_CheckViewActions()));

	_tools_menu = menuBar()->addMenu("&Tools");
	_tools_menu->addAction(_tool_paint_action);
	_tools_menu->addAction(_tool_swap_action);
	_tools_menu->addAction(_tool_erase_action);
	_tools_menu->addAction(_tool_inherit_action);
	_tools_menu->addSeparator();
	_tools_menu->addAction(_tool_area_select_action);
	_tools_menu->addAction(_tool_area_fill_action);
	_tools_menu->addAction(_tool_area_clear_action);
	_tools_menu->addAction(_tool_area_inherit_action);
	_tools_menu->addSeparator();
	_tools_menu->addAction(_tool_select_clear_action);
	_tools_menu->addAction(_tool_select_all_action);
	connect(_tools_menu, SIGNAL(aboutToShow()), this, SLOT(_CheckToolsActions()));

	_help_menu = menuBar()->addMenu("&Help");
	_help_menu->addAction(_help_action);
	_help_menu->addAction(_about_action);
	_help_menu->addAction(_about_qt_action);
}



void Editor::_CreateToolbars() {
	_tiles_toolbar = addToolBar("Tiles");
	_tiles_toolbar->addAction(_undo_action);
	_tiles_toolbar->addAction(_redo_action);
	_tiles_toolbar->addSeparator();
	_tiles_toolbar->addAction(_tool_paint_action);
	_tiles_toolbar->addAction(_tool_swap_action);
	_tiles_toolbar->addAction(_tool_erase_action);
	_tiles_toolbar->addAction(_tool_inherit_action);
	_tiles_toolbar->addSeparator();
	_tiles_toolbar->addAction(_tool_area_select_action);
	_tiles_toolbar->addAction(_tool_area_fill_action);
	_tiles_toolbar->addAction(_tool_area_clear_action);
	_tiles_toolbar->addAction(_tool_area_inherit_action);
	// TODO: are these tools important enough to have in the toolbar? If so, they need icon images
// 	_tiles_toolbar->addSeparator();
// 	_tiles_toolbar->addAction(_tool_select_clear_action);
// 	_tiles_toolbar->addAction(_tool_select_all_action);
}



void Editor::_ClearEditorState() {
	_map_view->SetGridVisible(false);
	_map_view->SetMissingOverlayVisible(false);
	_map_view->SetInheritedOverlayVisible(false);
	_map_view->SetEditMode(SELECT_AREA_MODE);

	_view_grid_action->setChecked(false);
	_view_missing_action->setChecked(false);
	_view_inherited_action->setChecked(false);

	_undo_stack->setClean();

	// Done so that the appropriate icons on the toolbar are enabled or disabled
	_CheckEditActions();
	_CheckToolsActions();

	// Update the visual display of each sub-widget
	_map_view->DrawMap();
	_layer_view->RefreshView();
	_context_view->RefreshView();
	_tileset_view->RefreshView();
}



bool Editor::_UnsavedDataPrompt() {
	if (_map_data.IsInitialized() == false || _map_data.IsMapModified() == false)
		return true;

	switch (QMessageBox::warning(this, "Unsaved File", "The document contains unsaved changes.\n"
		"Do you want to save these changes before proceeding?", "&Save", "&Discard", "Cancel", 0, 2))
	{
		case 0: // Selected Save
			_FileSave();
			break;
		case 1: // Selected Discard
			break;
		case 2: // Selected Cancel
		default:
			statusBar()->showMessage("Abandoned save", 5000);
			return false;
	}

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Editor class -- private slot functions
///////////////////////////////////////////////////////////////////////////////

void Editor::_CheckFileActions() {
	if (_map_data.IsInitialized() == true) {
		if (_map_data.GetMapFilename().isEmpty() == true) // Don't allow normal saves with a newly created map
			_save_action->setEnabled(false);
		else
			_save_action->setEnabled(_map_data.IsMapModified());
		_save_as_action->setEnabled(true);
		_close_action->setEnabled(true);
	}
	else {
		_save_action->setEnabled(false);
		_save_as_action->setEnabled(false);
		_close_action->setEnabled(false);
	}
}



void Editor::_CheckEditActions() {
	// TODO: Currently tilesets can only be edited when no map is open. This is done because if the tileset data is modified,
	// the editor may be using a stale tileset definition when saving a map. In the future, tilesets should always be able
	// to be edited, and any modified tilesets in use by the map should be reloaded after the tileset file is saved.

	if (_map_data.IsInitialized() == true) {
		// TODO: Undo/Redo feature needs to be reimplemented. This option is disabled until that time
		_undo_action->setEnabled(false);
		_redo_action->setEnabled(false);
// 		_undo_action->setText("Undo " + _undo_stack->undoText());
// 		_redo_action->setText("Redo " + _undo_stack->redoText());
		// TODO: Cut/Copy/Paste feature has not yet been implemented. Options disabled until it becomes available
		_cut_action->setEnabled(false);
		_copy_action->setEnabled(false);
		_paste_action->setEnabled(false);
		_tileset_properties_action->setEnabled(false);
		// TODO: map properties disabled until we have a dialog class for it
		_map_properties_action->setEnabled(false);
		_map_resize_action->setEnabled(true);
	}
	else {
		_undo_action->setEnabled(false);
		_redo_action->setEnabled(false);
		_cut_action->setEnabled(false);
		_copy_action->setEnabled(false);
		_paste_action->setEnabled(false);
		_tileset_properties_action->setEnabled(true);
		_map_properties_action->setEnabled(false);
		_map_resize_action->setEnabled(false);
	}
}



void Editor::_CheckViewActions() {
	if (_map_data.IsInitialized() == true) {
		_view_grid_action->setEnabled(true);
		_view_missing_action->setEnabled(true);
		_view_inherited_action->setEnabled(true);
		// TODO: View collision grid feature has not yet been implemented. This option will remain disabled until it is.
		_view_collision_action->setEnabled(false);
	}
	else {
		_view_grid_action->setEnabled(false);
		_view_missing_action->setEnabled(false);
		_view_inherited_action->setEnabled(false);
		_view_collision_action->setEnabled(false);
	}
}



void Editor::_CheckToolsActions() {
	if (_map_data.IsInitialized() == true) {
		_tool_paint_action->setEnabled(true);
		_tool_swap_action->setEnabled(true);
		_tool_erase_action->setEnabled(true);
		_tool_area_select_action->setEnabled(true);
		_tool_area_fill_action->setEnabled(true);
		_tool_area_clear_action->setEnabled(true);
		_tool_select_clear_action->setEnabled(true);
		_tool_select_all_action->setEnabled(true);

		// These tools can only be active when a context is inheriting
		if (_map_data.GetSelectedTileContext()->IsInheritingContext() == true) {
			_tool_inherit_action->setEnabled(true);
			_tool_area_inherit_action->setEnabled(true);
		}
		else {
			_tool_inherit_action->setEnabled(false);
			_tool_area_inherit_action->setEnabled(false);
			// When moving from an inheriting context to a non-inheriting one, reset the edit mode if either of the inherit tools are active
			if ((_map_view->GetEditMode() == INHERIT_MODE) || (_map_view->GetEditMode() == INHERIT_AREA_MODE)) {
				_tool_paint_action->setChecked(true);
			}
		}
	}
	else {
		_tool_paint_action->setEnabled(false);
		_tool_swap_action->setEnabled(false);
		_tool_erase_action->setEnabled(false);
		_tool_inherit_action->setEnabled(false);
		_tool_area_select_action->setEnabled(false);
		_tool_area_fill_action->setEnabled(false);
		_tool_area_clear_action->setEnabled(false);
		_tool_area_inherit_action->setEnabled(false);
		_tool_select_clear_action->setEnabled(false);
		_tool_select_all_action->setEnabled(false);
	}
}



void Editor::_FileNew() {
	if (_UnsavedDataPrompt() == false) {
		statusBar()->showMessage("New operation cancelled due to existing unsaved map data.", 5000);
		return;
	}
	_map_data.DestroyData();

	// Prompt the user with the dialog for them to enter the new map data
	NewMapDialog new_dialog(this, &_map_data);
	if (new_dialog.exec() != QDialog::Accepted) {
		statusBar()->showMessage("New map operation cancelled", 5000);
	}
	else {
		_ClearEditorState();
		statusBar()->showMessage("New map created", 5000);
		setWindowTitle("Allacrost Map Editor -- New Map");
	}
}



void Editor::_FileOpen() {
	if (_UnsavedDataPrompt() == false) {
		statusBar()->showMessage("New operation cancelled due to existing unsaved map data.", 5000);
		return;
	}

	// ---------- 1) Attempt to open the file that the user requested
	QString filename = QFileDialog::getOpenFileName(this, APP_NAME + " -- Open Map File", "lua/data/maps", "Maps (*.lua)");
	if (filename.isEmpty() == true) {
		statusBar()->showMessage("No map file was opened (empty filename)", 5000);
		return;
	}

	// ---------- 2) Clear out any existing map data
	_map_data.DestroyData();

	// ---------- 3) Load the map data and setup the TilesetTab widget with the loaded tileset data
	if (_map_data.LoadData(filename) == false) {
		QMessageBox::critical(this, APP_NAME, "Error while opening map file '" + filename + "'. Report errors:\n" + _map_data.GetErrorMessage());
		return;
	}
	MapSizeModified();
	MapLayersModified();
	MapContextsModified();

	_ClearEditorState();
	statusBar()->showMessage(QString("Opened map \'%1\'").arg(_map_data.GetMapFilename()), 5000);
	setWindowTitle(QString("Allacrost Map Editor -- %1").arg(_map_data.GetMapFilename()));
}



void Editor::_FileSave() {
	if (_map_data.IsInitialized() == false) {
		return;
	}

	if (_map_data.SaveData() == false) {
		return;
	}

	_undo_stack->setClean();
	setWindowTitle(QString("%1").arg(_map_data.GetMapFilename()));
	statusBar()->showMessage(QString("Saved map \'%1\' successfully!").arg(_map_data.GetMapFilename()), 5000);
}



void Editor::_FileSaveAs() {
	if (_map_data.IsInitialized() == false) {
		return;
	}

	// Get the file name to save to from the user
	QString filename = QFileDialog::getSaveFileName(this, "Allacrost Map Editor -- File Save", "lua/data/maps", "Maps (*.lua)");
	if (filename.isEmpty() == true) {
		statusBar()->showMessage("Save abandoned.", 5000);
		return;
	}

	if (_map_data.SaveData(filename) == false) {
		return;
	}

	_undo_stack->setClean();
	setWindowTitle(QString("Allacrost Map Editor -- %1").arg(_map_data.GetMapFilename()));
	statusBar()->showMessage(QString("Saved map \'%1\' successfully").arg(_map_data.GetMapFilename()), 5000);
}



void Editor::_FileClose() {
	if (_UnsavedDataPrompt() == false) {
		return;
	}

	_map_data.DestroyData();
	_ClearEditorState();
	setWindowTitle("Allacrost Map Editor");
}



void Editor::_FileQuit() {
	if (_UnsavedDataPrompt() == true)
		qApp->exit(0);
}



void Editor::_CutSelection() {
	// TODO: implement this feature
}



void Editor::_CopySelection() {
	// TODO: implement this feature
}



void Editor::_PasteSelection() {
	// TODO: implement this feature
}



void Editor::_EditTilesetProperties() {
	TilesetEditor* tileset_editor = new TilesetEditor(this);
	tileset_editor->exec();
	delete tileset_editor;
}



void Editor::_EditMapProperties() {
	// TODO: add once dialog class has been created
// 	MapPropertiesDialog dialog(this, &_map_data);
// 	if (dialog.exec() != QDialog::Accepted) {
// 		statusBar()->showMessage("Map properties were not modified", 5000);
// 		return;
// 	}
}



void Editor::_EditMapResize() {
	MapResizeDialog resize_dialog(this, &_map_data);
	if (resize_dialog.exec() == QDialog::Accepted) {
		resize_dialog.ModifyMapData();
		DrawMapView();
	}
}



void Editor::_HelpMessage() {
    QMessageBox::about(this, "Allacrost Map Editor -- Help",
		"<p>In-editor documentation is not yet available. Please visit http://wiki.allacrost.org for available documentation.</p>");
}



void Editor::_AboutMessage() {
    QMessageBox::about(this, "Allacrost Map Editor -- About",
		"<center><h2>Hero of Allacrost Map Editor</h2></center>"
		"<center><h3>Copyright 2004-2018</h3></center>"
		"<p>A map editor created for the Hero of Allacrost project. See 'http://www.allacrost.org/' for more information</p>");
}



void Editor::_AboutQtMessage() {
    QMessageBox::aboutQt(this, "Allacrost Map Editor -- About QT");
}

///////////////////////////////////////////////////////////////////////////////
// EditTileCommand class
///////////////////////////////////////////////////////////////////////////////

EditTileCommand::EditTileCommand(const QString& action_text, QUndoCommand* parent) :
	QUndoCommand(action_text, parent)
{}



void EditTileCommand::undo() {
	// TODO: restore tiles / map size and redraw the map view
}



void EditTileCommand::redo() {
	// TODO: restore tiles / map size and redraw the map view
}

} // namespace hoa_editor
