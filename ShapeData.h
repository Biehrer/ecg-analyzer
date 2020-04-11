#pragma once

// STL includes
#include <time.h>
#include <iostream>


// Qt includes
#include <qopenglbuffer.h>
#include <qlist.h>
#include <qvector.h>
#include <qopenglfunctions.h>
#include <qtimezone.h>
#include <qdebug.h>
#include <qelapsedtimer.h>
#include <qvector3d.h>
#include <qvector2d.h>
//#include "Sphere.h"

// Freetype
#include <ft2build.h>
#include FT_FREETYPE_H 

class ShapeData_C {
public:
	virtual int vertexBufferSize() = 0;
	virtual void Draw() {};
	virtual int getLifetime() { return 0; }

protected:
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
	QTime* lifetimer;
};

class BulletShapeData : public ShapeData_C {
	//used for bullets->flying objects
public:
	virtual void Draw() {};
	virtual int getLifetime() { return 0; }
	virtual QVector3D getPosition() { return QVector3D(-1, -1, -1); };

	virtual ~BulletShapeData() {};

protected:
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
	QTime* lifetimer;
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
        //char character;
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
    void Initialize()
    {
        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

        // FreeType
        FT_Library ft;
        // All functions return a value different than 0 whenever an error occurred
        if ( FT_Init_FreeType(&ft) )
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

        // Load font as face
        FT_Face face;
        if ( FT_New_Face(ft, "C:/Development/projects/EcgAnalyzer/ecg-analyzer-build/Resources/fonts/arial.ttf", 0, &face) )
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
            Characters.insert(std::pair<GLchar, Character>(c, character));
        }
        f->glBindTexture(GL_TEXTURE_2D, 0);

