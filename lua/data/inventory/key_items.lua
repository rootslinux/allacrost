------------------------------------------------------------------------------[[
-- Filename: key_items.lua
--
-- Description: This file contains the definitions of all key items that exist in
-- Hero of Allacrost. Each key item has a unique integer identifier that is used
-- as its key in the key_items table below. Key item IDs are unique not only among
-- each other, but among other inventory game objects as well (items, weapons, armor,
-- etc).
--
-- Object IDs 70,001 through 80,000 are reserved for items. Do not break this 
-- limit, because other value ranges correspond to other types of inventory objects.
--
-- Key item IDs do -not- need to be sequential. When you make a new key item, keep it 
-- grouped with items that are similar (used for the same map or quest, for example).
--
-- All item entries needs the following data to be defined:
-- {name}: Text that defines the name of the item.
-- {description}: A brief description about the item.
-- {icon}: The filepath to the image icon representing this icon.
--
-- Key items do not do anything by themselves. Rather, their presence or absence
-- in the player's inventory is checked throughout the game to determine if some
-- event should happen or other change in the game state should take place. For
-- example, a certain door key must be held by the player to access a dungeon.
------------------------------------------------------------------------------]]

-- All item definitions are stored in this table
if (key_items == nil) then
	key_items = {}
end


--------------------------------------------------------------------------------
-- IDs 70,001 - 80,000 are reserved for key items
--------------------------------------------------------------------------------



