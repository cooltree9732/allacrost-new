--------------------------------------------------------------------------------
-- a01_harrvah_capital_attack.lua
--
-- A script specific to the main storyline events. The Harrvah Capital is under
-- attack by demons and the player has to navigate his party through the chaos
-- to find the king.
--------------------------------------------------------------------------------
local ns = {}
setmetatable(ns, {__index = _G})
a01_harrvah_capital_attack = ns;
setfenv(1, ns);

-- Set to true to turn on debugging messages generated by this map script
DEBUG_PRINT = true;
-- Set to non-zero to enable different conditions when loading the map (refer to the DEBUG_Load() function)
DEBUG_LOAD_STATE = 0;

data_file = "lua/data/maps/harrvah_capital.lua";
location_filename = "img/portraits/locations/blank.png";
map_name = "Harrvah Capital";

sound_filenames = {};

music_filenames = {};
music_filenames[1] = "mus/Claudius.ogg";

-- Primary Map Classes
Map = {};
ObjectManager = {};
DialogueManager = {};
EventManager = {};
TreasureManager = {};
GlobalEvents = {};

enemy_ids = { }

-- Containers used to hold pointers to various class objects.
contexts = {};
zones = {};
objects = {};
sprites = {};
dialogues = {};
events = {};
sounds = {};

-- All custom map functions are contained within the following table.
-- String keys in this table serves as the names of these functions.
functions = {};

-- Shorthand names for map contexts
contexts["exterior"] = hoa_map.MapMode.CONTEXT_02; -- This is the "default" context for this map. CONTEXT_01 is not used
contexts["interior_a"] = hoa_map.MapMode.CONTEXT_03;
contexts["interior_b"] = hoa_map.MapMode.CONTEXT_04;
contexts["interior_c"] = hoa_map.MapMode.CONTEXT_05;
contexts["interior_d"] = hoa_map.MapMode.CONTEXT_06;
contexts["interior_e"] = hoa_map.MapMode.CONTEXT_07;

----------------------------------------------------------------------------
---------- Load Functions
----------------------------------------------------------------------------

function Load(m)
	Map = m;
	ObjectManager = Map.object_supervisor;
	DialogueManager = Map.dialogue_supervisor;
	EventManager = Map.event_supervisor;
	TreasureManager = Map.treasure_supervisor;
	GlobalEvents = Map.map_event_group;

	-- Setup the order in which we wish to draw the tile and object layers
	Map:ClearLayerOrder();
	Map:AddTileLayerToOrder(0);
	Map:AddTileLayerToOrder(1);
	Map:AddTileLayerToOrder(2);
	Map:AddObjectLayerToOrder(0);
	Map:AddTileLayerToOrder(3);
	Map:AddTileLayerToOrder(4);

	CreateZones();
	CreateObjects();
	CreateSprites();
	CreateEnemies();
	CreateDialogues();
	CreateEvents();

	-- Audio: load sounds
	sounds["door_locked"] = hoa_audio.SoundDescriptor();
	sounds["door_locked"]:LoadAudio("snd/door_locked.ogg");

	-- Visuals: night lightning
	VideoManager:EnableLightOverlay(hoa_video.Color(0.0, 0.0, 0.3, 0.6));
	Map:SetCurrentTrack(0);

	-- TODO: figure out if visuals should be disabled normally, or wait for control to be given to the player before they are displayed
	-- Map:DisableIntroductionVisuals();
	Map.virtual_focus:SetContext(contexts["exterior"]);
	Map.unlimited_stamina = true;
	Map:ShowStaminaBar(true);
	Map:ShowDialogueIcons(true);

	-- All character sprites are initially uncollidable, since they will "merge" into one sprite at the end of the opening scene
	if (DEBUG_LOAD_STATE == 0) then
		Map:SetCamera(sprites["claudius"]);
		sprites["claudius"]:SetNoCollision(true);
		sprites["mark"]:SetNoCollision(true);
		sprites["lukar"]:SetNoCollision(true);
		EventManager:StartEvent(event_chains["intro_scene"]);
	else
		DEBUG_Load();
	end


	IfPrintDebug(DEBUG_PRINT, "Map loading complete");
end -- Load(m)



