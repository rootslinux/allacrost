------------------------------------------------------------------------------[[
-- Filename: enemies_set02.lua
--
-- Description: This file contains the definitions of multiple foes that the
-- player encounters in battle. This file contains those enemies who have ids
-- from 101-200.
------------------------------------------------------------------------------]]

-- All enemy definitions are stored in this table
if (_G.enemies == nil) then
   enemies = {}
end

enemies[101] = {
	name = hoa_system.Translate("Daemarbora"),
	filename = "daemarbora",
	sprite_width = 128,
	sprite_height = 128,

	initial_stats = {
		hit_points = 80,
		skill_points = 20,
		experience_points = 15,
		strength = 12,
		vigor = 20,
		fortitude = 15,
		protection = 7,
		agility = 8,
		evade = 1.0,
		drunes = 55
	},

	skills = {

	},

	drop_objects = {

	}
}


enemies[102] = {
	name = hoa_system.Translate("Aerocephal"),
	filename = "aerocephal",
	sprite_width = 192,
	sprite_height = 192,

	initial_stats = {
		hit_points = 90,
		skill_points = 10,
		experience_points = 12,
		strength = 8,
		vigor = 0,
		fortitude = 7,
		protection = 4,
		agility = 20,
		evade = 10.0,
		drunes = 60
	},

	skills = {

	},

	drop_objects = {

	}
}


enemies[103] = {
	name = hoa_system.Translate("Arcana Drake"),
	filename = "arcana_drake",
	sprite_width = 192,
	sprite_height = 256,

	initial_stats = {
		hit_points = 85,
		skill_points = 10,
		experience_points = 45,
		strength = 20,
		vigor = 0,
		fortitude = 8,
		protection = 5,
		agility = 15,
		evade = 2.0,
		drunes = 80
	},

	skills = {

	},

	drop_objects = {

	}
}


enemies[104] = {
	name = hoa_system.Translate("Nagaruda"),
	filename = "nagaruda",
	sprite_width = 192,
	sprite_height = 256,

	initial_stats = {
		hit_points = 90,
		skill_points = 10,
		experience_points = 18,
		strength = 10,
		vigor = 0,
		fortitude = 8,
		protection = 4,
		agility = 12,
		evade = 2.0,
		drunes = 70
	},

	skills = {

	},

	drop_objects = {

	}
}


enemies[105] = {
	name = hoa_system.Translate("Deceleon"),
	filename = "deceleon",
	sprite_width = 256,
	sprite_height = 256,

	initial_stats = {
		hit_points = 100,
		skill_points = 10,
		experience_points = 18,
		strength = 15,
		vigor = 0,
		fortitude = 20,
		protection = 4,
		agility = 4,
		evade = 1.0,
		drunes = 85
	},

	skills = {

	},

	drop_objects = {

	}
}


enemies[106] = {
	name = hoa_system.Translate("Aurum Drakueli"),
	filename = "aurum-drakueli",
	sprite_width = 320,
	sprite_height = 256,


	initial_stats = {
		hit_points = 120,
		skill_points = 10,
		experience_points = 20,
		strength = 22,
		vigor = 0,
		fortitude = 8,
		protection = 4,
		agility = 18,
		evade = 2.0,
		drunes = 100
	},

	skills = {

	},

	drop_objects = {

	}
}

enemies[107] = {
	name = hoa_system.Translate("Kyle"),
	filename = "kyle",
	sprite_width = 48,
	sprite_height = 96,

	growth_stats = {
		hit_points = 5.0,
		skill_points = 1.0,
		experience_points = 1.5,
		strength = 5.0,
		vigor = 1.0,
		fortitude = 3.0,
		protection = 1.0,
		agility = 1.0,
		evade = 0.0,
		drunes = 2.0
	},

	skills = {

	},

	drop_objects = {

	}
}
