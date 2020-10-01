#pragma once
// Project includes
#include "font_c.h"

// STL includes
#include <vector>

//! Singleton FontManager_C class
class FontManager_C {

private:
    static std::vector<Font*> _fonts;

private:
    // All rendering is single threaded. 
    // This should be enough to declare this class Singleton
    FontManager_C();
    FontManager_C(const FontManager_C& other) = delete;
    FontManager_C& operator=(const FontManager_C& other) = delete;

public:
    //! Adds a new font to the manager
    //! returns 0 if the font is already initialized and 1 on success.
    //! When a font is added, its textures are loaded onto the gpu and characters can be rendered over quads by binding the
    //! corresponding texture ID of a character, before drawing the quad (See TextRenderer2D::RenderText(..))
    static int AddFont(Font2D_TP font); // emplaces(construct inplace) a new font inside _fonts map

    //! Get specific Character of a specific font.
    //! Returns invalid Character, when the Font2D_TP is not initialized
    static Character GetFontCharacter(Font2D_TP font, const GLchar character);

    //! Get a pointer to a Font object
    static const Font* GetFont(Font2D_TP font);

    //! Removes all loaded fonts
    static void Clean();
};