function DEBUG_Load()
	-- Skip introductory scene and dialogue so player has immediate control
	if (DEBUG_LOAD_STATE == 1) then
		Map:SetCamera(sprites["claudius"]);
		sprites["mark"]:SetVisible(false);
		sprites["lukar"]:SetVisible(false);
		sprites["claudius"]:SetXPosition(98, 0);
		sprites["claudius"]:SetYPosition(185, 0);
	-- Move player to just below the zone that triggers the demon spawn event
	elseif (DEBUG_LOAD_STATE == 2) then
		Map:SetCamera(sprites["claudius"]);
		sprites["mark"]:SetVisible(false);
		sprites["lukar"]:SetVisible(false);
		sprites["claudius"]:SetXPosition(5, 0);
		sprites["claudius"]:SetYPosition(174, 0);
	-- Move player to just outside the throne room
	elseif (DEBUG_LOAD_STATE == 3) then
		Map:SetCamera(sprites["claudius"]);
		sprites["mark"]:SetVisible(false);
		sprites["lukar"]:SetVisible(false);
		sprites["claudius"]:SetXPosition(98, 0);
		sprites["claudius"]:SetYPosition(65, 0);
	else
		IfPrintDebug(DEBUG_PRINT, "Unsupported value for DEBUG_LOAD_STATE: " .. DEBUG_LOAD_STATE);
	end
end



function CreateZones()
	IfPrintDebug(DEBUG_PRINT, "Creating zones...");

	-- Bottom-left of map: triggers the scene where the characters observe a demon spawning in from the shadows
	zones["witness_spawn"] = hoa_map.CameraZone(2, 12, 162, 164, contexts["exterior"]);
	Map:AddZone(zones["witness_spawn"]);

	-- Mid-right of map: triggers the scene where the player watches a citizen being chased by a demon
	zones["witness_chase"] = hoa_map.CameraZone(160, 164, 138, 140, contexts["exterior"]);
	Map:AddZone(zones["witness_chase"]);

	---------- Enemy Spawning Zones
	-- Zone #01: Bottom left of map
	zones["enemy01"] = hoa_map.EnemyZone(14, 62, 176, 188);
	Map:AddZone(zones["enemy01"]);

	-- Zone #02: Left of main road, second row of buildings
	zones["enemy02"] = hoa_map.EnemyZone(32, 88, 150, 158);
	Map:AddZone(zones["enemy02"]);

	-- Zone #03: Weapon shop area
	zones["enemy03"] = hoa_map.EnemyZone(140, 192, 150, 162);
	Map:AddZone(zones["enemy03"]);

	-- Zone #04: North of park, below left cliff face
	zones["enemy04"] = hoa_map.EnemyZone(62, 102, 96, 110);
	Map:AddZone(zones["enemy04"]);

end



function CreateObjects()
	IfPrintDebug(DEBUG_PRINT, "Creating objects...");
end



