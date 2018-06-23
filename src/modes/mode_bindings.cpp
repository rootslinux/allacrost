///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    mode_bindings.cpp
*** \author  Daniel Steuernol (Steu)
*** \brief   Lua bindings for game mode code
***
*** All bindings for the game mode code is contained within this file.
*** Therefore, everything that you see bound within this file will be made
*** available in Lua.
***
*** \note To most C++ programmers, the syntax of the binding code found in this
*** file may be very unfamiliar and obtuse. Refer to the Luabind documentation
*** as necessary to gain an understanding of this code style.
*** **************************************************************************/

#include "defs.h"

// Notification engine is used for derived NotificationEvent classes
#include "notification.h"

// Common code headers
#include "dialogue.h"
#include "global_actors.h"
#include "global_effects.h"

#include "battle.h"
#include "battle_actors.h"
#include "battle_command.h"
#include "battle_dialogue.h"
#include "battle_effects.h"
#include "battle_utils.h"
#include "boot.h"
#include "custom.h"
#include "map.h"
#include "map_dialogue.h"
#include "map_events.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "map_sprite_events.h"
#include "map_tiles.h"
#include "map_transition.h"
#include "map_treasure.h"
#include "map_utils.h"
#include "map_zones.h"
#include "menu.h"
#include "shop.h"
#include "test.h"

using namespace luabind;

