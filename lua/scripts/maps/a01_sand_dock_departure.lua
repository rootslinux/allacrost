--------------------------------------------------------------------------------
-- 01_sand_dock_departure.lua
--
-- This script controls the events that happen on the Harrvah sand dock map toward
-- the end of chapter 1. The script is pretty small and straight forward, without
-- many event sequences. Finishing the events on this map sends the player to chapter 2.
--------------------------------------------------------------------------------
local ns = {}
setmetatable(ns, {__index = _G})
a01_sand_dock_departure = ns;
setfenv(1, ns);

-- Set to non-zero to enable different conditions when loading the map (refer to the DEBUG_Load() function)
DEBUG_STATE = 0;

data_file = "lua/data/maps/harrvah_sand_dock.lua";
location_filename = "img/portraits/locations/blank.png";
map_name = "Harrvah - Sand Dock";

sound_filenames = {};

music_filenames = {};
music_filenames[1] = "mus/Seeking_New_Worlds.ogg";

-- Primary Map Classes
Map = {};
LocalRecords = {};
GlobalRecords = {};
DialogueManager = {};
EventManager = {};
ObjectManager = {};
TileManager = {};
TreasureManager = {};
TransitionManager= {};

-- Containers used to hold pointers to various class objects.
contexts = {};
zones = {};
objects = {};
sprites = {};
dialogues = {};
event_sequences = {};
sounds = {};

-- All custom map functions are contained within the following table.
-- String keys in this table serves as the names of these functions.
functions = {};

function Load(m)
	Map = m;
	LocalRecords = Map.local_record_group;
	GlobalRecords = Map.global_record_group;
	DialogueManager = Map.dialogue_supervisor;
	EventManager = Map.event_supervisor;
	ObjectManager = Map.object_supervisor;
	TileManager = Map.tile_supervisor;
	TransitionManager = Map.transition_supervisor;
	TreasureManager = Map.treasure_supervisor;

	-- Setup the order in which we wish to draw the tile and object layers
	Map:ClearLayerOrder();
	Map:AddTileLayerToOrder(0);
	Map:AddTileLayerToOrder(1);
	Map:AddTileLayerToOrder(2);
	Map:AddObjectLayerToOrder(0);
	Map:AddTileLayerToOrder(3);

	-- Activate the record that enables the final departure sequence
	if (DEBUG_STATE == 1) then
		GlobalRecords:SetRecord("departure_ready", 1);
	end

	CreateObjects();
	CreateSprites();
	CreateDialogueReferences();
	CreateEvents();
	CreateDialogues();

	-- Adjust player's position to be at the end of the dock
	if (DEBUG_STATE == 1) then
		sprites["claudius"]:SetXPosition(96, 0);
		sprites["claudius"]:SetYPosition(48, 0);
	end

	-- Music: should use the same as the current music playing from previous map (a01_harrvah_captial_aftermath.lua)
	Map:SetCurrentTrack(0);

	-- This entire map is played out in scene state with no GUI objects on screen other than the dialogue window
	Map:DisableIntroductionVisuals();
	Map.unlimited_stamina = true;
	Map:ShowStaminaBar(false);
	Map:ShowDialogueIcons(true);

	Map:SetCamera(sprites["claudius"]);
	Map:SetPlayerSprite(sprites["claudius"]);
	IfPrintDebug(DEBUG, "Map loading complete");
end -- Load(m)

----------------------------------------------------------------------------
---------- Update Functions
----------------------------------------------------------------------------

function Update()
	-- Process all notification events that we care about
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
end


-- Processes collision notifications and takes appropriate action depending on the type and location of the collision
function HandleCollisionNotification(notification)
	-- We're only concerned with collisions by the player sprite for this map
	local sprite = notification.sprite;
	if (sprite:GetObjectID() ~= Map:GetPlayerSprite():GetObjectID()) then
		return;
	end

	-- The player is only able to collide with the map boundary at the bottom center of the map. If this happens,
	-- change to the capital aftermath map
	if (notification.collision_type == hoa_map.MapMode.BOUNDARY_COLLISION) then
		if (EventManager:IsEventActive(event_sequences["leave_map"]) == false) then
			EventManager:StartEvent(event_sequences["leave_map"]);
		end
	end