function CreateSprites()
	IfPrintDebug(DEBUG_PRINT, "Creating sprites...");

	local sprite;
	local animation;

	-- This X/Y position represents the bottom middle point of the map, just outside the city wall gates
	local entrance_x = 98;
	local entrance_y = 208;

	-- Create sprites for the three playable characters
	sprites["claudius"] = ConstructSprite("Claudius", ObjectManager:GenerateObjectID(), entrance_x - 2, entrance_y - 2);
	sprites["claudius"]:SetContext(contexts["exterior"]);
	sprites["claudius"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["claudius"]);
	Map:SetPlayerSprite(sprites["claudius"]);

	sprites["mark"] = ConstructSprite("Knight01", ObjectManager:GenerateObjectID(), entrance_x + 2, entrance_y - 2);
	sprites["mark"]:SetContext(contexts["exterior"]);
	sprites["mark"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["mark"]:SetName(hoa_system.Translate("Mark"));
	ObjectManager:AddObject(sprites["mark"]);

	sprites["lukar"] = ConstructSprite("Knight01", ObjectManager:GenerateObjectID(), entrance_x, entrance_y - 4);
	sprites["lukar"]:SetContext(contexts["exterior"]);
	sprites["lukar"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["lukar"]:SetName(hoa_system.Translate("Lukar"));
	ObjectManager:AddObject(sprites["lukar"]);

	-- Create the captain, NPCs, and enemies fighting the battle straight ahead of the city entrance
	-- Coordinates for NPC battle that lies straight ahead on the road after entering the town
	sprites["captain"] = ConstructSprite("Knight06", ObjectManager:GenerateObjectID(), 98, 180);
	sprites["captain"]:SetContext(contexts["exterior"]);
	sprites["captain"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["captain"]:SetName(hoa_system.Translate("Captain Bravis"));
	ObjectManager:AddObject(sprites["captain"]);

	sprites["enemy_01"] = ConstructEnemySprite("scorpion", ObjectManager:GenerateObjectID(), 98, 177);
	sprites["enemy_01"]:SetContext(contexts["exterior"]);
	sprites["enemy_01"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["enemy_01"]:SetStationaryMovement(true);
	sprites["enemy_01"]:ChangeState(hoa_map.EnemySprite.ACTIVE);
	ObjectManager:AddObject(sprites["enemy_01"]);

	sprites["knight01"] = ConstructSprite("Knight01", ObjectManager:GenerateObjectID(), 93, 177);
	sprites["knight01"]:SetContext(contexts["exterior"]);
	sprites["knight01"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["knight01"]:SetStationaryMovement(true);
	ObjectManager:AddObject(sprites["knight01"]);

	sprites["enemy_02"] = ConstructEnemySprite("scorpion", ObjectManager:GenerateObjectID(), 93, 174);
	sprites["enemy_02"]:SetContext(contexts["exterior"]);
	sprites["enemy_02"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["enemy_02"]:SetStationaryMovement(true);
	sprites["enemy_02"]:ChangeState(hoa_map.EnemySprite.ACTIVE);
	ObjectManager:AddObject(sprites["enemy_02"]);

	sprites["knight02"] = ConstructSprite("Knight02", ObjectManager:GenerateObjectID(), 103.5, 176.5);
	sprites["knight02"]:SetContext(contexts["exterior"]);
	sprites["knight02"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["knight02"]:SetStationaryMovement(true);
	ObjectManager:AddObject(sprites["knight02"]);

	sprites["enemy_03"] = ConstructEnemySprite("scorpion", ObjectManager:GenerateObjectID(), 104, 173.5);
	sprites["enemy_03"]:SetContext(contexts["exterior"]);
	sprites["enemy_03"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["enemy_03"]:SetStationaryMovement(true);
	sprites["enemy_03"]:ChangeState(hoa_map.EnemySprite.ACTIVE);
	ObjectManager:AddObject(sprites["enemy_03"]);

	-- Create the senior knight and others fighting near the inn
	-- Coordinates for NPC battle that lies near the entrance to the inn
	sprites["senior_knight"] = ConstructSprite("Knight04", ObjectManager:GenerateObjectID(), 118, 182);
	sprites["senior_knight"]:SetContext(contexts["exterior"]);
	sprites["senior_knight"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["senior_knight"]:SetStationaryMovement(true);
	ObjectManager:AddObject(sprites["senior_knight"]);

	sprites["enemy_04"] = ConstructEnemySprite("scorpion", ObjectManager:GenerateObjectID(), 118, 183);
	sprites["enemy_04"]:SetContext(contexts["exterior"]);
	sprites["enemy_04"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["enemy_04"]:SetStationaryMovement(true);
	sprites["enemy_04"]:ChangeState(hoa_map.EnemySprite.ACTIVE);
	ObjectManager:AddObject(sprites["enemy_04"]);

	sprites["knight03"] = ConstructSprite("Knight03", ObjectManager:GenerateObjectID(), 112, 186);
	sprites["knight03"]:SetContext(contexts["exterior"]);
	sprites["knight03"]:SetDirection(hoa_map.MapMode.EAST);
	sprites["knight03"]:SetStationaryMovement(true);
	ObjectManager:AddObject(sprites["knight03"]);

	sprites["enemy_05"] = ConstructEnemySprite("scorpion", ObjectManager:GenerateObjectID(), 114, 185.5);
	sprites["enemy_05"]:SetContext(contexts["exterior"]);
	sprites["enemy_05"]:SetDirection(hoa_map.MapMode.WEST);
	sprites["enemy_05"]:SetStationaryMovement(true);
	sprites["enemy_05"]:ChangeState(hoa_map.EnemySprite.ACTIVE);
	ObjectManager:AddObject(sprites["enemy_05"]);

	-- Create the sergeant and others fighting near the item shop entrance
	sprites["sergeant"] = ConstructSprite("Knight05", ObjectManager:GenerateObjectID(), 82, 183);
	sprites["sergeant"]:SetContext(contexts["exterior"]);
	sprites["sergeant"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["sergeant"]:SetStationaryMovement(true);
	sprites["sergeant"]:SetName(hoa_system.Translate("Sergeant Methus"));
	ObjectManager:AddObject(sprites["sergeant"]);

	sprites["enemy_06"] = ConstructEnemySprite("scorpion", ObjectManager:GenerateObjectID(), 82, 180.5);
	sprites["enemy_06"]:SetContext(contexts["exterior"]);
	sprites["enemy_06"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["enemy_06"]:SetStationaryMovement(true);
	sprites["enemy_06"]:ChangeState(hoa_map.EnemySprite.ACTIVE);
	ObjectManager:AddObject(sprites["enemy_06"]);

	sprites["knight04"] = ConstructSprite("Knight01", ObjectManager:GenerateObjectID(), 90, 189);
	sprites["knight04"]:SetContext(contexts["exterior"]);
	sprites["knight04"]:SetDirection(hoa_map.MapMode.WEST);
	sprites["knight04"]:SetStationaryMovement(true);
	ObjectManager:AddObject(sprites["knight04"]);

	sprites["enemy_07"] = ConstructEnemySprite("scorpion", ObjectManager:GenerateObjectID(), 88, 188);
	sprites["enemy_07"]:SetContext(contexts["exterior"]);
	sprites["enemy_07"]:SetDirection(hoa_map.MapMode.EAST);
	sprites["enemy_07"]:SetStationaryMovement(true);
	sprites["enemy_07"]:ChangeState(hoa_map.EnemySprite.ACTIVE);
	ObjectManager:AddObject(sprites["enemy_07"]);

	-- This enemy spawns in during an event scene, so it is in the initial unspawned state
	sprites["enemy_spawn"] = ConstructEnemySprite("scorpion", ObjectManager:GenerateObjectID(), 9, 152);
	sprites["enemy_spawn"]:SetContext(contexts["exterior"]);
	sprites["enemy_spawn"]:SetDirection(hoa_map.MapMode.SOUTH);
	ObjectManager:AddObject(sprites["enemy_spawn"]);

end -- function CreateSprites()



function CreateEnemies()
	IfPrintDebug(DEBUG_PRINT, "Creating enemies...");
end



function CreateDialogues()
	IfPrintDebug(DEBUG_PRINT, "Creating dialogues...");

	event_dialogues = {}; -- Holds IDs of the dialogues used during events

	local dialogue;
	local text;

	----------------------------------------------------------------------------
	---------- Dialogues attached to characters
	----------------------------------------------------------------------------
	dialogue = hoa_map.MapDialogue.Create(10);
		text = hoa_system.Translate("The city is under attack by demons. We'll protect the citizens. Make your way to the castle with haste!");
		dialogue:AddLine(text, sprites["captain"]:GetObjectID());
	sprites["captain"]:AddDialogueReference(10);

	dialogue = hoa_map.MapDialogue.Create(11);
		text = hoa_system.Translate("Go! We'll manage here.");
		dialogue:AddLine(text, sprites["sergeant"]:GetObjectID());
	sprites["sergeant"]:AddDialogueReference(11);

	----------------------------------------------------------------------------
	---------- Dialogues triggered by events
	----------------------------------------------------------------------------
	event_dialogues["opening"] = 100;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["opening"]);
		text = hoa_system.Translate("Wh...what the hell is going on? What the hell are they?!");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());
		text = hoa_system.Translate("Captain!");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("Lukar! Take your squad and make your way to the castle. Inform the captain of the royal guard of our return and ask for orders.");
		dialogue:AddLine(text, sprites["captain"]:GetObjectID());
		text = hoa_system.Translate("The rest of us will repel these demons! Now go! Defend our people and our homes!");
		dialogue:AddLine(text, sprites["captain"]:GetObjectID());

	event_dialogues["locked_door"] = 101;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["locked_door"]);
		text = hoa_system.Translate("With all the chaos going on out here, all the citizens have surely locked themselves in their homes. Stop wasting time and head for the castle.");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());

	event_dialogues["demon_spawn1"] = 102;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["demon_spawn1"]);
		text = hoa_system.Translate("...!");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());

	event_dialogues["demon_spawn2"] = 103;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["demon_spawn2"]);
		text = hoa_system.Translate("I don't believe what I just saw.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("That demon...just emerged from the shadows...?");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("Well, that's just great! How the hell are we supposed to stop an invasion that comes through shadows?");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());
		text = hoa_system.Translate("Here it comes!");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());

	event_dialogues["save_citizen"] = 104;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["save_citizen"]);
		text = hoa_system.Translate("Where are you going, Claudius? That's not the way to the castle.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("Our orders are to find the king. We don't have time for this!");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());
		text = hoa_system.Translate("I understand that you want to help that citizen. We do as well. But we were given a mission of grave importance. Are you abandoning your duty?");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("...");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("Follow orders and continue searching for the king.");
		dialogue:AddOption(text, 3);
		text = hoa_system.Translate("Ignore orders and help the cornered citizen.");
		dialogue:AddOption(text, 4);
		text = hoa_system.Translate("You're right, we can't help everyone along our way. Let's keep going.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("But, I can't just leave them to die! I'll catch up with you at the castle.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("God dammit rookie!");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());
		text = hoa_system.Translate("If that's your decision, so be it. Try to catch back up to us. And stay alive.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());

	event_dialogues["citizen_escapes"] = 105;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["citizen_escapes"]);
		text = hoa_system.Translate("Th, thank you sirs!");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID()); -- TODO: change to citizen NPC sprite
		text = hoa_system.Translate("Get back to your home and barracade your door. Now!");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
end -- function CreateDialogues()


-- Creates all events and sets up the entire event sequence chain
function CreateEvents()
	IfPrintDebug(DEBUG_PRINT, "Creating events...");

	event_chains = {}; -- Holds IDs of the starting event for each event chain
	local event = {};

	-- Event Group #1: Initial map scene -- camera pans across a stretch of the city under attack before focusing on the captain
	event_chains["intro_scene"] = 1;
	event = hoa_map.CustomEvent.Create(event_chains["intro_scene"], "StartIntroScene", "");
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 1, 2000);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 1, Map.virtual_focus, 98, 130);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 2);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 2, Map.virtual_focus, sprites["captain"].x_position, sprites["captain"].y_position);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 3);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 4);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 5);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 3, sprites["claudius"], 0, -22);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 4, sprites["mark"], 0, -22);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 5, sprites["lukar"], 0, -22);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 6);
	event = hoa_map.DialogueEvent.Create(event_chains["intro_scene"] + 6, event_dialogues["opening"]);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 7);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 8);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 9);
	-- Move Claudius, Lukar and Mark to the same position
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 8, sprites["claudius"], 98, 185);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 8, sprites["mark"], 98, 185);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 9, sprites["lukar"], 98, 185);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 10);
	event = hoa_map.CustomEvent.Create(event_chains["intro_scene"] + 10, "", "EndIntroScene");

	-- Event Group #2: Party watches as enemy demon emerges from the shadows and attacks
	event_chains["demon_spawns"] = 20;
	event = hoa_map.PushMapStateEvent.Create(event_chains["demon_spawns"], hoa_map.MapMode.STATE_SCENE);
 	event:AddEventLinkAtStart(event_chains["demon_spawns"] + 1);
	event = hoa_map.DialogueEvent.Create(event_chains["demon_spawns"] + 1, event_dialogues["demon_spawn1"]);
	event:SetStopCameraMovement(true);
	event:AddEventLinkAtEnd(event_chains["demon_spawns"] + 2);
	event = hoa_map.CustomEvent.Create(event_chains["demon_spawns"] + 2, "SpawnSceneDemon", "IsSceneDemonActive");
	event:AddEventLinkAtEnd(event_chains["demon_spawns"] + 3, 1000);
	event = hoa_map.DialogueEvent.Create(event_chains["demon_spawns"] + 3, event_dialogues["demon_spawn2"]);
	event:AddEventLinkAtEnd(event_chains["demon_spawns"] + 4);
	event:AddEventLinkAtEnd(event_chains["demon_spawns"] + 5);
	event = hoa_map.PopMapStateEvent.Create(event_chains["demon_spawns"] + 4);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["demon_spawns"] + 5, sprites["enemy_spawn"], 0, 8);
	event:SetRelativeDestination(true);
	-- TODO: tell enemy sprite to roam and hunt

	-- Event Group #3: NPCs running around
	event_chains["fleeing_market"] = 40;

	-- Event Group #3: Party observes a citizen trying to escape from demons and decides whether or not to help
	event_chains["citizen_trapped"] = 60;

	-- Event Group #4: Enemies drop down between Claudius and his allies. Claudius continues on alone
	event_chains["claudius_separated"] = 80;

	-- Event Group #5: Claudius enters the throne room
	event_chains["throne_entered"] = 100;

	-- Event Group #6: Closing scene of map
	event_chains["closing_scene"] = 120;

	----------------------------------------------------------------------------
	---------- Miscellaneous Events
	---------------------------------------------------------------------------
	event_chains["locked_door"] = 1000;
	event = hoa_map.DialogueEvent.Create(event_chains["locked_door"], event_dialogues["locked_door"]);

	event_chains["pop_state"] = 1010;
	event = hoa_map.PopMapStateEvent.Create(event_chains["pop_state"])
