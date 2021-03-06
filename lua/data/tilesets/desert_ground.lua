local ns = {};
setmetatable(ns, {__index = _G});
desert_ground = ns;
setfenv(1, ns);

tileset_name = "Desert Ground"
image = "img/tilesets/desert_ground.png"

collisions = {}
collisions[0] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[2] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[3] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[4] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 15, 3, 7, 11, 15, 14 }
collisions[5] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 15, 15, 15, 14, 0, 0 }
collisions[6] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 15, 15, 14, 14, 12, 13 }
collisions[7] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 10, 0, 5, 10, 0, 5 }
collisions[8] = { 0, 0, 0, 0, 0, 0, 14, 12, 13, 15, 7, 13, 15, 10, 0, 5 }
collisions[9] = { 0, 0, 0, 0, 0, 0, 10, 0, 5, 14, 13, 12, 0, 15, 15, 15 }
collisions[10] = { 0, 0, 0, 0, 0, 0, 10, 0, 5, 0, 0, 0, 0, 15, 15, 15 }
collisions[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 15, 15 }
collisions[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0 }
collisions[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 12, 8, 0, 0, 0, 0, 0, 0 }
collisions[14] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 5, 15, 15, 15, 15, 15, 5, 10 }

autotiling = {}
autotiling[0] = "TameSand"
autotiling[1] = "TameSand"
autotiling[2] = "TameSand"
autotiling[3] = "TameSand"
autotiling[8] = "TameSand"
autotiling[9] = "WildSand"
autotiling[13] = "WildWeeds"
autotiling[14] = "WildWeeds"
autotiling[15] = "WildWeeds"
autotiling[16] = "TameSand"
autotiling[17] = "TameSand"
autotiling[18] = "TameSand"
autotiling[19] = "TameSand"
autotiling[22] = "TameSandstones"
autotiling[24] = "TameSand"
autotiling[25] = "WildWeeds"
autotiling[29] = "WildWeeds"
autotiling[30] = "WildWeeds"
autotiling[31] = "WildWeeds"
autotiling[32] = "TameSand"
autotiling[33] = "TameSand"
autotiling[34] = "TameSand"
autotiling[35] = "TameSand"
autotiling[40] = "TameSand"
autotiling[41] = "WildSand"
autotiling[45] = "WildSand"
autotiling[48] = "TameSand"
autotiling[49] = "TameSand"
autotiling[50] = "TameSand"
autotiling[51] = "TameSand"
autotiling[53] = "TameSandstones"
autotiling[54] = "TameSandstones"
autotiling[55] = "TameSandstones"
autotiling[56] = "TameSandstones"
autotiling[57] = "WildWeeds"
autotiling[58] = "WildSand"
autotiling[59] = "WildSand"
autotiling[60] = "WildSand"
autotiling[68] = "YellowSand"
autotiling[69] = "YellowSand"
autotiling[84] = "YellowSand"
autotiling[85] = "YellowSand"
autotiling[87] = "YellowPebbles"
autotiling[118] = "YellowPebbles"
autotiling[119] = "YellowPebbles"
autotiling[120] = "YellowPebbles"
autotiling[123] = "WildSand"

