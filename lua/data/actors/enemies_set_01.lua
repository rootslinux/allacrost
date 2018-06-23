------------------------------------------------------------------------------[[
-- Filename: enemies_set01.lua
--
-- Description: This file contains the definitions of multiple foes that the
-- player encounters in battle. This file contains those enemies who have ids
-- from 1-100.
--
-- At the top of each enemy entry, list the set of general traits that the enemy
-- is to possess. For example, "weak physical attack, high agility". This is done
-- so that if others modify the enemy's properties later in balancing efforts, they
-- know the general properties that the enemy needs to continue to represent. Below
-- is an example of a header you can use.
--
-- Traits -----------------------------------------------------------------------
-- HSP: high HP, low SP
-- ATK: low phys, no eth
-- DEF: low phys, med eth
-- SPD: med agi, low eva
-- XPD: med XP, vlow drunes
--------------------------------------------------------------------------------
--
-- The three letter acronym categories mean the following
-- HSP = HP/SP
-- ATK = Attack Ratings (physical and ethereal)
-- DEF = Defense Ratings (physical and ethereal)
-- SPD = Speed Ratings (agility and evade)
-- XPD = Experience points and drunes rewarded
--
-- To stay consistent, use the following degree indicators for each stat:
-- {zero, vlow, low, med, high, vhigh}
------------------------------------------------------------------------------]]

-- All enemy definitions are stored in this table
-- check to see if the enemies table has already been created by another script
if (_G.enemies == nil) then
   enemies = {}
end