end -- function CreateEvents()

----------------------------------------------------------------------------
---------- Update Functions
----------------------------------------------------------------------------

function Update()
	local index = 0;
	local notification = {};
	while (true) do
		notification = NotificationManager:GetNotificationEvent(index);
		if (notification == nil) then
			break;
		elseif (notification.category == "map" and notification.event == "collision") then
			HandleCollisionNotification(notification);
		end

		index = index + 1;
	end

	-- Check map zones for necessary actions
	if (zones["witness_spawn"]:IsPlayerSpriteEntering() == true) then
		if (EventManager:TimesEventStarted(event_chains["demon_spawns"]) == 0) then
			EventManager:StartEvent(event_chains["demon_spawns"]);
		end
	end
end


-- Processes collision notifications and takes appropriate action depending on the type and location of the collision
function HandleCollisionNotification(notification)
	-- We're only concerned with collisions by the player sprite for this map
	local sprite = notification.sprite;
	if (sprite:GetObjectID() ~= Map:GetPlayerSprite():GetObjectID()) then
		return;
	elseif (notification.collision_type == hoa_map.MapMode.OBJECT_COLLISION) then
		-- TODO: we may want to use this collision type to detect if the object was an enemy and start a battle if so
		return;
	end

	-- Determine the positions of each side of the sprite's collision rectangle
	local x_left = RoundToInteger(notification.x_position + notification.x_offset - sprite:GetCollHalfWidth());
	local x_right = RoundToInteger(notification.x_position + notification.x_offset + sprite:GetCollHalfWidth());
	local y_top = RoundToInteger(notification.y_position + notification.y_offset - sprite:GetCollHeight());
	local y_bottom = RoundToInteger(notification.y_position + notification.y_offset);

	local locked_door_collision = false;
	-- Collisions should now be checked to see if they play a "locked door" sound, or start a context switch
	if (sprite:GetContext() == contexts["exterior"]) then
		if (sprite:IsFacingDirection(hoa_map.MapMode.NORTH)) then
			-- There are a lot of south-facing doors, some which are locked (play a sound) and others which need to trigger a context switch
			-- The list below are the coordinates for every reachable door, starting from the top left of the map and going across and down
			-- TODO: there's probably a better/faster way to do this position checking (a lookup table?). For now this solution works fine though
			-- Castle doors
			if (y_top == 68 and notification.x_position >= 22 and notification.x_position <= 24) then
				SpriteContextTransition("enter_lcastle_side", sprite);
			elseif (y_top == 60 and notification.x_position >= 96 and notification.x_position <= 100) then
				SpriteContextTransition("balcony_to_throne", sprite);
			-- City top row doors
			elseif (y_top == 120 and notification.x_position > 12 and notification.x_position <= 16) then
				locked_door_collision = true;
			elseif (y_top == 124 and notification.x_position >= 50 and notification.x_position <= 54) then
				locked_door_collision = true;
			elseif (y_top == 124 and notification.x_position >= 116 and notification.x_position <= 120) then
				locked_door_collision = true;
			elseif (y_top == 120 and notification.x_position >= 140 and notification.x_position <= 144) then
				locked_door_collision = true;
			elseif (y_top == 122 and notification.x_position >= 168 and notification.x_position <= 172) then
				locked_door_collision = true;
			-- City middle row doors
			elseif (y_top == 150 and notification.x_position >= 20 and notification.x_position <= 24) then
				locked_door_collision = true;
			elseif (y_top == 148 and notification.x_position >= 48 and notification.x_position <= 52) then
				locked_door_collision = true;
			elseif (y_top == 150 and notification.x_position >= 78 and notification.x_position <= 82) then
				locked_door_collision = true;
			elseif (y_top == 152 and notification.x_position >= 148 and notification.x_position <= 152) then
				locked_door_collision = true;
			elseif (y_top == 146 and notification.x_position >= 178 and notification.x_position <= 182) then
				locked_door_collision = true;
			-- City bottom row doors
			elseif (y_top == 174 and notification.x_position >= 22 and notification.x_position <= 26) then
				locked_door_collision = true;
			elseif (y_top == 178 and notification.x_position >= 48 and notification.x_position <= 52) then
				locked_door_collision = true;
			elseif (y_top == 178 and notification.x_position >= 80 and notification.x_position <= 84) then
				locked_door_collision = true;
			elseif (y_top == 180 and notification.x_position >= 116 and notification.x_position <= 120) then
				locked_door_collision = true;
			end
		elseif (sprite:IsFacingDirection(hoa_map.MapMode.WEST)) then
			-- Castle Balcony, left side entrance
			if (x_left == 84 and notification.y_position > 60 and notification.y_position <= 66) then
				SpriteContextTransition("balcony_to_ltower", sprite);
			end
			-- x22-24, y68 = left side door
			-- x96-100, y60 = throne room
		end
	elseif (sprite:GetContext() == contexts["interior_a"]) then
		if (sprite:IsFacingDirection(hoa_map.MapMode.SOUTH)) then
			if (y_bottom == 60 and notification.x_position >= 96 and notification.x_position <= 100) then
				SpriteContextTransition("throne_to_balcony", sprite);
			end
		end
	elseif (sprite:GetContext() == contexts["interior_b"]) then
		if (sprite:IsFacingDirection(hoa_map.MapMode.SOUTH)) then
			if (y_bottom == 68 and notification.x_position >= 22 and notification.x_position <= 24) then
				SpriteContextTransition("exit_lcastle_side", sprite);
			end
		elseif (sprite:IsFacingDirection(hoa_map.MapMode.EAST)) then
			if (x_right == 84 and notification.y_position > 60 and notification.y_position <= 66) then
				SpriteContextTransition("ltower_to_balcony", sprite);
			end
		end
	end

	if (locked_door_collision) then
		-- Prevent playing the sound multiple times concurrently if the player keeps colliding with a locked door
		if (sounds["door_locked"]:IsPlaying() == false) then
			sounds["door_locked"]:Play();
		end
		if (EventManager:TimesEventStarted(event_chains["locked_door"]) == 0) then
			-- TODO: want to add a 50ms delay before starting the event here, but doing so
			-- somehow causes the user to be unable to exit the dialogue that the event starts
			EventManager:StartEvent(event_chains["locked_door"]);
		end
	end
