///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include "video.h"
#include "script.h"

#include "particle_manager.h"
#include "particle_effect.h"
#include "particle_system.h"
#include "particle_keyframe.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_script;

namespace hoa_video {

namespace private_video {

// -----------------------------------------------------------------------------
// ParticleManager class methods
// -----------------------------------------------------------------------------

ParticleEffectDef* ParticleManager::LoadEffect(const string& filename) {
	ReadScriptDescriptor script;

	if (script.OpenFile(filename) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to open the particle definition file: "
			<< filename << ", the particle effect was not loaded" << endl;
		return nullptr;
	}

	if (script.DoesTableExist("systems") == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "missing 'systems' table in particle definition file: "
			<< filename << endl;
		script.CloseFile();
		return nullptr;
	}

	script.OpenTable("systems");
	uint32 number_of_systems = script.GetTableSize();
	if (number_of_systems == 0) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "no particle systems were defined in the particle definition file: "
			<< filename << endl;
		script.CloseAllTables();
		script.CloseFile();
		return nullptr;
	}

	ParticleEffectDef* effect_definition = new ParticleEffectDef;

	// Read each particle system table
	for (uint32 system_number = 0; system_number < number_of_systems; ++system_number) {
		if (script.DoesTableExist(system_number) == false) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to read system table #" << system_number
				<< " in particle defintion file: " << filename << endl;
			delete effect_definition;
			script.CloseAllTables();
			script.CloseFile();
			return nullptr;
		}
		script.OpenTable(system_number);

		// Read the emitter table
		if (script.DoesTableExist("emitter") == false) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to read emitter table in system table #" << system_number
				<< " in particle defintion file: " << filename << endl;
			delete effect_definition;
			script.CloseAllTables();
			script.CloseFile();
			return nullptr;
		}
		script.OpenTable("emitter");

		ParticleSystemDef* system_definition = new ParticleSystemDef;
		// By putting the new system definition in the effect object, the effect object now takes responsibility
		// for delete the system object. Thus if we need to exit this function prematurely, we only need to delete
		// the effect definition and we do not need to delete the system definition.
		effect_definition->_systems.push_back(system_definition);

		system_definition->emitter._x = script.ReadFloat("x");
		system_definition->emitter._y = script.ReadFloat("y");
		system_definition->emitter._x2 = script.ReadFloat("x2");
		system_definition->emitter._y2 = script.ReadFloat("y2");
		system_definition->emitter._center_x = script.ReadFloat("center_x");
		system_definition->emitter._center_y = script.ReadFloat("center_y");
		system_definition->emitter._x_variation = script.ReadFloat("x_variation");
		system_definition->emitter._y_variation = script.ReadFloat("y_variation");
		system_definition->emitter._radius = script.ReadFloat("radius");

		string shape_string = script.ReadString("shape");
		if (shape_string == "point")
			system_definition->emitter._shape = EMITTER_SHAPE_POINT;
		else if (shape_string == "line")
			system_definition->emitter._shape = EMITTER_SHAPE_LINE;
		else if (shape_string == "circle outline")
			system_definition->emitter._shape = EMITTER_SHAPE_CIRCLE;
		else if (shape_string == "circle")
			system_definition->emitter._shape = EMITTER_SHAPE_FILLED_CIRCLE;
		else if (shape_string == "rectangle")
			system_definition->emitter._shape = EMITTER_SHAPE_FILLED_RECTANGLE;
		else {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "unknown emitter shape: " << shape_string
				<< ", when reading system table #" << system_number
				<< " in particle defintion file: " << filename << endl;
		}

		system_definition->emitter._omnidirectional = script.ReadBool("omnidirectional");
		system_definition->emitter._orientation = script.ReadFloat("orientation");
		system_definition->emitter._outer_cone = script.ReadFloat("outer_cone");
		system_definition->emitter._inner_cone = script.ReadFloat("inner_cone");
		system_definition->emitter._initial_speed = script.ReadFloat("initial_speed");
		system_definition->emitter._initial_speed_variation = script.ReadFloat("initial_speed_variation");
		system_definition->emitter._emission_rate = script.ReadFloat("emission_rate");
		system_definition->emitter._start_time = script.ReadFloat("start_time");

