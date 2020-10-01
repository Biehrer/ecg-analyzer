#pragma once

// Project includes
#include "timer_c.h"

// STL includes
#include <iostream>
#include <map>

// Qt includes
#include "qvector.h"
#include <qopenglfunctions.h>

// Freetype
#include "ft2build.h"
#include FT_FREETYPE_H 

//! Defines the font to use when rendering text
enum Font2D_TP {
    ARIAL,
    TNR,
    CALIBRI
};

//! Holds all state information relevant
//! to a character as loaded using FreeType
class Character {

public:
    Character& operator=(const Character& other) = default;

public:
    //! ID handle of the glyph texture
    GLuint TextureID;   
    //! Size of glyph
    QVector<GLfloat> Size;    
    //! Offset from baseline to left/top of glyph
    QVector<GLfloat> Bearing; 
    //! Horizontal byte_offset to advance to next glyph
    GLuint Advance;   
    //! Vertices of the char
    QVector<GLfloat> vertices;
    //! character this struct represents
    char character;
};

//! This class is repsonsible to load character glyphs via Freetype 
//! and offer the glyphs to the outside for text rendering inside a OpenGL context
class Font {
    // Construction / Destruction / Copying
public:
    //! Initialize the freetype library
    //! Creates the Freetype character glyphs and stores them in the member map
    //! This function requires an active OpenGL context to work
    //! 
    //! \param font the font style to use when loading character glyphs
    Font(Font2D_TP font);// remove scale because scale is only important inside SetText of the text label lass
    
    ~Font();

    Font(const Font& other) = default;
    Font& operator=(const Font& other) = default;

    // Public access function
public:
    //! Returns the Character struct ( with texture id and character vertices) from a ASCII character
    //! \param character the char (ASCII (-> c > 0 && c <128)) from which the vertices and Texture ID is required
    //! \returns the character struct of this font
    Character GetCharacter(const GLchar character);
    const Font2D_TP GetFont() const;

    // Private attributes
private:
    //! Stores all characters of this font
    std::map<GLchar, Character> _characters;

    //! Identifier of this font
    Font2D_TP _font;
};