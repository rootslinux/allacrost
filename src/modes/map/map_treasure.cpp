///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_treasure.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for map mode treasures.
*** ***************************************************************************/

// Allacrost engines
#include "input.h"
#include "mode_manager.h"
#include "system.h"
#include "video.h"

// Allacrost globals
#include "global.h"

// Other mode headers
#include "menu.h"

// Local map mode headers
#include "map.h"
#include "map_objects.h"
#include "map_treasure.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_input;
using namespace hoa_mode_manager;
using namespace hoa_system;
using namespace hoa_video;
using namespace hoa_gui;
using namespace hoa_global;
using namespace hoa_menu;

namespace hoa_map {

namespace private_map {

// -----------------------------------------------------------------------------
// ---------- TreasureContainer Class Functions
// -----------------------------------------------------------------------------

TreasureContainer::TreasureContainer() :
	_taken(false),
	_drunes(0)
{}



TreasureContainer::~TreasureContainer() {
	for (uint32 i = 0; i < _objects_list.size(); i++) {
		delete _objects_list[i];
	}
}



bool TreasureContainer::AddObject(uint32 id, uint32 quantity) {
	hoa_global::GlobalObject* obj = GlobalCreateNewObject(id, quantity);

	if (obj == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "invalid object id argument passed to function: " << id << endl;
		return false;
	}

	_objects_list.push_back(obj);
	return true;
}

// ----------------------------------------------------------------------------
// ---------- MapTreasure Class Functions
// ----------------------------------------------------------------------------

MapTreasure::MapTreasure(string image_file, uint8 num_total_frames, uint8 num_closed_frames, uint8 num_open_frames) :
	PhysicalObject()
{
	const uint32 DEFAULT_FRAME_TIME = 100; // The default number of milliseconds for frame animations

	_object_type = MAP_TREASURE_TYPE;

	std::vector<StillImage> frames;

	// (1) Load a the single row, multiple column multi image containing all of the treasure frames
	if (ImageDescriptor::LoadMultiImageFromElementGrid(frames, image_file, 1, num_total_frames) == false ) {
		PRINT_ERROR << "failed to load image file: " << image_file << endl;
		// TODO: throw exception
		return;
	}

	// Update the frame image sizes to work in the MapMode coordinate system
	for (uint32 i = 0; i < frames.size(); i++) {
		frames[i].SetWidth(frames[i].GetWidth() / (GRID_LENGTH / 2));
		frames[i].SetHeight(frames[i].GetHeight() / (GRID_LENGTH / 2));
	}

	// (2) Now that we know the total number of frames in the image, make sure the frame count arguments make sense
	if (num_open_frames == 0 || num_closed_frames == 0 || num_open_frames + num_closed_frames >= num_total_frames) {
		PRINT_ERROR << "invalid treasure image for image file: " << image_file << endl;
		// TODO: throw exception
		return;
	}

	// (3) Dissect the frames and create the closed, opening, and open animations
	hoa_video::AnimatedImage closed_anim, opening_anim, open_anim;

	for (uint8 i = 0; i < num_closed_frames; i++) {
		closed_anim.AddFrame(frames[i], DEFAULT_FRAME_TIME);
	}
	for (uint8 i = num_total_frames - num_open_frames; i < num_total_frames; i++) {
		open_anim.AddFrame(frames[i], DEFAULT_FRAME_TIME);
	}

	// Loop the opening animation only once
	opening_anim.SetNumberLoops(0);

	// If there are no additional frames for the opening animation, set the opening animation to be the open animation
	if (num_total_frames - num_closed_frames - num_open_frames == 0) {
		opening_anim = open_anim;
	}
	else {
		for (uint8 i = num_closed_frames; i < num_total_frames - num_open_frames; i++) {
			opening_anim.AddFrame(frames[i], DEFAULT_FRAME_TIME);
		}
	}

	AddAnimation(closed_anim);
	AddAnimation(opening_anim);
	AddAnimation(open_anim);

	// (4) Set the collision rectangle according to the dimensions of the first frame
	SetCollHalfWidth(frames[0].GetWidth() / 2.0f);
	SetCollHeight(frames[0].GetHeight());
} // MapTreasure::MapTreasure(string image_file, uint8 num_total_frames, uint8 num_closed_frames, uint8 num_open_frames)



void MapTreasure::LoadState() {
	string record_name = GetRecordName();

	// Check if the event corresponding to this treasure has already occurred
	if (MapMode::CurrentInstance()->GetGlobalRecordGroup()->DoesRecordExist(record_name) == true) {
		// If the event is non-zero, the treasure has already been opened
		if (MapMode::CurrentInstance()->GetGlobalRecordGroup()->GetRecord(record_name) != 0) {
			SetCurrentAnimation(TREASURE_OPEN_ANIM);
			_treasure_container.SetTaken(true);
		}
	}
}



void MapTreasure::Open() {
	if (_treasure_container.IsTaken() == true) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to retrieve an already taken treasure: " << object_id << endl;
		return;
	}

