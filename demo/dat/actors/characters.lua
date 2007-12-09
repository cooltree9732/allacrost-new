------------------------------------------------------------------------------[[
-- Filename: characters.lua
--
-- Description: This file contains the definitions of all characters that exist
-- in Hero of Allacrost. When a new character is added to the party, this file
-- is accessed and the character is created using the data loaded from this file.
------------------------------------------------------------------------------]]

-- All character definitions are stored in this table
characters = {}

characters[hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS] = {
	name = "Claudius",
	filename = "claudius",

	initial_stats = {
		experience_level = 1,
		experience_points = 0,
		max_hit_points = 60,
		max_skill_points = 15,
		strength = 12,
		vigor = 4,
		fortitude = 14,
		protection = 6,
		agility = 30,
		evade = 4.0,
		weapon = 10001,
		head_armor = 20001,
		torso_armor = 30001,
		arm_armor = 40001,
		leg_armor = 50001
	},

	attack_points = {
		[hoa_global.GameGlobal.GLOBAL_POSITION_HEAD] = {
			name = "Head",
			x_position = 31,
			y_position = 54,
			fortitude_modifier = -0.20,
			protection_modifier = 0.25,
			evade_modifier = 0.50
		},
		[hoa_global.GameGlobal.GLOBAL_POSITION_TORSO] = {
			name = "Torso",
			x_position = 37,
			y_position = 34,
			fortitude_modifier = 0.40,
			protection_modifier = 0.10,
			evade_modifier = -0.20
		},
		[hoa_global.GameGlobal.GLOBAL_POSITION_ARMS] = {
			name = "Arms",
			x_position = 31,
			y_position = 54,
			fortitude_modifier = 0.10,
			protection_modifier = 0.00,
			evade_modifier = 0.10
		},
		[hoa_global.GameGlobal.GLOBAL_POSITION_LEGS] = {
			name = "Legs",
			x_position = 37,
			y_position = 34,
			fortitude_modifier = 0.20,
			protection_modifier = 0.20,
			evade_modifier = 0.05
		}
	},

	growth_stats = {
		[1] = {
			hit_points = 5.0,
			skill_points = 1.0,
			strength = 2.0,
			vigor = 1.0,
			fortitude = 2.0,
			protection = 1.0,
			agility = 1.0,
			evade = 1.0
		},
		[4] = {
			hit_points = 5.0,
			skill_points = 2.0,
			strength = 3.0,
			vigor = 2.0,
			fortitude = 3.0,
			protection = 1.0,
			agility = 1.0,
			evade = 1.0
		}
	},

	skills = { [1] = 1, [2] = 2 }
} -- characters[hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS]



characters[hoa_global.GameGlobal.GLOBAL_CHARACTER_LAILA] = {
	name = "Laila",
	filename = "laila",

	initial_stats = {
		experience_level = 1,
		experience_points = 0,
		max_hit_points = 45,
		max_skill_points = 25,
		strength = 11,
		vigor = 4,
		fortitude = 14,
		protection = 6,
		agility = 32,
		evade = 4.0,
		weapon = 10501,
		head_armor = 20001,
		torso_armor = 30001,
		arm_armor = 40001,
		leg_armor = 50001
	},

	attack_points = {
		[hoa_global.GameGlobal.GLOBAL_POSITION_HEAD] = {
			name = "Head",
			x_position = 31,
			y_position = 54,
			fortitude_modifier = -0.20,
			protection_modifier = 0.25,
			evade_modifier = 0.50
		},
		[hoa_global.GameGlobal.GLOBAL_POSITION_TORSO] = {
			name = "Torso",
			x_position = 37,
			y_position = 34,
			fortitude_modifier = 0.40,
			protection_modifier = 0.10,
			evade_modifier = -0.20
		},
		[hoa_global.GameGlobal.GLOBAL_POSITION_ARMS] = {
			name = "Arms",
			x_position = 31,
			y_position = 54,
			fortitude_modifier = 0.10,
			protection_modifier = 0.00,
			evade_modifier = 0.10
		},
		[hoa_global.GameGlobal.GLOBAL_POSITION_LEGS] = {
			name = "Legs",
			x_position = 37,
			y_position = 34,
			fortitude_modifier = 0.20,
			protection_modifier = 0.20,
			evade_modifier = 0.05
		}
	},

	growth_stats = {
		[1] = {
			hit_points = 5.0,
			skill_points = 1.0,
			strength = 2.0,
			vigor = 1.0,
			fortitude = 2.0,
			protection = 1.0,
			agility = 1.0,
			evade = 1.0
		},
		[4] = {
			hit_points = 5.0,
			skill_points = 2.0,
			strength = 3.0,
			vigor = 2.0,
			fortitude = 3.0,
			protection = 1.0,
			agility = 1.0,
			evade = 1.0
		}
	},

	skills = { [1] = 3, [3] = 20001 }
} -- characters[hoa_global.GameGlobal.GLOBAL_CHARACTER_LAILA]


------------------------------------------------------------------------------[[
-- \brief Called everytime a character reaches a new experience level. 
-- \param character A pointer to the GlobalCharacter who has gained the new level
-- 
-- Before this function is called, the character should already have their 
-- _experience_level member incremented. What this function does is determine
-- the amount that each stat will grow by on the next level and if any new
-- skills are learned on this level. 
------------------------------------------------------------------------------]]
function DetermineGrowth(character)
	local new_level = character:GetExperienceLevel();        -- The value of the character's new XP level
	local character_table = characters[character:GetID()];   -- Reference to the character's definition table
	local growth_table = nil;                                -- Reference to the table containing the growth stats for this level
	local new_skills = character_table["skills"][new_level]; -- Reference to the number or table of the new skills learned
	local character_growth = character:GetGrowth();          -- Reference to the growth object contained within the character

	if (character_table == nil) then
		print("LUA ERROR: characters.lua::GainExperienceLevel() failed because the character's ID was invalid");
		return;
	end

	-- Find the appropriate index in the growth_stats table for determining how much the stats should grow by this level
	local growth_index = new_level 
	while (growth_index > 0) do
		if (character_table["growth_stats"][growth_index] ~= nil) then
			growth_table = character_table["growth_stats"][growth_index];
			break;
		end
		growth_index = growth_index - 1;
	end
	
	if (growth_table == nil) then
		print("LUA ERROR: characters.lua::GainExperienceLevel() failed because no growth stats were found");
		return;
	end

	-- Calculate how much the character's stats are going to grow by the next level
	character_growth._hit_points_growth = growth_table["hit_points"];
	character_growth._skill_points_growth = growth_table["skill_points"];
	character_growth._strength_growth = growth_table["strength"];
	character_growth._vigor_growth = growth_table["vigor"];
	character_growth._fortitude_growth = growth_table["fortitude"];
	character_growth._protection_growth = growth_table["protection"];
	character_growth._agility_growth = growth_table["agility"];
	character_growth._evade_growth = growth_table["evade"];
	
	-- Determine if the character learns any skills at this new level	
	if (type(new_skills) == "number") then
		character_growth:_AddSkill(new_skills);
	elseif (type(new_skills) == "table") then
		for i, skill in ipairs(new_skills) do
			character_growth:_AddSkill(skill);
		end
	elseif (type(new_skills) ~= "nil") then
		print("LUA ERROR: characters.lua::GainExperienceLevel() failed because of an unknown skill type");
		return;
	end
end -- function DetermineGrowth(character)
