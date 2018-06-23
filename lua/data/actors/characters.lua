------------------------------------------------------------------------------[[
-- Filename: characters.lua
--
-- Description: This file contains the definitions of all characters that exist
-- in Hero of Allacrost. When a new character is added to the party, this file
-- is accessed and the character is created using the data loaded from this file.
------------------------------------------------------------------------------]]

-- All character definitions are stored in this table
characters = {}

characters[CLAUDIUS] = {
	name = hoa_system.Translate("Claudius"),
	filename = "claudius",

	initial_stats = {
		experience_level = 3,
		experience_points = 0,
		max_hit_points = 100,
		max_skill_points = 10,
		strength = 15,
		vigor = 4,
		fortitude = 15,
		protection = 6,
		stamina = 5,
		resilience = 5,
		agility = 30,
		evade = 5.0,
		weapon = 10001,
		head_armor = 20001,
		torso_armor = 30001,
		arm_armor = 40001,
		leg_armor = 50001
	},

	-- Begin character growth tables. Every line within these tables contains 10 elements to represent the stat growth for every 10 levels
	growth = {
		experience_for_next_level = {
			100, 112, 126, 142, 161, 183, 209, 238, 273, 315,
			363, 421, 490, 572, 670, 788, 931, 1105, 1316, 1575
		},

		hit_points = {
			5, 5, 5, 5, 13, 13, 13, 13, 21, 21,
			21, 21, 29, 29, 29, 29, 37, 37, 37, 37
		},

		skill_points = {
			1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
			3, 3, 3, 3, 3, 4, 4, 4, 4, 4
		},

		strength = {
			2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
			3, 3, 4, 4, 4, 4, 4, 4, 5, 5
		},

		vigor = {
			1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
			2, 2, 2, 2, 3, 3, 3, 3, 3, 3
		},

		fortitude = {
			2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
			3, 3, 4, 4, 4, 4, 4, 4, 5, 5
		},

		protection = {
			1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
			2, 2, 2, 2, 3, 3, 3, 3, 3, 3
		},

		stamina = {
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0
		},

		resilience = {
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0
		},

		agility = {
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0
		},

		evade = {
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0
		}
	},

	skills = { [1] = 1, [2] = 2, [3] = 10001, [8] = 3, [12] = 20001, [15] = 4 }
} -- characters[CLAUDIUS]



characters[MARK] = {
	name = hoa_system.Translate("Mark"),
	filename = "knight",

	initial_stats = {
		experience_level = 7,
		experience_points = 1128,
		max_hit_points = 150,
		max_skill_points = 15,
		strength = 30,
		vigor = 8,
		fortitude = 10,
		protection = 10,
		stamina = 6,
		resilience = 5,
		agility = 35,
		evade = 5.2,
		weapon = 10002,
		head_armor = 20002,
		torso_armor = 30002,
		arm_armor = 40002,
		leg_armor = 50001
	},

	-- Begin character growth tables. Every line within these tables contains 10 elements to represent the stat growth for every 10 levels
	growth = {
		experience_for_next_level = {
			100, 112, 126, 142, 161, 183, 209, 238, 273, 315,
			363, 421, 490, 572, 670, 788, 931, 1105, 1316, 1575
		},

		hit_points = {
			5, 5, 5, 5, 13, 13, 13, 13, 21, 21,
			21, 21, 29, 29, 29, 29, 37, 37, 37, 37
		},

		skill_points = {
			1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
			3, 3, 3, 3, 3, 4, 4, 4, 4, 4
		},

		strength = {
			2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
			3, 3, 4, 4, 4, 4, 4, 4, 5, 5
		},

		vigor = {
			1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
			2, 2, 2, 2, 3, 3, 3, 3, 3, 3
		},

		fortitude = {
			2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
			3, 3, 4, 4, 4, 4, 4, 4, 5, 5
		},

		protection = {
			1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
			2, 2, 2, 2, 3, 3, 3, 3, 3, 3
		},

		stamina = {
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0
		},

		resilience = {
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0
		},

		agility = {
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0
		},

		evade = {
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0
		}
	},

	skills = { [1] = 1, [2] = 5, [3] = 3, [4] = 10001 }
} -- characters[MARK]



characters[LUKAR] = {
	name = hoa_system.Translate("Lukar"),
	filename = "knight",

	initial_stats = {
		experience_level = 18,
		experience_points = 6728,
		max_hit_points = 200,
		max_skill_points = 25,
		strength = 65,
		vigor = 21,
		fortitude = 20,
		protection = 23,
		stamina = 10,
		resilience = 5,
		agility = 40,
		evade = 7.0,
		weapon = 10003,
		head_armor = 20003,
		torso_armor = 30003,
		arm_armor = 40003,
		leg_armor = 50002
	},

	-- Begin character growth tables. Every line within these tables contains 10 elements to represent the stat growth for every 10 levels
	growth = {
		experience_for_next_level = {
			100, 112, 126, 142, 161, 183, 209, 238, 273, 315,
			363, 421, 490, 572, 670, 788, 931, 1105, 1316, 1575
		},

		hit_points = {
			5, 5, 5, 5, 13, 13, 13, 13, 21, 21,
			21, 21, 29, 29, 29, 29, 37, 37, 37, 37
		},

		skill_points = {
			1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
			3, 3, 3, 3, 3, 4, 4, 4, 4, 4
		},

		strength = {
			2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
			3, 3, 4, 4, 4, 4, 4, 4, 5, 5
		},

		vigor = {
			1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
			2, 2, 2, 2, 3, 3, 3, 3, 3, 3
		},

		fortitude = {
			2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
			3, 3, 4, 4, 4, 4, 4, 4, 5, 5
		},

		protection = {
			1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
			2, 2, 2, 2, 3, 3, 3, 3, 3, 3
		},

		stamina = {
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0
		},

		resilience = {
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0
		},

		agility = {
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
			1, 0, 1, 0, 1, 0, 1, 0, 1, 0
		},

		evade = {
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0
		}
	},

	skills = { [1] = 1, [3] = 2, [4] = 10001, [8] = 3, [12] = 20001, [15] = 4, [17] = 20002 }
} -- characters[LUKAR]