        // Destroy FreeType once we're finished
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        // Create the vertex array object
        SetupVAO();
    }


    void SetText(const std::string& text,
                 GLfloat x,
                 GLfloat y,
                 GLfloat scale)
    {
        auto* f = QOpenGLContext::currentContext()->functions();
        auto* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        f->glActiveTexture(GL_TEXTURE0);
        extra_f->glBindVertexArray(VAO);

        // Iterate through all characters
        std::string::const_iterator c;
        for ( c = text.begin(); c != text.end(); c++ )
        {
            Character ch = Characters[*c];
            GLfloat xpos = x + ch.Bearing[0] * scale;
            GLfloat ypos = y - ch.Size[1] - ch.Bearing[1] * scale;

            GLfloat w = ch.Size[0] * scale;
            GLfloat h = ch.Size[1] * scale;
            // Update VBO for each character
            QVector<GLfloat> vertices({
             xpos,     ypos + h,   0.0, 0.0,
             xpos,     ypos,       0.0, 1.0,
             xpos + w, ypos,       1.0, 1.0,

             xpos,     ypos + h,   0.0, 0.0,
             xpos + w, ypos,       1.0, 1.0,
             xpos + w, ypos + h,   1.0, 0.0
                });

            ch.vertices = vertices;
            current_text.push_back(ch);
            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }

        // Idea: write all characters at once
        //SetupVAOCustom();
        // Update content of VBO memory
        //f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
        //f->glBufferData(GL_ARRAY_BUFFER, sizeof(current_vertices), current_vertices.data(), GL_STATIC_DRAW); // Be sure to use glBufferSubData and not glBufferData
        //f->glBindBuffer(GL_ARRAY_BUFFER, 0);
        extra_f->glBindVertexArray(0);
        f->glBindTexture(GL_TEXTURE_2D, 0);
    }


    void SetTextCustom(const std::string& text,
        GLfloat x,
        GLfloat y,
        GLfloat scale)
    {
        auto* f = QOpenGLContext::currentContext()->functions();
        auto* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        f->glActiveTexture(GL_TEXTURE0);
        extra_f->glBindVertexArray(VAO);

        // Iterate through all characters
        std::string::const_iterator c;
        int offset = 0;
        for ( c = text.begin(); c != text.end(); c++ )
        {
            Character ch = Characters[*c];
            GLfloat xpos = x + ch.Bearing[0] * scale;
            GLfloat ypos = y - ch.Size[1] - ch.Bearing[1] * scale;

            GLfloat w = ch.Size[0] * scale;
            GLfloat h = ch.Size[1] * scale;
            // Update VBO for each character
            QVector<GLfloat> vertices({
             xpos,     ypos + h,   0.0, 0.0,
             xpos,     ypos,       0.0, 1.0,
             xpos + w, ypos,       1.0, 1.0,

             xpos,     ypos + h,   0.0, 0.0,
             xpos + w, ypos,       1.0, 1.0,
             xpos + w, ypos + h,   1.0, 0.0
                });
            
            ch.vertices = vertices;
            //ch.character = *c;
            //ch.character = text[offset];
            ++offset;
            current_text.push_back(ch);

            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.Advance >> 6) * scale; 
            // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }

        // Idea: write all characters at once
        SetupVAOCustom();

        f->glActiveTexture(GL_TEXTURE0);
        extra_f->glBindVertexArray(VAO);
        int byte_idx = 0;
        for ( const auto& ch : current_text ) {
            // Render glyph texture over quad
            f->glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            //extra_f->glBindVertexArray(VAO);
            // Update content of VBO memory
            f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
            int data_size_bytes = sizeof(GLfloat) * 6 * 4;
            // Be sure to use glBufferSubData and not glBufferData

            f->glBufferSubData(GL_ARRAY_BUFFER,
                byte_idx,
                sizeof(GLfloat) * 6 * 4,
                ch.vertices.data()
            );

            byte_idx += data_size_bytes;
            f->glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        extra_f->glBindVertexArray(0);
        f->glBindTexture(GL_TEXTURE_2D, 0);

        _num_bytes_of_current_text = byte_idx;

        // Update content of WHOLE VBO memory at once instead of sub buffering
        //f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
        //f->glBufferData(GL_ARRAY_BUFFER, sizeof(current_vertices), current_vertices.data(), GL_STATIC_DRAW); // Be sure to use glBufferSubData and not glBufferData
        //f->glBindBuffer(GL_ARRAY_BUFFER, 0);
        
    }

    void RenderTextFastest(QOpenGLShaderProgram &shader,
        QVector3D& color,
        QMatrix4x4& model_view_projection) 
    {
        auto* f = QOpenGLContext::currentContext()->functions();
        auto* extra_f = QOpenGLContext::currentContext()->extraFunctions();
        shader.bind();
        shader.setUniformValue("u_MVP", model_view_projection);
        shader.setUniformValue("u_text_color", color);
        f->glEnable(GL_CULL_FACE);
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        f->glActiveTexture(GL_TEXTURE0);
        extra_f->glBindVertexArray(VAO);
        int offset = 0;
        int stride = 6; // The quad is made of two triangles each of them made of three vertices 
        for ( const auto& character : current_text ) {
            f->glBindTexture(GL_TEXTURE_2D, character.TextureID);
            //f->glDrawArrays(GL_TRIANGLES, 0, 6 );
            f->glDrawArrays(GL_TRIANGLES, offset, offset + stride);
            f->glBindTexture(GL_TEXTURE_2D, 0);

            offset += stride;
        }
        
        //Goal: Render all character quads with one draw call. 
        //But then the Vertex array needs to store textures in slots 
        // -> how many textures can it store in its slots? this is limiting the text which can be displayed

        //f->glDrawArrays(GL_TRIANGLES, 0, (6 * current_text.size()));
        extra_f->glBindVertexArray(0);
        f->glBindTexture(GL_TEXTURE_2D, 0);

        f->glDisable(GL_BLEND);
        f->glDisable(GL_CULL_FACE);

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

        f->glEnable(GL_CULL_FACE);
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        f->glActiveTexture(GL_TEXTURE0);

        for ( const auto& ch : current_text ) {
            // Render glyph texture over quad
            f->glBindTexture(GL_TEXTURE_2D, ch.TextureID);

            extra_f->glBindVertexArray(VAO);
            // Update content of VBO memory
            f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
            // Be sure to use glBufferSubData and not glBufferData
            f->glBufferSubData(GL_ARRAY_BUFFER, 
                0,
                /*sizeof(ch.vertices.data())*/ sizeof(GLfloat) * 6 * 4 , 
                ch.vertices.data()
                ); 
            
            f->glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Render character quad
            f->glDrawArrays(GL_TRIANGLES, 0, 6 );
            extra_f->glBindVertexArray(0);
        }
        f->glBindTexture(GL_TEXTURE_2D, 0);
        f->glDisable(GL_BLEND);
        f->glDisable(GL_CULL_FACE);

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
            Character ch = Characters[*c];
            GLfloat xpos = x + ch.Bearing[0] * scale;
            GLfloat ypos = y - ch.Size[1] - ch.Bearing[1] * scale;

            GLfloat w = ch.Size[0] * scale;
            GLfloat h = ch.Size[1] * scale;
            // Update VBO for each character
            GLfloat vertices[6][4] = {
                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos,     ypos,       0.0, 1.0 },
                { xpos + w, ypos,       1.0, 1.0 },

                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos + w, ypos,       1.0, 1.0 },
                { xpos + w, ypos + h,   1.0, 0.0 }
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
        f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
        int size = current_text.size();
        f->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4 * size, NULL, GL_DYNAMIC_DRAW);
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



    // Private attributes