		string mode_string = script.ReadString("emitter_mode");
		if (mode_string == "looping")
			system_definition->emitter._emitter_mode = EMITTER_MODE_LOOPING;
		else if (mode_string == "one shot")
			system_definition->emitter._emitter_mode = EMITTER_MODE_ONE_SHOT;
		else if (mode_string == "burst")
			system_definition->emitter._emitter_mode = EMITTER_MODE_BURST;
		else if (mode_string == "always")
			system_definition->emitter._emitter_mode = EMITTER_MODE_ALWAYS;
		else {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "unknown emitter mode: " << mode_string
				<< ", when reading system table #" << system_number
				<< " in particle defintion file: " << filename << endl;
		}

		string spin_string = script.ReadString("spin");
		if (spin_string == "random")
			system_definition->emitter._spin = EMITTER_SPIN_RANDOM;
		else if (spin_string == "counterclockwise")
			system_definition->emitter._spin = EMITTER_SPIN_COUNTERCLOCKWISE;
		else if (spin_string == "clockwise")
			system_definition->emitter._spin = EMITTER_SPIN_CLOCKWISE;
		else {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "unknown emitter spin: " << spin_string
				<< ", when reading system table #" << system_number
				<< " in particle defintion file: " << filename << endl;
		}

		script.CloseTable(); // close the emitter table

