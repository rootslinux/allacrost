------------------------------------------------------------------------------[[
-- Filename: customs.lua
--
-- Description: All tests in this file create an instance of CustomMode. The character
-- party and their equipment, amount of drunes, and inventory is configured along
-- with the shop properties such as buy/sell prices, available wares, and stocks.
-- These tests are commonly used to test specific shop configurations that will be
-- seen in the game.

------------------------------------------------------------------------------]]

local ns = {}
setmetatable(ns, {__index = _G})
customs = ns;
setfenv(1, ns);

-- Test IDs 5,001 - 6,000 are reserved for customs
tests = {}

tests[5001] = {
	name = "Screen Display - Single Line";
	description = "Displays a background image and single line of text, then fades the screen out and starts a map";
	ExecuteTest = function()
		local custom = hoa_custom.CustomMode("lua/scripts/custom/screen_display.lua");
		custom:AddOption("image", "img/backdrops/boot_screen00.jpg");
		custom:AddOption("text1", "Several days later...");
		custom:AddOption("initial_time", "1000");
		custom:AddOption("display_time", "2000");
		custom:AddOption("map", "lua/scripts/maps/a01_opening_scene.lua");
		ModeManager:Push(custom);
	end
}
