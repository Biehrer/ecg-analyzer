#pragma once
#include <qvector.h>
#include <qvector3d.h>


class ShapeGenerator {
public:

	QVector<float> makeTriangleAtPos_(float x, float y, float z, float size)
	{
		float x_ = x;
		float y_ = y;
		float size_ = size;
		// P _______
		//	|	   /
		//	| 1	  / 
		//	|	 /	
		//	|	/ 
		//	|  /	
		//  | /

		QVector<float> vertexes;
		//left edge
		vertexes.append(x_);
		vertexes.append(y_);
		vertexes.append(z);

		//right bottom edge<
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(z);

		//top of triangle
		vertexes.append(x_);
		vertexes.append(y_ + size);
		vertexes.append(z);

		return vertexes;
	}

	QVector<float> makeTriangleAtPos_(QVector3D position, float size)
	{
		float x_ = position[0];
		float y_ = position[1];
		float z = position[2];
		float size_ = size;

		QVector<float> vertexes;
		//left edge
		vertexes.append(x_);
		vertexes.append(y_);
		vertexes.append(z);

		//right bottom edge
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(z);

		//top of triangle
		vertexes.append(x_);
		vertexes.append(y_ + size);
		vertexes.append(z);
		return vertexes;
	}




	QVector<float> makeQuadAtPos_(float x, float y, float z, float size)
	{
		float x_ = x; /// this->width();
		float y_ = y; /// this->height();
				// P _______
				//	|	   /|
				//	| 1	  / |
				//	|	 /	|
				//	|	/ 2	|
				//	|  /	|
				//  ---------
		QVector<float> vertexes;
		//first triangle
		//left top edge
		vertexes.append(x_);
		vertexes.append(y_);
		vertexes.append(z);

		//right top edge
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(z);

		//left bottom edge 
		vertexes.append(x_);
		vertexes.append(y_ + size);
		vertexes.append(z);

		//second triangle
		//left bottom edge
		vertexes.append(x_);
		vertexes.append(y_ + size);
		vertexes.append(z);

		//right bottom ege
		vertexes.append(x_ + size);
		vertexes.append(y_ + size);
		vertexes.append(z);

		//right top edge
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(z);
		return vertexes;
	}

	//starting Point				//direction				//length
	QVector<float> makeRayToDir(float x1, float y1, float z1, float dirX_normalized, float dirY_normalized, float dirZ_normalized, float length) {
		QVector<float> vertexes;

		vertexes.append(x1);
		vertexes.append(y1);
		vertexes.append(z1);

		vertexes.append(x1 + dirX_normalized * length);
		vertexes.append(y1 + dirY_normalized * length);
		vertexes.append(z1 + dirZ_normalized * length);
		return vertexes;
	}


	QVector<float> makeQuad4Vertexes(float x, float y, float z, float size) {
		QVector<float> vertexes;
		vertexes.append(x);
		vertexes.append(y);
		vertexes.append(z);

		vertexes.append(x + size);
		vertexes.append(y);
		vertexes.append(z);

		vertexes.append(x + size);
		vertexes.append(y + size);
		vertexes.append(z);

		vertexes.append(x);
		vertexes.append(y + size);
		vertexes.append(z);
		return vertexes;
	}



	QVector<float> makeTextureQuad4Vertexes(float x, float y, float z, float size) {
		QVector<float> vertexes;
		//bottom left
		vertexes.append(x - size);
		vertexes.append(y - size);
		vertexes.append(z);
		vertexes.append(0.0f);	//texCoord	
		vertexes.append(0.0f);	//texCoord
		//bottom right
		vertexes.append(x + size);
		vertexes.append(y - size);
		vertexes.append(z);
		vertexes.append(1.0f);	//texCoord	
		vertexes.append(0.0f);	//texCoord
		//top right
		vertexes.append(x + size);
		vertexes.append(y + size);
		vertexes.append(z);
		vertexes.append(1.0f);	//texCoord	
		vertexes.append(1.0f);	//texCoord
		//top left
		vertexes.append(x - size);
		vertexes.append(y + size);
		vertexes.append(z);
		vertexes.append(0.0f);	//texCoord	
		vertexes.append(1.0f);	//texCoord

		return vertexes;
	}


