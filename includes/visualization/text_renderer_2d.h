#pragma once

// STL includes
#include <iostream>
#include<assert.h>
#include <mutex>
// Qt includes
#include <qopenglbuffer.h>
#include <qvector.h>
#include <qopenglfunctions.h>
#include <qopenglextrafunctions.h>
#include <qopenglshaderprogram.h>
#include <qvector3d.h>
#include <qvector2d.h>

// Freetype
#include "ft2build.h"
#include FT_FREETYPE_H 

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
//! Defines the font to use when rendering text
enum Font2D_TP {
    ARIAL,
    TNR,
    CALIBRI
};
//
//!  Description: 
//!  A two dimensional opengl text renderer which 
//!  renders text with the freetype library and Qt OpenGL (QOpenGLWidget)
//! 
//! Usage:
//! Make sure there is an active OpenGL context (InitializeGLFunctions() was called inside the QOpenGLWidget)
//! - Call .Initialize() to setup the object internals and set the text font. ! Attention: This already requires an active opengl context 
//! - Call .SetText() to specify the render text and position of the text inside the OGL window.
//! - Call .RenderText() inside the rendering loop ( paintGL() ) to draw the previously specified text onscreen
//!
//! Requires an fragment shader which can manage textures (Sampler2D)
//
class OGLTextBox {
    // Construction / Destruction / Copying..
public:
    OGLTextBox()
    {
        //std::cout << "construct textbox, VAO = " << _VAO << ", VBO = " << _VBO << std::endl;
        _mutex = new std::mutex();
    }

    //OGLTextBox(const OGLTextBox& other) = delete;
    OGLTextBox& operator=(const OGLTextBox& other) = delete;

    OGLTextBox(const OGLTextBox& other) = delete;
    //{
    //    _mutex = other._mutex;
    //    _initialized = other._initialized;
    //    _VAO = other._VAO;
    //    _VBO = other._VBO;
    //    _characters = other._characters;
    //    _current_text = other._current_text;
    //    _text_set = other._text_set;
    //}
    
    //OGLTextBox& operator=(const OGLTextBox& other) 
    //{
    //    _mutex = new std::mutex();
    //    _mutex = other._mutex;
    //    _initialized = other._initialized;
    //    _VAO = other._VAO;
    //    _VBO = other._VBO;
    //    _characters = other._characters;
    //    _current_text = other._current_text;
    //    _text_set = other._text_set;
    //    return *this;
    //}

    ~OGLTextBox()
    {   
        std::cout << "Destruct textbox, VAO = " << _VAO << ", VBO = " << _VBO  << std::endl;
        if ( _initialized ){
            _mutex->lock();
            Cleanup();
            _mutex->unlock();
        }
        delete _mutex;

    }

private:
    //! Holds all state information relevant to a character as loaded using FreeType
    struct Character {
        GLuint TextureID;   // ID handle of the glyph texture
        QVector<GLfloat> Size;    // Size of glyph
        QVector<GLfloat> Bearing;  // Offset from baseline to left/top of glyph
        GLuint Advance;    // Horizontal byte_offset to advance to next glyph
        QVector<GLfloat> vertices;
        char character;
    };

    bool _initialized = false;

    std::mutex* _mutex;
    // Public access functions
public:
    //! Initialize the freetype library
    //! Creates the Freetype character glyphs and stores them in the member map
    //! This function requires an active OpenGL context to work
    //! 
    //! \param font the font style to use when loading character glyphs
    void Initialize(Font2D_TP font)
    {
        if ( !_initialized ){ 
            //_characters.clear();
            std::unique_lock<std::mutex> lck(*_mutex);
            //if( _characters.size() > 0){
            //    _characters.erase(_characters.begin(), _characters.end());
            //}

            // FreeType
            FT_Library ft;
            // All functions return a value different than 0 whenever an error occurred
            if ( FT_Init_FreeType(&ft) )
                std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

            // Refactor in function
            // Load path to the font
            // Todo: load paths from a textfile with a configuration manager ?
            std::string pathname_2_font = GetFontPathname(font);

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

            _initialized = true;
        }
    }

