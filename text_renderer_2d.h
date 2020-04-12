#pragma once

// STL includes
#include <iostream>

// Qt includes
#include <qopenglbuffer.h>
#include <qvector.h>
#include <qopenglfunctions.h>
#include <qopenglextrafunctions.h>
#include <qopenglshaderprogram.h>
#include <qvector3d.h>
#include <qvector2d.h>

// Freetype
#include <ft2build.h>
#include FT_FREETYPE_H 

enum Font2D_TP {
    ARIAL,
    TNR,
    CALIBRI
};

// Draws text with the freetype library
class TextBox {

public:
    // Should be private
    /// Holds all state information relevant to a character as loaded using FreeType
    struct Character {
        GLuint TextureID;   // ID handle of the glyph texture
        QVector<GLfloat> Size;    // Size of glyph
        QVector<GLfloat> Bearing;  // Offset from baseline to left/top of glyph
        GLuint Advance;    // Horizontal offset to advance to next glyph
        QVector<GLfloat> vertices;
        char character;
    };


    // Construction / Destruction / Copying..
public:
    TextBox()
    {
    }

    ~TextBox()
    {
        // DELETE THE VAO/ VBO
    }


    // Public access functions
public:

    //! Creates the VAO and Freetype character glyphs and stores them in the member map
    //! This function requires a active OpenGL context 
    void Initialize(Font2D_TP font)
    {
        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

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
            GLuint texture;
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
                face->glyph->advance.x
            };
            _characters.insert(std::pair<GLchar, Character>(c, character));
        }
        f->glBindTexture(GL_TEXTURE_2D, 0);

        // Destroy FreeType once we're finished
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        // Create the vertex array object
        //SetupVAO();
    }


    void SetText(const std::string& text,
        GLfloat x,
        GLfloat y,
        GLfloat scale)
    {
        _current_text.clear();

        auto* f = QOpenGLContext::currentContext()->functions();
        auto* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        f->glActiveTexture(GL_TEXTURE0);
        extra_f->glBindVertexArray(VAO);

        // Iterate through all characters
        std::string::const_iterator c;
        for ( c = text.begin(); c != text.end(); c++ )
        {
            Character ch = _characters[*c];
            GLfloat xpos = x + ch.Bearing[0] * scale;
            // Use the character T as reference to calculate the descent -> because T is always the biggest letter
            GLfloat ypos = y + (_characters['T'].Size[1] - ch.Bearing[1]) * scale;  

            GLfloat w = ch.Size[0] * scale;
            GLfloat h = ch.Size[1] * scale;
            // Update VBO for each character
            QVector<GLfloat> vertices({
                // Flip UV's second component -> seems to work
                 xpos,     ypos + h,   0.0, 1.0 ,
                 xpos,     ypos,       0.0, 0.0 ,
                 xpos + w, ypos,       1.0, 0.0 ,

                 xpos,     ypos + h,   0.0, 1.0 ,
                 xpos + w, ypos,       1.0, 0.0 ,
                 xpos + w, ypos + h,   1.0, 1.0
            });

            ch.vertices = vertices;
            _current_text.push_back(ch);
            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }

        // Idea: write all characters at once
        //SetupVAOCustom();
        // Update content of VBO memory
        //f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
        //f->glBufferData(GL_ARRAY_BUFFER, sizeof(_current_vertices), _current_vertices.data(), GL_STATIC_DRAW); // Be sure to use glBufferSubData and not glBufferData
        //f->glBindBuffer(GL_ARRAY_BUFFER, 0);
        extra_f->glBindVertexArray(0);
        f->glBindTexture(GL_TEXTURE_2D, 0);
    }


    void SetTextCustom(const std::string& text,
        GLfloat x,
        GLfloat y,
        GLfloat scale)
    {
        _current_text.clear();

        auto* f = QOpenGLContext::currentContext()->functions();
        auto* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        f->glActiveTexture(GL_TEXTURE0);
        extra_f->glBindVertexArray(VAO);

        // Iterate through all characters
        std::string::const_iterator c;
        int char_offset = 0;
        for ( c = text.begin(); c != text.end(); c++ )
        {
            Character ch = _characters[*c];
            GLfloat xpos = x + ch.Bearing[0] * scale;
            // Use the character T as reference to calculate the descent -> because T is always the biggest letter
            GLfloat ypos = y + (_characters['T'].Size[1] - ch.Bearing[1]) * scale;

            GLfloat w = ch.Size[0] * scale;
            GLfloat h = ch.Size[1] * scale;
            // Update VBO for each character
            QVector<GLfloat> vertices({
                // Flip UV's second component to flip the texture vertically
                 xpos,     ypos + h,   0.0, 1.0 ,
                 xpos,     ypos,       0.0, 0.0 ,
                 xpos + w, ypos,       1.0, 0.0 ,

                 xpos,     ypos + h,   0.0, 1.0 ,
                 xpos + w, ypos,       1.0, 0.0 ,
                 xpos + w, ypos + h,   1.0, 1.0
             });

            ch.vertices = vertices;
            ch.character = *c;
            //ch.character = text[char_offset];
            ++char_offset;
            _current_text.push_back(ch);

            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.Advance >> 6) * scale;
            // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }

        // Idea: write all characters at once
        SetupVAOCustom();

        f->glActiveTexture(GL_TEXTURE0);
        extra_f->glBindVertexArray(VAO);
        GLint byte_idx = 0;
        for ( const auto& ch : _current_text ) {
            // Render glyph texture over quad
            f->glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            //extra_f->glBindVertexArray(VAO);
            // Update content of VBO memory
            GLint char_glyph_size_bytes = sizeof(GLfloat) * 6 * 4;
            // Be sure to use glBufferSubData and not glBufferData
            f->glBindBuffer(GL_ARRAY_BUFFER, VBO);

            f->glBufferSubData(GL_ARRAY_BUFFER,
                byte_idx,
                char_glyph_size_bytes,
                ch.vertices.constData()
            );

            f->glBindBuffer(GL_ARRAY_BUFFER, 0);

            //f->glBufferSubData(GL_ARRAY_BUFFER,
            //    byte_idx,
            //    sizeof(ch.vertices),
            //    ch.vertices.constData()
            //);
            byte_idx += char_glyph_size_bytes;// +1;
        }

        extra_f->glBindVertexArray(0);
        f->glBindTexture(GL_TEXTURE_2D, 0);

        _num_bytes_of_current_text = byte_idx;
        
       assert( _num_bytes_of_current_text ==  _current_text.size() * 4 * 6 * sizeof(GLfloat) );
        // Update content of WHOLE VBO memory at once instead of sub buffering
        //f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
        //f->glBufferData(GL_ARRAY_BUFFER, sizeof(_current_vertices), _current_vertices.data(), GL_STATIC_DRAW); // Be sure to use glBufferSubData and not glBufferData
        //f->glBindBuffer(GL_ARRAY_BUFFER, 0);

    }

    //! Renders directly from the buffer and calls glDraw for each preprocessed character quad. Binds the texture before the draw
    //! Requiers that the text is set before with SetTextCustom(..);
    void RenderTextFastest(QOpenGLShaderProgram &shader,
        QVector3D& color,
        QMatrix4x4& model_view_projection)
    {
        auto* f = QOpenGLContext::currentContext()->functions();
        auto* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        shader.bind();
        shader.setUniformValue("u_MVP", model_view_projection);
        shader.setUniformValue("u_text_color", color);
        //f->glEnable(GL_CULL_FACE);
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        f->glActiveTexture(GL_TEXTURE0);
        int offset = 0;
        // The quad is made of two triangles each of them made of three vertices 
        int number_of_vertices_per_char = 6; 
        for ( const auto& character : _current_text ) {

            f->glBindTexture(GL_TEXTURE_2D, character.TextureID);
            extra_f->glBindVertexArray(VAO);
            f->glDrawArrays(GL_TRIANGLES, offset, number_of_vertices_per_char);

            offset += number_of_vertices_per_char; 
        }

        extra_f->glBindVertexArray(0);
        f->glBindTexture(GL_TEXTURE_2D, 0);

        //Goal: Render all character quads with one draw call. 
        //But then the Vertex array needs to store textures in slots 
        // -> how many textures can it store in its slots? this is limiting the text which can be displayed

        //f->glDrawArrays(GL_TRIANGLES, 0, (6 * _current_text.size()));
        //f->glBindTexture(GL_TEXTURE_2D, 0);

        f->glDisable(GL_BLEND);
        //f->glDisable(GL_CULL_FACE);

        shader.release();
    }

    void RenderTextCustom(QOpenGLShaderProgram &shader,
        QVector3D& color,
        QMatrix4x4& model_view_projection)
    {
        auto* f = QOpenGLContext::currentContext()->functions();
        auto* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        shader.bind();
        shader.setUniformValue("u_MVP", model_view_projection);
        shader.setUniformValue("u_text_color", color);

        //f->glEnable(GL_CULL_FACE);
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        f->glActiveTexture(GL_TEXTURE0);

        for ( const auto& ch : _current_text ) {
            // Render glyph texture over quad
            f->glBindTexture(GL_TEXTURE_2D, ch.TextureID);

            extra_f->glBindVertexArray(VAO);
            // Update content of VBO memory
            f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
            // Be sure to use glBufferSubData and not glBufferData
            f->glBufferSubData(GL_ARRAY_BUFFER,
                0,
                /*sizeof(ch.vertices.data())*/ sizeof(GLfloat) * 6 * 4,
                ch.vertices.data()
            );

            f->glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Render character quad
            f->glDrawArrays(GL_TRIANGLES, 0, 6);
            extra_f->glBindVertexArray(0);
        }
        f->glBindTexture(GL_TEXTURE_2D, 0);
        f->glDisable(GL_BLEND);
        //f->glDisable(GL_CULL_FACE);

        shader.release();
    }

    void RenderText(QOpenGLShaderProgram &shader,
        std::string text,
        GLfloat x,
        GLfloat y,
        GLfloat scale,
        QVector3D& color,
        QMatrix4x4& model_view_projection)
    {
        shader.bind();
        shader.setUniformValue("u_MVP", model_view_projection);
        shader.setUniformValue("u_text_color", color);

        auto* f = QOpenGLContext::currentContext()->functions();
        auto* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        //f->glEnable(GL_CULL_FACE);
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        f->glActiveTexture(GL_TEXTURE0);
        extra_f->glBindVertexArray(VAO);

        // Iterate through all characters
        std::string::const_iterator c;
        for ( c = text.begin(); c != text.end(); c++ )
        {
            Character ch = _characters[*c];
            GLfloat xpos = x + ch.Bearing[0] * scale;
            GLfloat ypos = y + (_characters['T'].Size[1] - ch.Bearing[1]) * scale;

            GLfloat w = ch.Size[0] * scale;
            GLfloat h = ch.Size[1] * scale;
            // Update VBO for each character
            GLfloat vertices[6][4] = {

                // Flip UV's second component -> seems to work
                { xpos,     ypos + h,   0.0, 1.0 },
                { xpos,     ypos,       0.0, 0.0 },
                { xpos + w, ypos,       1.0, 0.0 },

                { xpos,     ypos + h,   0.0, 1.0 },
                { xpos + w, ypos,       1.0, 0.0 },
                { xpos + w, ypos + h,   1.0, 1.0 }

                // p1 - top left
                // p2 - bottom left (unique)
                // p3 - bottom right
              
                // p4 - top left
                // p5 - bottom right
                // p6 - top right (unique)
                //p1/4----p6
                // |       | 
                // |       | 
                // p2-----p3/5
            };
            // Render glyph texture over quad
            f->glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            // Update content of VBO memory
            f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
            f->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            // Be sure to use glBufferSubData and not glBufferData
            f->glBindBuffer(GL_ARRAY_BUFFER, 0);
            // Render quad
            f->glDrawArrays(GL_TRIANGLES, 0, 6);
            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }
        extra_f->glBindVertexArray(0);
        f->glBindTexture(GL_TEXTURE_2D, 0);
        f->glDisable(GL_BLEND);
        //f->glDisable(GL_CULL_FACE);

        shader.release();
    }

    // Private helper functions
