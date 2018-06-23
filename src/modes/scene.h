///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    scene.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for scene mode interface.
***
*** This code handles the game event processing and frame drawing when the user
*** is in scene mode (when a full-screen art piece is drawn to the screen).
*** The user must wait a minimum amount of time, defined by the MIN_SCENE_UPDATES
*** constant, before the scene can be passed. This is to keep the user from
*** accidentally skipping over one of our beautiful artworks before they have
*** the chance to gawk at it in amazement. :)
*** ***************************************************************************/

#pragma once

#include <string>

#include "utils.h"
#include "defs.h"

#include "mode_manager.h"

//! All calls to scene mode are wrapped in this namespace.
namespace hoa_scene {

//! Determines whether the code in the hoa_scene namespace should print debug statements or not.
extern bool SCENE_DEBUG;

//! An internal namespace to be used only within the scene code. Don't use this namespace anywhere else!
namespace private_scene {

//! How many milliseconds must pass before the user can exit the scene
const uint32 MIN_SCENE_UPDATES = 750;

} // namespace private_scene


/** ****************************************************************************
*** \brief Handles everything that needs to be done when full-screen artwork is displayed.
***
*** This game mode displays a single full-screen art scene, which are used in
*** various places in the game. The scene can not be exited until the amount
*** of milliseconds defined in MIN_SCENE_UPDATES has expired, to ensure that
*** the user does not accidentally skip the scene and can take the time to
*** appreciate the art.
***
*** \todo This mode is not yet fully implemented nor used
*** ***************************************************************************/
class SceneMode : public hoa_mode_manager::GameMode {
public:
	SceneMode();

	~SceneMode();

	//! \brief Sets the descriptions of the possible scene command inputs
	void SetCommandDescriptions() {}

	//! \brief Resets appropriate class members. Called whenever SceneMode is made the active game mode.
	void Reset();

	//! \brief Updates the game state by the amount of time that has elapsed
	void Update();

	//! \brief Draws the next frame to be displayed on the screen
	void Draw();

private:
	//! Retains the number of milliseconds that have elapsed since this mode was initialized
	uint32 _scene_timer;

	//hoa_video::StillImage scene;
}; // class SceneMode

} // namespace hoa_scene