end



function Draw()
	Map:DrawMapLayers();
end



function CreateObjects()
	-- The sand glider rests just to the east of the north dock
	local glider = hoa_map.PhysicalObject();
	glider:SetObjectID(1000);
	glider:SetContext(hoa_map.MapMode.CONTEXT_01);
	glider:SetPosition(108, 54);
	glider:SetImgHeight(12);
	glider:SetImgHalfWidth(6);
	glider:SetCollHeight(12);
	glider:SetCollHalfWidth(6);
	glider:AddAnimation("img/sprites/objects/sand_glider.png");
	ObjectManager:AddObject(glider);
end



function CreateSprites()
	IfPrintDebug(DEBUG, "Creating sprites...");

	sprites["claudius"] = ConstructSprite("Claudius", 1, 98, 110);
	sprites["claudius"]:SetDirection(hoa_map.MapMode.NORTH);

	sprites["dock_guard"] = ConstructSprite("Knight01", 10, 91, 98);
	sprites["dock_guard"]:SetDirection(hoa_map.MapMode.SOUTH);

	sprites["glider_handler"] = ConstructSprite("Knight02", 11, 99, 48);
	sprites["glider_handler"]:SetDirection(hoa_map.MapMode.EAST);
	sprites["glider_handler"]:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);

	sprites["west_worker"] = ConstructSprite("Woman02", 12, 72, 76);
	sprites["west_worker"]:SetDirection(hoa_map.MapMode.WEST);

	sprites["east_worker"] = ConstructSprite("Man01", 13, 111, 81);
	sprites["east_worker"]:SetDirection(hoa_map.MapMode.SOUTH);

	sprites["north_worker"] = ConstructSprite("Knight01", 14, 102, 68);
	sprites["north_worker"]:SetDirection(hoa_map.MapMode.NORTH);

	-- Initially invisible, Lukar and Claudius' family walks in from the bottom of the screen for the final event sequence
	sprites["lukar"] = ConstructSprite("Knight01", 30, 93, 75);
	sprites["lukar"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["lukar"]:SetName(hoa_system.Translate("Lukar"));
	sprites["lukar"].visible = false;
	sprites["lukar"].collidable = false;

	sprites["marcus"] = ConstructSprite("Marcus", 32, 95, 75);
	sprites["marcus"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["marcus"].visible = false;
	sprites["marcus"].collidable = false;

	sprites["vanica"] = ConstructSprite("Vanica", 33, 97, 75);
	sprites["vanica"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["vanica"].visible = false;
	sprites["vanica"].collidable = false;

	sprites["laila"] = ConstructSprite("Laila", 31, 96, 78);
	sprites["laila"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["laila"].visible = false;
	sprites["laila"].collidable = false;
end -- function CreateSprites()


-- This is done outside of CreateDialogues() so that both events and dialogues being created can reference them
function CreateDialogueReferences()
	dialogues["confirm_departure"] = 100;
	dialogues["lukar_farewell_1"] = 110;
	dialogues["lukar_farewell_2"] = 111;
	dialogues["lukar_farewell_3"] = 112;
	dialogues["parents_farewell"] = 120;
	dialogues["laila_farewell_1"] = 130;
	dialogues["laila_farewell_2"] = 131;
	dialogues["laila_farewell_3"] = 132;
	dialogues["laila_farewell_4"] = 133;
	dialogues["laila_farewell_5"] = 134;
	dialogues["final_farewell"] = 140;
end



function CreateDialogues()
	IfPrintDebug(DEBUG, "Creating dialogues...");

	local dialogue;
	local text;

	----------------------------------------------------------------------------
	---------- Optional NPC Dialogues
	----------------------------------------------------------------------------
	dialogue = hoa_map.MapDialogue.Create(10);
		text = hoa_system.Translate("The sand dock was one of the few structures not damaged in the attack.");
		dialogue:AddLine(text, sprites["dock_guard"]:GetObjectID());
	sprites["dock_guard"]:AddDialogueReference(10);
	if (GlobalRecords:DoesRecordExist("departure_ready") == true) then
		dialogue = hoa_map.MapDialogue.Create(11);
			text = hoa_system.Translate("Ah, are you the one departing by sand glider? Talk to the handler at the north end of the docks.");
			dialogue:AddLine(text, sprites["dock_guard"]:GetObjectID());
		sprites["dock_guard"]:AddDialogueReference(11);
	end

	dialogue = hoa_map.MapDialogue.Create(20);
		text = hoa_system.Translate("It's fortunate that our supply lines haven't been interrupted, or the aftermath of this disaster would have been much worse.");
		dialogue:AddLine(text, sprites["east_worker"]:GetObjectID());
	sprites["east_worker"]:AddDialogueReference(20);

	dialogue = hoa_map.MapDialogue.Create(30);
		text = hoa_system.Translate("We were ordered to help labor the docks due to the shortage of available workers after the attack.");
		dialogue:AddLine(text, sprites["north_worker"]:GetObjectID());
	sprites["north_worker"]:AddDialogueReference(30);
	dialogue = hoa_map.MapDialogue.Create(31);
		text = hoa_system.Translate("I know it's in bad taste to complain at a time like this, but I didn't become a knight to do manual labor.");
		dialogue:AddLine(text, sprites["north_worker"]:GetObjectID());
	sprites["north_worker"]:AddDialogueReference(31);

	-- The glider handler only has this dialogue if the requirements for the final event sequence has not been met
	if (GlobalRecords:DoesRecordExist("departure_ready") == false) then
		dialogue = hoa_map.MapDialogue.Create(40);
			text = hoa_system.Translate("Sand gliders are the best way to get around our kingdom quickly. They require a lot of continuous upkeep maintainence though.");
			dialogue:AddLine(text, sprites["glider_handler"]:GetObjectID());
		sprites["glider_handler"]:AddDialogueReference(40);
	end

	----------------------------------------------------------------------------
	---------- Final Sequence Dialogues
	----------------------------------------------------------------------------
	if (GlobalRecords:DoesRecordExist("departure_ready") == true) then
		dialogue = hoa_map.MapDialogue.Create(dialogues["confirm_departure"]);
			text = hoa_system.Translate("You're Claudius, correct? If you have all of your effects and affairs in order, I can finish preparing the glider for your journey.");
			dialogue:AddLine(text, sprites["glider_handler"]:GetObjectID());
			-- Player selects Claudius' response
			dialogue:AddLine(hoa_system.Translate("..."), sprites["claudius"]:GetObjectID());
			text = hoa_system.Translate("I'm ready to depart.");
			dialogue:AddOption(text, 2);
			text = hoa_system.Translate("No, I still have things to take care of here.");
			dialogue:AddOption(text, 4);

			text = hoa_system.Translate("Are you sure you have everything? Once you board the glider, you won't be able to return here.");
			dialogue:AddLine(text, sprites["glider_handler"]:GetObjectID());
			-- Player confirms Claudius' response
			dialogue:AddLine(hoa_system.Translate("..."), sprites["claudius"]:GetObjectID());
			text = hoa_system.Translate("Yes, I'm ready.");
			dialogue:AddOption(text, 5);
			dialogue:AddOptionLocalRecord("player_starts_finale", 1);
			text = hoa_system.Translate("On second thought, I need a little more time to prepare.");
			dialogue:AddOption(text, 4);

			-- Final lines. If the player selected that they are ready, the final event sequence will now begin
			text = hoa_system.Translate("Go ahead and take your time. I'll be waiting here until you're ready to go.");
			dialogue:AddLine(text, sprites["glider_handler"]:GetObjectID(), hoa_common.CommonDialogue.END_DIALOGUE);
			text = hoa_system.Translate("Alright, hand me your belongings then. I'll be just a few moments so wait there.");
			dialogue:AddLine(text, sprites["glider_handler"]:GetObjectID());

			dialogue:AddEventAtEnd(event_sequences["check_start_finale"]);
		sprites["glider_handler"]:AddDialogueReference(dialogues["confirm_departure"]);
	end

	-- Lukar and Claudius bid farewell
	dialogue = hoa_map.MapDialogue.Create(dialogues["lukar_farewell_1"]);
		text = hoa_system.Translate("Claudius!");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());

	dialogue = hoa_map.MapDialogue.Create(dialogues["lukar_farewell_2"]);
		text = hoa_system.Translate("Lukar, what are you doing here?");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("I'm here to bid a comrade farewell, of course. Quite the assignment that you've been tasked with.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("... Truthfully, I have no idea why I was chosen for this mission. There are far more skilled and experienced knights serving his majesty.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("Well, our king has always been the type of man to go with his gut feeling. You made quite the impression on him by leaping in front of that beast in the throne room the other night.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("I know that you're feeling anxious about this.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		dialogue:AddLineEventAtEnd(event_sequences["claudius_walk_east4"]);
		text = hoa_system.Translate("...");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("Even though I only served with you for a few days, I saw in you a potential for enormous growth. If I didn't see that, I would have protested to my commander that you were inadequate for such a journey.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("I hope that gives you some peace of mind. Serve our king and complete your mission. With honor.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		dialogue:AddLineEventAtEnd(event_sequences["claudius_face_south"]);
		text = hoa_system.Translate("With honor.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());

	dialogue = hoa_map.MapDialogue.Create(dialogues["lukar_farewell_3"]);
		text = hoa_system.Translate("What of Mark? Did he send his regards?");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		dialogue:AddLineEventAtEnd(event_sequences["lukar_walk_west4"]);
		text = hoa_system.Translate("... He... wishes you the best.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("I see.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());

	-- Claudius and his parents bid farewell
	dialogue = hoa_map.MapDialogue.Create(dialogues["parents_farewell"]);
		text = hoa_system.Translate("Mom. Dad. Laila. Thanks for coming. I suppose this is goodby for a long while.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("Oh sweetie. Please take care of yourself. We're going to miss you every single day.");
		dialogue:AddLine(text, sprites["vanica"]:GetObjectID());
		text = hoa_system.Translate("We're proud of you son. You've grown up to be a fine man. Whatever challenges wait for you ahead, I know you will overcome them. ");
		dialogue:AddLine(text, sprites["marcus"]:GetObjectID());
		text = hoa_system.Translate("Thanks. I will miss you too. I'll be back some day. I don't know when, but I promise you that I will return.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("Of course you will.");
		dialogue:AddLine(text, sprites["vanica"]:GetObjectID());

	-- Claudius and Laila bid farewell, glider handler finishes preparations
	dialogue = hoa_map.MapDialogue.Create(dialogues["laila_farewell_1"]);
		text = hoa_system.Translate("Sister. Thank you for, well, everything really.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("Claude, I, I...");
		dialogue:AddLine(text, sprites["laila"]:GetObjectID());

	dialogue = hoa_map.MapDialogue.Create(dialogues["laila_farewell_2"]);
		text = hoa_system.Translate("Alright, your glider's all secure and ready to go.");
		dialogue:AddLine(text, sprites["glider_handler"]:GetObjectID());

	dialogue = hoa_map.MapDialogue.Create(dialogues["laila_farewell_3"]);
		text = hoa_system.Translate("Take care of yourself Laila.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());

	dialogue = hoa_map.MapDialogue.Create(dialogues["laila_farewell_4"]);
		text = hoa_system.Translate("Smile more!");
		dialogue:AddLine(text, sprites["laila"]:GetObjectID());
		text = hoa_system.Translate("I've noticed that you smile less and less these days, but I never told you. I want you to smile more, Claude.");
		dialogue:AddLine(text, sprites["laila"]:GetObjectID());
		text = hoa_system.Translate("... And, I think your real family would want the same thing.");
		dialogue:AddLine(text, sprites["laila"]:GetObjectID());

	dialogue = hoa_map.MapDialogue.Create(dialogues["laila_farewell_5"]);
		text = hoa_system.Translate("Laila, you are my real family.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());

	-- Final farewells
	dialogue = hoa_map.MapDialogue.Create(dialogues["final_farewell"]);
		text = hoa_system.Translate("Goodbye, everyone. I will see you all again. Some day.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());

end -- function CreateDialogues()


-- Creates all events and sets up the entire event sequence chain
function CreateEvents()
	IfPrintDebug(DEBUG, "Creating events...");

	local event = {};
	local event_id;

	-- Exits back to the capital aftermath map
	event_sequences["leave_map"] = 100;
	event = hoa_map.MapTransitionEvent.Create(event_sequences["leave_map"], "lua/scripts/maps/a01_harrvah_capital_aftermath.lua", 1);

	----------------------------------------------------------------------------
	---------- Various small independent events used within dialogues
	----------------------------------------------------------------------------
	event_sequences["claudius_face_east"] = 1;
	event = hoa_map.ChangePropertySpriteEvent.Create(event_sequences["claudius_face_east"], sprites["claudius"]);
	event:Direction(hoa_map.MapMode.EAST);
	event_sequences["claudius_face_south"] = 2;
	event = hoa_map.ChangePropertySpriteEvent.Create(event_sequences["claudius_face_south"], sprites["claudius"]);
	event:Direction(hoa_map.MapMode.SOUTH);
	event_sequences["claudius_walk_east4"] = 3;
	event = hoa_map.PathMoveSpriteEvent.Create(event_sequences["claudius_walk_east4"], sprites["claudius"], 4, 0);
	event:SetRelativeDestination(true);
	event_sequences["lukar_walk_west4"] = 4;
	event = hoa_map.PathMoveSpriteEvent.Create(event_sequences["lukar_walk_west4"], sprites["lukar"], -4, 0);
	event:SetRelativeDestination(true);

	----------------------------------------------------------------------------
	---------- Always-running events controlling NPC movement
	----------------------------------------------------------------------------
	-- TODO: west worker walks in place and switches direction between west and south
	-- TODO: east worker runs along the bottom rope line between the sacks and the ladder
	-- TODO: north worker walks in place facing north, with little side movement

	----------------------------------------------------------------------------
	---------- Final Sequence Dialogues
	----------------------------------------------------------------------------
	-- The finale starts if the player selected a certain option in a dialogue after that dialogue ends.
	-- We need a special event to handle checking that condition and starting the true finale event
	event_sequences["check_start_finale"] = 1000;
	event = hoa_map.CustomEvent.Create(event_sequences["check_start_finale"], "CheckStartFinale", "");

	-- Glider handler jumps down to the glider and moves around a bit
	event_sequences["start_finale"], event_id = 1001, 1001;
	event = hoa_map.PushMapStateEvent.Create(event_id, hoa_map.MapMode.STATE_SCENE);
	event:AddEventLinkAtEnd(event_id + 1);
	event_id = event_id + 1; event = hoa_map.ChangePropertySpriteEvent.Create(event_id, sprites["glider_handler"]);
	event:Collidable(false);
	event:AddEventLinkAtEnd(event_id + 1);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["glider_handler"], 100, 47);
	event:AddEventLinkAtEnd(event_id + 1);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["glider_handler"], 104, 51);
	event:AddEventLinkAtEnd(event_id + 1, 1000);

	-- Lukar calls out to Claudius to bid farewell
	event_id = event_id + 1; event = hoa_map.DialogueEvent.Create(event_id, dialogues["lukar_farewell_1"]);
	event:AddEventLinkAtEnd(event_id + 1);
	event_id = event_id + 1; event = hoa_map.ChangePropertySpriteEvent.Create(event_id, sprites["claudius"]);
	event:Direction(hoa_map.MapMode.SOUTH);
	event:AddEventLinkAtEnd(event_id + 1, 1000);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["claudius"], 91, 51);
	event:SetFinalDirection(hoa_map.MapMode.SOUTH);
	event:AddEventLinkAtEnd(event_id + 1);
	event:AddEventLinkAtEnd(event_id + 2);
	event_id = event_id + 1; event = hoa_map.ChangePropertySpriteEvent.Create(event_id, sprites["lukar"]);
	event:Visible(true);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["lukar"], 93, 54);
	event:AddEventLinkAtEnd(event_id + 1, 1000);
	event_id = event_id + 1; event = hoa_map.DialogueEvent.Create(event_id, dialogues["lukar_farewell_2"]);
	event:AddEventLinkAtEnd(event_sequences["claudius_face_east"], 200);
	event:AddEventLinkAtEnd(event_sequences["claudius_face_south"], 2000);
	event:AddEventLinkAtEnd(event_id + 1, 2000);
	event_id = event_id + 1; event = hoa_map.DialogueEvent.Create(event_id, dialogues["lukar_farewell_3"]);
	event:AddEventLinkAtEnd(event_id + 1, 1000);
	event:AddEventLinkAtEnd(event_id + 2);
	event_id = event_id + 1; event = hoa_map.ChangePropertySpriteEvent.Create(event_id, sprites["lukar"])
	event:Direction(hoa_map.MapMode.EAST);

	-- Claudius says goodbye to his parents
	event_id = event_id + 1; event = hoa_map.ChangePropertySpriteEvent.Create(event_id, sprites["marcus"]);
	event:AddSprite(sprites["vanica"]);
	event:AddSprite(sprites["laila"]);
	event:Visible(true);
	event:AddEventLinkAtEnd(event_id + 1);
	event:AddEventLinkAtEnd(event_id + 2);
	event:AddEventLinkAtEnd(event_id + 3);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["marcus"], 95, 51);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["vanica"], 97, 51);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["laila"], 96, 54);
	event:AddEventLinkAtEnd(event_id + 1);
	event_id = event_id + 1; event = hoa_map.DialogueEvent.Create(event_id, dialogues["parents_farewell"]);
	event:AddEventLinkAtEnd(event_id + 1);
	event:AddEventLinkAtEnd(event_id + 3, 200);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["marcus"], -5, 0);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(event_id + 1);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["marcus"], 0, -6);
	event:SetRelativeDestination(true);
	event:SetFinalDirection(hoa_map.MapMode.EAST);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["vanica"], -7, 0);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(event_id + 1);
	event:AddEventLinkAtEnd(event_id + 2);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["vanica"], 0, -4);
	event:SetRelativeDestination(true);
	event:SetFinalDirection(hoa_map.MapMode.EAST);

	-- Claudius says goodbye to Laila
	-- TODO: this event isn't being processed because a path could not be found, fix it
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["claudius"], 4, 6);
	event:SetRelativeDestination(true);
	event:SetFinalDirection(hoa_map.MapMode.SOUTH);
	event:AddEventLinkAtEnd(event_id + 1);
	event_id = event_id + 1; event = hoa_map.DialogueEvent.Create(event_id, dialogues["laila_farewell_1"]);
	event:AddEventLinkAtEnd(event_id + 1);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["glider_handler"], 100, 47);
	event:AddEventLinkAtEnd(event_id + 1);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["glider_handler"], 95, 50);
	event:SetFinalDirection(hoa_map.MapMode.SOUTH);
	event:AddEventLinkAtEnd(event_id + 1);
	event_id = event_id + 1; event = hoa_map.DialogueEvent.Create(event_id, dialogues["laila_farewell_2"]);
	event:AddEventLinkAtEnd(event_id + 1);
	event:AddEventLinkAtEnd(event_id + 2);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["glider_handler"], 92, 74);
	event_id = event_id + 1; event = hoa_map.DialogueEvent.Create(event_id, dialogues["laila_farewell_3"]);
	event:AddEventLinkAtEnd(event_id + 1);
	event:AddEventLinkAtEnd(event_id + 2, 100);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["claudius"], 0, -5);
	event:SetRelativeDestination(true);
	event_id = event_id + 1; event = hoa_map.DialogueEvent.Create(event_id, dialogues["laila_farewell_4"]);
	event:AddEventLinkAtEnd(event_id + 1, 700);
	event:AddEventLinkAtEnd(event_id + 2, 1000);
	event:AddEventLinkAtEnd(event_id + 3, 500);
	event_id = event_id + 1; event = hoa_map.ChangePropertySpriteEvent.Create(event_id, sprites["claudius"]);
	event:Direction(hoa_map.MapMode.EAST);
	event_id = event_id + 1; event = hoa_map.ChangePropertySpriteEvent.Create(event_id, sprites["claudius"]);
	event:Direction(hoa_map.MapMode.SOUTH);
	event_id = event_id + 1; event = hoa_map.DialogueEvent.Create(event_id, dialogues["laila_farewell_5"]);
	event:AddEventLinkAtEnd(event_id + 1, 1500);

	-- Claudius boards the craft and says final farewells
	event_id = event_id + 1; event = hoa_map.ChangePropertySpriteEvent.Create(event_id, sprites["claudius"]);
	event:Collidable(false);
	event:AddEventLinkAtEnd(event_id + 1);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["claudius"], 100, 47);
	event:SetFinalDirection(hoa_map.MapMode.EAST);
	event:AddEventLinkAtEnd(event_id + 1, 500);
	event:AddEventLinkAtEnd(event_id + 2, 500);
	event_id = event_id + 1; event = hoa_map.ChangePropertySpriteEvent.Create(event_id, sprites["claudius"]);
	event:Direction(hoa_map.MapMode.WEST);
	event_id = event_id + 1; event = hoa_map.DialogueEvent.Create(event_id, dialogues["final_farewell"]);
	event:AddEventLinkAtEnd(event_id + 1);
	event:AddEventLinkAtEnd(event_id + 2, 500);
	event_id = event_id + 1; event = hoa_map.CustomEvent.Create(event_id, "HoldCameraPosition", "");
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["claudius"], 104, 51);
	event:AddEventLinkAtStart(event_id + 1, 500);
	event:AddEventLinkAtEnd(event_id + 2);
	event_id = event_id + 1; event = hoa_map.ChangePropertySpriteEvent.Create(event_id, sprites["laila"]);
	event:Direction(hoa_map.MapMode.EAST);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["claudius"], 108, 51);
	event:SetFinalDirection(hoa_map.MapMode.EAST);
	event:AddEventLinkAtEnd(event_id + 1, 1000);

	-- Claudius sails off. Lukar, Marcus, and Vanica walk off screen. Laila stands silently at the end of the dock. Screen fades to transition screen
	-- TODO: move Claudius + glider off the screen to the right
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["lukar"], 0, 20);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtStart(event_id + 1, 250);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["marcus"], 0, 20);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtStart(event_id + 1, 100);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["vanica"], 0, 20);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtStart(event_id + 1, 100);
	event:AddEventLinkAtStart(event_id + 2, 500);
	event_id = event_id + 1; event = hoa_map.PathMoveSpriteEvent.Create(event_id, sprites["laila"], 96, 39);
	event:SetFinalDirection(hoa_map.MapMode.EAST);
	event:AddEventLinkAtEnd(event_id + 2, 2000);
	event_id = event_id + 1; event = hoa_map.CameraMoveEvent.Create(event_id, sprites["laila"], 1000);
	event_id = event_id + 1; event = hoa_map.CustomEvent.Create(event_id, "MapEndTransition", "");
end -- function CreateEvents()


-- Check if the player has selected the option that begins the finale event sequence and if so, begin that sequence
functions["CheckStartFinale"] = function()
	if (LocalRecords:GetRecord("player_starts_finale") == 1) then
		EventManager:StartEvent(event_sequences["start_finale"]);
	end
end


-- Called when the camera is focused on Claudius to move the virtual focus to his position and hold it there
functions["HoldCameraPosition"] = function()
	Map:GetVirtualFocus():MoveToObject(sprites["claudius"], true);
	Map:SetCamera(Map:GetVirtualFocus());
end


-- End the map through a transition to a custom mode that displays screen text, then returns to the main menu in boot mode
functions["MapEndTransition"] = function()
	local mode = hoa_custom.CustomMode("lua/scripts/custom/screen_display.lua");
	mode:AddOption("text1", "To Be Continued");
	mode:AddOption("initial_time", "1000");
	mode:AddOption("display_time", "2000");
	mode:AddOption("return_to_boot", "1");
	TransitionManager:SetTerminateMapOnCompletion();
	TransitionManager:StartGameModeTransition(mode);
end

