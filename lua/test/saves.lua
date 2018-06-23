------------------------------------------------------------------------------[[
-- Filename: saves.lua
--
-- Description: This file contains all tests that create an instance of SaveMode
-- with various configurations. A test may either save a game or load a game, but
-- it is not recommended to do both in the same test. Saved game files must exist
-- in order to run any of the loading tests successfully.
------------------------------------------------------------------------------]]

local ns = {}
setmetatable(ns, {__index = _G})
saves = ns;
setfenv(1, ns);

-- Test IDs 4,001 - 5,000 are reserved for saves
tests = {}

