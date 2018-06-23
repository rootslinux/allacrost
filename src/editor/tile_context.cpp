///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    tile_context.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for tile context data and view classes
*** **************************************************************************/

#include <QDebug>
#include <QMouseEvent>

#include "editor_utils.h"
#include "editor.h"
#include "map_data.h"
#include "tile_context.h"

using namespace std;
using namespace hoa_utils;

namespace hoa_editor {

///////////////////////////////////////////////////////////////////////////////
// TileContext class
///////////////////////////////////////////////////////////////////////////////

void TileContext::_ClearInheritingContext() {
	_inherited_context_id = INVALID_CONTEXT;

	for (uint32 i = 0; i < _tile_layers.size(); ++i) {
		_tile_layers[i].ReplaceTiles(INHERITED_TILE, MISSING_TILE);
	}
}



void TileContext::_AddTileLayer(TileLayer& layer) {
	if (layer.GetHeight() == 0 || layer.GetLength() == 0) {
		qDebug() << "could not add layer because one or both dimensions are zero" << endl;
		return;
	}

	// If no tile layers exist, we don't need to do any layer size checking
	if (_tile_layers.empty() == true) {
		_tile_layers.push_back(layer);
		return;
	}

	// Ensure that the height and length of the layer match an existing layer
	if (layer.GetHeight() != _tile_layers[0].GetHeight()) {
		qDebug() << "could not add layer because its height does not match the existing layers" << endl;
		return;
	}
	if (layer.GetLength() != _tile_layers[0].GetLength()) {
		qDebug() << "could not add layer because its length does not match the existing layers" << endl;
		return;
	}

	_tile_layers.push_back(layer);
}



void TileContext::_RemoveTileLayer(uint32 layer_index) {
	if (layer_index >= _tile_layers.size()) {
		qDebug() << "could not remove layer because the layer_index argument (" << layer_index
			<< ") exceeds the number of layers (" << layer_index << ")" << endl;
		return;
	}

	for (uint32 i = layer_index; i < _tile_layers.size() - 1; ++i) {
		_tile_layers[i] = _tile_layers[i+1];
	}
	_tile_layers.pop_back();
}



void TileContext::_CloneTileLayer(uint32 layer_index) {
	if (layer_index >= _tile_layers.size()) {
		qDebug() << "could not remove layer because the layer_index argument (" << layer_index
			<< ") exceeds the number of layers (" << layer_index << ")" << endl;
		return;
	}

	_tile_layers.push_back(_tile_layers[layer_index]);
}



void TileContext::_SwapTileLayers(uint32 first_index, uint32 second_index) {
	if (first_index >= _tile_layers.size() || second_index >= _tile_layers.size()) {
		qDebug() << "could not remove layer because one or both index arguments (" << first_index
			<< ", " << second_index << ") exceeds the number of layers (" << _tile_layers.size() << ")" << endl;
		return;
	}

	// TODO: see if this can be replaced with a call to std::swap
	TileLayer swap = _tile_layers[first_index];
	_tile_layers[first_index] = _tile_layers[second_index];
	_tile_layers[second_index] = swap;
}

///////////////////////////////////////////////////////////////////////////////
// ContextView class
///////////////////////////////////////////////////////////////////////////////

const uint32 ContextView::ID_COLUMN;
const uint32 ContextView::NAME_COLUMN;
const uint32 ContextView::INHERITS_COLUMN;

ContextView::ContextView(MapData* data) :
	QTreeWidget(),
	_map_data(data),
	_original_context_name(),
	_original_context_inheritance(),
	_right_click_item(nullptr),
	_right_click_menu(nullptr),
	_add_context_action(nullptr),
	_clone_context_action(nullptr),
	_rename_context_action(nullptr),
	_delete_context_action(nullptr)
{
	if (data == nullptr) {
		qDebug() << "constructor received nullptr map data argument" << endl;
		return;
	}

	// Enable settings so that contexts can be dragged and reordered
	setSelectionMode(QAbstractItemView::SingleSelection);
	setDragEnabled(true);
	viewport()->setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(QAbstractItemView::InternalMove);

	// Create column dimensions, headers, and properties
    setColumnCount(3);
	setColumnWidth(ID_COLUMN, 25); // Make this column small as it only contains the ID integer
	setColumnWidth(NAME_COLUMN, 200);
	QStringList context_headers;
	context_headers << "ID" << "Context" << "Inherits From";
	setHeaderLabels(context_headers);
	setIndentation(0);

	// Setup actions for the right click menu
	_add_context_action = new QAction("Add New Context", this);
	_add_context_action->setStatusTip("Adds a new empty context to the end of the context list");
	_clone_context_action = new QAction("Clone Context", this);
	_clone_context_action->setStatusTip("Adds a new context that clones the data and properties of an existing context");
	_rename_context_action = new QAction("Rename Context", this);
	_rename_context_action->setStatusTip("Renames the selected context (can also be activated by double-clicking the context's name)");
	_delete_context_action = new QAction("Delete Tile Context", this);
	_delete_context_action->setStatusTip("Deletes the selected context");

	_right_click_menu = new QMenu(this);
	_right_click_menu->addAction(_add_context_action);
	_right_click_menu->addAction(_clone_context_action);
	_right_click_menu->addAction(_rename_context_action);
	_right_click_menu->addAction(_delete_context_action);

	// Connect all signals and slots
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(_ChangeSelectedContext()));
	connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(_ValidateChangedData(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(_ChangeContextProperties(QTreeWidgetItem*, int)));
	connect(_add_context_action, SIGNAL(triggered()), this, SLOT(_AddTileContext()));
	connect(_clone_context_action, SIGNAL(triggered()), this, SLOT(_CloneTileContext()));
	connect(_rename_context_action, SIGNAL(triggered()), this, SLOT(_RenameTileContext()));
	connect(_delete_context_action, SIGNAL(triggered()), this, SLOT(_DeleteTileContext()));
}



ContextView::~ContextView() {
	delete _right_click_menu;
	delete _add_context_action;
	delete _clone_context_action;
	delete _rename_context_action;
	delete _delete_context_action;
}



void ContextView::mousePressEvent(QMouseEvent* event) {
	// Handle left clicks the standard way. Right clicks bring up the context action menu
	if (event->button() == Qt::LeftButton) {
		QTreeWidget::mousePressEvent(event);
	}
	else {
		if (_map_data->IsInitialized() == true) {
			// Determine which QTreeWidgetItem was selected, if any. Enable/disable menu actions appropriately
			_right_click_item = itemAt(event->pos());
			_add_context_action->setEnabled(true);
			if (_right_click_item != nullptr) {
				_clone_context_action->setEnabled(true);
				_rename_context_action->setEnabled(true);
				_delete_context_action->setEnabled(true);
			}
			else {
				// Clicked a space in the widget that did not point to any item
				_clone_context_action->setEnabled(false);
				_rename_context_action->setEnabled(false);
				_delete_context_action->setEnabled(false);
			}
		}
		else {
			_add_context_action->setEnabled(false);
			_clone_context_action->setEnabled(false);
			_rename_context_action->setEnabled(false);
			_delete_context_action->setEnabled(false);
		}

		_right_click_menu->exec(QCursor::pos());
		return;
	}
}



void ContextView::dropEvent(QDropEvent* event) {
	QTreeWidget::dropEvent(event);
	vector<uint32> context_order; // Holds the new context positions

	// Update the IDs for each tile context to correspond to the new context order
	QTreeWidgetItem* root = invisibleRootItem();
	for (uint32 i = 0; i < static_cast<uint32>(root->childCount()); ++i) {
		QTreeWidgetItem* child = root->child(i);
		context_order.push_back(child->text(ID_COLUMN).toUInt());
		child->setText(ID_COLUMN, QString::number(i + 1));
	}

	// Make the appropriate changes corresponding to the context order in the map data
	for (uint32 i = 0; i < context_order.size(); ++i) {
		// Skip over contexts that haven't been affected by the reordering
		if (context_order[i] == i + 1) {
			continue;
		}

		// Find the new location of this context and swap it with other context
		for (uint32 j = 0; j < _map_data->GetTileContextCount(); ++j) {
			if (context_order[j] == i + 1) {
				uint32 temp = context_order[i];
				context_order[i] = context_order[j];
				context_order[j] = temp;
				_map_data->SwapTileContexts(i + 1, j + 1);
				break;
			}
		}
	}

	Editor* editor = static_cast<Editor*>(topLevelWidget());
	editor->MapContextsModified();
}



void ContextView::RefreshView() {
	clear();

	// Add all tile contexts from the map data
	for (uint32 i = 0; i < _map_data->GetTileContextCount(); ++i) {
		TileContext* context = _map_data->FindTileContextByIndex(i);

		QTreeWidgetItem* item = new QTreeWidgetItem(this);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
		item->setText(ID_COLUMN, QString::number(context->GetContextID()));
		item->setText(NAME_COLUMN, context->GetContextName());
		if (context->IsInheritingContext() == false) {
			item->setText(INHERITS_COLUMN, "");
		}
		else {
			TileContext* inherited_context = _map_data->FindTileContextByID(context->GetInheritedContextID());
			if (inherited_context == nullptr) {
				// This case should never happen and indicates that there is an error in the map data
				QMessageBox::warning(this, "Invalid Inherited Context ID", "Bad inheriting context ID. Possible map data corruption.");
				item->setText(INHERITS_COLUMN, "");
			}
			else {
				item->setText(INHERITS_COLUMN, QString::fromStdString(NumberToString(inherited_context->GetContextID())));
			}
		}
	}

	setCurrentItem(itemAt(0, 0));
}



void ContextView::_ChangeSelectedContext() {
	// We only allow one selected context at a time. The size of selected items should only ever be 0 or 1.
	QList<QTreeWidgetItem*> selected_items = selectedItems();
	if (selected_items.size() != 1) {
		return;
	}

	QTreeWidgetItem* selection = selected_items.first();
	int32 context_id = selection->text(ID_COLUMN).toInt();
	if (_map_data->ChangeSelectedTileContext(context_id) == nullptr) {
		QMessageBox::warning(this, "Context Selection Failure", _map_data->GetErrorMessage());
		return;
	}

	// Update the available edit tools (some tools are only enabled for inheriting contexts) and redraw the map view to show the newly selected context
	static_cast<Editor*>(topLevelWidget())->UpdateSelectedContext();
}



void ContextView::_ChangeContextProperties(QTreeWidgetItem* item, int column) {
	if (item == nullptr)
		return;

	if (column == ID_COLUMN) {
		// User is not allowed to modify the IDs of contexts
		return;
	}
	else if (column == NAME_COLUMN) {
		// While technically this was not a right-click event, this allows us to use the same code path for performing rename operations
		_right_click_item = item;
		_RenameTileContext();
	}
	else if (column == INHERITS_COLUMN) {
		// While technically this was not a right-click event, this is needed so that _ValidateChangedData() knows to process these changes
		_right_click_item = item;
		_original_context_inheritance = _right_click_item->text(INHERITS_COLUMN);
		openPersistentEditor(item, INHERITS_COLUMN);
	}
	else {
		QMessageBox::warning(this, "Context Property Change Failure", "Invalid column clicked");
	}
}



void ContextView::_ValidateChangedData(QTreeWidgetItem* item, int column) {
	if (item != _right_click_item)
		return;

	if (column == NAME_COLUMN) {
		if (_original_context_name.isEmpty() == true)
			return;

		closePersistentEditor(item, column);
		if (_map_data->RenameTileContext(item->text(ID_COLUMN).toInt(), item->text(NAME_COLUMN)) == false) {
			// To prevent an infinite recursion loop, we must nullify _right_click_item before restoring the context's name
			_right_click_item = nullptr;
			item->setText(NAME_COLUMN, _original_context_name);
			QMessageBox::warning(this, "Context Rename Failure", _map_data->GetErrorMessage());
		}
		else {
			Editor* editor = static_cast<Editor*>(topLevelWidget());
			editor->MapContextsModified();
		}

		_right_click_item = nullptr;
		_original_context_name.clear();
	}
	else if (column == INHERITS_COLUMN) {
		closePersistentEditor(item, column);
		int32 new_inheritance = INVALID_CONTEXT;
		bool valid_change = false;
		QString error_message;
		// Check that the new value for this column is a valid value (must be blank or an integer value)
		if (item->text(INHERITS_COLUMN).isEmpty() == true) {
			new_inheritance = INVALID_CONTEXT;
			valid_change = true;
		}
		else {
			new_inheritance = item->text(INHERITS_COLUMN).toInt(&valid_change);
			if (valid_change == false) {
				error_message = "ERROR: Column must be set to an integer value.";
			}
			if (item->text(ID_COLUMN).toInt(&valid_change) == new_inheritance) {
				valid_change = false;
				error_message = "ERROR: A context can not inherit from itself.";
			}
		}

		// Now try setting the new context inheritance value
		if (valid_change == true) {
			if (_map_data->ChangeInheritanceTileContext(item->text(ID_COLUMN).toInt(), new_inheritance) == false) {
				valid_change = false;
				error_message = _map_data->GetErrorMessage();
			}
		}

		// If any errors occurred, restore the original contents of the column and display an appropriate aerror message
		if (valid_change == false) {
			// To prevent an infinite recursion loop, we must nullify _right_click_item before restoring the context's inheritance
			_right_click_item = nullptr;
			item->setText(INHERITS_COLUMN, _original_context_inheritance);
			_original_context_inheritance.clear();
			QMessageBox::warning(this, "Context Inheritance Change Failure", error_message);
		}
		else {
			_map_data->SetMapModified(true);
			static_cast<Editor*>(topLevelWidget())->UpdateSelectedContext(); // This must be done to activate or deactivate the inheriting tools
		}

		_right_click_item = nullptr;
		_original_context_inheritance.clear();
	}
}



void ContextView::_AddTileContext() {
	static uint32 new_context_number = 1; // Used so that each new tile context added is written as "New Context (#)"

	// Add the new context to the map data. If it fails and we haven't reached the maximum number of contexts,
	// increment the number to use a different context name and try again
	QString context_name;
	while (true) {
		context_name.clear();
		context_name = "New Context (" + QString::number(new_context_number) + QString(")");

		if (_map_data->AddTileContext(context_name, INVALID_CONTEXT) != nullptr) {
			_map_data->SetMapModified(true);
			break;
		}
		else if (_map_data->GetTileContextCount() >= MAX_CONTEXTS) {
			QMessageBox::warning(this, "Context Creation Failure", "This map already has the maximum number of contexts allowed. New contexts will not be added.");
			return;
		}
		else {
			new_context_number++;
		}
	}

	// Add the new item to the view. All new contexts will not inherit from another context
	QTreeWidgetItem* item = new QTreeWidgetItem(this);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
	item->setText(ID_COLUMN, QString::number(_map_data->GetTileContextCount()));
	item->setText(NAME_COLUMN, context_name);
	item->setText(INHERITS_COLUMN, "");

	Editor* editor = static_cast<Editor*>(topLevelWidget());
	editor->MapContextsModified();
	setCurrentItem(item); // Select the newly added item
	new_context_number++;
}



void ContextView::_CloneTileContext() {
	if (_right_click_item == nullptr)
		return;

	// Clone the context data
	uint32 context_id = _right_click_item->text(ID_COLUMN).toInt();
	TileContext* clone_context = _map_data->CloneTileContext(context_id);

	if (clone_context == nullptr) {
		QMessageBox::warning(this, "Context Clone Failure", _map_data->GetErrorMessage());
		return;
	}

	// Construct a new widget item using the context object that was just cloned
	QTreeWidgetItem* item = new QTreeWidgetItem(this);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
	item->setText(ID_COLUMN, QString::number(clone_context->GetContextID()));
	item->setText(NAME_COLUMN, clone_context->GetContextName());
	if (clone_context->GetInheritedContextID() == INVALID_CONTEXT)
		item->setText(INHERITS_COLUMN, "");
	else
		item->setText(INHERITS_COLUMN, QString::number(clone_context->GetInheritedContextID()));

	Editor* editor = static_cast<Editor*>(topLevelWidget());
	editor->MapContextsModified();
	setCurrentItem(item);
}



void ContextView::_RenameTileContext() {
	if (_right_click_item == nullptr)
		return;

	_original_context_name = _right_click_item->text(NAME_COLUMN);
	openPersistentEditor(_right_click_item, NAME_COLUMN);
}



void ContextView::_DeleteTileContext() {
	if (_right_click_item == nullptr)
		return;

	if (_map_data->GetTileContextCount() == 1) {
		QMessageBox::warning(this, "Context Deletion Failure", "You may not delete the last remaining context for a map.");
		return;
	}

	uint32 context_id = _right_click_item->text(ID_COLUMN).toInt();
	QString context_name = _map_data->FindTileContextByID(context_id)->GetContextName();
	QString warning_text = "Deleting a tile context from the map is an irreversible operation and all tile data on the context will be permanently lost.";
	warning_text = warning_text.append(" Are you sure that you wish to proceed with the deletion of the tile context '%1'?");
	warning_text = warning_text.arg(context_name);
	switch (QMessageBox::warning(this, "Delete Context Confirmation", warning_text, "&Confirm", "C&ancel", 0, 1))
	{
		case 0: // Selected Confirm
			break;
		case 1: // Selected Cancel
		default:
			return;
	}

	// Delete the context from the map data and make sure that it was successful
	if (_map_data->DeleteTileContext(context_id) == false) {
		QMessageBox::warning(this, "Context Deletion Failure", _map_data->GetErrorMessage());
		return;
	}
	_map_data->SetMapModified(true);

	// If the item being deleted is the selected item, change the selction to the item before it (or after if its the first item)
	if (currentItem() == _right_click_item) {
		QTreeWidgetItem* new_selection = itemAbove(_right_click_item);
		if (new_selection == nullptr)
			new_selection = itemBelow(_right_click_item);
		setCurrentItem(new_selection);
	}

	// Deleting the item directly also removes it from the QTreeWidget automatically
	delete _right_click_item;
	_right_click_item = nullptr;

	// Update the IDs and inheriting ids of the remaining contexs
	QTreeWidgetItem* root = invisibleRootItem();
	for (uint32 i = 0; i < static_cast<uint32>(root->childCount()); ++i) {
		root->child(i)->setText(ID_COLUMN, QString::number(i + 1));
	}

	Editor* editor = static_cast<Editor*>(topLevelWidget());
	editor->MapContextsModified();
	editor->DrawMapView();
	editor->statusBar()->showMessage(QString("Deleted map context '%1'").arg(context_name), 5000);
}

} // namespace hoa_editor
