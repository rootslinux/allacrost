-- Contains the definitions for all GUI themes that are available to use in the game

-- The default theme used when the user settings (from settings.lua) are either invalid or unavailable.
-- This is also the theme that the user will be first exposed to
default_theme = "Black Sleet";

-- Each entry in this table defines the image files that make up the gui theme.
-- All entries must contain the full set of files. It is perfectly fine to share
-- the same image between multiple themes.
themes = {
    ["Black Sleet"] = {
        background = "img/menus/black_sleet_texture.png",
        border = "img/menus/black_sleet_skin.png",
        cursor = "img/menus/cursor.png"
    }
}

