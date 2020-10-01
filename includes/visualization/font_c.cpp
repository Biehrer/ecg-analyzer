#include "font_c.h"


inline
GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

    while ( (errorCode = f->glGetError()) != GL_NO_ERROR )
    {
        std::string error;
        switch ( errorCode )
        {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 


//! Get a filepath from an Font2D_TP enum 
//!
//!\param font the font to use for drawing
//!\returns üath and filename of the .tff font file
std::string GetFontPathname(Font2D_TP font)
{
    std::string pathname_2_font;

    switch ( font )
    {
    case Font2D_TP::ARIAL:
        pathname_2_font = "C:/Development/projects/EcgAnalyzer/ecg-analyzer-build/Resources/fonts/arial.ttf";
        break;

    case Font2D_TP::CALIBRI:
        pathname_2_font = "C:/Development/projects/EcgAnalyzer/ecg-analyzer-build/Resources/fonts/calibri.ttf";
        break;

    default:
        // Load arial on default
        pathname_2_font = "C:/Development/projects/EcgAnalyzer/ecg-analyzer-build/Resources/fonts/arial.ttf";
        break;
    }

    return pathname_2_font;
}

Font::~Font() 
{
    Timer_C("Cleanup Font");
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    for ( const auto& character : _characters ) {
        f->glDeleteTextures(1, &character.second.TextureID);
        glCheckError();
    }
}

Font::Font(Font2D_TP font)
{
    Timer_C("init font");
    
    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if ( FT_Init_FreeType(&ft) )
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    // Refactor in function
    // Load path to the font
    // Todo: load paths from a textfile with a configuration manager ?
    std::string pathname_2_font = GetFontPathname(font);
    _font = font;

    // Load font as face
    FT_Face face;
    if ( FT_New_Face(ft, pathname_2_font.c_str(), 0, &face) )
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for ( GLubyte c = 0; c < 128; c++ )
    {
        // Load character glyph 
        if ( FT_Load_Char(face, c, FT_LOAD_RENDER) )
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }

        // Generate texture
        GLuint texture = 0; // Handle to the texture
        f->glGenTextures(1, &texture);
        f->glBindTexture(GL_TEXTURE_2D, texture);
        f->glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glCheckError();
        // Set texture options
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        QVector<GLfloat> vec2_x;
        vec2_x.push_back(face->glyph->bitmap.width);
        vec2_x.push_back(face->glyph->bitmap.rows);

        QVector<GLfloat> vec2_y;
        vec2_y.push_back(face->glyph->bitmap_left);
        vec2_y.push_back(face->glyph->bitmap_top);

        Character character = {
            texture,
            vec2_x,
            vec2_y,
            static_cast<GLuint>(face->glyph->advance.x)
        };

        _characters.insert(std::make_pair(c, character));
    }
    f->glBindTexture(GL_TEXTURE_2D, 0);

    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}


Character
Font::GetCharacter(const GLchar character)
{
    return _characters.at(character);
}

const 
Font2D_TP 
Font::GetFont() const
{
    return _font;
}