		// Read the keyframes table
		if (script.DoesTableExist("keyframes") == false) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to read keyframes table in system table #" << system_number
				<< " in particle defintion file: " << filename << endl;
			delete effect_definition;
			script.CloseAllTables();
			script.CloseFile();
			return nullptr;
		}
		script.OpenTable("keyframes");

		uint32 number_of_keyframes = script.GetTableSize();
		system_definition->keyframes.resize(number_of_keyframes);

		// Read each keyframe table
		for (uint32 i = 0; i < number_of_keyframes; ++i) {
			system_definition->keyframes[i] = new ParticleKeyframe;

			// Keyframe tables are unnamed in the definition file. Unnammed Lua tables begin at index 1, not 0.
			script.OpenTable(i + 1);

			system_definition->keyframes[i]->size_x = script.ReadFloat("size_x");
			system_definition->keyframes[i]->size_y = script.ReadFloat("size_y");
			system_definition->keyframes[i]->color = _ReadColor(script, "color");
			system_definition->keyframes[i]->rotation_speed = script.ReadFloat("rotation_speed");
			system_definition->keyframes[i]->size_variation_x = script.ReadFloat("size_variation_x");
			system_definition->keyframes[i]->size_variation_y = script.ReadFloat("size_variation_y");
			system_definition->keyframes[i]->color_variation = _ReadColor(script, "color_variation");
			system_definition->keyframes[i]->rotation_speed_variation = script.ReadFloat("rotation_speed_variation");
			system_definition->keyframes[i]->time = script.ReadFloat("time");

			script.CloseTable();
		}
		script.CloseTable(); // close the keyframes table

		// Read the animation frames and times
		script.ReadStringVector("animation_frames", system_definition->animation_frame_filenames);
		// At least one animation frame must be present
		if (system_definition->animation_frame_filenames.empty() == true) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to read animation frames in system table #" << system_number
				<< " in particle defintion file: " << filename << endl;
			delete effect_definition;
			script.CloseAllTables();
			script.CloseFile();
			return nullptr;
		}

		script.ReadIntVector("animation_frame_times", system_definition->animation_frame_times);
		// Make sure that the frames and times tables are of equal size
		if (system_definition->animation_frame_times.size() != system_definition->animation_frame_filenames.size()) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "animation_frames and animation_frame_times tables were of unequal size"
					<< " in system table #" << system_number << " in particle defintion file: " << filename << endl;
			delete effect_definition;
			script.CloseAllTables();
			script.CloseFile();
			return nullptr;
		}

		// Test that each animation frame file exists
		for (uint32 i = 0; i < system_definition->animation_frame_filenames.size(); i++) {
			if (DoesFileExist(system_definition->animation_frame_filenames[i]) == false) {
				IF_PRINT_WARNING(VIDEO_DEBUG) << "animation frame file did not exist: " << system_definition->animation_frame_filenames[i]
					<< ", in system table #" << system_number << " in particle defintion file: " << filename << endl;
				delete effect_definition;
				script.CloseAllTables();
				script.CloseFile();
				return nullptr;
			}
		}

		// Read the remaining particle system data
		system_definition->enabled = script.ReadBool("enabled");
		system_definition->blend_mode = script.ReadInt("blend_mode");
		system_definition->system_lifetime = script.ReadFloat("system_lifetime");

		system_definition->particle_lifetime = script.ReadFloat("particle_lifetime");
		system_definition->particle_lifetime_variation = script.ReadFloat("particle_lifetime_variation");
		system_definition->max_particles = script.ReadInt("max_particles");

		system_definition->damping = script.ReadFloat("damping");
		system_definition->damping_variation = script.ReadFloat("damping_variation");

		system_definition->acceleration_x = script.ReadFloat("acceleration_x");
		system_definition->acceleration_y = script.ReadFloat("acceleration_y");
		system_definition->acceleration_variation_x = script.ReadFloat("acceleration_variation_x");
		system_definition->acceleration_variation_y = script.ReadFloat("acceleration_variation_y");

		system_definition->wind_velocity_x = script.ReadFloat("wind_velocity_x");
		system_definition->wind_velocity_y = script.ReadFloat("wind_velocity_y");
		system_definition->wind_velocity_variation_x = script.ReadFloat("wind_velocity_variation_x");
		system_definition->wind_velocity_variation_y = script.ReadFloat("wind_velocity_variation_y");

		system_definition->wave_motion_used = script.ReadBool("wave_motion_used");
		system_definition->wave_length = script.ReadFloat("wave_length");
		system_definition->wave_length_variation = script.ReadFloat("wave_length_variation");
		system_definition->wave_amplitude = script.ReadFloat("wave_amplitude");
		system_definition->wave_amplitude_variation = script.ReadFloat("wave_amplitude_variation");

		system_definition->tangential_acceleration = script.ReadFloat("tangential_acceleration");
		system_definition->tangential_acceleration_variation = script.ReadFloat("tangential_acceleration_variation");

		system_definition->radial_acceleration = script.ReadFloat("radial_acceleration");
		system_definition->radial_acceleration_variation = script.ReadFloat("radial_acceleration_variation");

		system_definition->user_defined_attractor = script.ReadBool("user_defined_attractor");
		system_definition->attractor_falloff = script.ReadFloat("attractor_falloff");

		system_definition->rotation_used = script.ReadBool("rotation_used");
		system_definition->rotate_to_velocity = script.ReadBool("rotate_to_velocity");

		system_definition->speed_scale_used = script.ReadBool("speed_scale_used");
		system_definition->speed_scale = script.ReadFloat("speed_scale");
		system_definition->min_speed_scale = script.ReadFloat("min_speed_scale");
		system_definition->max_speed_scale = script.ReadFloat("max_speed_scale");

		system_definition->smooth_animation = script.ReadBool("smooth_animation");
		system_definition->modify_stencil = script.ReadBool("modify_stencil");

		string stencil_string = script.ReadString("stencil_op");
		if (stencil_string == "incr")
			system_definition->stencil_op = VIDEO_STENCIL_OP_INCREASE;
		else if (stencil_string == "decr")
			system_definition->stencil_op = VIDEO_STENCIL_OP_DECREASE;
		else if (stencil_string == "zero")
			system_definition->stencil_op = VIDEO_STENCIL_OP_ZERO;
		else if (stencil_string == "one")
			system_definition->stencil_op = VIDEO_STENCIL_OP_ONE;
		else {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "unknown stencil_op: " << stencil_string
				<< ", when reading system table #" << system_number
				<< " in particle defintion file: " << filename << endl;
		}

		system_definition->use_stencil = script.ReadBool("use_stencil");
		system_definition->random_initial_angle = script.ReadBool("random_initial_angle");

		script.CloseTable(); // close the system_number table
	} // for (uint32 system_number = 0; system_number < number_of_systems; ++system_number)
	script.CloseFile(); // close the systems table

	return effect_definition;
} // ParticleEffectDef* ParticleManager::LoadEffect(const string& filename)



