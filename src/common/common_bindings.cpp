///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    common_bindings.cpp
*** \author  Daniel Steuernol (Steu)
*** \brief   Lua bindings for common game code
***
*** All bindings for the common code is contained within this file.
*** Therefore, everything that you see bound within this file will be made
*** available in Lua. This file also binds some of the Allacrost utility code
*** found in src/utils.h.
***
*** \note To most C++ programmers, the syntax of the binding code found in this
*** file may be very unfamiliar and obtuse. Refer to the Luabind documentation
*** as necessary to gain an understanding of this code style.
*** **************************************************************************/

#include "defs.h"
#include "utils.h"

#include "common.h"
#include "dialogue.h"

#include "global.h"
#include "global_actors.h"
#include "global_effects.h"
#include "global_objects.h"
#include "global_skills.h"
#include "global_utils.h"

using namespace luabind;

namespace hoa_defs {

void BindCommonCode() {
	// ---------- Bind Utils Functions
	{
	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_utils")
	[
		def("RandomFloat", (float(*)(void)) &hoa_utils::RandomFloat),
		def("RandomBoundedInteger", &hoa_utils::RandomBoundedInteger),
		def("RandomProbability", &hoa_utils::RandomProbability)
	];
	}

	// ---------- Bind Common Components
	{
	using namespace hoa_common;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_common")
	[
		class_<CommonRecordGroup>("CommonRecordGroup")
			.def("DoesRecordExist", &CommonRecordGroup::DoesRecordExist)
			.def("AddNewRecord", &CommonRecordGroup::AddNewRecord)
			.def("GetRecord", &CommonRecordGroup::GetRecord)
			.def("SetRecord", &CommonRecordGroup::SetRecord)
			.def("ModifyRecord", &CommonRecordGroup::ModifyRecord)
			.def("DeleteRecord", &CommonRecordGroup::DeleteRecord)
			.def("GetNumberRecords", &CommonRecordGroup::GetNumberRecords)
			.def("GetGroupName", &CommonRecordGroup::GetGroupName)

			// Constants
			.enum_("constants") [
				value("BAD_RECORD", CommonRecordGroup::BAD_RECORD)
			],

		class_<CommonDialogue>("CommonDialogue")
			// TODO: add commented lines back in later. There is a build issue with the editor when these lines are included
// 			.def("AddLine", (void(CommonDialogue::*)(std::string))&CommonDialogue::AddLine)
// 			.def("AddLine", (void(CommonDialogue::*)(std::string, int32))&CommonDialogue::AddLine)
// 			.def("AddLineTimed", (void(CommonDialogue::*)(std::string, uint32))&CommonDialogue::AddLineTimed)
// 			.def("AddLineTimed", (void(CommonDialogue::*)(std::string, int32, uint32))&CommonDialogue::AddLineTimed)
// 			.def("AddOption", (void(CommonDialogue::*)(std::string))&CommonDialogue::AddOption)
// 			.def("AddOption", (void(CommonDialogue::*)(std::string, int32))&CommonDialogue::AddOption)
			.def("HasAlreadySeen", &CommonDialogue::HasAlreadySeen)
			.def("SetTimesSeen", &CommonDialogue::SetTimesSeen)
			.def("SetMaxViews", &CommonDialogue::SetMaxViews)

			// Constants
			.enum_("constants") [
				value("NEXT_LINE", COMMON_DIALOGUE_NEXT_LINE),
				value("END_DIALOGUE", COMMON_DIALOGUE_END)
			]
	];

	} // End using common namespace


	// ---------- Bind Global Components
	{
	using namespace hoa_global;

	def("GetTargetText", &GetTargetText),
	def("IsTargetActor", &IsTargetActor),
	def("IsTargetParty", &IsTargetParty),
	def("IsTargetSelf", &IsTargetSelf),
	def("IsTargetAlly", &IsTargetAlly),
	def("IsTargetFoe", &IsTargetFoe),
	// TODO: Luabind doesn't like these functions. I think its because they take reference arguments.
// 	def("IncrementIntensity", &IncrementIntensity),
// 	def("DecrementIntensity", &DecrementIntensity),

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GameGlobal>("GameGlobal")
			.def("ClearAllData", &GameGlobal::ClearAllData)
			.def("AddCharacter", (void(GameGlobal::*)(uint32)) &GameGlobal::AddCharacter)
			.def("RemoveCharacter", (void(GameGlobal::*)(uint32)) &GameGlobal::RemoveCharacter)
			.def("RestoreAllCharacterHitPoints", &GameGlobal::RestoreAllCharacterHitPoints)
			.def("RestoreAllCharacterSkillPoints", &GameGlobal::RestoreAllCharacterSkillPoints)
			.def("GetCharacter", &GameGlobal::GetCharacter)
			.def("GetDrunes", &GameGlobal::GetDrunes)
			.def("SetDrunes", &GameGlobal::SetDrunes)
			.def("AddDrunes", &GameGlobal::AddDrunes)
			.def("SubtractDrunes", &GameGlobal::SubtractDrunes)
			.def("AddToInventory", (void (GameGlobal::*)(uint32, uint32)) &GameGlobal::AddToInventory)
			.def("RemoveFromInventory", (void (GameGlobal::*)(uint32)) &GameGlobal::RemoveFromInventory)
			.def("IncrementObjectCount", &GameGlobal::IncrementObjectCount)
			.def("DecrementObjectCount", &GameGlobal::DecrementObjectCount)
			.def("DoesRecordGroupExist", &GameGlobal::DoesRecordGroupExist)
			.def("DoesRecordExist", &GameGlobal::DoesRecordExist)
			.def("AddNewRecordGroup", &GameGlobal::AddNewRecordGroup)
			.def("GetRecordGroup", &GameGlobal::GetRecordGroup)
			.def("GetRecordValue", &GameGlobal::GetRecordValue)
			.def("SetRecordValue", &GameGlobal::SetRecordValue)
			.def("GetNumberRecordGroups", &GameGlobal::GetNumberRecordGroups)
			.def("GetNumberRecords", &GameGlobal::GetNumberRecords)
			.def("SetLocation", (void(GameGlobal::*)(const std::string&)) &GameGlobal::SetLocation)
			.def("GetBattleSetting", &GameGlobal::GetBattleSetting)
			.def("SetBattleSetting", &GameGlobal::SetBattleSetting)

			// Namespace constants
			.enum_("constants") [
				// Character types
				value("GLOBAL_CHARACTER_INVALID", GLOBAL_CHARACTER_INVALID),
				value("GLOBAL_CHARACTER_ALL", GLOBAL_CHARACTER_ALL),
				// Object types
				value("GLOBAL_OBJECT_INVALID", GLOBAL_OBJECT_INVALID),
				value("GLOBAL_OBJECT_ITEM", GLOBAL_OBJECT_ITEM),
				value("GLOBAL_OBJECT_WEAPON", GLOBAL_OBJECT_WEAPON),
				value("GLOBAL_OBJECT_HEAD_ARMOR", GLOBAL_OBJECT_HEAD_ARMOR),
				value("GLOBAL_OBJECT_TORSO_ARMOR", GLOBAL_OBJECT_TORSO_ARMOR),
				value("GLOBAL_OBJECT_ARM_ARMOR", GLOBAL_OBJECT_ARM_ARMOR),
				value("GLOBAL_OBJECT_LEG_ARMOR", GLOBAL_OBJECT_LEG_ARMOR),
				value("GLOBAL_OBJECT_SHARD", GLOBAL_OBJECT_SHARD),
				value("GLOBAL_OBJECT_KEY_ITEM", GLOBAL_OBJECT_KEY_ITEM),
				// Item usage constants
				value("GLOBAL_USE_INVALID", GLOBAL_USE_INVALID),
				value("GLOBAL_USE_FIELD", GLOBAL_USE_FIELD),
				value("GLOBAL_USE_BATTLE", GLOBAL_USE_BATTLE),
				value("GLOBAL_USE_ALL", GLOBAL_USE_ALL),
				// Skill types
				value("GLOBAL_SKILL_INVALID", GLOBAL_SKILL_INVALID),
				value("GLOBAL_SKILL_ATTACK", GLOBAL_SKILL_ATTACK),
				value("GLOBAL_SKILL_DEFEND", GLOBAL_SKILL_DEFEND),
				value("GLOBAL_SKILL_SUPPORT", GLOBAL_SKILL_SUPPORT),
				// Battle settings
				value("GLOBAL_BATTLE_INVALID", GLOBAL_BATTLE_INVALID),
				value("GLOBAL_BATTLE_WAIT", GLOBAL_BATTLE_WAIT),
				value("GLOBAL_BATTLE_ACTIVE", GLOBAL_BATTLE_ACTIVE),
				value("GLOBAL_BATTLE_TOTAL", GLOBAL_BATTLE_TOTAL),
				// Elemental type constants
				value("GLOBAL_ELEMENTAL_FIRE", GLOBAL_ELEMENTAL_FIRE),
				value("GLOBAL_ELEMENTAL_WATER", GLOBAL_ELEMENTAL_WATER),
				value("GLOBAL_ELEMENTAL_VOLT", GLOBAL_ELEMENTAL_VOLT),
				value("GLOBAL_ELEMENTAL_EARTH", GLOBAL_ELEMENTAL_EARTH),
				value("GLOBAL_ELEMENTAL_SLASHING", GLOBAL_ELEMENTAL_SLASHING),
				value("GLOBAL_ELEMENTAL_PIERCING", GLOBAL_ELEMENTAL_PIERCING),
				value("GLOBAL_ELEMENTAL_CRUSHING", GLOBAL_ELEMENTAL_CRUSHING),
				value("GLOBAL_ELEMENTAL_MAULING", GLOBAL_ELEMENTAL_MAULING),
				// Status type constants
				value("GLOBAL_STATUS_INVALID", GLOBAL_STATUS_INVALID),
				value("GLOBAL_STATUS_STRENGTH_RAISE", GLOBAL_STATUS_STRENGTH_RAISE),
				value("GLOBAL_STATUS_STRENGTH_LOWER", GLOBAL_STATUS_STRENGTH_LOWER),
				value("GLOBAL_STATUS_VIGOR_RAISE", GLOBAL_STATUS_VIGOR_RAISE),
				value("GLOBAL_STATUS_VIGOR_LOWER", GLOBAL_STATUS_VIGOR_LOWER),
				value("GLOBAL_STATUS_FORTITUDE_RAISE", GLOBAL_STATUS_FORTITUDE_RAISE),
				value("GLOBAL_STATUS_FORTITUDE_LOWER", GLOBAL_STATUS_FORTITUDE_LOWER),
				value("GLOBAL_STATUS_PROTECTION_RAISE", GLOBAL_STATUS_PROTECTION_RAISE),
				value("GLOBAL_STATUS_PROTECTION_LOWER", GLOBAL_STATUS_PROTECTION_LOWER),
				value("GLOBAL_STATUS_AGILITY_RAISE", GLOBAL_STATUS_AGILITY_RAISE),
				value("GLOBAL_STATUS_AGILITY_LOWER", GLOBAL_STATUS_AGILITY_LOWER),
				value("GLOBAL_STATUS_EVADE_RAISE", GLOBAL_STATUS_EVADE_RAISE),
				value("GLOBAL_STATUS_EVADE_LOWER", GLOBAL_STATUS_EVADE_LOWER),
				value("GLOBAL_STATUS_HP_REGEN", GLOBAL_STATUS_HP_REGEN),
				value("GLOBAL_STATUS_HP_DRAIN", GLOBAL_STATUS_HP_DRAIN),
				value("GLOBAL_STATUS_SP_REGEN", GLOBAL_STATUS_SP_REGEN),
				value("GLOBAL_STATUS_SP_DRAIN", GLOBAL_STATUS_SP_DRAIN),
				value("GLOBAL_STATUS_PARALYSIS", GLOBAL_STATUS_PARALYSIS),
				value("GLOBAL_STATUS_STASIS", GLOBAL_STATUS_STASIS),
				// Intensity type constants
				value("GLOBAL_INTENSITY_NEG_EXTREME", GLOBAL_INTENSITY_NEG_EXTREME),
				value("GLOBAL_INTENSITY_NEG_GREATER", GLOBAL_INTENSITY_NEG_GREATER),
				value("GLOBAL_INTENSITY_NEG_MODERATE", GLOBAL_INTENSITY_NEG_MODERATE),
				value("GLOBAL_INTENSITY_NEG_LESSER", GLOBAL_INTENSITY_NEG_LESSER),
				value("GLOBAL_INTENSITY_NEUTRAL", GLOBAL_INTENSITY_NEUTRAL),
				value("GLOBAL_INTENSITY_POS_LESSER", GLOBAL_INTENSITY_POS_LESSER),
				value("GLOBAL_INTENSITY_POS_MODERATE", GLOBAL_INTENSITY_POS_MODERATE),
				value("GLOBAL_INTENSITY_POS_GREATER", GLOBAL_INTENSITY_POS_GREATER),
				value("GLOBAL_INTENSITY_POS_EXTREME", GLOBAL_INTENSITY_POS_EXTREME),
				// Target constants
				value("GLOBAL_TARGET_INVALID", GLOBAL_TARGET_INVALID),
				value("GLOBAL_TARGET_SELF", GLOBAL_TARGET_SELF),
				value("GLOBAL_TARGET_ALLY", GLOBAL_TARGET_ALLY),
				value("GLOBAL_TARGET_FOE", GLOBAL_TARGET_FOE),
				value("GLOBAL_TARGET_ALL_ALLIES", GLOBAL_TARGET_ALL_ALLIES),
				value("GLOBAL_TARGET_ALL_FOES", GLOBAL_TARGET_ALL_FOES)
			]
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalActor>("GlobalActor")
			.def("GetID", &GlobalActor::GetID)
			.def("GetName", &GlobalActor::GetName)
			.def("GetFilename", &GlobalActor::GetFilename)

			.def("GetHitPoints", &GlobalActor::GetHitPoints)
			.def("GetMaxHitPoints", &GlobalActor::GetMaxHitPoints)
			.def("GetActiveMaxHitPoints", &GlobalActor::GetActiveMaxHitPoints)
			.def("GetHitPointFatigue", &GlobalActor::GetHitPointFatigue)
			.def("GetSkillPoints", &GlobalActor::GetSkillPoints)
			.def("GetMaxSkillPoints", &GlobalActor::GetMaxSkillPoints)
			.def("GetActiveMaxSkillPoints", &GlobalActor::GetActiveMaxSkillPoints)
			.def("GetSkillPointFatigue", &GlobalActor::GetSkillPointFatigue)
			.def("GetExperienceLevel", &GlobalActor::GetExperienceLevel)
			.def("GetStrength", &GlobalActor::GetStrength)
			.def("GetVigor", &GlobalActor::GetVigor)
			.def("GetFortitude", &GlobalActor::GetFortitude)
			.def("GetProtection", &GlobalActor::GetProtection)
			.def("GetStamina", &GlobalActor::GetStamina)
			.def("GetResilience", &GlobalActor::GetResilience)
			.def("GetAgility", &GlobalActor::GetAgility)
			.def("GetEvade", &GlobalActor::GetEvade)

			.def("GetTotalPhysicalAttack", &GlobalActor::GetTotalPhysicalAttack)
			.def("GetTotalEtherealAttack", &GlobalActor::GetTotalEtherealAttack)
// 			.def("GetWeaponEquipped", &GlobalActor::GetWeaponEquipped)
// 			.def("GetArmorEquipped", (GlobalArmor* (GlobalActor::*)(uint32)) &GlobalActor::GetArmorEquipped)
// 			.def("GetAttackPoints", &GlobalActor::GetAttackPoints)
// 			.def("GetElementalAttackBonuses", &GlobalActor::GetElementalAttackBonuses)
// 			.def("GetStatusAttackBonuses", &GlobalActor::GetStatusAttackBonuses)
// 			.def("GetElementalDefenseBonuses", &GlobalActor::GetElementalDefenseBonuses)
// 			.def("GetStatusDefenseBonuses", &GlobalActor::GetStatusDefenseBonuses)

			.def("SetHitPoints", &GlobalActor::SetHitPoints)
			.def("SetMaxHitPoints", &GlobalActor::SetMaxHitPoints)
			.def("SetHitPointFatigue", &GlobalActor::SetHitPointFatigue)
			.def("SetSkillPoints", &GlobalActor::SetSkillPoints)
			.def("SetMaxSkillPoints", &GlobalActor::SetMaxSkillPoints)
			.def("SetSkillPointFatigue", &GlobalActor::SetSkillPointFatigue)
			.def("SetExperienceLevel", &GlobalActor::SetExperienceLevel)
			.def("SetStrength", &GlobalActor::SetStrength)
			.def("SetVigor", &GlobalActor::SetVigor)
			.def("SetFortitude", &GlobalActor::SetFortitude)
			.def("SetProtection", &GlobalActor::SetProtection)
			.def("SetStamina", &GlobalActor::SetStamina)
			.def("SetResilience", &GlobalActor::SetResilience)
			.def("SetAgility", &GlobalActor::SetAgility)
			.def("SetEvade", &GlobalActor::SetEvade)

			.def("AddHitPoints", &GlobalActor::AddHitPoints)
			.def("SubtractHitPoints", &GlobalActor::SubtractHitPoints)
			.def("AddMaxHitPoints", &GlobalActor::AddMaxHitPoints)
			.def("SubtractMaxHitPoints", &GlobalActor::SubtractMaxHitPoints)
			.def("AddHitPointFatigue", &GlobalActor::AddHitPointFatigue)
			.def("SubtractHitPointFatigue", &GlobalActor::SubtractHitPointFatigue)
			.def("AddSkillPoints", &GlobalActor::AddSkillPoints)
			.def("SubtractSkillPoints", &GlobalActor::SubtractSkillPoints)
			.def("AddMaxSkillPoints", &GlobalActor::AddMaxSkillPoints)
			.def("SubtractMaxSkillPoints", &GlobalActor::SubtractMaxSkillPoints)
			.def("AddSkillPointFatigue", &GlobalActor::AddSkillPointFatigue)
			.def("SubtractSkillPointFatigue", &GlobalActor::SubtractSkillPointFatigue)
			.def("AddStrength", &GlobalActor::AddStrength)
			.def("SubtractStrength", &GlobalActor::SubtractStrength)
			.def("AddVigor", &GlobalActor::AddVigor)
			.def("SubtractVigor", &GlobalActor::SubtractVigor)
			.def("AddFortitude", &GlobalActor::AddFortitude)
			.def("SubtractFortitude", &GlobalActor::SubtractFortitude)
			.def("AddProtection", &GlobalActor::AddProtection)
			.def("SubtractProtection", &GlobalActor::SubtractProtection)
			.def("AddStamina", &GlobalActor::AddStamina)
			.def("SubtractStamina", &GlobalActor::SubtractStamina)
			.def("AddResilience", &GlobalActor::AddResilience)
			.def("SubtractResilience", &GlobalActor::SubtractResilience)
			.def("AddAgility", &GlobalActor::AddAgility)
			.def("SubtractAgility", &GlobalActor::SubtractAgility)
			.def("AddEvade", &GlobalActor::AddEvade)
			.def("SubtractEvade", &GlobalActor::SubtractEvade)
			.def("RestoreAllHitPoints", &GlobalActor::RestoreAllHitPoints)
			.def("RestoreAllSkillPoints", &GlobalActor::RestoreAllSkillPoints)
			.def("RemoveAllHitPointFatigue", &GlobalActor::RemoveAllHitPointFatigue)
			.def("RemoveAllSkillPointFatigue", &GlobalActor::RemoveAllSkillPointFatigue)

			.def("IsAlive", &GlobalActor::IsAlive)
// 			.def("EquipWeapon", &GlobalActor::EquipWeapon)
// 			.def("EquipArmor", &GlobalActor::EquipArmor)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalCharacter, GlobalActor>("GlobalCharacter")
			.def_readwrite("_hit_points_growth", &GlobalCharacter::_hit_points_growth)
			.def_readwrite("_skill_points_growth", &GlobalCharacter::_skill_points_growth)
			.def_readwrite("_strength_growth", &GlobalCharacter::_strength_growth)
			.def_readwrite("_vigor_growth", &GlobalCharacter::_vigor_growth)
			.def_readwrite("_fortitude_growth", &GlobalCharacter::_fortitude_growth)
			.def_readwrite("_protection_growth", &GlobalCharacter::_protection_growth)
			.def_readwrite("_stamina_growth", &GlobalCharacter::_stamina_growth)
			.def_readwrite("_resilience_growth", &GlobalCharacter::_resilience_growth)
			.def_readwrite("_agility_growth", &GlobalCharacter::_agility_growth)
			.def_readwrite("_evade_growth", &GlobalCharacter::_evade_growth)
			.def("AddExperienceForNextLevel", &GlobalCharacter::AddExperienceForNextLevel)
			.def("AddSkill", &GlobalCharacter::AddSkill)
			.def("AddNewSkillLearned", &GlobalCharacter::AddNewSkillLearned)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalParty>("GlobalParty")
			.def("AddHitPoints", &GlobalParty::AddHitPoints)
			.def("AddSkillPoints", &GlobalParty::AddSkillPoints)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalEnemy, GlobalActor>("GlobalEnemy")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalObject>("GlobalObject")
			.def("GetID", &GlobalObject::GetID)
			.def("GetName", &GlobalObject::GetName)
			.def("GetType", &GlobalObject::GetObjectType)
			.def("GetCount", &GlobalObject::GetCount)
			.def("IncrementCount", &GlobalObject::IncrementCount)
			.def("DecrementCount", &GlobalObject::DecrementCount)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalItem, GlobalObject>("GlobalItem")
// 			.def(constructor<>(uint32, uint32))
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalWeapon, GlobalObject>("GlobalWeapon")
			.def("GetUsableBy", &GlobalWeapon::GetUsableBy)
// 			.def(constructor<>(uint32, uint32))
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalArmor, GlobalObject>("GlobalArmor")
			.def("GetUsableBy", &GlobalArmor::GetUsableBy)
// 			.def(constructor<>(uint32, uint32))
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalStatusEffect>("GlobalStatusEffect")
			.def("GetType", &GlobalStatusEffect::GetType)
			.def("GetIntensity", &GlobalStatusEffect::GetIntensity)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalElementalEffect>("GlobalElementalEffect")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalSkill>("GlobalSkill")
	];

	} // End using global namespaces