	SetCurrentAnimation(TREASURE_OPENING_ANIM);
}



void MapTreasure::Update() {
	PhysicalObject::Update();

	if ((current_animation == TREASURE_OPENING_ANIM) && (animations[TREASURE_OPENING_ANIM].IsLoopsFinished() == true)) {
		SetCurrentAnimation(TREASURE_OPEN_ANIM);
		MapMode::CurrentInstance()->GetTreasureSupervisor()->Initialize(this);

		// Add an event to the map group indicating that the treasure has now been opened
		string record_name = GetRecordName();
		if (MapMode::CurrentInstance()->GetGlobalRecordGroup()->DoesRecordExist(record_name) == true) {
			MapMode::CurrentInstance()->GetGlobalRecordGroup()->SetRecord(record_name, 1);
		}
		else {
			MapMode::CurrentInstance()->GetGlobalRecordGroup()->AddNewRecord(record_name, 1);
		}
	}
}

// ----------------------------------------------------------------------------
// ---------- GlimmerTreasure Class Functions
// ----------------------------------------------------------------------------

const uint32 GlimmerTreasure::GLIMMER_WAIT_COMMON          = 8000;
const uint32 GlimmerTreasure::GLIMMER_WAIT_UNCOMMON        = 14000;
const uint32 GlimmerTreasure::GLIMMER_WAIT_RARE            = 22000;
const std::string GlimmerTreasure::DEFAULT_IMAGE_FILE      = "img/misc/golden_glimmer.png";
const uint32 GlimmerTreasure::DEFAULT_FRAME_TIME           = 100;
const float GlimmerTreasure::DEFAULT_DEVIATION_MULTIPLIER  = 0.05f;



GlimmerTreasure::GlimmerTreasure(string image_file, uint32 frame_time, uint32 average_delay) :
	PhysicalObject(),
	_display_enabled(true),
	_display_forced(false)
{
	_object_type = GLIMMER_TREASURE_TYPE;
	collidable = false;
	coll_half_width = 0.5f;
	coll_height = 1.0f;

	if (image_file == "") {
		PRINT_ERROR << "empty string argument passed to class constructor" << endl;
		return;
	}

	// Load a the single row, multiple column multi image containing all of the treasure frames
	std::vector<StillImage> frames;
	if (ImageDescriptor::LoadMultiImageFromElementSize(frames, image_file, 32, 32) == false) {
		PRINT_ERROR << "failed to load image file: " << image_file << endl;
		return;
	}

	// Create the image representing the glimmer animation
	hoa_video::AnimatedImage glimmer_animation;
	glimmer_animation.SetNumberLoops(0);

	// Update the frame image sizes to work in the MapMode coordinate system and add them to the animation
	for (uint32 i = 0; i < frames.size(); i++) {
		frames[i].SetWidth(frames[i].GetWidth() / (GRID_LENGTH / 2));
		frames[i].SetHeight(frames[i].GetHeight() / (GRID_LENGTH / 2));
		glimmer_animation.AddFrame(frames[i], frame_time);
	}

	AddAnimation(glimmer_animation);
	SetCurrentAnimation(0);

	// Setup the display timer
	SetDisplayDelay(average_delay);
	_ResetWaitTimer();
}



void GlimmerTreasure::SetDisplayDelay(uint32 average, float standard_deviation) {
	if (average == 0) {
		return;
	}

	if (standard_deviation <= 0.0f) {
		return;
	}

	_average_wait = average;
	_standard_deviation_wait = standard_deviation;
	_ResetWaitTimer();
}



void GlimmerTreasure::SetDisplayEnabled(bool enable) {
	if (_display_enabled == enable)
		return;

	_display_enabled = enable;
	animations[0].SetLoopsFinished(false);
	animations[0].SetFrameIndex(0);
	_ResetWaitTimer();
}



void GlimmerTreasure::ForceDisplay() {
	_display_forced = true;
	animations[0].SetLoopsFinished(false);
	animations[0].SetFrameIndex(0);
}



void GlimmerTreasure::Acquire() {
	if (_treasure_container.IsTaken() == true) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to retrieve an already taken treasure: " << object_id << endl;
		return;
	}