------------------------------------------------------------------------------[[
-- \brief Sets the growth data for a character when they have gained a level
-- \param character A pointer to the GlobalCharacter object to act on
--
-- Before this function is called, the character should already have their
-- _experience_level member incremented to the new level. What this function does
-- is determine the amount that each stat will grow by on the -next- level gained
-- (-not- the current level gained). This function should be called every time a
-- character gains a level, and also when a new character is constructed from an
-- initial state.
------------------------------------------------------------------------------]]
function DetermineNextLevelGrowth(character)
	local new_level = character:GetExperienceLevel();        -- The value of the character's new XP level
	local character_table = characters[character:GetID()];   -- Reference to the character's definition table
	local growth_table = nil;                                -- Reference to the table containing the character's growth stats

	if (character_table == nil) then
		print("LUA ERROR: characters.lua::DetermineNextLevelGrowth() failed because the character's ID was invalid");
		return;
	end

	growth_table = character_table["growth"];
	if (growth_table == nil) then
		print("LUA ERROR: characters.lua::DetermineNextLevelGrowth() failed because no growth table for the character was found");
		return;
	end

	character:AddExperienceForNextLevel(growth_table["experience_for_next_level"][new_level]);

	-- All growth members should be zero when this function is called. Warn if this is not the case
	if (character._hit_points_growth ~= 0) then
		print("LUA WARN: character.lua:DetermineNextLevelGrowth() called when hit_points_growth was non-zero.");
	end
	if (character._skill_points_growth ~= 0) then
		print("LUA WARN: character.lua:DetermineNextLevelGrowth() called when skill_points_growth was non-zero.");
	end
	if (character._strength_growth ~= 0) then
		print("LUA WARN: character.lua:DetermineNextLevelGrowth() called when strength_growth was non-zero.");
	end
	if (character._vigor_growth ~= 0) then
		print("LUA WARN: character.lua:DetermineNextLevelGrowth() called when vigor_growth was non-zero.");
	end
	if (character._fortitude_growth ~= 0) then
		print("LUA WARN: character.lua:DetermineNextLevelGrowth() called when fortitude_growth was non-zero.");
	end
	if (character._protection_growth ~= 0) then
		print("LUA WARN: character.lua:DetermineNextLevelGrowth() called when protection_growth was non-zero.");
	end
	if (character._agility_growth ~= 0) then
		print("LUA WARN: character.lua:DetermineNextLevelGrowth() called when agility_growth was non-zero.");
	end
	if (character._evade_growth ~= 0) then
		print("LUA WARN: character.lua:DetermineNextLevelGrowth() called when evade_growth was non-zero.");
	end

	-- Copy over the character's stat growth data
	character._hit_points_growth = growth_table["hit_points"][new_level];
	character._skill_points_growth = growth_table["skill_points"][new_level];
	character._strength_growth = growth_table["strength"][new_level];
	character._vigor_growth = growth_table["vigor"][new_level];
	character._fortitude_growth = growth_table["fortitude"][new_level];
	character._protection_growth = growth_table["protection"][new_level];
	character._stamina_growth = growth_table["stamina"][new_level];
	character._resilience_growth = growth_table["resilience"][new_level];
	character._agility_growth = growth_table["agility"][new_level];
	character._evade_growth = growth_table["evade"][new_level];
end -- function DetermineNextLevelGrowth(character)


------------------------------------------------------------------------------[[
-- \brief Adds any learned skills for a character based on their current experience level
-- \param character A pointer to the GlobalCharacter object to act on
--
-- Before this function is called, the character should already have their
-- _experience_level member incremented to the new level. What this function does
-- is determine the amount that each stat will grow by on the -next- level gained
-- (-not- the current level gained) and if any new skills will be learned by
-- reaching this level.
------------------------------------------------------------------------------]]
function DetermineNewSkillsLearned(character)
	local new_level = character:GetExperienceLevel();	      -- The value of the character's new XP level
	local character_table = characters[character:GetID()];  -- Reference to the character's definition table
	local new_skills = nil;                                 -- Reference to the number or table of the new skills learned

	if (character_table == nil) then
		print("LUA ERROR: characters.lua::DeterminedNewSkillsLearned() failed because the character's ID was invalid");
		return;
	end

	new_skills = character_table["skills"][new_level]

	-- Case 1: no new skills are learned
	if (type(new_skills) == nil) then
		return;
	-- Case 2: one new skill is learned
	elseif (type(new_skills) == "number") then
		character:AddNewSkillLearned(new_skills);
	-- Case 3: multiple new skills are learned
	elseif (type(new_skills) == "table") then
		for i, skill in ipairs(new_skills) do
			character:AddNewSkillLearned(skill);
		end
	else
		print("LUA ERROR: characters.lua::DetermineNewSkillsLearned() failed because of an unexpected skill table key type");
	end
end -- function DetermineSkillsLearned(character)
