--------------------------------------------------------------------------------
-- global.lua
--
-- This script is opened as the application is initializing and remains open until
-- it exits. It contains various constants and functions that are used by numerous
-- other scripts. The contents of this file are not wrapped in a tablespace, so one
-- must take care not to override these names.
--------------------------------------------------------------------------------

-- Character IDs. Each ID can have only a single bit active as IDs are used in bitmask operations.
CLAUDIUS  = 1;
MARK      = 2;
LUKAR     = 4;

-- Prints a message to the command line if the debug variable evaluates to true.
-- This function mimics the IF_PRINT_DEBUG macro used in the C++ code.
--     debug: A variable (usually a boolean) to evaluate if the message should be printed or not
--     messsage: The string to print
function IfPrintDebug(debug, message)
	-- TODO: figure out if there is a convenient means of retrieving the filename, function, and line number
	-- of whatever code invoked this function
	if (debug) then
		print(message);
	end
end


-- A utility function which converts any number to an integer
--     value: A numeric value to round to an integer. May be positive or negative
function RoundToInteger(value)
	return value >= 0 and math.floor(value + 0.5) or math.ceil(value - 0.5);
end


-- Begins a new game. Called from boot mode to begin
function NewGame()
	-- Make sure that any global data is cleared away
	GlobalManager:ClearAllData();

	-- Create the initial party, drunes, and inventory
	GlobalManager:AddCharacter(LUKAR);
	GlobalManager:AddCharacter(MARK);
	GlobalManager:AddCharacter(CLAUDIUS);
	GlobalManager:AddNewRecordGroup("global_events"); -- This group stores the primary list of events completed in the game
	GlobalManager:SetDrunes(100);
	GlobalManager:AddToInventory(1, 3); -- Healing potions

	-- Set the location, load the opening map and add it to the game stack, and remove boot mode from the game stack
	local location_name = "lua/scripts/maps/a01_opening_scene.lua"
	GlobalManager:SetLocation(location_name);
	local opening_map = hoa_map.MapMode(location_name);

	ModeManager:Pop();
	ModeManager:Push(opening_map);
end


-- Removes the current game mode and pushes a new MapMode instance with the chosen map
--     map_name: The filename of the map file (usually of the form: lua/scripts/maps/my_map.lua)
function LoadNewMap(map_name)
	ModeManager:Pop();
	local new_map = hoa_map.MapMode(map_name);
	ModeManager:Push(new_map);
end


--
function LoadNewShop(...)
	local i, v, item;
	local shop = hoa_shop.ShopMode();
	for i,v in ipairs(arg) do
		if (i % 2 == 1) then
			item = v
		else
			shop:AddObject(item, v)
		end
	end
	ModeManager:Push(shop);
end