	MapMode::CurrentInstance()->GetTreasureSupervisor()->Initialize(&_treasure_container);

	// Add an event to the map group indicating that the treasure has now been opened
	string record_name = GetRecordName();
	if (MapMode::CurrentInstance()->GetGlobalRecordGroup()->DoesRecordExist(record_name) == true) {
		MapMode::CurrentInstance()->GetGlobalRecordGroup()->SetRecord(record_name, 1);
	}
	else {
		MapMode::CurrentInstance()->GetGlobalRecordGroup()->AddNewRecord(record_name, 1);
	}
}



void GlimmerTreasure::Update() {
	// When forcing the display, update the animation until it is finished, then reset
	if (_display_forced == true) {
		// NOTE: we are explicitly ignoring the object's "updatable" member here
		animations[0].Update();
		if (animations[0].IsLoopsFinished() == true) {
			_display_forced = false;
			animations[0].SetFrameIndex(0);
			_ResetWaitTimer();
		}
		return;
	}

	// Otherwise if the display is enabled, process the timer and animation updates normally
	if (_display_enabled == false) {
		return;
	}

	bool was_finished = _wait_timer.IsFinished();
	_wait_timer.Update();

	if (_wait_timer.IsFinished() == true) {
		if (was_finished == false) {
			animations[0].SetLoopsFinished(false);
		}
		else {
			// NOTE: we are explicitly ignoring the object's "updatable" member here
			animations[0].Update();
			if (animations[0].IsLoopsFinished() == true) {
				animations[0].SetFrameIndex(0);
				_ResetWaitTimer();
			}
		}
	}
}



void GlimmerTreasure::Draw() {
	if (_treasure_container.IsTaken() == true)
		return;

	else if (_display_forced == true || _wait_timer.IsFinished() == true)
		PhysicalObject::Draw();
}



void GlimmerTreasure::_ResetWaitTimer() {
	uint32 next_time = GaussianRandomValue(_average_wait, _standard_deviation_wait, true);
	_wait_timer.Initialize(next_time);
	_wait_timer.Run();
}

// -----------------------------------------------------------------------------
// ---------- TreasureSupervisor class methods
// -----------------------------------------------------------------------------

TreasureSupervisor::TreasureSupervisor() :
	_treasure(nullptr),
	_selection(ACTION_SELECTED),
	_window_title(UTranslate("Treasure Contents"), TextStyle("title24", Color::white, VIDEO_TEXT_SHADOW_DARK, 1, -2)),
	_selection_name(),
	_selection_icon(nullptr)
{
	// Create the menu windows and option boxes used for the treasure menu and
	// align them at the appropriate locations on the screen
	_action_window.Create(768.0f, 64.0f, ~VIDEO_MENU_EDGE_BOTTOM);
	_action_window.SetPosition(512.0f, 460.0f);
	_action_window.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_TOP);
	_action_window.SetDisplayMode(VIDEO_MENU_INSTANT);

	_list_window.Create(768.0f, 236.0f);
	_list_window.SetPosition(512.0f, 516.0f);
	_list_window.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_TOP);
	_list_window.SetDisplayMode(VIDEO_MENU_INSTANT);

	_action_options.SetPosition(30.0f, 18.0f);
	_action_options.SetDimensions(726.0f, 32.0f, 1, 1, 1, 1);
	_action_options.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_action_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_action_options.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_action_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_action_options.SetCursorOffset(-50.0f, -25.0f);
	_action_options.SetTextStyle(TextStyle("title22", Color::white, VIDEO_TEXT_SHADOW_DARK, 1, -2));
	_action_options.AddOption(UTranslate("Finished"));
	_action_options.SetSelection(0);
	_action_options.SetOwner(&_action_window);

	_list_options.SetPosition(20.0f, 20.0f);
	_list_options.SetDimensions(726.0f, 200.0f, 1, 255, 1, 5);
	_list_options.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_list_options.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_list_options.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_list_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_list_options.SetCursorOffset(-50.0f, -25.0f);
	_list_options.SetTextStyle(TextStyle("text22", Color::white, VIDEO_TEXT_SHADOW_DARK, 1, -2));
	_list_options.SetOwner(&_list_window);
	// TODO: this currently does not work (text will be blank). Re-enable it once the scissoring bug is fixed in the video engine
