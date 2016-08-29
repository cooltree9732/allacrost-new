local ns = {};
setmetatable(ns, {__index = _G});
stone_house_interior = ns;
setfenv(1, ns);

tileset_name = "Stone House Interior"
image = "img/tilesets/stone_house_interior.png"

collisions = {}
collisions[0] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[2] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[3] = { 15, 15, 15, 0, 15, 15, 15, 15, 15, 15, 15, 7, 11, 15, 15, 15 }
collisions[4] = { 15, 15, 15, 0, 15, 15, 4, 12, 12, 8, 15, 14, 13, 15, 15, 15 }
collisions[5] = { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 3, 3 }
collisions[6] = { 15, 15, 15, 15, 13, 14, 15, 15, 13, 14, 15, 15, 15, 15, 15, 15 }
collisions[7] = { 15, 15, 15, 15, 15, 15, 15, 15, 10, 5, 15, 15, 15, 15, 15, 15 }
collisions[8] = { 15, 0, 7, 11, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 }
collisions[9] = { 15, 15, 15, 15, 3, 3, 15, 15, 15, 15, 15, 15, 15, 15, 0, 0 }
collisions[10] = { 0, 0, 0, 0, 0, 0, 10, 5, 5, 10, 0, 0, 0, 0, 0, 0 }
collisions[11] = { 0, 0, 0, 0, 0, 0, 10, 5, 5, 10, 0, 0, 0, 0, 0, 0 }
collisions[12] = { 0, 0, 0, 0, 0, 0, 10, 5, 5, 10, 0, 0, 0, 0, 0, 0 }
collisions[13] = { 0, 0, 0, 0, 0, 0, 10, 5, 5, 10, 0, 0, 0, 0, 0, 0 }
collisions[14] = { 0, 0, 0, 0, 0, 0, 10, 5, 5, 10, 0, 0, 0, 0, 0, 0 }
collisions[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