-- Traits -----------------------------------------------------------------------
-- HSP: low HP, vlow SP
-- ATK: low phys, zero eth
-- DEF: low phys, vlow eth
-- SPD: low agi, low eva
-- XPD: low XP, vlow drunes
--------------------------------------------------------------------------------
enemies[1] = {
	name = hoa_system.Translate("Green Slime"),
	filename = "green_slime",
	sprite_width = 64,
	sprite_height = 64,

	base_stats = {
		hit_points = 52,
		skill_points = 10,
		strength = 45,
		vigor = 0,
		fortitude = 10,
		protection = 10,
		agility = 25,
		evade = 2.0,
		experience_points = 5,
		drunes = 10
	},

	skills = {
		1001
	},

	drop_objects = {

	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: low HP, vlow SP
-- ATK: low phys, zero eth
-- DEF: med phys, low eth
-- SPD: low agi, low eva
-- XPD: low XP, low drunes
--------------------------------------------------------------------------------
enemies[2] = {
	name = hoa_system.Translate("Spider"),
	filename = "spider",
	sprite_width = 64,
	sprite_height = 64,

	base_stats = {
		hit_points = 52,
		skill_points = 10,
		strength = 25,
		vigor = 10,
		fortitude = 10,
		protection = 10,
		agility = 30,
		evade = 2.0,
		experience_points = 6,
		drunes = 12
	},

	skills = {
		1002, 1006
	},

	drop_objects = {
		{ 1, 0.10 } -- Minor Healing Potion
	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: med HP, low SP
-- ATK: med phys, zero eth
-- DEF: med phys, low eth
-- SPD: med agi, low eva
-- XPD: low XP, med drunes
--------------------------------------------------------------------------------
enemies[3] = {
	name = hoa_system.Translate("Snake"),
	filename = "snake",
	sprite_width = 128,
	sprite_height = 64,

	base_stats = {
		hit_points = 60,
		skill_points = 10,
		strength = 50,
		vigor = 0,
		fortitude = 10,
		protection = 5,
		agility = 25,
		evade = 2.0,
		experience_points = 7,
		drunes = 14
	},

	skills = {
		1003
	},

	drop_objects = {
		{ 1, 0.10 } -- Minor Healing Potion
	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: med HP, low SP
-- ATK: med phys, zero eth
-- DEF: med phys, low eth
-- SPD: med agi, med eva
-- XPD: low XP, med drunes
--------------------------------------------------------------------------------
enemies[4] = {
	name = hoa_system.Translate("Rat"),
	filename = "rat",
	sprite_width = 64,
	sprite_height = 64,

	base_stats = {
		hit_points = 150,
		skill_points = 5,
		strength = 35,
		vigor = 0,
		fortitude = 10,
		protection = 5,
		agility = 45,
		evade = 5.0,
		experience_points = 5,
		drunes = 18
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Head"),
			x_position = -24,
			y_position = 50,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Chest"),
			x_position = -8,
			y_position = 25,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		}
	},

	skills = {
		1004
	},

	drop_objects = {
		{ 1, 0.15 } -- Minor Healing Potion
	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: low HP, low SP
-- ATK: med phys, zero eth
-- DEF: med phys, low eth
-- SPD: high agi, med eva
-- XPD: med XP, low drunes
--------------------------------------------------------------------------------
enemies[5] = {
	name = hoa_system.Translate("Scorpion"),
	filename = "scorpion",
	sprite_width = 64,
	sprite_height = 64,

	base_stats = {
		hit_points = 30,
		skill_points = 1,
		strength = 6,
		vigor = 0,
		fortitude = 10,
		protection = 5,
		agility = 40,
		evade = 2.0,
		experience_points = 8,
		drunes = 12
	},

	skills = {
		1005,1002
	},

	drop_objects = {
		{ 1, 0.15 } -- Minor Healing Potion
	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: med HP, low SP
-- ATK: low phys, zero eth
-- DEF: low phys, low eth
-- SPD: high agi, med eva
-- XPD: low XP, low drunes
--------------------------------------------------------------------------------
enemies[6] = {
	name = hoa_system.Translate("Bat"),
	filename = "bat",
	sprite_width = 64,
	sprite_height = 128,

	base_stats = {
		hit_points = 53,
		skill_points = 6,
		strength = 24,
		vigor = 0,
		fortitude = 10,
		protection = 5,
		agility = 60,
		evade = 45.0,
		experience_points = 8,
		drunes = 12
	},

	skills = {
		1002 -- TEMP: Uses spider bite attack until we have appropriate bat skill
	},

	drop_objects = {
		{ 1, 0.15 } -- Minor Healing Potion
	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: ??? HP, ??? SP
-- ATK: ??? phys, ??? eth
-- DEF: ??? phys, ??? eth
-- SPD: ??? agi, ??? eva
-- XPD: ??? XP, ??? drunes
--------------------------------------------------------------------------------
enemies[7] = {
	name = hoa_system.Translate("Dune Crawler"),
	filename = "dune_crawler",
	sprite_width = 64,
	sprite_height = 64,

	base_stats = {
		hit_points = 122,
		skill_points = 10,
		strength = 18,
		vigor = 0,
		fortitude = 12,
		protection = 4,
		agility = 14,
		evade = 2.0,
		experience_points = 8,
		drunes = 12
	},

	skills = {
		1002
	},

	drop_objects = {

	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: med HP, med SP
-- ATK: high phys, zero eth
-- DEF: high phys, low eth
-- SPD: low agi, low eva
-- XPD: med XP, med drunes
--------------------------------------------------------------------------------
enemies[8] = {
	name = hoa_system.Translate("Skeleton"),
	filename = "skeleton",
	sprite_width = 64,
	sprite_height = 128,

	base_stats = {
		hit_points = 124,
		skill_points = 10,
		strength = 15,
		vigor = 0,
		fortitude = 14,
		protection = 4,
		agility = 13,
		evade = 2.0,
		experience_points = 5,
		drunes = 18
	},

	skills = {
		1004
	},

	drop_objects = {
		{ 1, 0.15 } -- Minor Healing Potion
	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: high HP, med SP
-- ATK: med phys, zero eth
-- DEF: med phys, low eth
-- SPD: low agi, low eva
-- XPD: med XP, high drunes
--------------------------------------------------------------------------------
enemies[9] = {
	name = hoa_system.Translate("Stygian Lizard"),
	filename = "stygian_lizard",
	sprite_width = 192,
	sprite_height = 192,

	base_stats = {
		hit_points = 124,
		skill_points = 10,
		strength = 15,
		vigor = 0,
		fortitude = 14,
		protection = 4,
		agility = 13,
		evade = 2.0,
		experience_points = 5,
		drunes = 18
	},

	skills = {
		1004
	},

	drop_objects = {

	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: med HP, med SP
-- ATK: low phys, med eth
-- DEF: low phys, med eth
-- SPD: med agi, med eva
-- XPD: med XP, med drunes
--------------------------------------------------------------------------------
enemies[10] = {
	name = hoa_system.Translate("Demonic Essence"),
	filename = "demonic_essence",
	sprite_width = 128,
	sprite_height = 192,

	base_stats = {
		hit_points = 124,
		skill_points = 10,
		strength = 15,
		vigor = 0,
		fortitude = 14,
		protection = 4,
		agility = 13,
		evade = 2.0,
		experience_points = 5,
		drunes = 18
	},

	skills = {
		1004
	},

	drop_objects = {

	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: vhigh HP, med SP
-- ATK: high phys, zero eth
-- DEF: high phys, low eth
-- SPD: med agi, low eva
-- XPD: vhigh XP, high drunes
-- Notes: First boss in prologue module
--------------------------------------------------------------------------------
enemies[91] = {
	name = hoa_system.Translate("Scorpion Goliath"),
	filename = "scorpion_goliath",
	sprite_width = 512,
	sprite_height = 448,

	base_stats = {
		hit_points = 500,
		skill_points = 45,
		strength = 55,
		vigor = 0,
		fortitude = 20,
		protection = 5,
		agility = 25,
		evade = 3.0,
		experience_points = 242,
		drunes = 135
	},

	skills = {
		1002 -- TEMP until specific boss skills available
	},

	drop_objects = {

	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: high HP, med SP
-- ATK: med phys, zero eth
-- DEF: med phys, low eth
-- SPD: med agi, low eva
-- XPD: high XP, high drunes
-- Notes: Second boss in prologue module
--------------------------------------------------------------------------------
enemies[92] = {
	name = hoa_system.Translate("Armored Beast"),
	filename = "armored_beast",
	sprite_width = 256,
	sprite_height = 256,

	base_stats = {
		hit_points = 122,
		skill_points = 10,
		strength = 10,
		vigor = 0,
		fortitude = 10,
		protection = 4,
		agility = 30,
		evade = 18.0,
		experience_points = 8,
		drunes = 12
	},

	skills = {

	},

	drop_objects = {

	}
}

-- Traits -----------------------------------------------------------------------
-- Testing enemy
-- very low hp, returns a huge amount of xp.
--------------------------------------------------------------------------------
enemies[93] = {
	name = hoa_system.Translate("Test Green Slime"),
	filename = "green_slime",
	sprite_width = 64,
	sprite_height = 64,

	base_stats = {
		hit_points = 5,
		skill_points = 10,
		strength = 45,
		vigor = 0,
		fortitude = 10,
		protection = 10,
		agility = 25,
		evade = 2.0,
		experience_points = 1800,
		drunes = 10
	},

	skills = {
		1001
	},

	drop_objects = {

	}
}
