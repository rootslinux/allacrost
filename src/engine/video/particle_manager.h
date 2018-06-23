///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    particle_manager.h
*** \author  Raj Sharma (roos)
*** \brief   Header file for particle manager
***
*** The particle manager is very simple. Every time you want to draw an effect,
*** you call AddEffect() with a pointer to the effect definition structure.
*** Then every frame, call Update() and Draw() to draw all the effects.
*** ***************************************************************************/

#pragma once

#include "defs.h"
#include "utils.h"

//! \brief A particle effet ID is an int
typedef int32 ParticleEffectID;

//! \brief Represent an invalid ID
const ParticleEffectID VIDEO_INVALID_EFFECT = -1;

namespace hoa_video {

namespace private_video {

/** ****************************************************************************
***  \brief Used to store, update, and draw all particle effects.
*** ***************************************************************************/
class ParticleManager {
public:
	ParticleManager()
		{ _current_id = 0; }

	/** \brief Loads an effect definition from a particle file
	*** \param filename The file to load the effect definition from
	*** \return A pointer to the newly loaded effect definition
	**/
	ParticleEffectDef* LoadEffect(const std::string &filename);

	/** \brief Creates a new instance of an effect at (x,y)
	*** \param definition A pointer to the new effect to add
	*** \param x The x coordinate of where to add the effect
	*** \param y The y coordinate of where to add the effect
	*** \return The ID number corresponding to the added effect
	**/
	ParticleEffectID AddEffect(const ParticleEffectDef* definition, float x, float y);

	/** \brief Updates all active particle effects
	*** \param frame_time The number of milliseconds to update the effects by
	*** \return True if all effects were updated successfully
	**/
	bool Update(int32 frame_time);

	/** \brief Draws all active particleeffects
	*** \return True if all effects were drawn successfully
	**/
	bool Draw();

	/** \brief Stops all active particle effects from emitting more particles
	*** \param kill_immediately If true, the effects are all killed as well (default value = false).
	***
	*** \note Particle effects which have stopped emitting particles will eventually be deleted after
	*** all particles that were emitted expire.
	**/
	void StopAll(bool kill_immediately = false);

	//! \brief Returns the total number of particles among all active effects
	int32 GetNumParticles()
		{ return _num_particles; }

	/** \brief Retrieves the particle effect object that corresponds to an effect ID
	*** \param id The ID of the effect to retrieve
	*** \return A pointer to the desired effect, or nullptr if no effect was found with the specified ID
	***
	*** \note The pointers that this function returns are valid only up until the next call to Update(),
	*** as they may be deleted at any time. Thus, you should never store pointers returned by this method.
	*** Use them only for the current frame and call GetEffect if you need to use the object again during
	*** another frame.
	**/
	ParticleEffect *GetEffect(ParticleEffectID id);

	//! \brief Destroys the particle manager and all effects that it manages
	void Destroy();

private:
	//! The next time we create an effect, its id will be _current_id
	int32 _current_id;

	//! Total number of particles among all the active effects. This is updated
	//! during each call to Update(), so that when GetNumParticles() is called,
	//! we can just return this value instead of having to calculate it
	int32 _num_particles;

	//! All the effects currently being managed. An std::map is used so that
	//! we can convert easily between an id and a pointer
	std::map<ParticleEffectID, ParticleEffect*> _effects;

	/** \brief Creates a new particle effect from a provided effect definition
	*** \param definition A pointer to the definition data of the effect
	*** \return A pointer to the created ParticleEffect object
	**/
	ParticleEffect* _CreateEffect(const ParticleEffectDef *definition);

	/** \brief A helper function that is used to read a table of color data (four floats)
	*** \param script A reference to the script to read the data from
	*** \param parameter_name The name of the parameter containing the float data to read
	*** \return The Color object created by reading the color data
	***
	*** \note The function will check that the parameter referred to a table of four floating point
	*** values (holding the color data). If the parameter refers to any other data type, a warning
	*** is issued and the function returns an uninitialized Color.
	**/
	Color _ReadColor(hoa_script::ReadScriptDescriptor& script, std::string parameter_name);
}; // class ParticleManager

} // namespace private_video

} // namespace hoa_video