private:
    // IDea: allocate memory to store the whole text from the beginning
   // this function needs to be called when the text changes because the vbo has to adapt its size
    //!! Batch all quads with the textures in one single VBO!!!
    void SetupVAOCustom() {
        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
        QOpenGLExtraFunctions* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        // Configure VAO/VBO for texture quads
        extra_f->glGenVertexArrays(1, &VAO);
        f->glGenBuffers(1, &VBO);
        extra_f->glBindVertexArray(VAO);
        GLint buffer_size = _current_text.size() * 4 * 6 * sizeof(GLfloat);
        std::cout << "VAO Size: " << buffer_size << std::endl
            << "this can fit:" << ( buffer_size / ( 4 * sizeof(GLfloat) / 6 / 2 ) ) << "# of characters" << std::endl;
        f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
        f->glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        f->glBindBuffer(GL_ARRAY_BUFFER, 0);
        extra_f->glBindVertexArray(0);
    }

    void SetupVAO() {
        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
        QOpenGLExtraFunctions* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        // Configure VAO/VBO for texture quads
        extra_f->glGenVertexArrays(1, &VAO);
        f->glGenBuffers(1, &VBO);
        extra_f->glBindVertexArray(VAO);
        f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
        f->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        f->glBindBuffer(GL_ARRAY_BUFFER, 0);
        extra_f->glBindVertexArray(0);
    }


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
        }

        return pathname_2_font;
    }

    // Private attributes
private:
    // Vertex array and buffer object
    GLuint VAO, VBO;
    // 128 ascii character glyphs
    std::map<GLchar, Character> _characters;

    //! The characters of the current text
    std::vector<Character> _current_text;

    //! The vertices to the characters inside _current_text
    QVector<GLfloat> _current_vertices;

    //! Number of bytes needed to dispaly the _current_text
    int _num_bytes_of_current_text;
};

