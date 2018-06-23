--------------------------------------------------------------------------------
-- screen_display.lua
--
-- This custom mode is used for displaying simple full-screen images and/or series of text strings
-- to the screen for a duration of time. Its most common two uses are for display a scene image, and
-- for using text in between a mode transition to indicate a passage of time, narration, etc.
--
-- The option keys that this implementation of CustomMode recognizes are as follows:
--   "return_to_boot" : Pops all game modes from the stack and returns to the main menu in boot mode
--   "map"            : Filename of a map to load as this mode terminates
--   "image"          : Filename of the background image to display
--   "initial_time"   : Number of milliseconds to wait after the mode starts before displaying the first line
--   "display_time"   : Number of milliseconds to wait after a line of text has been actively displayed before continuing
--   "text1"          : String of text to display. text1 is for the first line, text2 is for the second, etc.
--
-- TODO: add support for screen fading both into and out of custom mode
--------------------------------------------------------------------------------
local ns = {}
setmetatable(ns, {__index = _G})
screen_display = ns;
setfenv(1, ns);


-- This table represents the mode class and holds all of the member data
Mode = {
	 -- A pointer to the CustomMode object that is running this script
	instance = nil,

	-- The array of lines of text that will be displayed to the screen
	text_lines = {},

	-- A counter used to keep track of the current line being displayed (0 == no line)
	line_counter = 0,

	-- The total number of lines of text to display
	text_line_count = 0,

	-- Used to determine when the initial time for the class has expired. Always true if there is no initial time
	initial_time_finished = true,

	-- Number of milliseconds to wait between class initialization and beginning to display the first line of text
	initial_time = 0,

	-- Number of milliseconds to wait after a line of text is fully displayed (rendering animation finished) before moving to the next
	display_time = 0,

	-- A hoa_gui.TextBox object that renders the text to the screen
	textbox = nil,

	-- A hoa_system.SystemTimer object used for line timings
	timer = nil,

	-- Background image to display. Should be a full-screen sized image
	image = nil
};


-- Called by CustomMode::Reset() whenever the mode becomes the active game mode
function Reset(mode)
	if (Mode.instance == nil) then
		Mode.instance = mode;
	end

	if (Mode.instance._load_complete == false) then
		_Initialize();
	end

	VideoManager:SetStandardCoordSys();
	VideoManager:SetDrawFlag(hoa_video.VideoEngine.VIDEO_X_LEFT);
	VideoManager:SetDrawFlag(hoa_video.VideoEngine.VIDEO_Y_TOP);
end


-- Called by CustomMode::Update() on every iteration of the game loop
function Update()
	Mode.timer:Update();

	-- Don't do anything else until after the initial wait time is finished
	if (Mode.initial_time_finished == false) then
		if (Mode.timer:IsFinished() == true) then
			Mode.timer:Reset();
			Mode.initial_time_finished = true;
		end
		return;
	end

	-- Update the text box and if it is finished displaying the text, process the display time as needed
	Mode.textbox:Update(SystemManager:GetUpdateTime());
	if (Mode.textbox:IsFinished() == true) then
		if (Mode.display_time > 0) then
			if (Mode.timer:IsInitial() == true) then
				Mode.timer:Run();
			elseif (Mode.timer:IsRunning() == true) then
				return;
			elseif (Mode.timer:IsFinished() == true) then
				_FinishLine();
			end
		else
			_FinishLine();
		end
	end
end


-- Called by CustomMode::Draw() to draw all elements to the screen
function Draw()
	if (Mode.image ~= nil) then
		Mode.image:Draw();
	end

 	Mode.textbox:Draw();
end


-- Sets up all objects and loads appropriate data. Call only on the very first invocation of the Reset() function.
function _Initialize()
	local option;

	-- Load the background image if one exists
	option = Mode.instance:GetOption("image");
	if (option ~= "") then
		Mode.image = hoa_video.StillImage(false);
		Mode.image:Load(option);
	end

	-- Load all the lines of text
	local i = 1;
	while (true) do
		option = Mode.instance:GetOption("text" .. i);
		if (option == "") then
			break;
		else
			table.insert(Mode.text_lines, option);
			Mode.line_counter = 1;
			Mode.text_line_count = Mode.text_line_count + 1;
			i = i + 1;
		end
	end

	-- Setup the text box used to display the lines of text
	Mode.textbox = hoa_gui.TextBox(512 - 400, 200, 800, 30, hoa_gui.TextBox.VIDEO_TEXT_FADELINE);
	-- TODO: the text box doesn't appear to properly respect these flags and center itself horizontally on the screen
	Mode.textbox:SetAlignment(hoa_video.VideoEngine.VIDEO_X_CENTER, hoa_video.VideoEngine.VIDEO_Y_CENTER);
	Mode.textbox:SetTextStyle(hoa_video.TextStyle("text24"));
	Mode.textbox:SetTextAlignment(hoa_video.VideoEngine.VIDEO_X_RIGHT, hoa_video.VideoEngine.VIDEO_Y_CENTER);
	Mode.textbox:SetDisplaySpeed(20);
	if (Mode.line_counter > 0) then
		Mode.textbox:SetDisplayText(Mode.text_lines[1]);
	end

	-- Setup the timer and timing data
	option = Mode.instance:GetOption("initial_time");
	if (option ~= "") then
		Mode.initial_time = tonumber(option);
	end
	option = Mode.instance:GetOption("display_time");
	if (option ~= "") then
		Mode.display_time = tonumber(option);
	end

	Mode.timer = hoa_system.SystemTimer(Mode.initial_time, 0);
	if (Mode.initial_time ~= 0) then
		Mode.initial_time_finished = false;
		Mode.timer:Run();
	end
end


-- Marks the current line of text as complete and loads the next. If there are no lines remaining, terminates the mode instance
function _FinishLine()
	Mode.line_counter = Mode.line_counter + 1;
	if (Mode.line_counter > Mode.text_line_count) then
		_Terminate();
	else
		Mode.timer:Reset();
		Mode.textbox:SetDisplayText(Mode.text_lines[Mode.line_counter]);
	end
end


-- Ends the custom mode by removing it from the stack. May push another game mode to the stack if certain options were set
function _Terminate()
	ModeManager:Pop();

	local option;
	-- Check if the game should remove all modes and return to the main menu screen in boot mode
	option = Mode.instance:GetOption("return_to_boot");
	if (option == "1") then
		ModeManager:PopAll();
		ModeManager:Push(hoa_boot.BootMode());
		return;
	end

	-- Check if we should load a new map
	option = Mode.instance:GetOption("map");
	if (option ~= "") then
		ModeManager:Push(hoa_map.MapMode(option));
		return;
	end
end