end


--! \brief Initiates necessary actions for a sprite to transition from one context on the map to another
--! \param transition_key An string determining what sort of transition should take place
--! \param sprite A pointer to the sprite making the transition
function SpriteContextTransition(transition_key, sprite)
	local transition_time = 1000;
	local new_context;

	sprite:SetMoving(false);
	-- Set the virtual focus to the sprite's original location.
	Map:GetVirtualFocus():MoveToObject(sprite, true);
	Map:SetCamera(Map:GetVirtualFocus());

	if (transition_key == "enter_lcastle_side") then
		new_context = contexts["interior_b"];
		sprite:ModifyYPosition(-2, -0.5);
	elseif (transition_key == "exit_lcastle_side") then
		new_context = contexts["exterior"];
		sprite:ModifyYPosition(2, 0.5);
	elseif (transition_key == "balcony_to_ltower") then
		new_context = contexts["interior_b"];
		sprite:ModifyXPosition(-2, -0.5);
	elseif (transition_key == "ltower_to_balcony") then
		new_context = contexts["exterior"];
		sprite:ModifyXPosition(2, 0.5);
	elseif (transition_key == "balcony_to_throne") then
		new_context = contexts["interior_a"];
		sprite:ModifyYPosition(-2, -0.5);
	elseif (transition_key == "throne_to_balcony") then
		new_context = contexts["exterior"];
		sprite:ModifyYPosition(2, 0.5);
	else
		new_context = contexts["exterior"];
		-- TODO: print warning message about unknown transition key
	end

	sprite:SetContext(new_context);
	Map:ContextTransitionBlackColor(new_context, transition_time);
	-- Animate the camera moving to the sprite's new location
	Map:SetCamera(sprite, transition_time);
	-- Prevent player from controlling sprite until transition completes
	Map:PushState(hoa_map.MapMode.STATE_SCENE);
	EventManager:StartEvent(event_chains["pop_state"], transition_time);