    bool IsTextSet() const {
        return _text_set;
    }
    //! Set the text to render via RenderText(..)
    //! The test must be set before it can be rendered
    void SetText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale)
    {
        //_current_text.clear();
        //std::vector<Character>().swap(_current_text);
        //_current_text.erase(_current_text.begin(), _current_text.end());
        //_current_text.resize(0);
        assert(_current_text.size() == 0);

        auto* f = QOpenGLContext::currentContext()->functions();
        auto* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        //f->glActiveTexture(GL_TEXTURE0); // GO BACK
        //extra_f->glBindVertexArray(_VAO);

        // Iterate through all characters
        // Create vertices (quads made from triangles) for the text.
        for ( std::string::const_iterator c = text.begin(); c != text.end(); c++ ) {
            Character ch = _characters[*c];
            // Calculate character position
            GLfloat xpos = x + ch.Bearing[0] * scale;
            // Use the character T as reference to calculate the descent 
            // (descent=distance of the letter to some reference line at the top / bottom of the text line )
            // -> Use 'T', because T is always the biggest letter
            GLfloat ypos = y + (_characters['T'].Size[1] - ch.Bearing[1]) * scale;

            // Calculate character width / height. 
            // Scaling could also be done inside the shader
            // -> but because my solution does this only when the text is initialized, it should be not that much of a perfomance difference
            GLfloat w = ch.Size[0] * scale;
            GLfloat h = ch.Size[1] * scale;
            QVector<GLfloat> vertices({
                // Flip UV's second component to flip the texture vertically
                 xpos,     ypos + h,   0.0, 1.0 ,
                 xpos,     ypos,       0.0, 0.0 ,
                 xpos + w, ypos,       1.0, 0.0 ,

                 xpos,     ypos + h,   0.0, 1.0 ,
                 xpos + w, ypos,       1.0, 0.0 ,
                 xpos + w, ypos + h,   1.0, 1.0
                });
            // Store the text for rendering
            ch.vertices = vertices;
            ch.character = *c;
            _current_text.push_back(ch);
            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.Advance >> 6) * scale;
            // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }
        //_mutex->lock();

        // Allocate an vertex buffer which can store the current text
        SetupVAO();

        // Send the data to the graphics card
        WriteTextToVBO();

        //_mutex->unlock();
        _text_set = true;
    }

    //! Renders directly from the buffer and calls glDraw for each preprocessed character quad.
    //! Requires, that the text is set before with SetText(..);
    //! Draws all character quads with their corresponding character texture, each ine one draw call.
    //! 
    //! \param shader a shader which can render text (handle textures)
    //! \param color the color the text will be rendered with
    //! \param model_view_projection the current active mvp transform
    void RenderText(QOpenGLShaderProgram &shader,
        QVector3D& color,
        QMatrix4x4& model_view_projection) /*const*/
    {
        _mutex->lock();

        auto* f = QOpenGLContext::currentContext()->functions();
        auto* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        shader.bind();
        shader.setUniformValue("u_MVP", model_view_projection);
        shader.setUniformValue("u_text_color", color);
        
        //f->glEnable(GL_CULL_FACE);
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        f->glActiveTexture(GL_TEXTURE0);
        glCheckError();
        int byte_offset = 0;
        // The char quad is made of two triangles, ach of them made of three vertices
        // Each quad is drawn by itself
        int number_of_vertices_per_char = 6; 
        extra_f->glBindVertexArray(_VAO);
        // WHEN THE TEXTBOX IS DELETED FROM THE SIGNAL TRIGGERED BY ME, SOMEONE STILL IS TRYING TO RENDER IT, although it should be removed 
        glCheckError();

        for ( const auto& character : _current_text ) {
            f->glBindTexture(GL_TEXTURE_2D, character.TextureID);
            //extra_f->glBindVertexArray(_VAO);
            glCheckError();

            f->glDrawArrays(GL_TRIANGLES, byte_offset, number_of_vertices_per_char);
            glCheckError();
            byte_offset += number_of_vertices_per_char; 
        }
        extra_f->glBindVertexArray(0);
        f->glBindTexture(GL_TEXTURE_2D, 0);
        f->glDisable(GL_BLEND);
        shader.release();
        _mutex->unlock();
    }

    // Private helper functions