ParticleEffectID ParticleManager::AddEffect(const ParticleEffectDef* definition, float x, float y) {
	if (definition == nullptr) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to add effect because function received a nullptr argument" << endl;
		return VIDEO_INVALID_EFFECT;
	}

	if (definition->_systems.empty() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to add effect because particle definition contained no systems" << endl;
		return VIDEO_INVALID_EFFECT;
	}

	ParticleEffect* effect = _CreateEffect(definition);
	if (effect == nullptr) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to add effect because the effect failed to create from the particle definition" << endl;
		return VIDEO_INVALID_EFFECT;
	}

	effect->Move(x, y);
	_effects[_current_id] = effect;
	++_current_id;

	// Take care to return the id of the created particle, not the id of the next particle to create
	return (_current_id - 1);
}



bool ParticleManager::Update(int32 frame_time) {
	float frame_time_seconds = static_cast<float>(frame_time) / 1000.0f;
	bool success = true;
	_num_particles = 0;

	for (map<ParticleEffectID, ParticleEffect*>::iterator i = _effects.begin(); i != _effects.end();) {
		// Remove any particle effects that have completed their life cycle
		if ((i->second)->IsAlive() == false) {
			map<ParticleEffectID, ParticleEffect*>::iterator finished_effect = i;
			++i;
			_effects.erase(finished_effect);
		}
		else {
			if ((i->second)->_Update(frame_time_seconds) == false) {
				success = false;
				IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to update particle effect with ID #" << i->first << endl;
			}

			_num_particles += i->second->GetNumParticles();
			++i;
		}
	}

	return success;
}



bool ParticleManager::Draw() {
	VideoManager->PushState();
	// NOTE: the particle manager is using inverted y coordinates compared to how most of the rest of the code aligns the y axis
	VideoManager->SetCoordSys(CoordSys(0.0f, 1024.0f, 768.0f, 0.0f));
	VideoManager->DisableScissoring();

	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);

	bool success = true;

	for (map<ParticleEffectID, ParticleEffect*>::iterator i = _effects.begin(); i != _effects.end(); ++i) {
		if ((i->second)->_Draw() == false) {
			success = false;
			IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to draw particle effect with ID #" << i->first << endl;
		}
	}

	VideoManager->PopState();
	return success;
}



void ParticleManager::StopAll(bool kill_immediately) {
	for (map<ParticleEffectID, ParticleEffect*>::iterator i = _effects.begin(); i != _effects.end(); ++i) {
		(i->second)->Stop(kill_immediately);
	}
}



ParticleEffect* ParticleManager::GetEffect(ParticleEffectID id) {
	map<ParticleEffectID, ParticleEffect*>::iterator i = _effects.find(id);
	if (i == _effects.end())
		return nullptr;
	else
		return (i->second);
}



void ParticleManager::Destroy() {
	for (map<ParticleEffectID, ParticleEffect*>::iterator i = _effects.begin(); i != _effects.end(); ++i) {
		(i->second)->_Destroy();
		delete (i->second);
	}

	_effects.clear();
}



ParticleEffect *ParticleManager::_CreateEffect(const ParticleEffectDef *definition) {
	if (definition == nullptr) {
		return nullptr;
	}

	if (definition->_systems.empty() == true) {
		return nullptr;
	}

	ParticleEffect* effect = new ParticleEffect;
	effect->_effect_def = definition;

	for (list<ParticleSystemDef*>::const_iterator i = definition->_systems.begin(); i != definition->_systems.end(); ++i) {
		if ((*i)->enabled == false) {
			continue;
		}

		ParticleSystem* system = new ParticleSystem;
		// If any systems fail to create, delete all allocated resources and bail
		if (system->Create(*i) == false) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to create particle system for effect. The effect was not created." << endl;
			system->Destroy();
			delete system;

			// Delete all previously loaded systems
			for (list<ParticleSystem*>::const_iterator j = effect->_systems.begin(); j != effect->_systems.end(); ++j) {
				(*j)->Destroy();
				delete (*j);
			}

			delete effect;
			return nullptr;
		}

		effect->_systems.push_back(system);
	}

	effect->_alive = true;
	effect->_age = 0.0f;
	return effect;
}



Color ParticleManager::_ReadColor(ReadScriptDescriptor& script, string parameter_name) {
	vector<float> color_values;

	script.ReadFloatVector(parameter_name, color_values);
	if (color_values.size() < 4) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "invalid read operation: failed to read parameter " << parameter_name
			<< " from script file " << script.GetFilename() << parameter_name << endl;
		return Color();
	}

	return Color(color_values[0], color_values[1], color_values[2], color_values[3]);
}

} // namespace private_video

} // namespace hoa_video