	QVector<float> makeTextureTestVertexes(float x, float y, float z, float size) {
		QVector<float> vertexes;
		//bottom left
		vertexes.append(-1.0*size);
		vertexes.append(-1.0*size);
		vertexes.append(z);
		vertexes.append(0.0f);	//texCoord	
		vertexes.append(0.0f);	//texCoord

				//top left
		vertexes.append(-1.0 * size);
		vertexes.append(1.0 * size);
		vertexes.append(z);
		vertexes.append(0.0f);	//texCoord	
		vertexes.append(1.0f);	//texCoord


				//top right
		vertexes.append(1.0*size);
		vertexes.append(1.0*size);
		vertexes.append(z);
		vertexes.append(1.0f);	//texCoord	
		vertexes.append(1.0f);	//texCoord

		//bottom right
		vertexes.append(1.0*size);
		vertexes.append(-1.0*size);
		vertexes.append(z);
		vertexes.append(1.0f);	//texCoord	
		vertexes.append(0.0f);	//texCoord


		return vertexes;
	}


	QVector<float> makeMiddleQuad4Vertexes(float x, float y, float z, float size) {
		QVector<float> vertexes;

		vertexes.append(x - size);
		vertexes.append(y - size);
		vertexes.append(z);

		vertexes.append(x - size);
		vertexes.append(y + size);
		vertexes.append(z);

		vertexes.append(x + size);
		vertexes.append(y + size);
		vertexes.append(z);

		vertexes.append(x + size);
		vertexes.append(y - size);
		vertexes.append(z);

		return vertexes;
	}

	QVector<float> makeCubeMap() {

		float skyboxVertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};

		QVector<float> vertices;
		vertices.reserve(sizeof(skyboxVertices));
		std::copy(skyboxVertices, skyboxVertices + sizeof(skyboxVertices), std::back_inserter(vertices));

