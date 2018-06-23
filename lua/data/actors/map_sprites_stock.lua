--------------------------------------------------------------------------------
--! \file map_sprites_stock.lua
--! \brief Data definitions for map sprites and functions that construct map sprite objects
--!
--! This is used by map scripts to construct MapSprite and EnemySprite objects.
--------------------------------------------------------------------------------

-- Table containing all definitions for non-enemy map sprites
sprites = {}
-- Table containing all definition for enemy map sprites
enemies = {}

-- Shortcut constants for map sprite speeds
local NORMAL_SPEED = hoa_map.MapMode.NORMAL_SPEED;
local SLOW_SPEED = hoa_map.MapMode.SLOW_SPEED;
local VERY_SLOW_SPEED = hoa_map.MapMode.VERY_SLOW_SPEED;
local VERY_FAST_SPEED = hoa_map.MapMode.VERY_FAST_SPEED;


--! \brief Builds and returns a MapSprite object with default properties
--! \param name The identification name of the sprite definition in the sprites table
--! \param id The object ID number to assign to the new sprite object
--! \param x Optional. The x position to place the sprite on the map
--! \param y Optional. The y position to place the sprite on the map
--! \return A MapSprite object, or nil if the object could not be constructed
--! \note Provide either both x and y or neither. Providing one but not the other is treated the same as omitting both.
function ConstructSprite(name, id, x, y)
	if (sprites[name] == nil) then
		return nil;
	end

	-- Convert the X/Y cooridnates into integer + offset values
	local x_int;
	local x_off;
	local y_int;
	local y_off;
	if (x and y) then
		x_int = math.floor(x);
		x_off = x - x_int;
		y_int = math.floor(y);
		y_off = y - y_int;
	end

	local direction = (2 ^ math.random(0, 3));

	local sprite = hoa_map.MapSprite.Create(id);
	if (sprites[name].name) then
		sprite:SetName(sprites[name].name);
	end
	sprite:SetContext(hoa_map.MapMode.CONTEXT_01);
	if (x and y) then
		sprite:SetXPosition(x_int, x_off);
		sprite:SetYPosition(y_int, y_off);
	end
	sprite:SetCollHalfWidth(sprites[name].coll_half_width);
	sprite:SetCollHeight(sprites[name].coll_height);
	sprite:SetImgHalfWidth(sprites[name].img_half_width);
	sprite:SetImgHeight(sprites[name].img_height);
	sprite:SetMovementSpeed(sprites[name].movement_speed);
	sprite:LoadStandardAnimations(sprites[name].standard_animations);
	if (sprites[name].running_animations) then
		sprite:LoadRunningAnimations(sprites[name].running_animations);
	end
	if (sprites[name].face_portrait) then
		sprite:LoadFacePortrait(sprites[name].face_portrait);
	end
	sprite:SetDirection(direction);
	return sprite;
end


--! \brief Builds and returns an EnemySprite object with default properties
--! \param name The identification name of the sprite definition in the enemies table
--! \param id The object ID number to assign to the new sprite object
--! \param x Optional. The x position to place the sprite on the map
--! \param y Optional. The y position to place the sprite on the map
--! \return An EnemySprite object, or nil if the object could not be constructed
--! \note Provide either both x and y or neither. Providing one but not the other is treated the same as omitting both.
function ConstructEnemySprite(name, id, x, y)
	if (enemies[name] == nil) then
		return nil;
	end

	-- Convert the X/Y cooridnates into integer + offset values
	local x_int;
	local x_off;
	local y_int;
	local y_off;
	if (x and y) then
		x_int = math.floor(x);
		x_off = x - x_int;
		y_int = math.floor(y);
		y_off = y - y_int;
	end

	local direction = (2 ^ math.random(0, 3));

	local enemy = hoa_map.EnemySprite.Create(id);
	enemy:SetContext(hoa_map.MapMode.CONTEXT_01);
	if (x and y) then
		enemy:SetXPosition(x_int, x_off);
		enemy:SetYPosition(y_int, y_off);
	end
	enemy:SetCollHalfWidth(enemies[name].coll_half_width);
	enemy:SetCollHeight(enemies[name].coll_height);
	enemy:SetImgHalfWidth(enemies[name].img_half_width);
	enemy:SetImgHeight(enemies[name].img_height);
	enemy:SetMovementSpeed(enemies[name].movement_speed);
	enemy:LoadStandardAnimations(enemies[name].standard_animations);
	enemy:SetDirection(direction);
	return enemy;
