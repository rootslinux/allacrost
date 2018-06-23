------------------------------------------------------------------------------[[
-- Filename: maps.lua
--
-- Description: This file contains all tests that create an instance of MapMode.
-- The tests here are primarily for testing map files and their scripted code.
-- Generally it is a good idea to create a test for each map file, or possibly
-- multiple tests for a file so that it is easy to get to a certain point in the
-- just before an important event or sequence, such as a boss encounter.
--
-- Note: It is strongly advised to state the name of the map that will be opened
-- in the test name. This makes the life of the user much easier. Recommended format:
-- "[Name Of Map] - Configuration Details".
------------------------------------------------------------------------------]]

local ns = {}
setmetatable(ns, {__index = _G})
maps = ns;
setfenv(1, ns);

-- Character IDs. Each ID can have only a single bit active as IDs are used in bitmask operations.
CLAUDIUS  = 1;
MARK      = 2;
LUKAR     = 4;

--------------------------------------------------------------------------------
-- Common Functions
--------------------------------------------------------------------------------

-- Sets the map location and creates and pushes the MapMode instance
function StartMap(location)
	GlobalManager:SetLocation(location);
	local map = hoa_map.MapMode(location);
	ModeManager:Push(map);
end

-- Test IDs 1 - 1,000 are reserved for maps
tests = {}

tests[1] = {
	name = "[01_opening_scene] - New Game";
	description = "Equivalent to selecting the \"New Game\" option on the boot screen.";
	ExecuteTest = function()
		GlobalManager:AddCharacter(LUKAR);
		GlobalManager:AddCharacter(MARK);
		GlobalManager:AddCharacter(CLAUDIUS);
		GlobalManager:AddNewRecordGroup("global_records");
		GlobalManager:SetDrunes(100);
		GlobalManager:AddToInventory(1, 4);

		-- When selecting new game in boot mode, the screen will turn black before the map is loaded
		VideoManager:FadeScreen(hoa_video.Color(0.0, 0.0, 0.0, 1.0), 0);
		StartMap("lua/scripts/maps/a01_opening_scene.lua");
	end
}


tests[2] = {
	name = "[01_unblock_underground_river] - New Game Data";
	description = "Places the user in the first dungeon with the party and status that the player receives upon beginning a new game. " ..
		"Effectively, this test is a way to start a new game and skip over the opening scene.";
	ExecuteTest = function()
		GlobalManager:AddCharacter(LUKAR);
		GlobalManager:AddCharacter(MARK);
		GlobalManager:AddCharacter(CLAUDIUS);
		GlobalManager:AddNewRecordGroup("global_records");
		GlobalManager:SetDrunes(100);
		GlobalManager:AddToInventory(1, 4);

		StartMap("lua/scripts/maps/a01_unblock_underground_river.lua");
	end
}

-- tests[3]: Reserved for a test of s01_unblock_underground_river that spawns the player just before the boss battle at the end of the dungeon


tests[4] = {
	name = "[01_return_scene] - Return to City";
	description = "The map scene that takes place after the events in the cave when the group of knights is returning to the city";
	ExecuteTest = function()

		GlobalManager:AddCharacter(LUKAR);
		GlobalManager:AddCharacter(MARK);
		GlobalManager:AddCharacter(CLAUDIUS);
		GlobalManager:AddNewRecordGroup("global_records");
		GlobalManager:SetDrunes(200);
		GlobalManager:AddToInventory(1, 5);

		StartMap("lua/scripts/maps/a01_return_scene.lua");
	end
}


tests[5] = {
	name = "[01_harrvah_capital_attack] - Harrvah Capital Under Attack";
	description = "Places the player at the entrance to Harrvah for the first visit to the city. This is when Claudius is " ..
		"joined by other knights returning from their mission to the cave, where they find the city under attack by the " ..
		"demonic forces.";

	ExecuteTest = function()
		GlobalManager:AddCharacter(LUKAR);
		GlobalManager:AddCharacter(MARK);
		GlobalManager:AddCharacter(CLAUDIUS);
		GlobalManager:AddNewRecordGroup("global_records");
		GlobalManager:SetDrunes(200);
		GlobalManager:AddToInventory(1, 5);

		StartMap("lua/scripts/maps/a01_harrvah_capital_attack.lua");
	end
}


tests[6] = {
	name = "[01_harrvah_capital_aftermath] - Harrvah Capital Attack Aftermath";
	description = "Harrvah after the attack scene";

	ExecuteTest = function()
		-- TODO: below is temporary. Replace when the a01_harrvah_captial_aftermath.lua map is available
		GlobalManager:AddCharacter(LUKAR);
		GlobalManager:AddCharacter(MARK);
		GlobalManager:AddCharacter(CLAUDIUS);
		GlobalManager:AddNewRecordGroup("global_records");
		GlobalManager:SetDrunes(200);
		GlobalManager:AddToInventory(1, 5);

		StartMap("lua/scripts/maps/a01_harrvah_capital_aftermath.lua");
	end
}


tests[7] = {
	name = "[01_sand_dock_depature] - Harrvah Capital Sand Dock";
	description = "Claudius departs for his journey by leaving Harrvah through the sand dock of Harrvah.";

	ExecuteTest = function()
		GlobalManager:AddCharacter(CLAUDIUS);
		GlobalManager:AddNewRecordGroup("global_records");
		GlobalManager:SetDrunes(200);
		GlobalManager:AddToInventory(1, 5);

		StartMap("lua/scripts/maps/a01_sand_dock_departure.lua");
	end
}

tests[8] = {
	name = "[graphics_test] - Graphics Test Map";
	description = "A debugging map used for diagnosing graphical issues";

	ExecuteTest = function()
		GlobalManager:AddCharacter(CLAUDIUS);
		GlobalManager:AddNewRecordGroup("global_records");
		GlobalManager:SetDrunes(200);
		GlobalManager:AddToInventory(1, 5);

		StartMap("lua/scripts/maps/graphics_test.lua");
	end
}