		//scale coordinates
		for (int i = 0; i < vertices.size(); i++) {
			vertices[i] = vertices[i] * 5.0;	//prev:99
		}
		return vertices;
	}


	QVector<float> makeMiddleQuadTextured4Vertexes(float x, float y, float z, float size) {
		QVector<float> vertexes;
		vertexes.append(x - size);
		vertexes.append(y - size);
		vertexes.append(z);
		vertexes.append(0.0f);	//texCoord	
		vertexes.append(0.0f);	//texCoord

		vertexes.append(x - size);
		vertexes.append(y + size);
		vertexes.append(z);
		vertexes.append(0.0f);	//texCoord	
		vertexes.append(1.0f);	//texCoord

		vertexes.append(x + size);
		vertexes.append(y + size);
		vertexes.append(z);
		vertexes.append(1.0f);	//texCoord	
		vertexes.append(1.0f);	//texCoord

		vertexes.append(x + size);
		vertexes.append(y - size);
		vertexes.append(z);
		vertexes.append(1.0f);	//texCoord	
		vertexes.append(0.0f);	//texCoord

		return vertexes;
	}



	QVector<float> makeFloorPlaneAtPos(float x, float height, float z, float size)
	{
		float x_ = x; /// this->width();
		float y_ = height; /// this->height();
				// P _______
				//	|	   /|
				//	| 1	  / |
				//	|	 /	|
				//	|	/ 2	|
				//	|  /	|
				//  ---------
		QVector<float> vertexes;
		//first triangle
		//left bottom edge
		vertexes.append(x_);
		vertexes.append(height);
		vertexes.append(z);

		//left top edge
		vertexes.append(x_);
		vertexes.append(height);
		vertexes.append(z + size);

		//right bottom edge 
		vertexes.append(x_ + size);
		vertexes.append(height);
		vertexes.append(z);

		//second triangle
		//left top edge
		vertexes.append(x_);
		vertexes.append(height);
		vertexes.append(z + size);

		//right top ege
		vertexes.append(x_ + size);
		vertexes.append(height);
		vertexes.append(z + size);

		//right bottom edge
		vertexes.append(x_ + size);
		vertexes.append(height);
		vertexes.append(z);
		return vertexes;
	}




	QVector<float> makeRayQuad(float origin_x, float origin_y, float target_x, float target_y, float rayLength) {

		QVector<float> vertexes;
		/*
		vertexes.append(origin_x);
		vertexes.append();
		vertexes.append();

		//left top edge
		vertexes.append(x_);
		vertexes.append(height);
		vertexes.append(z + size);

		*/
		return vertexes;
	}



	QVector<float> makeQuadWithIndicesAtPos_(float x, float y, float size) {
		QVector<float> vertexes; return vertexes;
	}


	QVector<float> makePointAtPos_(float x, float y, float z) {
		QVector<float> vertexes;
		vertexes.append(x);
		vertexes.append(y);
		vertexes.append(z);
		return vertexes;
	}


	QVector<float> makeCubeAtPos(float x_, float y_, float size) {
		QVector<float> vertexes;
		//---front face of cube--
		//first triangle
		//left top edge
		vertexes.append(x_);
		vertexes.append(y_);
		vertexes.append(0.0f);

		//right top edge
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(0.0f);

		//left bottom edge 
		vertexes.append(x_);
		vertexes.append(y_ + size);
		vertexes.append(0.0f);

		//second triangle
		//left bottom edge
		vertexes.append(x_);
		vertexes.append(y_ + size);
		vertexes.append(0.0f);

		//right bottom ege
		vertexes.append(x_ + size);
		vertexes.append(y_ + size);
		vertexes.append(0.0f);

		//right top edge
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(0.0f);

		//--right side of cube--

		//thid triang-P2
		vertexes.append(x_ + size);
		vertexes.append(y_ + size);
		vertexes.append(0.0f);

		//third triang-P1
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(0.0f - size);

		//third triang-P3
		vertexes.append(x_ + size);
		vertexes.append(y_ - size);
		vertexes.append(-size);

		//second triang
		vertexes.append(x_ + size);
		vertexes.append(y_ - size);
		vertexes.append(-size);

		vertexes.append(x_ + size);
		vertexes.append(y_ - size);
		vertexes.append(0.0f);

		vertexes.append(x_ + size);
		vertexes.append(y_ + size);
		vertexes.append(0.0f);
		//--behind side of cube--opposite of front--
		//--left side of cube--
		//--top side of cube--
		//--bottom side of cube--
		return vertexes;
	}

	QVector<float> makeCube() {
		// Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
		// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
		const float g_vertex_buffer_data[] = {
				  -1.0f,-1.0f,-1.0f, // triangle 1 : begin
				  -1.0f,-1.0f, 1.0f,
				  -1.0f, 1.0f, 1.0f, // triangle 1 : end
				   1.0f, 1.0f,-1.0f, // triangle 2 : begin
				  -1.0f,-1.0f,-1.0f,
				  -1.0f, 1.0f,-1.0f, // triangle 2 : end

				  1.0f,-1.0f, 1.0f,
				  -1.0f,-1.0f,-1.0f,
				  1.0f,-1.0f,-1.0f,
				  1.0f, 1.0f,-1.0f,
				  1.0f,-1.0f,-1.0f,
				  -1.0f,-1.0f,-1.0f,

				  -1.0f,-1.0f,-1.0f,
				  -1.0f, 1.0f, 1.0f,
				  -1.0f, 1.0f,-1.0f,
				  1.0f,-1.0f, 1.0f,
				  -1.0f,-1.0f, 1.0f,
				  -1.0f,-1.0f,-1.0f,

				  -1.0f, 1.0f, 1.0f,
				  -1.0f,-1.0f, 1.0f,
				  1.0f,-1.0f, 1.0f,
				  1.0f, 1.0f, 1.0f,
				  1.0f,-1.0f,-1.0f,
				  1.0f, 1.0f,-1.0f,

				  1.0f,-1.0f,-1.0f,
				  1.0f, 1.0f, 1.0f,
				  1.0f,-1.0f, 1.0f,
				  1.0f, 1.0f, 1.0f,
				  1.0f, 1.0f,-1.0f,
				  -1.0f, 1.0f,-1.0f,

				  1.0f, 1.0f, 1.0f,
				  -1.0f, 1.0f,-1.0f,
				  -1.0f, 1.0f, 1.0f,
				  1.0f, 1.0f, 1.0f,
				  -1.0f, 1.0f, 1.0f,
				  1.0f,-1.0f, 1.0f
		};

		QVector<float> vertices;
		vertices.reserve(sizeof(g_vertex_buffer_data));
		std::copy(g_vertex_buffer_data, g_vertex_buffer_data + 108/* sizeof(g_vertex_buffer_data)*/, std::back_inserter(vertices));

		for (int i = 0; i < vertices.size(); i++) {

			vertices[i] = vertices[i] * 10;
		}
		return vertices;
	}


	QVector<float> makeBulletCube() {
		// Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
		// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
		const float g_vertex_buffer_data[] = {
			  -1.0f,-1.0f,-1.0f, // triangle 1 : begin
			  -1.0f,-1.0f, 1.0f,
			  -1.0f, 1.0f, 1.0f, // triangle 1 : end
			   1.0f, 1.0f,-1.0f, // triangle 2 : begin
			  -1.0f,-1.0f,-1.0f,
			  -1.0f, 1.0f,-1.0f, // triangle 2 : end

			   1.0f,-1.0f, 1.0f,
			  -1.0f,-1.0f,-1.0f,
			   1.0f,-1.0f,-1.0f,
			   1.0f, 1.0f,-1.0f,
			   1.0f,-1.0f,-1.0f,
			  -1.0f,-1.0f,-1.0f,

			  -1.0f,-1.0f,-1.0f,
			  -1.0f, 1.0f, 1.0f,
			  -1.0f, 1.0f,-1.0f,
			   1.0f,-1.0f, 1.0f,
			  -1.0f,-1.0f, 1.0f,
			  -1.0f,-1.0f,-1.0f,

			  -1.0f, 1.0f, 1.0f,
			  -1.0f,-1.0f, 1.0f,
			   1.0f,-1.0f, 1.0f,
			   1.0f, 1.0f, 1.0f,
			   1.0f,-1.0f,-1.0f,
			   1.0f, 1.0f,-1.0f,

			  1.0f,-1.0f,-1.0f,
			  1.0f, 1.0f, 1.0f,
			  1.0f,-1.0f, 1.0f,
			  1.0f, 1.0f, 1.0f,
			  1.0f, 1.0f,-1.0f,
			 -1.0f, 1.0f,-1.0f,

			 1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			 1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			 1.0f,-1.0f, 1.0f
		};

		QVector<float> vertices;
		vertices.reserve(sizeof(g_vertex_buffer_data));
		std::copy(g_vertex_buffer_data, g_vertex_buffer_data + sizeof(g_vertex_buffer_data), std::back_inserter(vertices));
		/*
		for (int i = 0; i < vertices.size(); i++) {
			vertices[i] = vertices[i] * 10;
		}*/

		return vertices;
	}


};
/*
namespace ShapeGenerator {

	QVector<float> makeTriangleAtPos_(float x, float y, float z, float size)
	{
		float x_ = x; 
		float y_ = y; 
		float size_ = size;
			// P _______
			//	|	   /
			//	| 1	  / 
			//	|	 /	
			//	|	/ 
			//	|  /	
			//  | /

		QVector<float> vertexes;
		//left edge
		vertexes.append(x_);
		vertexes.append(y_);
		vertexes.append(z);

		//right bottom edge<
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(z);

		//top of triangle
		vertexes.append(x_);
		vertexes.append(y_ + size);
		vertexes.append(z);

		return vertexes;
	}

	QVector<float> makeTriangleAtPos_(QVector3D position, float size)
	{
		float x_ = position[0];
		float y_ = position[1];
		float z = position[2];
		float size_ = size;

		QVector<float> vertexes;
		//left edge
		vertexes.append(x_);
		vertexes.append(y_);
		vertexes.append(z);

		//right bottom edge
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(z);

		//top of triangle
		vertexes.append(x_);
		vertexes.append(y_ + size);
		vertexes.append(z);
		return vertexes;
	}




	QVector<float> makeQuadAtPos_(float x, float y, float z, float size)
	{
		float x_ = x; /// this->width();
		float y_ = y; /// this->height();
				// P _______
				//	|	   /|
				//	| 1	  / |
				//	|	 /	|
				//	|	/ 2	|
				//	|  /	|
				//  ---------
		QVector<float> vertexes;
		//first triangle
		//left top edge
		vertexes.append(x_);
		vertexes.append(y_);
		vertexes.append(z);

		//right top edge
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(z);

		//left bottom edge 
		vertexes.append(x_);
		vertexes.append(y_ + size);
		vertexes.append(z);

		//second triangle
		//left bottom edge
		vertexes.append(x_);
		vertexes.append(y_+size);
		vertexes.append(z);

		//right bottom ege
		vertexes.append(x_+size);
		vertexes.append(y_ + size);
		vertexes.append(z);

		//right top edge
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(z);
		return vertexes;
	}

						    //starting Point				//direction				//length
	QVector<float> makeRayToDir(float x1, float y1, float z1, float dirX_normalized, float dirY_normalized, float dirZ_normalized, float length) {
		QVector<float> vertexes;

		vertexes.append(x1);
		vertexes.append(y1);
		vertexes.append(z1);

		vertexes.append(x1 + dirX_normalized*length);
		vertexes.append(y1 + dirY_normalized*length);
		vertexes.append(z1 + dirZ_normalized*length);
		return vertexes;
	}


	QVector<float> makeQuad4Vertexes(float x, float y, float z, float size) {	
		QVector<float> vertexes;
		vertexes.append(x);
		vertexes.append(y);
		vertexes.append(z);

		vertexes.append(x+size);
		vertexes.append(y);
		vertexes.append(z);

		vertexes.append(x+size);
		vertexes.append(y+size);
		vertexes.append(z);

		vertexes.append(x);
		vertexes.append(y+size);
		vertexes.append(z);
		return vertexes;
	}



	QVector<float> makeTextureQuad4Vertexes(float x, float y, float z, float size) {
		QVector<float> vertexes;
		//bottom left
		vertexes.append(x - size);
		vertexes.append(y - size);
		vertexes.append(z);
		vertexes.append(0.0f);	//texCoord	
		vertexes.append(0.0f);	//texCoord
		//bottom right
		vertexes.append(x + size);
		vertexes.append(y - size);
		vertexes.append(z);
		vertexes.append(1.0f);	//texCoord	
		vertexes.append(0.0f);	//texCoord
		//top right
		vertexes.append(x + size);
		vertexes.append(y + size);
		vertexes.append(z);
		vertexes.append(1.0f);	//texCoord	
		vertexes.append(1.0f);	//texCoord
		//top left
		vertexes.append(x - size);
		vertexes.append(y + size);
		vertexes.append(z);
		vertexes.append(0.0f);	//texCoord	
		vertexes.append(1.0f);	//texCoord

		return vertexes;
	}


	QVector<float> makeTextureTestVertexes(float x, float y, float z, float size) {
		QVector<float> vertexes;
		//bottom left
		vertexes.append(-1.0*size);
		vertexes.append(-1.0*size);
		vertexes.append(z);
		vertexes.append(0.0f);	//texCoord	
		vertexes.append(0.0f);	//texCoord

				//top left
		vertexes.append(-1.0 * size);
		vertexes.append(1.0 * size);
		vertexes.append(z);
		vertexes.append(0.0f);	//texCoord	
		vertexes.append(1.0f);	//texCoord


				//top right
		vertexes.append(1.0*size);
		vertexes.append(1.0*size);
		vertexes.append(z);
		vertexes.append(1.0f);	//texCoord	
		vertexes.append(1.0f);	//texCoord

		//bottom right
		vertexes.append(1.0*size);
		vertexes.append(-1.0*size);
		vertexes.append(z);
		vertexes.append(1.0f);	//texCoord	
		vertexes.append(0.0f);	//texCoord


		return vertexes;
	}


	QVector<float> makeMiddleQuad4Vertexes(float x, float y, float z, float size) {
		QVector<float> vertexes;

		vertexes.append(x-size);
		vertexes.append(y-size);
		vertexes.append(z);

		vertexes.append(x - size);
		vertexes.append(y + size);
		vertexes.append(z);

		vertexes.append(x + size);
		vertexes.append(y + size);
		vertexes.append(z);

		vertexes.append(x + size);
		vertexes.append(y - size);
		vertexes.append(z);

		return vertexes;
	}

	QVector<float> makeCubeMap() {

		float skyboxVertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};
	
		QVector<float> vertices;
		vertices.reserve(sizeof(skyboxVertices));
		std::copy(skyboxVertices, skyboxVertices + sizeof(skyboxVertices), std::back_inserter(vertices));

		//scale coordinates
		for (int i = 0; i < vertices.size(); i++) {	
			vertices[i] = vertices[i] * 99.9;	//prev:500
		}		
		return vertices;
	}


	QVector<float> makeMiddleQuadTextured4Vertexes(float x, float y, float z, float size) {
		QVector<float> vertexes;
		vertexes.append(x - size);
		vertexes.append(y - size);
		vertexes.append(z);
		vertexes.append(0.0f);	//texCoord	
		vertexes.append(0.0f);	//texCoord

		vertexes.append(x - size);
		vertexes.append(y + size);
		vertexes.append(z);
		vertexes.append(0.0f);	//texCoord	
		vertexes.append(1.0f);	//texCoord

		vertexes.append(x + size);
		vertexes.append(y + size);
		vertexes.append(z);
		vertexes.append(1.0f);	//texCoord	
		vertexes.append(1.0f);	//texCoord

		vertexes.append(x + size);
		vertexes.append(y - size);
		vertexes.append(z);
		vertexes.append(1.0f);	//texCoord	
		vertexes.append(0.0f);	//texCoord

		return vertexes;
	}



	QVector<float> makeFloorPlaneAtPos(float x, float height, float z, float size)
	{
		float x_ = x; /// this->width();
		float y_ = height; /// this->height();
				// P _______
				//	|	   /|
				//	| 1	  / |
				//	|	 /	|
				//	|	/ 2	|
				//	|  /	|
				//  ---------
		QVector<float> vertexes;
		//first triangle
		//left bottom edge
		vertexes.append(x_);
		vertexes.append(height);
		vertexes.append(z);

		//left top edge
		vertexes.append(x_ );
		vertexes.append(height);
		vertexes.append(z + size);

		//right bottom edge 
		vertexes.append(x_ + size);
		vertexes.append(height);
		vertexes.append(z);

		//second triangle
		//left top edge
		vertexes.append(x_);
		vertexes.append(height);
		vertexes.append(z + size);

		//right top ege
		vertexes.append(x_ + size);
		vertexes.append(height);
		vertexes.append(z + size);

		//right bottom edge
		vertexes.append(x_ + size);
		vertexes.append(height);
		vertexes.append(z);
		return vertexes;
	}




	QVector<float> makeRayQuad(float origin_x, float origin_y, float target_x, float target_y, float rayLength) {

		QVector<float> vertexes;
		
		vertexes.append(origin_x);
		vertexes.append();
		vertexes.append();

		//left top edge
		vertexes.append(x_);
		vertexes.append(height);
		vertexes.append(z + size);

		
		return vertexes;
	}



	QVector<float> makeQuadWithIndicesAtPos_(float x, float y, float size) {
		QVector<float> vertexes; return vertexes;
	}


	QVector<float> makePointAtPos_(float x, float y, float z) {
		QVector<float> vertexes;
		vertexes.append(x);
		vertexes.append(y);
		vertexes.append(z);
		return vertexes;
	}


	QVector<float> makeCubeAtPos(float x_, float y_, float size) {
		QVector<float> vertexes;
		//---front face of cube--
		//first triangle
		//left top edge
		vertexes.append(x_);
		vertexes.append(y_);
		vertexes.append(0.0f);

		//right top edge
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(0.0f);

		//left bottom edge 
		vertexes.append(x_);
		vertexes.append(y_ + size);
		vertexes.append(0.0f);

		//second triangle
		//left bottom edge
		vertexes.append(x_);
		vertexes.append(y_ + size);
		vertexes.append(0.0f);

		//right bottom ege
		vertexes.append(x_ + size);
		vertexes.append(y_ + size);
		vertexes.append(0.0f);

		//right top edge
		vertexes.append(x_ + size);
		vertexes.append(y_);
		vertexes.append(0.0f);

		//--right side of cube--

		//thid triang-P2
		vertexes.append(x_ + size);
		vertexes.append(y_ + size);
		vertexes.append(0.0f);

		//third triang-P1
		vertexes.append(x_+size);
		vertexes.append(y_);
		vertexes.append(0.0f - size);
		
		//third triang-P3
		vertexes.append(x_ + size);
		vertexes.append(y_ - size);
		vertexes.append(-size);

		//second triang
		vertexes.append(x_ + size);
		vertexes.append(y_ - size);
		vertexes.append(-size);

		vertexes.append(x_ + size);
		vertexes.append(y_-size);
		vertexes.append(0.0f);

		vertexes.append(x_ + size);
		vertexes.append(y_ + size);
		vertexes.append(0.0f);
		//--behind side of cube--opposite of front--
		//--left side of cube--
		//--top side of cube--
		//--bottom side of cube--
		return vertexes;
	}

	QVector<float> makeCube() {
		// Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
		// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	const float g_vertex_buffer_data[] = {
			  -1.0f,-1.0f,-1.0f, // triangle 1 : begin
			  -1.0f,-1.0f, 1.0f,
			  -1.0f, 1.0f, 1.0f, // triangle 1 : end
			   1.0f, 1.0f,-1.0f, // triangle 2 : begin
			  -1.0f,-1.0f,-1.0f,
			  -1.0f, 1.0f,-1.0f, // triangle 2 : end

			  1.0f,-1.0f, 1.0f,
			  -1.0f,-1.0f,-1.0f,
			  1.0f,-1.0f,-1.0f,
			  1.0f, 1.0f,-1.0f,
			  1.0f,-1.0f,-1.0f,
			  -1.0f,-1.0f,-1.0f,

			  -1.0f,-1.0f,-1.0f,
			  -1.0f, 1.0f, 1.0f,
			  -1.0f, 1.0f,-1.0f,
			  1.0f,-1.0f, 1.0f,
			  -1.0f,-1.0f, 1.0f,
			  -1.0f,-1.0f,-1.0f,

			  -1.0f, 1.0f, 1.0f,
			  -1.0f,-1.0f, 1.0f,
			  1.0f,-1.0f, 1.0f,
			  1.0f, 1.0f, 1.0f,
			  1.0f,-1.0f,-1.0f,
			  1.0f, 1.0f,-1.0f,
			  
			  1.0f,-1.0f,-1.0f,
			  1.0f, 1.0f, 1.0f,
			  1.0f,-1.0f, 1.0f,
			  1.0f, 1.0f, 1.0f,
			  1.0f, 1.0f,-1.0f,
			  -1.0f, 1.0f,-1.0f,

			  1.0f, 1.0f, 1.0f,
			  -1.0f, 1.0f,-1.0f,
			  -1.0f, 1.0f, 1.0f,
			  1.0f, 1.0f, 1.0f,
			  -1.0f, 1.0f, 1.0f,
			  1.0f,-1.0f, 1.0f
		};
		
		QVector<float> vertices;
		vertices.reserve(sizeof(g_vertex_buffer_data));
		std::copy(g_vertex_buffer_data, g_vertex_buffer_data +108, std::back_inserter(vertices));//sizeof(g_vertex_buffer_data) = 108
		
		for (int i = 0; i < vertices.size(); i++) {
		
			vertices[i] = vertices[i] * 10;
		}
		return vertices;
	}


QVector<float> makeBulletCube() {
		// Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
		// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
		const float g_vertex_buffer_data[] = {
			  -1.0f,-1.0f,-1.0f, // triangle 1 : begin
			  -1.0f,-1.0f, 1.0f,
			  -1.0f, 1.0f, 1.0f, // triangle 1 : end
		   	   1.0f, 1.0f,-1.0f, // triangle 2 : begin
			  -1.0f,-1.0f,-1.0f,
			  -1.0f, 1.0f,-1.0f, // triangle 2 : end

			   1.0f,-1.0f, 1.0f,
			  -1.0f,-1.0f,-1.0f,
			   1.0f,-1.0f,-1.0f,
			   1.0f, 1.0f,-1.0f,
			   1.0f,-1.0f,-1.0f,
			  -1.0f,-1.0f,-1.0f,

			  -1.0f,-1.0f,-1.0f,
			  -1.0f, 1.0f, 1.0f,
			  -1.0f, 1.0f,-1.0f,
		   	   1.0f,-1.0f, 1.0f,
			  -1.0f,-1.0f, 1.0f,
			  -1.0f,-1.0f,-1.0f,

			  -1.0f, 1.0f, 1.0f,
			  -1.0f,-1.0f, 1.0f,
			   1.0f,-1.0f, 1.0f,
			   1.0f, 1.0f, 1.0f,
			   1.0f,-1.0f,-1.0f,
			   1.0f, 1.0f,-1.0f,

			  1.0f,-1.0f,-1.0f,
			  1.0f, 1.0f, 1.0f,
			  1.0f,-1.0f, 1.0f,
			  1.0f, 1.0f, 1.0f,
			  1.0f, 1.0f,-1.0f,
			 -1.0f, 1.0f,-1.0f,

			 1.0f, 1.0f, 1.0f,
		    -1.0f, 1.0f,-1.0f,
		    -1.0f, 1.0f, 1.0f,
		     1.0f, 1.0f, 1.0f,
		    -1.0f, 1.0f, 1.0f,
			 1.0f,-1.0f, 1.0f
		};

	QVector<float> vertices;
	vertices.reserve(sizeof(g_vertex_buffer_data));
	std::copy(g_vertex_buffer_data, g_vertex_buffer_data + sizeof(g_vertex_buffer_data), std::back_inserter(vertices));
	
	//for (int i = 0; i < vertices.size(); i++) {
	//	vertices[i] = vertices[i] * 10;
	//}

	return vertices;
	}


}//end of namespace ShapeGenerator
*/
