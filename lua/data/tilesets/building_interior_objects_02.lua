local ns = {};
setmetatable(ns, {__index = _G});
building_interior_objects_02 = ns;
setfenv(1, ns);

tileset_name = "Interior Objects - 02"
image = "img/tilesets/building_interior_objects_02.png"

collisions = {}
collisions[0] = { 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 15, 3, 3, 0 }
collisions[1] = { 15, 15, 15, 3, 12, 12, 15, 12, 3, 15, 3, 3, 15, 3, 3, 0 }
collisions[2] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3 }
collisions[3] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3 }
collisions[4] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[5] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[6] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[7] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[8] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[14] = { 15, 15, 15, 15, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[15] = { 15, 15, 15, 15, 15, 15, 15, 15, 3, 3, 3, 3, 3, 3, 3, 3 }

animations = {}
animations[0] = { 224, 150, 225, 150, 226, 150, 227, 150, 228, 150 }
animations[1] = { 240, 150, 241, 150, 242, 150, 243, 150, 244, 150, 245, 150, 246, 150, 247, 150 }
animations[2] = { 248, 150, 249, 150, 250, 150, 251, 150, 252, 150, 253, 150, 254, 150, 255, 150 }
