local ns = {};
setmetatable(ns, {__index = _G});
castle_greystone_interior_01 = ns;
setfenv(1, ns);

tileset_name = "Castle Greystone Interior 01"
image = "img/tilesets/castle_greystone_interior_01.png"

collisions = {}
collisions[0] = { 14, 12, 13, 4, 12, 8, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[1] = { 15, 15, 15, 5, 15, 10, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[2] = { 15, 15, 15, 15, 15, 15, 4, 8, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[3] = { 15, 15, 15, 15, 15, 15, 0, 0, 0, 5, 0, 0, 10, 0, 0, 0 }
collisions[4] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[5] = { 0, 0, 0, 0, 0, 0, 15, 15, 0, 0, 0, 0, 0, 0, 3, 0 }
collisions[6] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0 }
collisions[7] = { 3, 3, 3, 15, 15, 15, 0, 0, 15, 15, 0, 0, 0, 0, 0, 0 }
collisions[8] = { 15, 15, 15, 15, 15, 15, 15, 0, 12, 12, 0, 0, 0, 0, 0, 0 }
collisions[9] = { 12, 12, 12, 0, 0, 15, 0, 0, 15, 15, 0, 0, 15, 15, 15, 0 }
collisions[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 12, 12, 0, 0, 0, 0, 0, 0 }
collisions[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[14] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[15] = { 1, 15, 1, 15, 15, 2, 15, 2, 1, 15, 1, 15, 15, 2, 15, 2 }