// 	_list_options.Scissoring(true, true);

	_detail_textbox.SetPosition(20.0f, 90.0f);
	_detail_textbox.SetDimensions(726.0f, 128.0f);
	_detail_textbox.SetDisplaySpeed(50);
	_detail_textbox.SetTextStyle(TextStyle("text22", Color::white, VIDEO_TEXT_SHADOW_DARK, 1, -2));
	_detail_textbox.SetDisplayMode(VIDEO_TEXT_REVEAL);
	_detail_textbox.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_detail_textbox.SetOwner(&_list_window);

	_selection_name.SetStyle(TextStyle("text22", Color::white, VIDEO_TEXT_SHADOW_DARK, 1, -2));

	if (_drunes_icon.Load("img/icons/drunes.png") == false)
		IF_PRINT_WARNING(MAP_DEBUG) << "failed to load drunes icon for treasure menu" << endl;
} // TreasureSupervisor::TreasureSupervisor()



TreasureSupervisor::~TreasureSupervisor() {
	_action_window.Destroy();
	_list_window.Destroy();
}



void TreasureSupervisor::Initialize(MapTreasure* map_object) {
	if (map_object == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument was nullptr" << endl;
		return;
	}

	Initialize(map_object->GetTreasureContainer());
}



void TreasureSupervisor::Initialize(TreasureContainer* treasure) {
	if (treasure == nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument was nullptr" << endl;
		return;
	}

	if (_treasure != nullptr) {
		IF_PRINT_WARNING(MAP_DEBUG) << "_treasure member was not nullptr when method was called" << endl;
		return;
	}

	_treasure = treasure;
	MapMode::CurrentInstance()->PushState(STATE_TREASURE);

	// Construct the object list, including any drunes that were contained within the treasure
	if (_treasure->_drunes != 0) {
		_list_options.AddOption(MakeUnicodeString("<img/icons/drunes.png>       Drunes<R>" + NumberToString(_treasure->_drunes)));
	}

	for (uint32 i = 0; i < _treasure->_objects_list.size(); i++) {
		if (_treasure->_objects_list[i]->GetCount() > 1) {
			_list_options.AddOption(MakeUnicodeString("<" + _treasure->_objects_list[i]->GetIconImage().GetFilename() + ">       ") +
				_treasure->_objects_list[i]->GetName() +
				MakeUnicodeString("<R>x" + NumberToString(_treasure->_objects_list[i]->GetCount())));
		}
		else {
			_list_options.AddOption(MakeUnicodeString("<" + _treasure->_objects_list[i]->GetIconImage().GetFilename() + ">       ") +
				_treasure->_objects_list[i]->GetName());
		}
	}

	for (uint32 i = 0; i < _list_options.GetNumberOptions(); i++) {
		_list_options.GetEmbeddedImage(i)->SetDimensions(30.0f, 30.0f);
	}

	_action_options.SetSelection(0);
	_action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	_list_options.SetSelection(0);
	_list_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

	_selection = ACTION_SELECTED;
	_action_window.Show();
	_list_window.Show();

	// Immediately add the drunes and objects to the player's inventory
	GlobalManager->AddDrunes(_treasure->_drunes);

	// The AddToInventory function will delete the pointer that it is given if that type of object
	// already exists in the inventory. Because we still require all of the object pointers to remain in
	// memory while the menu is being displayed, we check if an object exists in the inventory, increment
	// the inventory count if it does, and keep a record that we must delete the object once the menu is closed
	for (uint32 i = 0; i < _treasure->_objects_list.size(); i++) {
		GlobalObject* obj = _treasure->_objects_list[i];
		if (GlobalManager->IsObjectInInventory(obj->GetID()) == false) {
			GlobalManager->AddToInventory(_treasure->_objects_list[i]);
		}
		else {
			GlobalManager->IncrementObjectCount(obj->GetID(), obj->GetCount());
			_objects_to_delete.push_back(obj);
		}
	}
} // void TreasureSupervisor::Initialize(TreasureContainer* treasure)



void TreasureSupervisor::Update() {
	_action_window.Update();
	_list_window.Update();
	_action_options.Update();
	_list_options.Update();
	_detail_textbox.Update();

	// Allow the user to go to menu mode at any time when the treasure menu is open
	if (InputManager->MenuPress()) {
		MenuMode *MM = new MenuMode();
		ModeManager->Push(MM);
		return;
	}

	// Update the menu according to which sub-window is currently selected
	if (_selection == ACTION_SELECTED)
		_UpdateAction();
	else if (_selection == LIST_SELECTED)
		_UpdateList();
	else if (_selection == DETAIL_SELECTED)
		_UpdateDetail();
	else
		IF_PRINT_WARNING(MAP_DEBUG) << "unknown selection state: " << _selection << endl;
}



