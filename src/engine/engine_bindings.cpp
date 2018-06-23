///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    engine_bindings.cpp
*** \author  Daniel Steuernol (Steu)
*** \brief   Lua bindings for Allacrost engine code
***
*** All bindings for the engine code is contained within this file.
*** Therefore, everything that you see bound within this file will be made
*** available in Lua.
***
*** \note To most C++ programmers, the syntax of the binding code found in this
*** file may be very unfamiliar and obtuse. Refer to the Luabind documentation
*** as necessary to gain an understanding of this code style.
*** **************************************************************************/

#include "defs.h"

#include "audio.h"
#include "input.h"
#include "mode_manager.h"
#include "notification.h"
#include "script.h"
#include "system.h"
#include "video.h"

#include "global.h"

using namespace luabind;

namespace hoa_defs {

void BindEngineCode() {
	// ----- Audio Engine Bindings
	{
	using namespace hoa_audio;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_audio")
	[
		class_<AudioEngine>("AudioEngine")
			.def("PlaySound", &AudioEngine::PlaySound)

			// Namespace constants
			.enum_("constants") [
				// Map states
				value("AUDIO_STATE_UNLOADED", AUDIO_STATE_UNLOADED),
				value("AUDIO_STATE_STOPPED", AUDIO_STATE_STOPPED),
				value("AUDIO_STATE_PLAYING", AUDIO_STATE_PLAYING),
				value("AUDIO_STATE_PAUSED", AUDIO_STATE_PAUSED),
				value("AUDIO_LOAD_STATIC", AUDIO_LOAD_STATIC),
				value("AUDIO_LOAD_STREAM_FILE", AUDIO_LOAD_STREAM_FILE),
				value("AUDIO_LOAD_STREAM_MEMORY", AUDIO_LOAD_STREAM_MEMORY),
				value("NUMBER_STREAMING_BUFFERS", private_audio::NUMBER_STREAMING_BUFFERS)
			],

		class_<AudioDescriptor>("AudioDescriptor")
			.def("LoadAudio", (bool(AudioDescriptor::*)(const std::string&))&AudioDescriptor::LoadAudio)
			.def("FreeAudio", &AudioDescriptor::FreeAudio)
			.def("GetState", &AudioDescriptor::GetState)
			.def("IsPlaying", &AudioDescriptor::IsPlaying)
			.def("IsStopped", &AudioDescriptor::IsStopped)
			.def("IsPaused", &AudioDescriptor::IsPaused)
			.def("Play", &AudioDescriptor::Play)
			.def("Stop", &AudioDescriptor::Stop)
			.def("Pause", &AudioDescriptor::Pause)
			.def("Resume", &AudioDescriptor::Resume)
			.def("Rewind", &AudioDescriptor::Rewind)
			.def("IsLooping", &AudioDescriptor::IsLooping)
			.def("SetLooping", &AudioDescriptor::SetLooping)
			.def("SetLoopStart", &AudioDescriptor::SetLoopStart)
			.def("SetLoopEnd", &AudioDescriptor::SetLoopEnd)
			.def("SeekSample", &AudioDescriptor::SeekSample)
			.def("SeekSecond", &AudioDescriptor::SeekSecond)
			.def("GetVolume", &AudioDescriptor::GetVolume)
			.def("SetVolume", &AudioDescriptor::SetVolume)
			.def("SetPosition", &AudioDescriptor::SetPosition)
			.def("SetVelocity", &AudioDescriptor::SetVelocity)
			.def("SetDirection", &AudioDescriptor::SetDirection)
			.def("GetPosition", &AudioDescriptor::GetPosition)
			.def("GetVelocity", &AudioDescriptor::GetVelocity)
			.def("GetDirection", &AudioDescriptor::GetDirection),

		class_<SoundDescriptor, AudioDescriptor>("SoundDescriptor")
			.def(constructor<>()),

		class_<MusicDescriptor, AudioDescriptor>("MusicDescriptor")
			.def(constructor<>())
	];

	} // End using audio namespaces



	// ----- Input Engine Bindings
	{
	using namespace hoa_input;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_input")
	[
		class_<InputEngine>("InputEngine")
	];

	} // End using input namespaces



	// ----- Mode Manager Engine Bindings
	{
	using namespace hoa_mode_manager;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_mode_manager")
	[
		class_<GameMode>("GameMode")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_mode_manager")
	[
		class_<ModeEngine>("ModeEngine")
			.def("Push", &ModeEngine::Push, adopt(_2))
			.def("Pop", &ModeEngine::Pop)
			.def("PopAll", &ModeEngine::PopAll)
			.def("GetTop", &ModeEngine::GetTop)
			.def("GetMode", &ModeEngine::GetMode)
			.def("GetModeType", (GAME_MODE_TYPE (ModeEngine::*)(uint32))&ModeEngine::GetModeType)
			.def("GetModeType", (GAME_MODE_TYPE (ModeEngine::*)())&ModeEngine::GetModeType)
	];

	} // End using mode manager namespaces



	// ----- Notification Engine Bindings
	{
	using namespace hoa_notification;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_notification")
	[
		class_<NotificationEvent>("NotificationEvent")
			.def(constructor<std::string, std::string>())
			.def_readonly("category", &NotificationEvent::category)
			.def_readonly("event", &NotificationEvent::event),

		class_<NotificationEngine>("NotificationEngine")
			.def("Notify", &NotificationEngine::Notify, adopt(_2))
			.def("CreateAndNotify", &NotificationEngine::CreateAndNotify)
			.def("GetNotificationCount", &NotificationEngine::GetNotificationCount)
			.def("GetNotificationEvent", &NotificationEngine::GetNotificationEvent)
			.def("DEBUG_PrintNotificationEvents", &NotificationEngine::DEBUG_PrintNotificationEvents)
	];

	} // End using script namespaces



	// ----- Script Engine Bindings
	{
	using namespace hoa_script;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_script")
	[
		class_<ScriptEngine>("ScriptEngine")
	];

	} // End using script namespaces



	// ----- System Engine Bindings
	{
	using namespace hoa_system;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_system")
	[
		def("Translate", &hoa_system::Translate),

		class_<SystemTimer>("SystemTimer")
			.def(constructor<>())
			.def(constructor<uint32, int32>())
			.def("Initialize", &SystemTimer::Initialize)
			.def("EnableAutoUpdate", &SystemTimer::EnableAutoUpdate)
			.def("EnableManualUpdate", &SystemTimer::EnableManualUpdate)
			.def("Update", (void(SystemTimer::*)(void)) &SystemTimer::Update)
			.def("Update", (void(SystemTimer::*)(uint32)) &SystemTimer::Update)
			.def("Reset", &SystemTimer::Reset)
			.def("Run", &SystemTimer::Run)
			.def("Pause", &SystemTimer::Pause)
			.def("Finish", &SystemTimer::Finish)
			.def("IsInitial", &SystemTimer::IsInitial)
			.def("IsRunning", &SystemTimer::IsRunning)
			.def("IsPaused", &SystemTimer::IsPaused)
			.def("IsFinished", &SystemTimer::IsFinished)
			.def("CurrentLoop", &SystemTimer::CurrentLoop)
			.def("TimeLeft", &SystemTimer::TimeLeft)
			.def("PercentComplete", &SystemTimer::PercentComplete)
			.def("SetDuration", &SystemTimer::SetDuration)
			.def("SetNumberLoops", &SystemTimer::SetNumberLoops)
			.def("SetModeOwner", &SystemTimer::SetModeOwner)
			.def("GetState", &SystemTimer::GetState)
			.def("GetDuration", &SystemTimer::GetDuration)
			.def("GetNumberLoops", &SystemTimer::GetNumberLoops)
			.def("IsAutoUpdate", &SystemTimer::IsAutoUpdate)
			.def("GetModeOwner", &SystemTimer::GetModeOwner)
			.def("GetTimeExpired", &SystemTimer::GetTimeExpired)
			.def("GetTimesCompleted", &SystemTimer::GetTimesCompleted),

		class_<SystemEngine>("SystemEngine")
			.def("GetUpdateTime", &SystemEngine::GetUpdateTime)
			.def("SetPlayTime", &SystemEngine::SetPlayTime)
			.def("GetPlayHours", &SystemEngine::GetPlayHours)
			.def("GetPlayMinutes", &SystemEngine::GetPlayMinutes)
			.def("GetPlaySeconds", &SystemEngine::GetPlaySeconds)
			.def("GetLanguage", &SystemEngine::GetLanguage)
			.def("SetLanguage", &SystemEngine::SetLanguage)
			.def("NotDone", &SystemEngine::NotDone)
			.def("ExitGame", &SystemEngine::ExitGame)
	];

	} // End using system namespaces



	// ----- Video Engine Bindings
	{
	using namespace hoa_video;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_video")
	[
		class_<Color>("Color")
			.def(constructor<float, float, float, float>()),

		class_<CoordSys>("CoordSys")
			.def(constructor<>())
			.def(constructor<float, float, float, float>())
			.def_readonly("_left", &CoordSys::_left)
			.def_readonly("_right", &CoordSys::_right)
			.def_readonly("_bottom", &CoordSys::_bottom)
			.def_readonly("_top", &CoordSys::_top)
			.def_readonly("_vertical_direction", &CoordSys::_vertical_direction)
			.def_readonly("_horizontal_direction", &CoordSys::_horizontal_direction),

		class_<ImageDescriptor>("ImageDescriptor")
			.def("Clear", &ImageDescriptor::Clear)
			.def("Draw", (void(ImageDescriptor::*)()const)&ImageDescriptor::Draw)
			.def("Draw", (void(ImageDescriptor::*)(const Color&)const)&ImageDescriptor::Draw)
			.def("GetWidth", &ImageDescriptor::GetWidth)
			.def("GetHeight", &ImageDescriptor::GetHeight)
			.def("IsGrayScale", &ImageDescriptor::IsGrayScale)
			.def("EnableGrayScale", &ImageDescriptor::EnableGrayScale)
			.def("DisableGrayScale", &ImageDescriptor::DisableGrayScale)
			.def("SetStatic", &ImageDescriptor::SetStatic)
			.def("SetWidth", &ImageDescriptor::SetWidth)
			.def("SetHeight", &ImageDescriptor::SetHeight)
			.def("SetDimensions", &ImageDescriptor::SetDimensions)
			.def("SetUVCoordinates", &ImageDescriptor::SetUVCoordinates)
			.def("SetColor", &ImageDescriptor::SetColor)
			.def("SetVertexColors", &ImageDescriptor::SetVertexColors)
			.def("DEBUG_PrintInfo", &ImageDescriptor::DEBUG_PrintInfo),

		class_<StillImage, ImageDescriptor>("StillImage")
			.def(constructor<bool>())
			.def("Load", (bool(StillImage::*)(const std::string&))&StillImage::Load)
			.def("Load", (bool(StillImage::*)(const std::string&, float, float))&StillImage::Load)
			.def("Save", &StillImage::Save)
			.def("GetFilename", &StillImage::GetFilename)
			.def("SetWidthKeepRatio", &StillImage::SetWidthKeepRatio)
			.def("SetHeightKeepRatio", &StillImage::SetHeightKeepRatio),

		class_<AnimatedImage, ImageDescriptor>("AnimatedImage")
			.def(constructor<bool>())
			.def("Save", &AnimatedImage::Save)
			.def("ResetAnimation", &AnimatedImage::ResetAnimation)
			.def("Update", (void(AnimatedImage::*)(uint32))&AnimatedImage::Update)
			.def("Update", (void(AnimatedImage::*)(uint32))&AnimatedImage::Update)
			.def("AddFrame", (bool(AnimatedImage::*)(const std::string&, uint32))&AnimatedImage::AddFrame)
			.def("AddFrame", (bool(AnimatedImage::*)(const StillImage&, uint32))&AnimatedImage::AddFrame)
			.def("RandomizeCurrentLoopProgress", &AnimatedImage::RandomizeCurrentLoopProgress)
			.def("GetNumberOfFrames", &AnimatedImage::GetNumberOfFrames)
			.def("GetCurrentFrame", &AnimatedImage::GetCurrentFrame)
			.def("GetCurrentFrameIndex", &AnimatedImage::GetCurrentFrameIndex)
			.def("GetAnimationLength", &AnimatedImage::GetAnimationLength)
			.def("GetFrame", &AnimatedImage::GetFrame)
			.def("GetTimeProgress", &AnimatedImage::GetTimeProgress)
			.def("GetPercentProgress", &AnimatedImage::GetPercentProgress)
			.def("IsLoopsFinished", &AnimatedImage::IsLoopsFinished)
			.def("SetWidthKeepRatio", &AnimatedImage::SetWidthKeepRatio)
			.def("SetHeightKeepRatio", &AnimatedImage::SetHeightKeepRatio)
			.def("SetFrameIndex", &AnimatedImage::SetFrameIndex)
			.def("SetTimeProgress", &AnimatedImage::SetTimeProgress)
			.def("SetNumberLoops", &AnimatedImage::SetNumberLoops)
			.def("SetLoopCounter", &AnimatedImage::SetLoopCounter)
			.def("SetLoopsFinished", &AnimatedImage::SetLoopsFinished),

		class_<TextStyle>("TextStyle")
			.def(constructor<>())
			.def(constructor<std::string>())
			.def(constructor<Color>())
			.def(constructor<TEXT_SHADOW_STYLE>())
			.def(constructor<std::string, Color>())
			.def(constructor<std::string, TEXT_SHADOW_STYLE>())
			.def(constructor<Color, TEXT_SHADOW_STYLE>())
			.def(constructor<std::string, Color, TEXT_SHADOW_STYLE>())
			.def(constructor<std::string, Color, TEXT_SHADOW_STYLE, int32, int32>())
			.def_readwrite("font", &TextStyle::font)
			.def_readwrite("color", &TextStyle::color)
			.def_readwrite("shadow_style", &TextStyle::shadow_style)
			.def_readwrite("shadow_offset_x", &TextStyle::shadow_offset_x)
			.def_readwrite("shadow_offset_y", &TextStyle::shadow_offset_y),

		class_<VideoEngine>("VideoEngine")
			.def("SetDrawFlag", &VideoEngine::SetDrawFlag)
			.def("Clear", (void(VideoEngine::*)())&VideoEngine::Clear)
			.def("Clear", (void(VideoEngine::*)(const Color&))&VideoEngine::Clear)
			.def("Display", &VideoEngine::Display)
			.def("GetCoordSys", &VideoEngine::GetCoordSys)
			.def("SetCoordSys", (void(VideoEngine::*)(float, float, float, float))&VideoEngine::SetCoordSys)
			.def("SetCoordSys", (void(VideoEngine::*)(const CoordSys&))&VideoEngine::SetCoordSys)
			.def("SetStandardCoordSys", &VideoEngine::SetStandardCoordSys)
			.def("Move", &VideoEngine::Move)
			.def("MoveRelative", &VideoEngine::MoveRelative)
			.def("FadeScreen", &VideoEngine::FadeScreen)
			.def("IsFading", &VideoEngine::IsFading)
			.def("ShakeScreen", &VideoEngine::ShakeScreen)
			.def("StopShaking", &VideoEngine::StopShaking)
			.def("EnableLightOverlay", &VideoEngine::EnableLightOverlay)
			.def("DisableLightOverlay", &VideoEngine::DisableLightOverlay)
			.def("EnableAmbientOverlay", &VideoEngine::EnableAmbientOverlay)
			.def("DisableAmbientOverlay", &VideoEngine::DisableAmbientOverlay)
			.def("LoadLightningEffect", &VideoEngine::LoadLightningEffect)
			.def("EnableLightning", &VideoEngine::EnableLightning)
			.def("DisableLightning", &VideoEngine::DisableLightning)
			.def("DrawOverlays", &VideoEngine::DrawOverlays)
			.def("AddParticleEffect", &VideoEngine::AddParticleEffect)
			.def("StopAllParticleEffects", &VideoEngine::StopAllParticleEffects)

			// Namespace constants
			.enum_("constants") [
				// Draw flags
				value("VIDEO_X_LEFT", VIDEO_X_LEFT),
				value("VIDEO_X_CENTER", VIDEO_X_CENTER),
				value("VIDEO_X_RIGHT", VIDEO_X_RIGHT),
				value("VIDEO_Y_TOP", VIDEO_Y_TOP),
				value("VIDEO_Y_CENTER", VIDEO_Y_CENTER),
				value("VIDEO_Y_BOTTOM", VIDEO_Y_BOTTOM),
				value("VIDEO_X_FLIP", VIDEO_X_FLIP),
				value("VIDEO_X_NOFLIP", VIDEO_X_NOFLIP),
				value("VIDEO_Y_FLIP", VIDEO_Y_FLIP),
				value("VIDEO_Y_NOFLIP", VIDEO_Y_NOFLIP),
				value("VIDEO_NO_BLEND", VIDEO_NO_BLEND),
				value("VIDEO_BLEND", VIDEO_BLEND),
				value("VIDEO_BLEND_ADD", VIDEO_BLEND_ADD),
				// Text shadow types
				value("VIDEO_TEXT_SHADOW_NONE", VIDEO_TEXT_SHADOW_NONE),
				value("VIDEO_TEXT_SHADOW_DARK", VIDEO_TEXT_SHADOW_DARK),
				value("VIDEO_TEXT_SHADOW_LIGHT", VIDEO_TEXT_SHADOW_LIGHT),
				value("VIDEO_TEXT_SHADOW_BLACK", VIDEO_TEXT_SHADOW_BLACK),
				value("VIDEO_TEXT_SHADOW_COLOR", VIDEO_TEXT_SHADOW_COLOR),
				value("VIDEO_TEXT_SHADOW_INVCOLOR", VIDEO_TEXT_SHADOW_INVCOLOR),
				// Screen shake fall off types
				value("VIDEO_FALLOFF_NONE", VIDEO_FALLOFF_NONE),
				value("VIDEO_FALLOFF_EASE", VIDEO_FALLOFF_EASE),
				value("VIDEO_FALLOFF_LINEAR", VIDEO_FALLOFF_LINEAR),
				value("VIDEO_FALLOFF_GRADUAL", VIDEO_FALLOFF_GRADUAL),
				value("VIDEO_FALLOFF_SUDDEN", VIDEO_FALLOFF_SUDDEN)
			]
	];

	} // End using video namespaces

	// ---------- Bind engine class objects
	luabind::object global_table = luabind::globals(hoa_script::ScriptManager->GetGlobalState());
	global_table["AudioManager"]         = hoa_audio::AudioManager;
	global_table["InputManager"]         = hoa_input::InputManager;
	global_table["ModeManager"]          = hoa_mode_manager::ModeManager;
	global_table["ScriptManager"]        = hoa_script::ScriptManager;
	global_table["SystemManager"]        = hoa_system::SystemManager;
	global_table["NotificationManager"]  = hoa_notification::NotificationManager;
	global_table["VideoManager"]         = hoa_video::VideoManager;
} // void BindEngineCode()

} // namespace hoa_defs
