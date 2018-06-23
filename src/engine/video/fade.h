///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    fade.h
*** \author  Raj Sharma (roos)
*** \brief   Header file for ScreenFader class.
*** ***************************************************************************/

#pragma once

#include "defs.h"
#include "utils.h"

#include "color.h"
#include "image.h"

namespace hoa_video {

namespace private_video {

/** ****************************************************************************
*** \brief Used to monitor progress for a fading screen.
***
*** This class is used internally by the video engine to calculate how much to
*** fade the screen by. It keeps track of the current color and figures out whether
*** it should implement the fade using modulation or an overlay.
***
*** \note Fades are either implemented with overlays or with modulation, depending
*** on whether it's a simple fade to black or a fade to a different color.
*** ***************************************************************************/
class ScreenFader {
public:
	ScreenFader();

	/** \brief Begins a new screen fading process
	*** \param final The color to fade the screen to.
	*** \param time The number of milliseconds that the fade should last for.
	**/
	void BeginFade(const Color &final, uint32 time);

	/** \brief Updates the amount of fading for the screen
	*** \param time The number of milliseconds that have passed since last update.
	**/
	void Update(uint32 time);

	//! \brief Draws the fade overlay on to the screen
	void Draw();

	//! \brief Class Member Accessor Functions
	bool IsUseFadeOverlay() const
		{ return _use_fade_overlay; }

	const Color& GetFadeOverlayColor() const
		{ return _fade_overlay_color; }

	float GetFadeModulation() const
		{ return _fade_modulation; }

	bool IsFadeActive() const
		{ return _fade_active; }

private:
	//! \brief The color that the screen is currently faded to.
	Color _current_color;

	//! \brief The initial color of the screen before the fade started.
	Color _initial_color;

	//! \brief The destination color that the screen is being fade to.
	Color _final_color;

	//! \brief The image used to apply the current color drawn to the screen
	StillImage _fade_image;

	//! \brief The number of milliseconds that have passed since the fading began.
	uint32 _current_time;

	//! \brief The number of milliseconds that this fade was set to last for.
	uint32 _end_time;

	//! \brief True if the class is currently in the process of fading
	bool _fade_active;

	//! \brief Set to true if using an overlay, false if using modulation.
	bool _use_fade_overlay;

	//! \brief Color of the overlay, if one is being used.
	Color _fade_overlay_color;

	//! \brief A float determining the degree of modulation.
	float _fade_modulation;

	//! \brief Set to true if the fading process requires interpolation of RGB values between colors
	bool _interpolate_rgb_values;
	//@}
}; // class ScreenFader

} // namespace private_video

} // namespace hoa_video