namespace hoa_defs {

void BindModeCode() {
	// ----- Battle Mode bindings
	{
	using namespace hoa_battle;
	using namespace hoa_battle::private_battle;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		def("CalculateStandardEvasion", (bool(*)(BattleTarget*)) &CalculateStandardEvasion),
		def("CalculateStandardEvasionAdder", (bool(*)(BattleTarget*, float)) &CalculateStandardEvasion),
		def("CalculateStandardEvasionMultiplier", (bool(*)(BattleTarget*, float)) &CalculateStandardEvasionMultiplier),
		def("CalculatePhysicalDamage", (uint32(*)(BattleActor*, BattleTarget*)) &CalculatePhysicalDamage),
		def("CalculatePhysicalDamage", (uint32(*)(BattleActor*, BattleTarget*, float)) &CalculatePhysicalDamage),
		def("CalculatePhysicalDamageAdder", (uint32(*)(BattleActor*, BattleTarget*, int32)) &CalculatePhysicalDamageAdder),
		def("CalculatePhysicalDamageAdder", (uint32(*)(BattleActor*, BattleTarget*, int32, float)) &CalculatePhysicalDamageAdder),
		def("CalculatePhysicalDamageMultiplier", (uint32(*)(BattleActor*, BattleTarget*, float)) &CalculatePhysicalDamageMultiplier),
		def("CalculatePhysicalDamageMultiplier", (uint32(*)(BattleActor*, BattleTarget*, float, float)) &CalculatePhysicalDamageMultiplier),
		def("CalculateEtherealDamage", (uint32(*)(BattleActor*, BattleTarget*)) &CalculateEtherealDamage),
		def("CalculateEtherealDamage", (uint32(*)(BattleActor*, BattleTarget*, float)) &CalculateEtherealDamage),
		def("CalculateEtherealDamageAdder", (uint32(*)(BattleActor*, BattleTarget*, int32)) &CalculateEtherealDamageAdder),
		def("CalculateEtherealDamageAdder", (uint32(*)(BattleActor*, BattleTarget*, int32, float)) &CalculateEtherealDamageAdder),
		def("CalculateEtherealDamageMultiplier", (uint32(*)(BattleActor*, BattleTarget*, float)) &CalculateEtherealDamageMultiplier),
		def("CalculateEtherealDamageMultiplier", (uint32(*)(BattleActor*, BattleTarget*, float, float)) &CalculateEtherealDamageMultiplier),

		class_<BattleMode, hoa_mode_manager::GameMode>("BattleMode")
			.def(constructor<>())
			.def("AddEnemy", (void(BattleMode::*)(uint32)) &BattleMode::AddEnemy)
			.def("LoadBattleScript", &BattleMode::LoadBattleScript)
			.def("RestartBattle", &BattleMode::RestartBattle)
			.def("FreezeTimers", &BattleMode::FreezeTimers)
			.def("UnFreezeTimers", &BattleMode::UnFreezeTimers)
			.def("GetState", &BattleMode::GetState)
			.def("ChangeState", &BattleMode::ChangeState)
			.def("OpenCommandMenu", &BattleMode::OpenCommandMenu)
			.def("IsBattleFinished", &BattleMode::IsBattleFinished)
			.def("SetPlayFinishMusic", &BattleMode::SetPlayFinishMusic)
			.def("GetNumberOfCharacters", &BattleMode::GetNumberOfCharacters)
			.def("GetNumberOfEnemies", &BattleMode::GetNumberOfEnemies)
			.def("GetMedia", &BattleMode::GetMedia)
			.def("GetDialogueSupervisor", &BattleMode::GetDialogueSupervisor)
			.def("GetCommandSupervisor", &BattleMode::GetCommandSupervisor)

			// Namespace constants
			.enum_("constants") [
				// Battle states
				value("BATTLE_STATE_INITIAL", BATTLE_STATE_INITIAL),
				value("BATTLE_STATE_NORMAL", BATTLE_STATE_NORMAL),
				value("BATTLE_STATE_COMMAND", BATTLE_STATE_COMMAND),
				value("BATTLE_STATE_EVENT", BATTLE_STATE_EVENT),
				value("BATTLE_STATE_VICTORY", BATTLE_STATE_VICTORY),
				value("BATTLE_STATE_DEFEAT", BATTLE_STATE_DEFEAT),
				value("BATTLE_STATE_EXITING", BATTLE_STATE_EXITING)
			],

		class_<BattleMedia>("BattleMedia")
			.def("SetBackgroundImage", &BattleMedia::SetBackgroundImage)
			.def("SetBattleMusic", &BattleMedia::SetBattleMusic),

		class_<BattleActor, hoa_global::GlobalActor>("BattleActor")
			.def("ChangeSpriteAnimation", &BattleActor::ChangeSpriteAnimation)
			.def("RegisterDamage", (void(BattleActor::*)(uint32)) &BattleActor::RegisterDamage)
			.def("RegisterHealing", &BattleActor::RegisterHealing)
			.def("RegisterMiss", &BattleActor::RegisterMiss)
			.def("RegisterStatusChange", &BattleActor::RegisterStatusChange)
			.def("ResetHitPoints", &BattleActor::ResetHitPoints)
			.def("ResetMaxHitPoints", &BattleActor::ResetMaxHitPoints)
			.def("ResetSkillPoints", &BattleActor::ResetSkillPoints)
			.def("ResetMaxSkillPoints", &BattleActor::ResetMaxSkillPoints)
			.def("ResetStrength", &BattleActor::ResetStrength)
			.def("ResetVigor", &BattleActor::ResetVigor)
			.def("ResetFortitude", &BattleActor::ResetFortitude)
			.def("ResetProtection", &BattleActor::ResetProtection)
			.def("ResetAgility", &BattleActor::ResetAgility)
			.def("ResetEvade", &BattleActor::ResetEvade)
			.def("SetStatePaused", &BattleActor::SetStatePaused),

		class_<BattleCharacter, BattleActor>("BattleCharacter")
			.def("ChangeSpriteAnimation", &BattleCharacter::ChangeSpriteAnimation),

		class_<BattleEnemy, BattleActor>("BattleEnemy")
			.def("ChangeSpriteAnimation", &BattleEnemy::ChangeSpriteAnimation),

		class_<CommandSupervisor>("CommandSupervisor"),

		class_<BattleDialogue, hoa_common::CommonDialogue>("BattleDialogue")
			.def(constructor<uint32>())
			.def("AddLine", (void(BattleDialogue::*)(std::string, uint32))&BattleDialogue::AddLine)
			.def("AddLine", (void(BattleDialogue::*)(std::string, uint32, int32))&BattleDialogue::AddLine)
			.def("AddLineTimed", (void(BattleDialogue::*)(std::string, uint32, uint32))&BattleDialogue::AddLineTimed)
			.def("AddLineTimed", (void(BattleDialogue::*)(std::string, uint32, int32, uint32))&BattleDialogue::AddLineTimed)
			.def("AddOption", (void(BattleDialogue::*)(std::string))&BattleDialogue::AddOption)
			.def("AddOption", (void(BattleDialogue::*)(std::string, int32))&BattleDialogue::AddOption)
			.def("Validate", &BattleDialogue::Validate)
			.def("SetHaltBattleAction", &BattleDialogue::SetHaltBattleAction),

		class_<DialogueSupervisor>("DialogueSupervisor")
			.def("AddDialogue", &DialogueSupervisor::AddDialogue, adopt(_2))
			.def("AddCharacterSpeaker", &DialogueSupervisor::AddCharacterSpeaker)
			.def("AddEnemySpeaker", &DialogueSupervisor::AddEnemySpeaker)
			.def("AddCustomSpeaker", &DialogueSupervisor::AddCustomSpeaker)
			.def("ChangeSpeakerName", &DialogueSupervisor::ChangeSpeakerName)
			.def("ChangeSpeakerPortrait", &DialogueSupervisor::ChangeSpeakerPortrait)
			.def("BeginDialogue", &DialogueSupervisor::BeginDialogue)
			.def("EndDialogue", &DialogueSupervisor::EndDialogue)
			.def("ForceNextLine", &DialogueSupervisor::ForceNextLine)
			.def("IsDialogueActive", &DialogueSupervisor::IsDialogueActive)
			.def("GetCurrentDialogue", &DialogueSupervisor::GetCurrentDialogue)
			.def("GetLineCounter", &DialogueSupervisor::GetLineCounter),

		class_<BattleTarget>("BattleTarget")
			.def("SetActorTarget", &BattleTarget::SetActorTarget)
			.def("SetPartyTarget", &BattleTarget::SetPartyTarget)
			.def("IsValid", &BattleTarget::IsValid)
			.def("SelectNextActor", &BattleTarget::SelectNextActor)
			.def("GetType", &BattleTarget::GetType)
			.def("GetActor", &BattleTarget::GetActor)
			.def("GetPartyActor", &BattleTarget::GetPartyActor),

		class_<BattleEffect>("BattleEffect")
			.def("GetEffectActor", &BattleEffect::GetEffectActor),

		class_<StatusEffect, BattleEffect>("StatusEffect")
			.def("GetDurationTimer", &StatusEffect::GetDurationTimer)
			.def("GetIntensity", &StatusEffect::GetIntensity)
			.def("IncrementIntensity", &StatusEffect::IncrementIntensity)
			.def("DecrementIntensity", &StatusEffect::DecrementIntensity)
			.def("SetIntensity", &StatusEffect::SetIntensity)
			.def("IsIntensityChanged", &StatusEffect::IsIntensityChanged)
	];

	} // End using battle mode namespaces

	// ----- Boot Mode bindings
	{
	using namespace hoa_boot;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_boot")
	[
		class_<BootMode, hoa_mode_manager::GameMode>("BootMode")
			.def(constructor<>())
	];

	} // End using boot mode namespaces

	// ----- Custom Mode bindings
	{
	using namespace hoa_custom;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_custom")
	[
		class_<CustomMode, hoa_mode_manager::GameMode>("CustomMode")
			.def(constructor<std::string>())
			.def("AddOption", &CustomMode::AddOption)
			.def("GetOption", &CustomMode::GetOption)
			.def_readonly("_load_complete", &CustomMode::_load_complete)

	];

	} // End using custom mode namespaces

