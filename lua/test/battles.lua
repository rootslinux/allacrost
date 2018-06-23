------------------------------------------------------------------------------[[
-- Filename: battle.lua
--
-- Description: This file contains all tests that create an instance of BattleMode
-- with various configurations. The tests here are primarily for testing different
-- enemy parties and character strengths.
--
-- Note: To keep the tests in this file organized, reserve the first 100 tests (3001-3100)
-- for tests intended to test BattleMode itself. Tests primarily intended for balancing
-- should follow after this set. Try to keep tests sorted in order from more basic to more
-- advanced, or from early-game to late-game battles.
------------------------------------------------------------------------------]]

local ns = {}
setmetatable(ns, {__index = _G})
battles = ns;
setfenv(1, ns);

-- Test IDs 1,001 - 2,000 are reserved for battles
tests = {}

-- Begin tests intended for BattleMode interface and features

tests[1001] = {
	name = "Early Game Battle";
	description = "The initial character party in the game against four early game enemies. A handful of potions are available " ..
		"in the party inventory as well.";
	ExecuteTest = function()
		GlobalManager:SetBattleSetting(hoa_global.GameGlobal.GLOBAL_BATTLE_WAIT);
		GlobalManager:AddCharacter(1); -- Claudius
		GlobalManager:AddCharacter(2); -- Mark
		GlobalManager:AddCharacter(4); -- Lukar
		GlobalManager:AddToInventory(1, 3); -- Minor healing potions

		local battle = hoa_battle.BattleMode();
		battle:AddEnemy(2);
		battle:AddEnemy(2);
		battle:AddEnemy(3);
		battle:AddEnemy(5);
		ModeManager:Push(battle);
	end
}

tests[1002] = {
	name = "Fatigue Early Game Battle";
	description = "The same battle as the previous one, but the characters begin the battle with a significant amount of fatigue " ..
		"and low stamina/resilience for testing fatigue's effects.";
	ExecuteTest = function()
		GlobalManager:SetBattleSetting(hoa_global.GameGlobal.GLOBAL_BATTLE_WAIT);
		GlobalManager:AddCharacter(1); -- Claudius
		GlobalManager:AddCharacter(2); -- Mark
		GlobalManager:AddCharacter(4); -- Lukar
		GlobalManager:AddToInventory(1, 3); -- Minor healing potions

		local claudius = GlobalManager:GetCharacter(1);
		local mark = GlobalManager:GetCharacter(2);
		local lukar = GlobalManager:GetCharacter(4);
		claudius:SubtractHitPoints(15);
		claudius:SubtractSkillPoints(6);
		claudius:SetStamina(2);
		claudius:SetResilience(1);
		claudius:AddHitPointFatigue(20);
		claudius:AddSkillPointFatigue(4);

		mark:SubtractHitPoints(37);
		mark:SubtractSkillPoints(5);
		mark:SetStamina(3);
		mark:SetResilience(1);
		mark:AddHitPointFatigue(30);
		mark:AddSkillPointFatigue(3);

		lukar:SubtractHitPoints(62);
		lukar:SubtractSkillPoints(12);
		lukar:SetStamina(4);
		lukar:SetResilience(2);
		lukar:AddHitPointFatigue(40);
		lukar:AddSkillPointFatigue(9);

		local battle = hoa_battle.BattleMode();
		battle:AddEnemy(2);
		battle:AddEnemy(2);
		battle:AddEnemy(3);
		battle:AddEnemy(5);
		ModeManager:Push(battle);
	end
}

tests[1003] = {
	name = "Victory Screen";
	description = "The character fights against one monster with low hp, which will return enough xp for the character to " ..
		"level up.";
	ExecuteTest = function()
		GlobalManager:SetBattleSetting(hoa_global.GameGlobal.GLOBAL_BATTLE_WAIT);
		GlobalManager:AddCharacter(1); -- Claudius

		local claudius = GlobalManager:GetCharacter(1);

		local battle = hoa_battle.BattleMode();
		battle:AddEnemy(93);

		ModeManager:Push(battle);
	end
}

-- Begin tests intended for battle balancing
