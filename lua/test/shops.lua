------------------------------------------------------------------------------[[
-- Filename: shops.lua
--
-- Description: All tests in this file create an instance of ShopMode. The character
-- party and their equipment, amount of drunes, and inventory is configured along
-- with the shop properties such as buy/sell prices, available wares, and stocks.
-- These tests are commonly used to test specific shop configurations that will be
-- seen in the game.
--
-- Note: To keep the tests in this file organized, reserve the first 50 tests (3001-3050)
-- for tests intended to test ShopMode itself. Tests primarily intended for shop balancing
-- should follow after this set. Try to keep tests sorted in order from more basic to more
-- advanced, or from early-game shops to late-game shops.
------------------------------------------------------------------------------]]

local ns = {}
setmetatable(ns, {__index = _G})
shops = ns;
setfenv(1, ns);

-- Test IDs 3,001 - 4,000 are reserved for shops
tests = {}

-- Begin tests intended for ShopMode interface testing

tests[3001] = {
	name = "Basic Early Game Shop";
	description = "A shop containing items found in the beginning of the game along with Claudius as the only party character. " ..
		"This is used primarily as a test of the basic functionalities of shop mode.";
	ExecuteTest = function()
		GlobalManager:AddCharacter(1); -- Claudius
		GlobalManager:AddDrunes(100);
		GlobalManager:AddToInventory(1, 5);
		GlobalManager:AddToInventory(1001, 2);
		GlobalManager:AddToInventory(10001, 1);

		local shop = hoa_shop.ShopMode();
		shop:AddObject(1, 12);
		shop:AddObject(1001, 5);
		shop:AddObject(10001, 2);
		shop:AddObject(10002, 4);
		shop:AddObject(10003, 12);
		shop:AddObject(20002, 1);
		shop:AddObject(20003, 2);
		shop:AddObject(30001, 3);
		shop:AddObject(30002, 10);
		shop:AddObject(30003, 11);
		shop:AddObject(40001, 2);
		shop:AddObject(40002, 1);
		shop:AddObject(40003, 1);
		shop:AddObject(40004, 1);
		shop:AddObject(50001, 1);
		shop:AddObject(50002, 1);
		ModeManager:Push(shop); 
	end
}

-- Begin tests intended for shop balancing