void TreasureSupervisor::Draw() {
	if (IsActive() == false) {
		IF_PRINT_WARNING(MAP_DEBUG) << "treasure supervisor not initialized" << endl;
		return;
	}

	VideoManager->PushState();
	VideoManager->SetStandardCoordSys();

	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	_action_window.Draw();
	if (_selection != DETAIL_SELECTED) {
		_action_options.Draw();
	}
	_list_window.Draw();

	VideoManager->Move(512.0f, 465.0f);
	_window_title.Draw();

	if (_selection == DETAIL_SELECTED) {
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
		// Move to the upper left corner and draw the object icon
		if (_selection_icon != nullptr) {
			VideoManager->Move(150.0f, 535.0f);
			_selection_icon->Draw();
		}

		// Draw the name of the selected object to the right of the icon
		VideoManager->MoveRelative(80.0f, 20.0f);
		_selection_name.Draw();

		_detail_textbox.Draw();
	}
	else {
		_list_options.Draw();
	}

	VideoManager->PopState();
} // void TreasureSupervisor::Draw()



void TreasureSupervisor::Finish() {
	for (uint32 i = 0; i < _objects_to_delete.size(); i++) {
		delete _objects_to_delete[i];
	}
	_objects_to_delete.clear();

	_treasure->_taken = true;
	_treasure->_drunes = 0;
	_treasure->_objects_list.clear();
	_treasure = nullptr;

	_action_window.Hide();
	_list_window.Hide();
	_list_options.ClearOptions();

	MapMode::CurrentInstance()->PopState();
}



void TreasureSupervisor::_UpdateAction() {
	if (InputManager->ConfirmPress()) {
		if (_action_options.GetSelection() == 0) // "Take All" action
			Finish();
		else
			IF_PRINT_WARNING(MAP_DEBUG) << "unhandled action selection in OptionBox: " << _action_options.GetSelection() << endl;
	}

	else if (InputManager->LeftPress())
		_action_options.InputLeft();

	else if (InputManager->RightPress())
		_action_options.InputRight();

	else if (InputManager->UpPress()) {
		_selection = LIST_SELECTED;
		_list_options.SetSelection(_list_options.GetNumberOptions() - 1);
		_action_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		_list_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	}

	else if (InputManager->DownPress()) {
		_selection = LIST_SELECTED;
		_list_options.SetSelection(0);
		_action_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		_list_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	}
}



void TreasureSupervisor::_UpdateList() {
	if (InputManager->ConfirmPress()) {
		_selection = DETAIL_SELECTED;
		_list_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

		uint32 list_selection = _list_options.GetSelection();
		if (list_selection == 0 && _treasure->_drunes != 0) { // If true, the drunes have been selected
			_selection_name.SetText(UTranslate("Drunes"));
			_selection_icon = &_drunes_icon;
			_detail_textbox.SetDisplayText(UTranslate("With the additional ") + MakeUnicodeString(NumberToString(_treasure->_drunes)) +
			UTranslate(" drunes found in this treasure added, the party now holds a total of ") + MakeUnicodeString(NumberToString(GlobalManager->GetDrunes()))
			+ MakeUnicodeString(" drunes."));
		}
		else { // Otherwise, a GlobalObject is selected
			if (_treasure->_drunes != 0)
				list_selection--;
			_selection_name.SetText(_treasure->_objects_list[list_selection]->GetName());
			// TODO: this is not good practice. We should probably either remove the const status from the GetIconImage() call
			_selection_icon = const_cast<StillImage*>(&_treasure->_objects_list[list_selection]->GetIconImage());
			_detail_textbox.SetDisplayText(_treasure->_objects_list[list_selection]->GetDescription());
		}
	}

	else if (InputManager->CancelPress()) {
		_selection = ACTION_SELECTED;
		_action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		_list_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	}

	else if (InputManager->UpPress()) {
		if (_list_options.GetSelection() == 0) {
			_selection = ACTION_SELECTED;
			_action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_list_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		}
		else {
			_list_options.InputUp();
		}
	}

	else if (InputManager->DownPress()) {
		if (static_cast<uint32>(_list_options.GetSelection()) == (_list_options.GetNumberOptions() - 1)) {
			_selection = ACTION_SELECTED;
			_action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_list_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		}
		else {
			_list_options.InputDown();
		}
	}
}



void TreasureSupervisor::_UpdateDetail() {
	if (InputManager->ConfirmPress() || InputManager->CancelPress()) {
		if (_detail_textbox.IsFinished() == false) {
			_detail_textbox.ForceFinish();
		}
		else {
			_selection = LIST_SELECTED;
			_list_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		}
	}
}

} // namespace private_map

} // namespace hoa_map