private:
    GLuint VAO, VBO;
    std::map<GLchar, Character> Characters;

    std::vector<Character> current_text;
    
    QVector<GLfloat> current_vertices;

    int _num_bytes_of_current_text;
};



class BulletDataWVelocity : public ShapeData_C {
	//used for bullets->flying objects
public:
	virtual void Draw() {};
	virtual int getLifetime() { return 0; }
	virtual QVector3D getPosition() { return QVector3D(-1, -1, -1); };
	virtual void Update(double deltaTime) {};

protected:
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
	QTime* lifetimer;
};

class DShapeData : public ShapeData_C {
	//used for target objects. D->Deletable?!
public:
	virtual int vertexBufferSize() = 0;
	virtual void Draw() {};
	virtual int getLifetime() { return 0; }
	//returns Boundaries of Object on each axis
	virtual float getXMin() { return xMin; }
	virtual float getXMax() { return xMax; }
	virtual float getYMin() { return yMin; }
	virtual float getYMax() { return yMax; }
	virtual float getZLowThresh() { return zLowThresh; }
	virtual float getZHighThresh() { return zHighThresh; }

	virtual bool recalculateBounds(float deltaX, float deltaY, float deltaZ, QMatrix4x4 mvp) { return false;}

	virtual ~DShapeData(){};

protected:
	float xMin;
	float xMax;
	float yMin;
	float yMax;
	float zLowThresh;
	float zHighThresh;
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
	QTime* lifetimer;
};


class DTexturedQuad : public DShapeData {
public:
	DTexturedQuad(QVector<float> vertexData) 
        : _vertices(vertexData) 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.create();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glEnableVertexAttribArray(1);
		//tell OpenGl how the data in VBO is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
		f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		_vbo.allocate(_vertices.constData(), _vertices.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		f->glDisableVertexAttribArray(1);
		_vbo.release();

		//Calculate Min and Max boundaries of the object on each axis.
		xMin = _vertices[0];
		xMax = _vertices[0];
		yMin = _vertices[1];
		yMax = _vertices[1];

		//assume all z-coordinates are the same
		zLowThresh = _vertices[2] - 10.5;
		zHighThresh = _vertices[2] + 10.5;

		int i = 0;

		while(i < 20){
			//x coordinate
			if (_vertices[i] < xMin) {
				xMin = _vertices[i];
			}
			if (_vertices[i]  > xMax) {
				xMax = _vertices[i];
			}

			//y coordinate
			if (_vertices[i+1]  < yMin) {
				//yMin = _vertices[i + 1 + 5];
				yMin = _vertices[i + 1];
			}
			if (_vertices[i+1]  > yMax) {
				// yMax = _vertices[i + 1];
				 yMax = _vertices[i + 1];
			}
			//stride is 5 because of Textured Quad has position coordinates (3)+ textured coordinates(2). To get to the next position coordinates ogl needs to skip 2 texture coordinates
			i = i + 5;
		}

	}