	// ---------- Bind GUI Components
	{
	using namespace hoa_gui;
	using namespace hoa_gui::private_gui;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_gui")
	[
		class_<GUIElement>("GUIElement")
			.def("SetDimensions", &GUIElement::SetDimensions)
			.def("SetPosition", &GUIElement::SetPosition)
			.def("SetAlignment", &GUIElement::SetAlignment),

		class_<GUIControl, GUIElement>("GUIControl")
			.def("SetOwner", &GUIControl::SetOwner),

		class_<TextBox, GUIControl>("TextBox")
			.def(constructor<>())
			.def(constructor<float, float, float, float, TEXT_DISPLAY_MODE>())
			.def("ClearText", &TextBox::ClearText)
			.def("Update", &TextBox::Update)
			.def("Draw", &TextBox::Draw)
			.def("ForceFinish", &TextBox::ForceFinish)
			.def("SetDimensions", &TextBox::SetDimensions)
			.def("SetTextAlignment", &TextBox::SetTextAlignment)
			.def("SetTextStyle", &TextBox::SetTextStyle)
			.def("SetDisplayMode", &TextBox::SetDisplayMode)
			.def("SetDisplaySpeed", &TextBox::SetDisplaySpeed)
			.def("SetDisplayText", (void(TextBox::*)(const std::string&))&TextBox::SetDisplayText)
			.def("GetTextAlignment", &TextBox::GetTextAlignment)
			.def("GetTextStyle", &TextBox::GetTextStyle)
			.def("GetDisplayMode", &TextBox::GetDisplayMode)
			.def("GetDisplaySpeed", &TextBox::GetDisplaySpeed)
			.def("IsFinished", &TextBox::IsFinished)
			.def("IsEmpty", &TextBox::IsEmpty)
			.def("IsInitialized", &TextBox::IsInitialized)
			.def("CalculateTextHeight", &TextBox::CalculateTextHeight)

			.enum_("constants") [
				value("VIDEO_TEXT_INSTANT", VIDEO_TEXT_INSTANT),
				value("VIDEO_TEXT_CHAR", VIDEO_TEXT_CHAR),
				value("VIDEO_TEXT_FADELINE", VIDEO_TEXT_FADELINE),
				value("VIDEO_TEXT_FADECHAR", VIDEO_TEXT_FADECHAR),
				value("VIDEO_TEXT_REVEAL", VIDEO_TEXT_REVEAL)
			]
	];

	} // End using gui namespace

	// Bind the GlobalManager object to Lua
	luabind::object global_table = luabind::globals(hoa_script::ScriptManager->GetGlobalState());
	global_table["GlobalManager"] = hoa_global::GlobalManager;
} // void BindCommonCode()

} // namespace hoa_defs