end

----------------------------------------------------------------------------
---------- Draw Functions
----------------------------------------------------------------------------

function Draw()
	Map:DrawMapLayers();
end

----------------------------------------------------------------------------
---------- Event Functions
----------------------------------------------------------------------------

-- Move camera to just below the right staircase leading to the castle for the start of the scene
functions["StartIntroScene"] = function()
	VideoManager:FadeScreen(hoa_video.Color(0.0, 0.0, 0.0, 1.0), 0); -- Initially set the screen to black
	VideoManager:FadeScreen(hoa_video.Color(0.0, 0.0, 0.0, 0.0), 1000); -- Gradually fade the screen back in
	Map:PushState(hoa_map.MapMode.STATE_SCENE);
	Map:MoveVirtualFocus(130, 98);
	Map:SetCamera(Map.virtual_focus, 0);
	Map.virtual_focus:SetMovementSpeed(hoa_map.MapMode.VERY_FAST_SPEED - 25); -- Custom speed for fast camera panning
end


-- Hide all character sprites but Claudius and give control over to the player. Will not return success unless all
-- character sprites are not in motion
functions["EndIntroScene"] = function()
	if (sprites["claudius"]:IsMoving() == true or sprites["mark"]:IsMoving() == true or sprites["lukar"]:IsMoving() == true) then
		return false;
	end

	sprites["captain"]:SetStationaryMovement(true);
	sprites["mark"]:SetVisible(false);
	sprites["lukar"]:SetVisible(false);
	sprites["claudius"]:SetNoCollision(false);
	sprites["claudius"]:SetDirection(hoa_map.MapMode.NORTH);
	Map:SetCamera(sprites["claudius"], 500);
	Map:PopState();
	return true;
end


-- Spawn the enemy demon and move the camera to it
functions["SpawnSceneDemon"] = function()
	sprites["enemy_spawn"]:ChangeState(hoa_map.EnemySprite.SPAWN);
	sprites["enemy_spawn"]:SetSpawnedState(hoa_map.EnemySprite.ACTIVE);
	Map:SetCamera(sprites["enemy_spawn"], 2000);
end


-- Returns true once the enemy_spawn demon is in the active state and fully spawned in
functions["IsSceneDemonActive"] = function()
	if (sprites["enemy_spawn"]:GetState() == hoa_map.EnemySprite.ACTIVE) then
		Map:SetCamera(sprites["claudius"], 1000); -- Move the camera back to the player
		sprites["enemy_spawn"]:ChangeState(hoa_map.EnemySprite.HUNT);
		return true;
	else
		return false;
	end
end