	void Draw() override {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glEnableVertexAttribArray(1);
		//tell OpenGl how the data in vbo is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
		f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		f->glDrawArrays(GL_QUADS, 0, 4);

		f->glDisableVertexAttribArray(0);
		f->glDisableVertexAttribArray(1);
		_vbo.release();
	}

	bool recalculateBounds(float deltaX, float deltaY, float deltaZ, QMatrix4x4 modelView) override{
		//shift boundaries
		xMin += deltaX;
		xMax += deltaX;
		yMin += deltaY;
		yMax += deltaY;
		zLowThresh += deltaZ;
		zHighThresh += deltaZ;

		//apply transformation 
		QVector3D mins_T(modelView * 	QVector3D(xMin, yMin, zLowThresh));
		QVector3D maxs_T(modelView * QVector3D(xMax, yMax, zHighThresh));
		
		xMin_T = -mins_T[0];
		xMax_T = -maxs_T[0];

		yMin_T = mins_T[1];
		yMax_T = maxs_T[1];

		zLowThresh_T = mins_T[2];
		zHighThresh_T = maxs_T[2];

		return true;
	}

	int vertexBufferSize() override {
		return _vertices.size() * sizeof(float);
	}

	float getXMin() override{ return xMin_T; }
	float getXMax() override { return xMax_T; }
	float getYMin() override { return yMin_T; }
	float getYMax() override { return yMax_T; }
	float getZLowThresh() override { return zLowThresh_T; }
	float getZHighThresh() override { return zHighThresh_T; }
	
	~DTexturedQuad() {
		_vbo.destroy();
	}

protected:
	//Transformed data
	float xMin_T;
	float xMax_T;
	float yMin_T;
	float yMax_T;
	float zLowThresh_T;
	float zHighThresh_T;

	QMatrix4x4 modelView;

	float xMin;
	float xMax;
	float yMin;
	float yMax;
	float zLowThresh;
	float zHighThresh;
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
};


class Ray : public ShapeData_C {
public:
	Ray(QVector<float> vertexData , QVector3D direction, QVector3D cameraPos) 
        : m_direction(direction), 
         m_cameraPos(cameraPos)
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.create();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		_vbo.allocate(_vertices.constData(), _vertices.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		_vbo.release();
		lifetimer = new QElapsedTimer();
		lifetimer->start();
	}

	~Ray() {
		_vbo.destroy();
		delete lifetimer;
	}
	void Draw () override 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_LINE, 0, 2);
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	int vertexBufferSize() override {
		return 0;
	}
protected:
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;

private:
	QVector3D m_direction;
	QVector3D m_position;
	QVector3D m_cameraPos;
	QElapsedTimer* lifetimer;
};


class Triangle : public ShapeData_C {

public:
	Triangle(QVector<float> vertexData) 
        : _vertices(vertexData) 
    {
        CreateVBO(_vertices);
	}

    Triangle() 
    {
    }

    void CreateVBO(const QVector<float>& data) 
    {
        _vertices = data;
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        _vbo.create();
        _vbo.bind();
        //tell OpenGl how the data is placed in the buffer(buffer layout)
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        _vbo.allocate(_vertices.constData(), _vertices.size() * sizeof(float));
        f->glDisableVertexAttribArray(0);
        _vbo.release();
    }

	~Triangle() {
		_vbo.destroy();
	}

	void Draw() override 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);		
		f->glDrawArrays(GL_TRIANGLES, 0, 3);
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	int vertexBufferSize() override {
		return _vertices.size() * sizeof(float);
	}

protected:
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
};


class BulletTriangle : public BulletShapeData {
public:

	BulletTriangle(QVector<float> vertexData, QVector3D direction) 
        : _vertices(vertexData),
        _direction(direction)
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.create();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		_vbo.allocate(_vertices.constData(), _vertices.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		_vbo.release();
		velocity = 30; //30 floats per second		//s(depth) = dt(time elapsed since last update) * v

		position_new[0] = _vertices[0];
		position_new[1] = _vertices[1];
		position_new[2] = _vertices[2];

		lifetimer = new QElapsedTimer();
		lifetimer->start();
	}

	BulletTriangle(QVector<float> vertexData) 
        : _vertices(vertexData) 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.create();
		_vbo.bind();
		//tell OpenGl how the data is placed in the buffer(buffer layout)
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		_vbo.allocate(_vertices.constData(), _vertices.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		_vbo.release();
		velocity = 30; //30 floats per second		//s(depth) = dt(time elapsed since last update) * v

		position_new[0] = _vertices[0];
		position_new[1] = _vertices[1];
		position_new[2] = _vertices[2];

		lifetimer = new QElapsedTimer();
		lifetimer->start();
	}

	~BulletTriangle() {
		_vbo.destroy();
		delete lifetimer;
	}
	
	void Draw() override 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		this->update();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_TRIANGLES, 0, 3);
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	void update() {
		//position_new = velocity * timeElapsed;	
		position_new += _direction;
		applyGravity();
	}

	void applyGravity() {
		position_new -= QVector3D(0, +0.110f, 0);

	}

	int vertexBufferSize()  {
		return _vertices.size() * sizeof(float);
	}

	int getLifetime() {
		return lifetimer->elapsed();
	}

	float getDepth() {
		return depth;
	}

	QVector3D getPosition() override {
		//return QVector3D(posX, posY, posZ);
		return position_new;
	}

	void setDepth(float depth_) {
		depth= depth_;
	}

protected:
	float posX;
	float posY;
	float posZ;
	
	QVector3D position_new;
	float timeElapsed;
	float depth;
	float velocity;

	QVector3D _direction;
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
	QElapsedTimer* lifetimer;
};

class Quad : public ShapeData_C {

public:
	Quad(QVector<float> vertexData) 
        : _vertices(vertexData) 
    {
        CreateVBO(_vertices);
	}

    Quad() 
    {
    }

    void CreateVBO(QVector<float>& vertex_data) 
    {
        _vertices = vertex_data;
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        _vbo.create();
        _vbo.bind();
        f->glEnableVertexAttribArray(0);
        //tells OpenGl how the data in m_vao is placed
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        _vbo.allocate(_vertices.data(), _vertices.size() * sizeof(float));
        f->glDisableVertexAttribArray(0);
        _vbo.release();
    }

	~Quad() {
		_vbo.destroy();
	}

	void Draw() override 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_TRIANGLES, 0, 6);
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	int vertexBufferSize() override {
		return _vertices.size() * sizeof(float);
	}

protected:
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
};


class RQuad : public BulletShapeData {
public:
	RQuad(QVector<float> vertexData, QVector3D position, QVector3D direction) 
        : _vertices(vertexData), 
        m_position(position),
        m_direction(direction)  
    {

		//shift in front of camera
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.create();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		_vbo.allocate(_vertices.constData(), _vertices.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		_vbo.release();
		
		lifetimer = new QElapsedTimer();
		lifetimer->start();
	}

	~RQuad() {
		_vbo.destroy();
		delete lifetimer;
	}
	void Draw() override {
		this->update();

		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_QUADS, 0, 4);
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	void update() {
		m_position += m_direction;
	}

	QVector3D getPosition() override {
		return m_position;
	}

	int vertexBufferSize()  {
		return _vertices.size() * sizeof(float);
	}

protected:
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;

private:
	QVector3D m_direction;
	QVector3D m_position;
	QVector3D m_cameraPos;
	QElapsedTimer* lifetimer;
};


class Cube : public ShapeData_C {
public:
	Cube(QVector<float> vertexData) : _vertices(vertexData) {

		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.create();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		_vbo.allocate(_vertices.constData(), _vertices.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}
	~Cube() {
		_vbo.destroy();
	}

	void Draw() override {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_TRIANGLES, 0, 36);		//12 triangles each 3 _vertices times 3 floats(3x3x12).
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	int vertexBufferSize() override {
		return _vertices.size() * sizeof(float);
	}

protected:
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
};


class CubeMap : public ShapeData_C {

public:
	CubeMap(QVector<float> vertexData, unsigned int texID)
        : _vertices(vertexData), texture_id(texID) 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		//vertexesVAO.create();
		_vbo.create();
		//vertexesVAO.bind();
		_vbo.bind();
		_vbo.allocate(_vertices.constData(), _vertices.size() * sizeof(float));
		f->glEnableVertexAttribArray(0);
		// Tell OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	~CubeMap() {
		_vbo.destroy();
	}