	// ----- Map Mode Bindings
	{
	using namespace hoa_map;
	using namespace hoa_map::private_map;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapMode, hoa_mode_manager::GameMode>("MapMode")
			.def(constructor<const std::string&>())
			.def(constructor<const std::string&, int32>())
			.def_readonly("dialogue_supervisor", &MapMode::_dialogue_supervisor)
			.def_readonly("event_supervisor", &MapMode::_event_supervisor)
			.def_readonly("object_supervisor", &MapMode::_object_supervisor)
			.def_readonly("tile_supervisor", &MapMode::_tile_supervisor)
			.def_readonly("transition_supervisor", &MapMode::_transition_supervisor)
			.def_readonly("treasure_supervisor", &MapMode::_treasure_supervisor)
			.def_readonly("global_record_group", &MapMode::_global_record_group)
			.def_readonly("local_record_group", &MapMode::_local_record_group)

			.def_readonly("load_point", &MapMode::_load_point)
			.def_readonly("camera", &MapMode::_camera)
			.def_readonly("player_sprite", &MapMode::_player_sprite)
			.def_readonly("virtual_focus", &MapMode::_virtual_focus)
			.def_readwrite("unlimited_stamina", &MapMode::_unlimited_stamina)
			.def_readwrite("run_disabled", &MapMode::_run_disabled)
			.def_readwrite("run_stamina", &MapMode::_run_stamina)

			.def("PlayMusic", &MapMode::PlayMusic)
			.def("AddZone", &MapMode::AddZone, adopt(_2))
			.def("SetCamera", (void(MapMode::*)(private_map::VirtualSprite*))&MapMode::SetCamera)
			.def("SetCamera", (void(MapMode::*)(private_map::VirtualSprite*, uint32))&MapMode::SetCamera)
			.def("GetPlayerSprite", &MapMode::GetPlayerSprite)
			.def("SetPlayerSprite", &MapMode::SetPlayerSprite)
			.def("GetVirtualFocus", &MapMode::GetVirtualFocus)
			.def("MoveVirtualFocus", (void(MapMode::*)(uint16, uint16))&MapMode::MoveVirtualFocus)
			.def("MoveVirtualFocus", (void(MapMode::*)(uint16, uint16, uint32))&MapMode::MoveVirtualFocus)
			.def("IsCameraOnVirtualFocus", &MapMode::IsCameraOnVirtualFocus)
			.def("ClearLayerOrder", &MapMode::ClearLayerOrder)
			.def("AddTileLayerToOrder", &MapMode::AddTileLayerToOrder)
			.def("AddObjectLayerToOrder", &MapMode::AddObjectLayerToOrder)
			.def("IsDialogueIconsVisible", &MapMode::IsDialogueIconsVisible)
			.def("ShowDialogueIcons", &MapMode::ShowDialogueIcons)
			.def("IsStaminaBarHidden", &MapMode::IsStaminaBarVisible)
			.def("ShowStaminaBar", &MapMode::ShowStaminaBar)
			.def("DisableIntroductionVisuals", &MapMode::DisableIntroductionVisuals)
			.def("SetCurrentTrack", &MapMode::SetCurrentTrack)
			.def("CurrentState", &MapMode::CurrentState)
			.def("PushState", &MapMode::PushState)
			.def("PopState", &MapMode::PopState)
			.def("GetGlobalRecordGroup", &MapMode::GetGlobalRecordGroup)
			.def("GetLocalRecordGroup", &MapMode::GetLocalRecordGroup)
			.def("DrawMapLayers", &MapMode::_DrawMapLayers)

			// Namespace constants
			.enum_("constants") [
				// Map states
				value("STATE_EXPLORE", STATE_EXPLORE),
				value("STATE_SCENE", STATE_SCENE),
				value("STATE_DIALOGUE", STATE_DIALOGUE),
				value("STATE_TREASURE", STATE_TREASURE),
				value("STATE_TRANSITION", STATE_TRANSITION),
				// Map contexts
				value("CONTEXT_NONE", MAP_CONTEXT_NONE),
				value("CONTEXT_01", MAP_CONTEXT_01),
				value("CONTEXT_02", MAP_CONTEXT_02),
				value("CONTEXT_03", MAP_CONTEXT_03),
				value("CONTEXT_04", MAP_CONTEXT_04),
				value("CONTEXT_05", MAP_CONTEXT_05),
				value("CONTEXT_06", MAP_CONTEXT_06),
				value("CONTEXT_07", MAP_CONTEXT_07),
				value("CONTEXT_08", MAP_CONTEXT_08),
				value("CONTEXT_09", MAP_CONTEXT_09),
				value("CONTEXT_10", MAP_CONTEXT_10),
				value("CONTEXT_11", MAP_CONTEXT_11),
				value("CONTEXT_12", MAP_CONTEXT_12),
				value("CONTEXT_13", MAP_CONTEXT_13),
				value("CONTEXT_14", MAP_CONTEXT_14),
				value("CONTEXT_15", MAP_CONTEXT_15),
				value("CONTEXT_16", MAP_CONTEXT_16),
				value("CONTEXT_17", MAP_CONTEXT_17),
				value("CONTEXT_18", MAP_CONTEXT_18),
				value("CONTEXT_19", MAP_CONTEXT_19),
				value("CONTEXT_20", MAP_CONTEXT_20),
				value("CONTEXT_21", MAP_CONTEXT_21),
				value("CONTEXT_22", MAP_CONTEXT_22),
				value("CONTEXT_23", MAP_CONTEXT_23),
				value("CONTEXT_24", MAP_CONTEXT_24),
				value("CONTEXT_25", MAP_CONTEXT_25),
				value("CONTEXT_26", MAP_CONTEXT_26),
				value("CONTEXT_27", MAP_CONTEXT_27),
				value("CONTEXT_28", MAP_CONTEXT_28),
				value("CONTEXT_29", MAP_CONTEXT_29),
				value("CONTEXT_30", MAP_CONTEXT_30),
				value("CONTEXT_31", MAP_CONTEXT_31),
				value("CONTEXT_32", MAP_CONTEXT_32),
				value("CONTEXT_ALL", MAP_CONTEXT_ALL),
				// Object types
				value("PHYSICAL_TYPE", PHYSICAL_TYPE),
				value("VIRTUAL_TYPE", VIRTUAL_TYPE),
				value("SPRITE_TYPE", SPRITE_TYPE),
				// Sprite directions
				value("NORTH", NORTH),
				value("SOUTH", SOUTH),
				value("EAST", EAST),
				value("WEST", WEST),
				value("NW_NORTH", NW_NORTH),
				value("NW_WEST", NW_WEST),
				value("NE_NORTH", NE_NORTH),
				value("NE_EAST", NE_EAST),
				value("SW_SOUTH", SW_SOUTH),
				value("SW_WEST", SW_WEST),
				value("SE_SOUTH", SE_SOUTH),
				value("SE_EAST", SE_EAST),
				// Sprite animations
				value("ANIM_STANDING_SOUTH", ANIM_STANDING_SOUTH),
				value("ANIM_STANDING_NORTH", ANIM_STANDING_NORTH),
				value("ANIM_STANDING_WEST", ANIM_STANDING_WEST),
				value("ANIM_STANDING_EAST", ANIM_STANDING_EAST),
				value("ANIM_WALKING_SOUTH", ANIM_WALKING_SOUTH),
				value("ANIM_WALKING_NORTH", ANIM_WALKING_NORTH),
				value("ANIM_WALKING_WEST", ANIM_WALKING_WEST),
				value("ANIM_WALKING_EAST", ANIM_WALKING_EAST),
				value("ANIM_ATTACKING_EAST", ANIM_ATTACKING_EAST),
				// Sprite speeds
				value("VERY_SLOW_SPEED", static_cast<uint32>(VERY_SLOW_SPEED)),
				value("SLOW_SPEED", static_cast<uint32>(SLOW_SPEED)),
				value("NORMAL_SPEED", static_cast<uint32>(NORMAL_SPEED)),
				value("FAST_SPEED", static_cast<uint32>(FAST_SPEED)),
				value("VERY_FAST_SPEED", static_cast<uint32>(VERY_FAST_SPEED)),
				// Collision types
				value("NO_COLLISION", static_cast<uint32>(NO_COLLISION)),
				value("BOUNDARY_COLLISION", static_cast<uint32>(BOUNDARY_COLLISION)),
				value("GRID_COLLISION", static_cast<uint32>(GRID_COLLISION)),
				value("OBJECT_COLLISION", static_cast<uint32>(OBJECT_COLLISION))
			],

		class_<MapCollisionNotificationEvent, hoa_notification::NotificationEvent>("MapCollisionNotificationEvent")
			.def_readonly("collision_type", &MapCollisionNotificationEvent::collision_type)
			.def_readonly("sprite", &MapCollisionNotificationEvent::sprite)
			.def_readonly("x_position", &MapCollisionNotificationEvent::x_position)
			.def_readonly("x_offset", &MapCollisionNotificationEvent::x_offset)
			.def_readonly("y_position", &MapCollisionNotificationEvent::y_position)
			.def_readonly("y_offset", &MapCollisionNotificationEvent::y_offset)
			.def_readonly("object", &MapCollisionNotificationEvent::object),

		class_<CameraZoneNotificationEvent, hoa_notification::NotificationEvent>("CameraZoneNotificationEvent")
			.def_readonly("zone", &CameraZoneNotificationEvent::zone),

		class_<ObjectSupervisor>("ObjectSupervisor")
			.def("GenerateObjectID", &ObjectSupervisor::GenerateObjectID)
			.def("GetNumberObjects", &ObjectSupervisor::GetNumberObjects)
			.def("GetObjectByIndex", &ObjectSupervisor::GetObjectByIndex)
			.def("GetObject", &ObjectSupervisor::GetObject)
			.def("AddObjectLayer", &ObjectSupervisor::AddObjectLayer)
			.def("AddObject", (void(private_map::ObjectSupervisor::*)(private_map::MapObject*))&ObjectSupervisor::AddObject, adopt(_2))
			.def("AddObject", (void(private_map::ObjectSupervisor::*)(private_map::MapObject*, uint32))&ObjectSupervisor::AddObject, adopt(_2))
			.def("MoveObjectToLayer", &ObjectSupervisor::MoveObjectToLayer),

		class_<MapObject>("MapObject")
			.def_readwrite("updatable", &MapObject::updatable)
			.def_readwrite("visible", &MapObject::visible)
			.def_readwrite("collidable", &MapObject::collidable)
			// TEMP: because GetXPosition and GetYPostiion seem to give a runtime error in Lua
			.def_readonly("x_position", &MapObject::x_position)
			.def_readonly("y_position", &MapObject::y_position)
			.def("ModifyPosition", &MapObject::ModifyPosition)
			.def("ModifyXPosition", &MapObject::ModifyXPosition)
			.def("ModifyYPosition", &MapObject::ModifyYPosition)
			.def("MoveToObject", &MapObject::MoveToObject)
			.def("SetObjectID", &MapObject::SetObjectID)
			.def("SetContext", &MapObject::SetContext)
			.def("SetPosition", (void(MapObject::*)(uint16, uint16))&MapObject::SetPosition)
			.def("SetPosition", (void(MapObject::*)(uint16, float, uint16, float))&MapObject::SetPosition)
			.def("SetXPosition", &MapObject::SetXPosition)
			.def("SetYPosition", &MapObject::SetYPosition)
			.def("SetImgHalfWidth", &MapObject::SetImgHalfWidth)
			.def("SetImgHeight", &MapObject::SetImgHeight)
			.def("SetCollHalfWidth", &MapObject::SetCollHalfWidth)
			.def("SetCollHeight", &MapObject::SetCollHeight)
			.def("GetObjectID", &MapObject::GetObjectID)
			.def("GetObjectLayerID", &MapObject::GetObjectLayerID)
			.def("GetContext", &MapObject::GetContext)
//			.def("GetXPosition", &MapObject::GetXPosition)
//			.def("GetYPosition", &MapObject::GetYPosition)
			.def("GetImgHalfWidth", &MapObject::GetImgHalfWidth)
			.def("GetImgHeight", &MapObject::GetImgHeight)
			.def("GetCollHalfWidth", &MapObject::GetCollHalfWidth)
			.def("GetCollHeight", &MapObject::GetCollHeight),


		class_<PhysicalObject, MapObject>("PhysicalObject")
			.def(constructor<>())
			.def("AddAnimation", (void(PhysicalObject::*)(std::string))&PhysicalObject::AddAnimation)
			.def("SetCurrentAnimation", &PhysicalObject::SetCurrentAnimation)
			.def("SetAnimationProgress", &PhysicalObject::SetAnimationProgress)
			.def("GetCurrentAnimation", &PhysicalObject::GetCurrentAnimation),

		class_<MapTreasure, PhysicalObject>("MapTreasure")
			.def(constructor<std::string, uint8, uint8, uint8>())
			.def("GetTreasureContainer", &MapTreasure::GetTreasureContainer),

		class_<GlimmerTreasure, PhysicalObject>("GlimmerTreasure")
			.def(constructor<>())
			.def(constructor<std::string, uint32, uint32>())
			.def("GetTreasureContainer", &GlimmerTreasure::GetTreasureContainer)
			.def("SetDisplayDelay", (void(GlimmerTreasure::*)(uint32))&GlimmerTreasure::SetDisplayDelay)
			.def("SetDisplayDelay", (void(GlimmerTreasure::*)(uint32, float))&GlimmerTreasure::SetDisplayDelay)
			.def("SetDisplayEnabled", &GlimmerTreasure::SetDisplayEnabled)
			.def("ForceDisplay", &GlimmerTreasure::ForceDisplay)
			.def("Acquire", &GlimmerTreasure::Acquire)
			.def("Update", &GlimmerTreasure::Update)
			.def("Draw", &GlimmerTreasure::Draw)

			// Namespace constants
			.enum_("constants") [
				value("GLIMMER_WAIT_COMMON", GlimmerTreasure::GLIMMER_WAIT_COMMON),
				value("GLIMMER_WAIT_UNCOMMON", GlimmerTreasure::GLIMMER_WAIT_UNCOMMON),
				value("GLIMMER_WAIT_RARE", GlimmerTreasure::GLIMMER_WAIT_RARE),
				value("DEFAULT_FRAME_TIME", GlimmerTreasure::DEFAULT_FRAME_TIME)
			],

		class_<VirtualSprite, MapObject>("VirtualSprite")
			.def(constructor<>())
			.def("SetMoving", &VirtualSprite::SetMoving)
			.def("SetRunning", &VirtualSprite::SetRunning)
			.def("SetDirection", &VirtualSprite::SetDirection)
			.def("SetRandomDirection", &VirtualSprite::SetRandomDirection)
			.def("SetMovementSpeed", &VirtualSprite::SetMovementSpeed)
			.def("IsFacingDirection", &VirtualSprite::IsFacingDirection)
			.def("IsMoving", &VirtualSprite::IsMoving)
			.def("GetDirection", &VirtualSprite::GetDirection)
			.def("GetMovementSpeed", &VirtualSprite::GetMovementSpeed),

		class_<MapSprite, VirtualSprite>("MapSprite")
			.scope
			[
				def("Create", &MapSprite::Create)
			]
			.def(constructor<>())
			.def("SetName", &MapSprite::SetName)
			.def("SetDirection", &MapSprite::SetDirection)
			.def("SetRandomDirection", &MapSprite::SetRandomDirection)
			.def("SetStationaryMovement", &MapSprite::SetStationaryMovement)
			.def("SetReverseMovement", &MapSprite::SetReverseMovement)
			.def("SetCurrentAnimation", &MapSprite::SetCurrentAnimation)
			.def("GetCurrentAnimation", (uint8(MapSprite::*)()const)&MapSprite::GetCurrentAnimation)
			.def("GetCurrentAnimation", (hoa_video::AnimatedImage&(MapSprite::*)())&MapSprite::GetCurrentAnimation)
			.def("GetAnimation", &MapSprite::GetAnimation)
			.def("LoadFacePortrait", &MapSprite::LoadFacePortrait)
			.def("LoadStandardAnimations", &MapSprite::LoadStandardAnimations)
			.def("LoadRunningAnimations", &MapSprite::LoadRunningAnimations)
			.def("LoadAttackAnimations", &MapSprite::LoadAttackAnimations)
			.def("AddDialogueReference", &MapSprite::AddDialogueReference)
			.def("ClearDialogueReferences", &MapSprite::ClearDialogueReferences)
			.def("RemoveDialogueReference", &MapSprite::RemoveDialogueReference),

		class_<EnemySprite, MapSprite>("EnemySprite")
			.scope
			[
				def("Create", &EnemySprite::Create)
			]
			.def(constructor<>())
			.def("Reset", &EnemySprite::Reset)
			.def("NewEnemyParty", &EnemySprite::NewEnemyParty)
			.def("AddEnemy", &EnemySprite::AddEnemy)
			.def("GetState", &EnemySprite::GetState)
			.def("GetPursuitRange", &EnemySprite::GetPursuitRange)
			.def("GetDirectionChangeTime", &EnemySprite::GetDirectionChangeTime)
			.def("GetFadeTime", &EnemySprite::GetFadeTime)
			.def("SetSpawnedState", &EnemySprite::SetSpawnedState)
			.def("SetZone", &EnemySprite::SetZone)
			.def("SetPursuitRange", &EnemySprite::SetPursuitRange)
			.def("SetDirectionChangeTime", &EnemySprite::SetDirectionChangeTime)
			.def("SetFadeTime", &EnemySprite::SetFadeTime)
			.def("SetBattleMusicFile", &EnemySprite::SetBattleMusicFile)
			.def("SetBattleBackgroundFile", &EnemySprite::SetBattleBackgroundFile)
			.def("SetBattleScriptFile", &EnemySprite::SetBattleScriptFile)
			.def("ChangeState", &EnemySprite::ChangeState)

			// State constants
			.enum_("constants") [
				value("INACTIVE", EnemySprite::INACTIVE),
				value("SPAWN", EnemySprite::SPAWN),
				value("ACTIVE", EnemySprite::ACTIVE),
				value("HUNT", EnemySprite::HUNT),
				value("DISSIPATE", EnemySprite::DISSIPATE)
			],

		class_<MapZone>("MapZone")
			.def(constructor<>())
			.def(constructor<uint16, uint16, uint16, uint16>())
			.def(constructor<uint16, uint16, uint16, uint16, MAP_CONTEXT>())
			.def("AddSection", &MapZone::AddSection)
			.def("IsInsideZone", &MapZone::IsInsideZone)
			.def("GetZoneID", &MapZone::GetZoneID)
			.def("GetActiveContexts", &MapZone::GetActiveContexts)
			.def("SetZoneID", &MapZone::SetZoneID)
			.def("SetActiveContexts", &MapZone::SetActiveContexts),

		class_<CameraZone, MapZone>("CameraZone")
			.def(constructor<>())
			.def(constructor<uint16, uint16, uint16, uint16>())
			.def(constructor<uint16, uint16, uint16, uint16, MAP_CONTEXT>())
			.def("IsCameraInside", &CameraZone::IsCameraInside)
			.def("IsCameraEntering", &CameraZone::IsCameraEntering)
			.def("IsCameraExiting", &CameraZone::IsCameraExiting)
			.def("IsPlayerSpriteInside", &CameraZone::IsPlayerSpriteInside)
			.def("IsPlayerSpriteEntering", &CameraZone::IsPlayerSpriteEntering)
			.def("IsPlayerSpriteExiting", &CameraZone::IsPlayerSpriteExiting),

		class_<ResidentZone, MapZone>("ResidentZone")
			.def(constructor<>())
			.def(constructor<uint16, uint16, uint16, uint16>())
			.def(constructor<uint16, uint16, uint16, uint16, MAP_CONTEXT>())
			.def("IsResidentEntering", &ResidentZone::IsResidentEntering)
			.def("IsResidentExiting", &ResidentZone::IsResidentExiting)
			.def("IsSpriteResident", (bool(ResidentZone::*)(uint32)const)&ResidentZone::IsSpriteResident)
			.def("IsSpriteResident", (bool(ResidentZone::*)(VirtualSprite*)const)&ResidentZone::IsSpriteResident)
			.def("IsCameraResident", &ResidentZone::IsCameraResident)
			.def("IsSpriteEntering", (bool(ResidentZone::*)(uint32)const)&ResidentZone::IsSpriteEntering)
			.def("IsSpriteEntering", (bool(ResidentZone::*)(VirtualSprite*)const)&ResidentZone::IsSpriteEntering)
			.def("IsCameraEntering", &ResidentZone::IsCameraEntering)
			.def("IsSpriteExiting", (bool(ResidentZone::*)(uint32)const)&ResidentZone::IsSpriteExiting)
			.def("IsSpriteExiting", (bool(ResidentZone::*)(VirtualSprite*)const)&ResidentZone::IsSpriteExiting)
			.def("IsCameraExiting", &ResidentZone::IsCameraExiting)
			.def("GetResident", &ResidentZone::GetResident)
			.def("GetEnteringResident", &ResidentZone::GetEnteringResident)
			.def("GetExitingResident", &ResidentZone::GetExitingResident)
			.def("GetNumberResidents", &ResidentZone::GetNumberResidents)
			.def("GetNumberEnteringResidents", &ResidentZone::GetNumberEnteringResidents)
			.def("GetNumberExitingResidents", &ResidentZone::GetNumberExitingResidents),

		class_<EnemyZone, MapZone>("EnemyZone")
			.def(constructor<>())
			.def(constructor<uint16, uint16, uint16, uint16>())
			.def("AddEnemy", &EnemyZone::AddEnemy)
			.def("AddSpawnSection", &EnemyZone::AddSpawnSection)
			.def("ForceSpawnAllEnemies", &EnemyZone::ForceSpawnAllEnemies)
			.def("IsRoamingRestrained", &EnemyZone::IsRoamingRestrained)
			.def("IsSpawningDisabled", &EnemyZone::IsSpawningDisabled)
			.def("GetSpawnTime", &EnemyZone::GetSpawnTime)
			.def("SetRoamingRestrained", &EnemyZone::SetRoamingRestrained)
			.def("SetSpawningDisabled", &EnemyZone::SetSpawningDisabled)
			.def("SetSpawnTime", &EnemyZone::SetSpawnTime),

		class_<ContextZone, MapZone>("ContextZone")
			.def(constructor<MAP_CONTEXT, MAP_CONTEXT>())
			.def("AddSection", (void(ContextZone::*)(uint16, uint16, uint16, uint16, bool))&ContextZone::AddSection),

		class_<DialogueSupervisor>("DialogueSupervisor")
			.def("BeginDialogue", &DialogueSupervisor::BeginDialogue)
			.def("EndDialogue", &DialogueSupervisor::EndDialogue)
			.def("GetDialogue", &DialogueSupervisor::GetDialogue)
			.def("GetCurrentDialogue", &DialogueSupervisor::GetCurrentDialogue),

		class_<MapDialogue, hoa_common::CommonDialogue>("MapDialogue")
			.scope
			[
				def("Create", &MapDialogue::Create)
			]
			.def("AddEventAtStart", (void(MapDialogue::*)(uint32))&MapDialogue::AddEventAtStart)
			.def("AddEventAtStart", (void(MapDialogue::*)(uint32, uint32))&MapDialogue::AddEventAtStart)
			.def("AddEventAtEnd", (void(MapDialogue::*)(uint32))&MapDialogue::AddEventAtEnd)
			.def("AddEventAtEnd", (void(MapDialogue::*)(uint32, uint32))&MapDialogue::AddEventAtEnd)
			.def("AddLine", (void(MapDialogue::*)(std::string, uint32))&MapDialogue::AddLine)
			.def("AddLine", (void(MapDialogue::*)(std::string, uint32, int32))&MapDialogue::AddLine)
			.def("AddLine", (void(MapDialogue::*)(std::string))&MapDialogue::AddLine)
			.def("AddLineTiming", (void(MapDialogue::*)(uint32))&MapDialogue::AddLineTiming)
			.def("AddLineTiming", (void(MapDialogue::*)(uint32, uint32))&MapDialogue::AddLineTiming)
			.def("AddLineGlobalRecord", &MapDialogue::AddLineGlobalRecord)
			.def("AddLineLocalRecord", &MapDialogue::AddLineLocalRecord)
			.def("AddLineEventAtStart", (void(MapDialogue::*)(uint32))&MapDialogue::AddLineEventAtStart)
			.def("AddLineEventAtStart", (void(MapDialogue::*)(uint32, uint32))&MapDialogue::AddLineEventAtStart)
			.def("AddLineEventAtEnd", (void(MapDialogue::*)(uint32))&MapDialogue::AddLineEventAtEnd)
			.def("AddLineEventAtEnd", (void(MapDialogue::*)(uint32, uint32))&MapDialogue::AddLineEventAtEnd)
			.def("AddOption", (void(MapDialogue::*)(std::string))&MapDialogue::AddOption)
			.def("AddOption", (void(MapDialogue::*)(std::string, int32))&MapDialogue::AddOption)
			.def("AddOptionGlobalRecord", &MapDialogue::AddOptionGlobalRecord)
			.def("AddOptionLocalRecord", &MapDialogue::AddOptionLocalRecord)
			.def("AddOptionEvent", (void(MapDialogue::*)(uint32))&MapDialogue::AddOptionEvent)
			.def("AddOptionEvent", (void(MapDialogue::*)(uint32, uint32))&MapDialogue::AddOptionEvent)
			.def("Validate", &MapDialogue::Validate)
			.def("SetInputBlocked", &MapDialogue::SetInputBlocked)
			.def("SetRestoreState", &MapDialogue::SetRestoreState),

		class_<EventSupervisor>("EventSupervisor")
			.def("RegisterEvent", &EventSupervisor::RegisterEvent, adopt(_2))
			.def("StartEvent", (void(EventSupervisor::*)(uint32))&EventSupervisor::StartEvent)
			.def("StartEvent", (void(EventSupervisor::*)(MapEvent*))&EventSupervisor::StartEvent)
			.def("StartEvent", (void(EventSupervisor::*)(uint32, uint32))&EventSupervisor::StartEvent)
			.def("StartEvent", (void(EventSupervisor::*)(MapEvent*, uint32))&EventSupervisor::StartEvent)
			.def("TerminateEvent", &EventSupervisor::TerminateEvent)
			.def("IsEventActive", &EventSupervisor::IsEventActive)
			.def("TimesEventStarted", &EventSupervisor::TimesEventStarted)
			.def("HasActiveEvent", &EventSupervisor::HasActiveEvent)
			.def("HasLaunchEvent", &EventSupervisor::HasLaunchEvent)
			.def("GetEvent", &EventSupervisor::GetEvent),

		class_<MapEvent>("MapEvent")
			.def("GetEventID", &MapEvent::GetEventID)
			.def("AddEventLinkAtStart", (void(MapEvent::*)(uint32))&MapEvent::AddEventLinkAtStart)
			.def("AddEventLinkAtStart", (void(MapEvent::*)(uint32, uint32))&MapEvent::AddEventLinkAtStart)
			.def("AddEventLinkAtEnd", (void(MapEvent::*)(uint32))&MapEvent::AddEventLinkAtEnd)
			.def("AddEventLinkAtEnd", (void(MapEvent::*)(uint32, uint32))&MapEvent::AddEventLinkAtEnd)
			.def("AddGlobalRecord", &MapEvent::AddGlobalRecord)
			.def("AddLocalRecord", &MapEvent::AddLocalRecord),

		class_<PushMapStateEvent, MapEvent>("PushMapStateEvent")
			.scope
			[
				def("Create", &PushMapStateEvent::Create)
			]
			.def("StopCameraMovement", &PushMapStateEvent::StopCameraMovement),

		class_<PopMapStateEvent, MapEvent>("PopMapStateEvent")
			.scope
			[
				def("Create", &PopMapStateEvent::Create)
			],

		class_<CameraMoveEvent, MapEvent>("CameraMoveEvent")
			.scope
			[
				def("Create", (CameraMoveEvent*(*)(uint32, VirtualSprite*, uint32))&CameraMoveEvent::Create),
				def("Create", (CameraMoveEvent*(*)(uint32, uint32, uint32, uint32))&CameraMoveEvent::Create)
			]
			.def("SetCameraContext", &CameraMoveEvent::SetCameraContext),

		class_<DialogueEvent, MapEvent>("DialogueEvent")
			.scope
			[
				def("Create", &DialogueEvent::Create)
			]
			.def("SetStopCameraMovement", &DialogueEvent::SetStopCameraMovement),

		class_<ShopEvent, MapEvent>("ShopEvent")
			.scope
			[
				def("Create", &ShopEvent::Create)
			]
			.def("AddWare", &ShopEvent::AddWare),

		class_<SoundEvent, MapEvent>("SoundEvent")
			.scope
			[
				def("Create", &SoundEvent::Create)
			],

		class_<MapTransitionEvent, MapEvent>("MapTransitionEvent")
			.scope
			[
				def("Create", (MapTransitionEvent*(*)(uint32, std::string))&MapTransitionEvent::Create),
				def("Create", (MapTransitionEvent*(*)(uint32, std::string, int32))&MapTransitionEvent::Create)
			]
			.def("SetFadeTime", &MapTransitionEvent::SetFadeTime),

		class_<BattleEncounterEvent, MapEvent>("BattleEncounterEvent")
			.scope
			[
				def("Create", &BattleEncounterEvent::Create)
			]
			.def("SetMusic", &BattleEncounterEvent::SetMusic)
			.def("SetBackground", &BattleEncounterEvent::SetBackground)
			.def("AddEnemy", &BattleEncounterEvent::AddEnemy),

		class_<CustomEvent, MapEvent>("CustomEvent")
			.scope
			[
				def("Create", &CustomEvent::Create)
			],

		class_<SpriteEvent, MapEvent>("SpriteEvent"),

		class_<ChangePropertySpriteEvent, SpriteEvent>("ChangePropertySpriteEvent")
			.scope
			[
				def("Create", (ChangePropertySpriteEvent*(*)(uint32, VirtualSprite*))&ChangePropertySpriteEvent::Create),
				def("Create", (ChangePropertySpriteEvent*(*)(uint32, uint16))&ChangePropertySpriteEvent::Create)
			]
			.def("AddSprite", &ChangePropertySpriteEvent::AddSprite)
			.def("PositionChangeRelative", &ChangePropertySpriteEvent::PositionChangeRelative)
			.def("Updatable", &ChangePropertySpriteEvent::Updatable)
			.def("Visible", &ChangePropertySpriteEvent::Visible)
			.def("Collidable", &ChangePropertySpriteEvent::Collidable)
			.def("Context", &ChangePropertySpriteEvent::Context)
			.def("Position", (void(ChangePropertySpriteEvent::*)(int16, int16)) &ChangePropertySpriteEvent::Position)
			.def("Position", (void(ChangePropertySpriteEvent::*)(int16, float, int16, float)) &ChangePropertySpriteEvent::Position)
			.def("Direction", &ChangePropertySpriteEvent::Direction)
			.def("MovementSpeed", &ChangePropertySpriteEvent::MovementSpeed)
			.def("Moving", &ChangePropertySpriteEvent::Moving)
			.def("Running", &ChangePropertySpriteEvent::Running)
			.def("StationaryMovement", &ChangePropertySpriteEvent::StationaryMovement)
			.def("ReverseMovement", &ChangePropertySpriteEvent::ReverseMovement),

		class_<AnimateSpriteEvent, MapEvent>("AnimateSpriteEvent")
			.scope
			[
				def("Create", (AnimateSpriteEvent*(*)(uint32, VirtualSprite*))&AnimateSpriteEvent::Create),
				def("Create", (AnimateSpriteEvent*(*)(uint32, uint16))&AnimateSpriteEvent::Create)
			]
			.def("AddFrame", &AnimateSpriteEvent::AddFrame)
			.def("SetLoopCount", &AnimateSpriteEvent::SetLoopCount),

		class_<RandomMoveSpriteEvent, SpriteEvent>("RandomMoveSpriteEvent")
			.scope
			[
				def("Create", (RandomMoveSpriteEvent*(*)(uint32, VirtualSprite*, uint32, uint32))&RandomMoveSpriteEvent::Create),
				def("Create", (RandomMoveSpriteEvent*(*)(uint32, uint16, uint32, uint32))&RandomMoveSpriteEvent::Create)
			],

		class_<PathMoveSpriteEvent, SpriteEvent>("PathMoveSpriteEvent")
			.scope
			[
				def("Create", (PathMoveSpriteEvent*(*)(uint32, VirtualSprite*, int16, int16))&PathMoveSpriteEvent::Create),
				def("Create", (PathMoveSpriteEvent*(*)(uint32, uint16, int16, int16))&PathMoveSpriteEvent::Create)
			]
			.def("SetRelativeDestination", &PathMoveSpriteEvent::SetRelativeDestination)
			.def("SetDestination", &PathMoveSpriteEvent::SetDestination)
			.def("SetFinalDirection", &PathMoveSpriteEvent::SetFinalDirection),

		class_<CustomSpriteEvent, SpriteEvent>("CustomSpriteEvent")
			.scope
			[
				def("Create", (CustomSpriteEvent*(*)(uint32, VirtualSprite*, std::string, std::string))&CustomSpriteEvent::Create),
				def("Create", (CustomSpriteEvent*(*)(uint32, uint16, std::string, std::string))&CustomSpriteEvent::Create)
			],

		class_<TileSupervisor>("TileSupervisor")
			.def("GetRowCount", &TileSupervisor::GetRowCount)
			.def("GetColumnCount", &TileSupervisor::GetColumnCount)
			.def("GetTileLayerCount", &TileSupervisor::GetTileLayerCount)
			.def("GetInheritedContext", &TileSupervisor::GetInheritedContext),

		class_<TransitionSupervisor>("TransitionSupervisor")
			.def("StartContextTransition", (bool(TransitionSupervisor::*)(MAP_CONTEXT))&TransitionSupervisor::StartContextTransition)
			.def("StartContextTransition", (bool(TransitionSupervisor::*)(MAP_CONTEXT, uint32))&TransitionSupervisor::StartContextTransition)
			.def("StartGameModeTransition", (bool(TransitionSupervisor::*)(hoa_mode_manager::GameMode*, uint32))&TransitionSupervisor::StartGameModeTransition, adopt(_2))
			.def("StartGameModeTransition", (bool(TransitionSupervisor::*)(hoa_mode_manager::GameMode*, uint32))&TransitionSupervisor::StartGameModeTransition, adopt(_2))
			.def("IsTransitionActive", &TransitionSupervisor::IsTransitionActive)
			.def("SetTransitionColor", &TransitionSupervisor::SetTransitionColor)
			.def("SetTerminateMapOnCompletion", &TransitionSupervisor::SetTerminateMapOnCompletion)
			.def("SetContextCameraChanges", &TransitionSupervisor::SetContextCameraChanges),

		class_<TreasureContainer>("TreasureContainer")
			.def(constructor<>())
			.def("AddDrunes", &TreasureContainer::AddDrunes)
			.def("AddObject", &TreasureContainer::AddObject)
			.def("IsTaken", &TreasureContainer::IsTaken)
			.def("SetTaken", &TreasureContainer::SetTaken),

		class_<TreasureSupervisor>("TreasureSupervisor")
			.def("Initialize", (void(TreasureSupervisor::*)(MapTreasure*))&TreasureSupervisor::Initialize)
			.def("Initialize", (void(TreasureSupervisor::*)(TreasureContainer*))&TreasureSupervisor::Initialize)
	];

	} // End using map mode namespaces



	// ----- Menu Mode bindings
	{
	using namespace hoa_menu;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_menu")
	[
		class_<MenuMode, hoa_mode_manager::GameMode>("MenuMode")
			.def(constructor<>())
	];

	} // End using menu mode namespaces

	// ----- Shop Mode bindings
	{
	using namespace hoa_shop;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_shop")
	[
		class_<ShopMode, hoa_mode_manager::GameMode>("ShopMode")
			.def(constructor<>())
			.def("AddObject", &ShopMode::AddObject)
	];

	} // End using shop mode namespaces

	// ----- Test Mode bindings
	{
	using namespace hoa_test;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_test")
	[
		class_<TestMode, hoa_mode_manager::GameMode>("TestMode")
			.def("SetImmediateTestID", &TestMode::SetImmediateTestID)
	];

	} // End using test mode namespaces

} // void BindModeCode()

} // namespace hoa_defs