end


-- Sprite Defintions
sprites["Claudius"] = {
	name = hoa_system.Translate("Claudius"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/claudius_walk.png",
	running_animations = "img/sprites/characters/claudius_run.png",
	face_portrait = "img/portraits/face/claudius.png"
}

sprites["Laila"] = {
	name = hoa_system.Translate("Laila"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/laila_walk.png",
	running_animations = "img/sprites/characters/laila_run.png",
	face_portrait = "img/portraits/face/laila.png"
}

sprites["Kyle"] = {
	name = hoa_system.Translate("Kyle"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/kyle_walk.png",
	face_portrait = "img/portraits/face/kyle.png"
}

sprites["Knight01"] = {
	name = hoa_system.Translate("Knight"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/knight_01_walk.png"
}

sprites["Knight02"] = {
	name = hoa_system.Translate("Knight"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/knight_02_walk.png"
}

sprites["Knight03"] = {
	name = hoa_system.Translate("Knight"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/knight_03_walk.png"
}

sprites["Knight04"] = {
	name = hoa_system.Translate("Knight"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/knight_04_walk.png"
}

sprites["Knight05"] = {
	name = hoa_system.Translate("Knight"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/knight_05_walk.png"
}

sprites["Knight06"] = {
	name = hoa_system.Translate("Knight"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/knight_06_walk.png"
}

sprites["Marcus"] = {
	name = hoa_system.Translate("Marcus"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/marcus_walk.png",
	face_portrait = "img/portraits/face/marcus.png"
}

sprites["Vanica"] = {
	name = hoa_system.Translate("Vanica"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/vanica_walk.png",
	face_portrait = "img/portraits/face/vanica.png"
}

sprites["Man01"] = {
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/man_npc01_walk.png"
}

sprites["Man02"] = {
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/man_npc02_walk.png"
}

sprites["MaleChild01"] = {
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/boy_npc01_walk.png"
}

sprites["FemaleMerchant01"] = {
	name = hoa_system.Translate("Female Merchant"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/woman_npc01_walk.png"
}

sprites["Woman01"] = {
	name = hoa_system.Translate("Woman"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/girl_npc02_walk.png"
}

sprites["Woman02"] = {
	name = hoa_system.Translate("Woman"),
	coll_half_width = 1.0,
	coll_height = 2.0,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/woman_npc02_walk.png"
}

sprites["Mak Hound"] = {
	name = hoa_system.Translate("Mak Hound"),
	coll_half_width = 3.0,
	coll_height = 3.0,
	img_half_width = 4.0,
	img_height = 8.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/creatures/mak_hound.png"
}


-- Enemy Sprite Defintions
enemies["slime"] = {
	coll_half_width = 1.0,
	coll_height = 2.0,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,
	standard_animations = "img/sprites/enemies/slime_walk.png"
}

enemies["snake"] = {
	coll_half_width = 1.0,
	coll_height = 2.0,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,
	standard_animations = "img/sprites/enemies/snake_walk.png"
}

enemies["scorpion"] = {
	coll_half_width = 1.0,
	coll_height = 2.0,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,
	standard_animations = "img/sprites/enemies/scorpion_walk.png"
}

enemies["goliath_scorpion"] = {
	coll_half_width = 3.0,
	coll_height = 6.0,
	img_half_width = 3.0,
	img_height = 12.0,
	movement_speed = NORMAL_SPEED,
	standard_animations = "img/sprites/enemies/goliath_scorpion_walk.png"
}