	void Draw() override 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		//vertexesVAO.bind();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 , nullptr);

		f->glActiveTexture(GL_TEXTURE0);
		f->glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

		f->glDrawArrays(GL_TRIANGLES, 0, 36);		//12 triangles each 3 _vertices times 3 floats(3x3x12).
		f->glDisableVertexAttribArray(0);

		_vbo.release();
		//vertexesVAO.release();
	}

	int vertexBufferSize() override {
		return _vertices.size() * sizeof(float);
	}

protected:
	QOpenGLBuffer _vbo;
	//QOpenGLVertexArrayObject vertexesVAO;
	QVector<float> _vertices;
	unsigned int texture_id;
};

class BulletCube : public ShapeData_C {

public:
	BulletCube(QVector<float> vertexData)
        : _vertices(vertexData)
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.create();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		_vbo.allocate(_vertices.constData(), _vertices.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	void Draw() override 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_TRIANGLES, 0, 36);		//12 triangles each 3 _vertices times 3 floats(3x3x12).
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	int vertexBufferSize() override {
		return _vertices.size() * sizeof(float);
	}

	~BulletCube() {
		_vbo.destroy();
	}

protected:
	bool drawed;
	int lifetime;
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
};


class TexturedQuad : public ShapeData_C {

	//implement texture binding when the vertexvbo is bound
    // ...also implement shader which can draw textures(sampler2D)..therefor implement shader system(maybe hardcoded reading of textfiles?)
public:
	TexturedQuad(QVector<float> vertexData) 
        : _vertices(vertexData) 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.create();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glEnableVertexAttribArray(1);
		// tell OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
		f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		_vbo.allocate(_vertices.constData(), _vertices.size() * sizeof(float));
		
		f->glDisableVertexAttribArray(0);
		f->glDisableVertexAttribArray(1);
		_vbo.release();
	}

	void Draw() override 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glEnableVertexAttribArray(1);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);	//Position Coordinates
		f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));	//2 Texture Coordinates

		f->glDrawArrays(GL_QUADS, 0, 4);

		f->glDisableVertexAttribArray(0);
		f->glDisableVertexAttribArray(1);
		_vbo.release();
	}

	int vertexBufferSize() override {
		return _vertices.size() * sizeof(float);
	}

protected:
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
};

class Point : public ShapeData_C {

public:
	Point(QVector<float> vertexData)
        : _vertices(vertexData) 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.create();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		_vbo.allocate(_vertices.constData(), _vertices.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	void Draw() override 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_POINTS, 0, 1);
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	int vertexBufferSize() override {
		return _vertices.size() * sizeof(float);
	}

	~Point() {
		_vbo.destroy();
	}

protected:
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
};


class Quad_f : public ShapeData_C {

public:
	Quad_f(QVector<float> vertexData) 
        : _vertices(vertexData) 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.create();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		_vbo.allocate(_vertices.constData(), _vertices.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	~Quad_f() 
    {
		_vbo.destroy();
	}

	void Draw() override 
    {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		_vbo.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_QUADS, 0, 4);
		f->glDisableVertexAttribArray(0);
		_vbo.release();
	}

	int vertexBufferSize() override 
    {
		return _vertices.size() * sizeof(float);
	}
protected:
	QOpenGLBuffer _vbo;
	QVector<float> _vertices;
};
