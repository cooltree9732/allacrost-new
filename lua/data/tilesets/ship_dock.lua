local ns = {};
setmetatable(ns, {__index = _G});
ship_dock = ns;
setfenv(1, ns);

tileset_name = "Ship Dock"
image = "img/tilesets/ship_dock.png"

collisions = {}
collisions[0] = { 5, 3, 15, 3, 10, 14, 12, 12, 13, 0, 0, 0, 10, 0, 5, 15 }
collisions[1] = { 5, 0, 0, 0, 10, 11, 3, 3, 7, 3, 2, 1, 10, 0, 5, 15 }
collisions[2] = { 5, 0, 0, 0, 10, 0, 0, 0, 5, 10, 10, 5, 10, 0, 5, 15 }
collisions[3] = { 5, 0, 0, 0, 10, 0, 0, 0, 3, 0, 7, 10, 0, 10, 0, 5 }
collisions[4] = { 5, 12, 15, 12, 10, 0, 0, 0, 12, 15, 13, 10, 0, 10, 0, 5 }
collisions[5] = { 15, 0, 15, 0, 15, 3, 0, 0, 0, 15, 3, 7, 15, 10, 0, 5 }
collisions[6] = { 15, 0, 15, 0, 15, 3, 12, 12, 5, 10, 12, 13, 15, 0, 0, 0 }
collisions[7] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[8] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[14] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