private:
    //!  Allocate memory to store the whole text 
    //! this function needs to be called when the text changes because the vbo has to adapt its size
    void SetupVAO() {
        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
        QOpenGLExtraFunctions* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        // Configure VAO/VBO for texture quads
        //if( !_text_set ){
            std::cout << "generate buffers ( because text was not set before and we needed to generate)" << std::endl;
            extra_f->glGenVertexArrays(1, &_VAO);
            glCheckError();
            f->glGenBuffers(1, &_VBO);
            glCheckError();
        //}
        std::cout << "construct textbox, VAO = " << _VAO << ", VBO = " << _VBO << std::endl;

        extra_f->glBindVertexArray(_VAO);
        glCheckError();
        f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glCheckError();
        // This does resize the buffer! it's not necessary to delete the old one and create a new one
        // DO I need to respect the texture size?
        f->glBufferData(GL_ARRAY_BUFFER, this->GetVBOSizeInBytes(), nullptr, GL_DYNAMIC_DRAW /*GL_STATIC_DRAW*/); 
        glCheckError();
        f->glEnableVertexAttribArray(0);
        glCheckError();
        f->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0); //  <vec2 pos, vec2 tex>
        glCheckError();
        f->glBindBuffer(GL_ARRAY_BUFFER, 0);
        extra_f->glBindVertexArray(0);
    }

    void Cleanup() 
    {
        if ( _text_set ) {
            QOpenGLExtraFunctions* f_extra = QOpenGLContext::currentContext()->extraFunctions();
            QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
            //QOpenGLContext::currentContext()->makeCurrent(ui._openGlWidget.GetSurface());
            f_extra->glBindVertexArray(_VAO);
            glCheckError();
            f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
            glCheckError();
            GLint size = 0;
            f_extra->glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
            if ( GetVBOSizeInBytes() != size )
            {
                std::cout << "Size of VBO differs from real size" << std::endl;
                //throw std::runtime_error("Error while freeing graphics memory");
                // Log the error
            }

            // The textures never are deleted, because before this function is called, 
            // the last text inside _current_text already was already cleared so its empty

            for (const auto& character : _characters /*_current_text*/) {
                f->glDeleteTextures(1, &character.second.TextureID);
                glCheckError();
            }
            f_extra->glDeleteVertexArrays(1, &_VAO);
            glCheckError();
            f_extra->glDeleteBuffers(1, &_VBO);
            glCheckError();
            //_text_set = false;
        }
    }

    //! Writes the current text to the VBO as textured GL_QUADS
    void WriteTextToVBO()
    {
        auto* f = QOpenGLContext::currentContext()->functions();
        auto* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        // Store the quads + texture data for each quad inside the VBO
        f->glActiveTexture(GL_TEXTURE0);
        extra_f->glBindVertexArray(_VAO);
        glCheckError();
        f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glCheckError();
        // Offset to write each character for itself inside the VBO in bytes
        GLint byte_write_offset = 0;
        // The size of one character-glyph in bytes
        GLint char_glyph_size_bytes = sizeof(GLfloat) * 6 * 4;
        for ( const auto& ch : _current_text ) {
            // Render glyph texture over quad
            f->glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            glCheckError();
            // Update content of VBO memory
            // Be sure to use glBufferSubData and not glBufferData
            //f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
            f->glBufferSubData(GL_ARRAY_BUFFER,
                byte_write_offset,
                char_glyph_size_bytes,
                ch.vertices.constData()
            );
            glCheckError();
            //f->glBindBuffer(GL_ARRAY_BUFFER, 0);
            byte_write_offset += char_glyph_size_bytes;
        }
        f->glBindBuffer(GL_ARRAY_BUFFER, 0);

        assert(byte_write_offset == this->GetVBOSizeInBytes());

        extra_f->glBindVertexArray(0);
        f->glBindTexture(GL_TEXTURE_2D, 0);
    }

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

    //! Returns the size of the vertex buffer object in bytes
    //! Calculation:
    //! Each Quad (=2 x 3 = 6vertices) requires 
    //! 4 floats ( 2 position coordinates and 2 texture coorindates)
    //! => two triangles(6verts in sum) per char. Each verte

    // one triangle = 3 verts. => 6 verts for triangle
    // for the quad = 4 coordinates(2 texture positions + 2vertex positions )
    //! One quad can be used to display one letter of text
    //!
    //! \returns the size of the vertex buffer object in bytes
    GLint GetVBOSizeInBytes() {
        return _current_text.size() * 4 * 6 * sizeof(GLfloat);
    }

    // Private attributes
private:
    //! Vertex array and buffer object
    GLuint _VAO = 0;
    GLuint _VBO = 0;

    //! Stores 128 ascii character glyphs
    std::map<GLchar, Character> _characters;

    //! The characters of the current text
    std::vector<Character> _current_text;

    //! True when the text of the object was set via SetText(..)
    bool _text_set = false;
};

