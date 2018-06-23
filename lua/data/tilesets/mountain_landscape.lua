local ns = {};
setmetatable(ns, {__index = _G});
mountain_landscape = ns;
setfenv(1, ns);

tileset_name = "Mountain - Landscape"
image = "img/tilesets/mountain_landscape.png"

collisions = {}
collisions[0] = { 0, 1, 3, 3, 3, 0, 0, 1, 3, 0, 0, 0, 0, 0, 0, 0 }
collisions[1] = { 1, 14, 0, 0, 5, 2, 7, 12, 4, 15, 0, 0, 0, 0, 0, 0 }
collisions[2] = { 5, 0, 0, 0, 0, 10, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0 }
collisions[3] = { 5, 0, 0, 0, 0, 10, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0 }
collisions[4] = { 5, 0, 0, 0, 5, 10, 11, 3, 3, 3, 0, 0, 0, 0, 0, 0 }
collisions[5] = { 5, 15, 15, 15, 15, 8, 13, 15, 15, 15, 0, 0, 0, 0, 0, 0 }
collisions[6] = { 4, 15, 15, 15, 15, 0, 4, 15, 15, 0, 0, 0, 0, 0, 0, 0 }
collisions[7] = { 14, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[8] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[10] = { 11, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[11] = { 15, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[12] = { 11, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[13] = { 15, 11, 7, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[14] = { 5, 15, 15, 10, 15, 15, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[15] = { 5, 15, 15, 0, 13, 15, 14, 15, 15, 15, 15, 15, 15, 15, 5, 10 }

autotiling = {}
autotiling[10] = "CrackedEarth"
autotiling[14] = "Pavers"
autotiling[15] = "Pavers"
autotiling[26] = "CrackedEarth"
autotiling[30] = "Pavers"
autotiling[31] = "Pavers"
autotiling[42] = "CrackedEarth"
autotiling[76] = "Pavers"
autotiling[78] = "Grass"
autotiling[79] = "Grass"
autotiling[94] = "Grass"
autotiling[95] = "Grass"

