local ns = {}
setmetatable(ns, {__index = _G})
desert_cave_ground = ns
setfenv(1, ns)

tileset_name = "Desert Cave - Ground"
image = "img/tilesets/desert_cave_ground.png"

collisions = {}
collisions[0] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 15, 15 }
collisions[2] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 15, 15 }
collisions[3] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[4] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 15, 15, 15, 0, 0, 0 }
collisions[5] = { 15, 15, 15, 15, 15, 15, 15, 0, 0, 15, 15, 15, 15, 0, 0, 0 }
collisions[6] = { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 0, 0, 0, 0, 15 }
collisions[7] = { 15, 15, 15, 15, 15, 15, 15, 15, 15, 0, 0, 0, 0, 0, 0, 0 }
collisions[8] = { 5, 10, 14, 13, 0, 0, 12, 13, 14, 0, 0, 0, 0, 0, 0, 0 }
collisions[9] = { 5, 10, 0, 0, 0, 0, 0, 15, 15, 0, 0, 5, 10, 0, 10, 0 }
collisions[10] = { 4, 8, 15, 15, 15, 0, 0, 0, 0, 0, 15, 0, 0, 15, 5, 0 }
collisions[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 15, 0, 10 }
collisions[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5 }
collisions[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 15, 15, 3, 3, 3, 3, 7, 11 }
collisions[14] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 15, 15, 15, 15, 15 }
collisions[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 15, 15, 15, 15, 15, 15, 15, 15 }

autotiling = {}
autotiling[0] = "CaveDirt"
autotiling[1] = "CaveDirt"
autotiling[2] = "CaveDirt"
autotiling[4] = "CaveFloor_CaveDirt_South"
autotiling[7] = "CaveFloor_CaveDirt_North"
autotiling[9] = "CaveFloor"
autotiling[10] = "CaveFloor"
autotiling[11] = "CaveFloor"
autotiling[12] = "CaveFloor"
autotiling[19] = "CaveFloor_CaveDirt_East"
autotiling[21] = "CaveFloor_CaveDirt_West"
autotiling[22] = "CaveFloor_CaveDirt_West"
autotiling[24] = "CaveFloor_CaveDirt_East"
autotiling[25] = "CaveFloor"
autotiling[26] = "CaveFloor"
autotiling[27] = "CaveFloor"
autotiling[28] = "CaveFloor"
autotiling[36] = "CaveFloor_CaveDirt_North"
autotiling[39] = "CaveFloor_CaveDirt_South"
autotiling[41] = "CaveFloor"
autotiling[42] = "CaveFloor"
autotiling[43] = "CaveFloor"
autotiling[44] = "CaveFloor"

