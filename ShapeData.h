#pragma once

// Project includes
#include "Sphere.h"
// STD includes
#include <time.h>
#include <iostream>
// Qt includes
#include <qopenglbuffer.h>
#include <qlist.h>
#include <qvector.h>
#include <qvector3d.h>
#include <qmatrix.h>
#include <qmatrix4x4.h>
#include <qopenglfunctions.h>
#include <qtimezone.h>
#include <qdebug.h>
#include <qelapsedtimer.h>
#include "qopenglfunctions_3_2_core.h"

class ShapeData {
public:
	virtual int vertexBufferSize() = 0;
	virtual void Draw() {};
	virtual int getLifetime() { return 0; }

protected:

	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
	QTime* lifetimer;
};

class BulletShapeData : public ShapeData {
	//used for bullets->flying objects
public:
	virtual void Draw() {};
	virtual int getLifetime() { return 0; }
	virtual QVector3D getPosition() { return QVector3D(-1, -1, -1); };

	virtual ~BulletShapeData() {};

protected:
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
	QTime* lifetimer;
};


class BulletDataWVelocity : public ShapeData {
	//used for bullets->flying objects
public:
	virtual void Draw() {};
	virtual int getLifetime() { return 0; }
	virtual QVector3D getPosition() { return QVector3D(-1, -1, -1); };
	virtual void Update(double deltaTime) {};

protected:
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
	QTime* lifetimer;
};

class DShapeData : public ShapeData {
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
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
	QTime* lifetimer;
};


class DTexturedQuad : public DShapeData {
public:
	DTexturedQuad(QVector<float> vertexData) : vertexesRaw(vertexData) {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.create();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glEnableVertexAttribArray(1);
		//tell OpenGl how the data in VBO is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
		f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		f->glDisableVertexAttribArray(1);
		vertexesVBO.release();

		//Calculate Min and Max boundaries of the object on each axis.
		xMin = vertexesRaw[0];
		xMax = vertexesRaw[0];
		yMin = vertexesRaw[1];
		yMax = vertexesRaw[1];

		//assume all z-coordinates are the same
		zLowThresh = vertexesRaw[2] - 10.5;
		zHighThresh = vertexesRaw[2] + 10.5;

		int i = 0;

		while(i < 20){
			//x coordinate
			if (vertexesRaw[i] < xMin) {
				xMin = vertexesRaw[i];
			}
			if (vertexesRaw[i]  > xMax) {
				xMax = vertexesRaw[i];
			}

			//y coordinate
			if (vertexesRaw[i+1]  < yMin) {
				//yMin = vertexesRaw[i + 1 + 5];
				yMin = vertexesRaw[i + 1];
			}
			if (vertexesRaw[i+1]  > yMax) {
				// yMax = vertexesRaw[i + 1];
				 yMax = vertexesRaw[i + 1];
			}
			//stride is 5 because of Textured Quad has position coordinates (3)+ textured coordinates(2). To get to the next position coordinates ogl needs to skip 2 texture coordinates
			i = i + 5;
		}

	}

	void Draw() override {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glEnableVertexAttribArray(1);
		//tell OpenGl how the data in vbo is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
		f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		f->glDrawArrays(GL_QUADS, 0, 4);

		f->glDisableVertexAttribArray(0);
		f->glDisableVertexAttribArray(1);
		vertexesVBO.release();
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
		return vertexesRaw.size() * sizeof(float);
	}

	float getXMin() override{ return xMin_T; }
	float getXMax() override { return xMax_T; }
	float getYMin() override { return yMin_T; }
	float getYMax() override { return yMax_T; }
	float getZLowThresh() override { return zLowThresh_T; }
	float getZHighThresh() override { return zHighThresh_T; }
	
	~DTexturedQuad() {
		vertexesVBO.destroy();
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
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
};




//class SphereBullet : public BulletShapeData {
//public:
//	SphereBullet(Sphere sphere, QVector3D direction, QVector3D cameraPos) : m_direction(direction), m_cameraPos(cameraPos) {
//		//icosahaedron sphere from the internet-> first google hit when looking for (OpenGL sphere)-> see Sphere.h / Sphere.cpp
//		this->sphere = new Sphere(sphere);

//		//Shift sphere bullet from the coordinate origin to the front of the camera
//		m_position = cameraPos;

//		lifetimer = new QElapsedTimer();
//		lifetimer->start();
//	}
	
//	void Draw() override {
//		sphere->draw();
//		update();
//	}

//	void update() {
//		m_position += m_direction;
//		//applyGravity();
//	}

//	void setZPosition(float zCoord) {

//	}



//	void applyGravity() {
//		//can be done better
//		m_position -= QVector3D(0, +0.0610f, -0.0007);
//	}

//	QVector3D getPosition() override {
//		return m_position;
//	}

//	int vertexBufferSize()  {
//		return 0;
//	}

//	int getLifetime() {
//		return lifetimer->elapsed();
//	}

//	~SphereBullet() {
//		delete lifetimer;
//		//sphere->~Sphere();
//		delete sphere;
//	}
//private:
//	Sphere* sphere;
//	QVector3D m_direction;
//	QVector3D m_position;
//	QVector3D m_cameraPos;
//	QElapsedTimer* lifetimer;
//};


////A Textured shere with velocity and a update function which takes delta time as input
//class TexturedSphere1 : public BulletDataWVelocity {
//public:
//	TexturedSphere1(QVector3D direction, QVector3D cameraPos) : m_direction(direction), m_cameraPos(cameraPos), m_position(cameraPos) {
//		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
//		Sphere sph1(5.0f, 36, 18);

//		//icosahaedron sphere from the internet-> first google hit when looking for (OpenGL sphere)-> see Sphere.h / Sphere.cpp
//		this->sphere_ = new Sphere(sph1);
//		//generate and fill buffers with data
//		f->glGenBuffers(1, &vboId);
//		f->glBindBuffer(GL_ARRAY_BUFFER, vboId);
//		f->glBufferData(GL_ARRAY_BUFFER, sphere_->getInterleavedVertexSize(), sphere_->getInterleavedVertices(), GL_STATIC_DRAW);
//		f->glBindBuffer(GL_ARRAY_BUFFER, 0);
//		f->glGenBuffers(1, &iboId);
//		f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
//		f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_->getIndexSize(), sphere_->getIndices(), GL_STATIC_DRAW);

//		// unbind
//		f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

//		//Shift sphere bullet from the coordinate origin to the front of the camera
//		//m_position = cameraPos;

//		lifetimer = new QElapsedTimer();
//		lifetimer->start();
//	}

//	void Draw() override {
//		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
//		f->glBindBuffer(GL_ARRAY_BUFFER, vboId);//vertex buffer
//		f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);//index buffer
//		f->glEnableVertexAttribArray(0);
//		f->glEnableVertexAttribArray(1);

//		// set attrib arrays using glVertexAttribPointer()
//		int stride = sphere_->getInterleavedStride();
//		f->glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, 0);
//		f->glVertexAttribPointer(1, 2, GL_FLOAT, false, stride, (void*)(6 * sizeof(float)));//offset to texcords = 6 -> 3 position cords, 3 normal cords, then 2 texture cords

//		glDrawElements(GL_TRIANGLES, sphere_->getIndexCount(), GL_UNSIGNED_INT, (void*)0);

//		f->glDisableVertexAttribArray(0);
//		f->glDisableVertexAttribArray(1);

//		f->glBindBuffer(GL_ARRAY_BUFFER, 0);
//		f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);//index buffer

//	}

//	void setZPosition(float zCoord) {

//	}

//	void Update(double deltaTime) override {
//		float velocity = 6.0;
//		m_position += m_direction * velocity * deltaTime;	// calculate delta S(strecke) and add to actual position
//				//m_position += m_direction;		//QVector3D velocity(0.2, 0.2, 0.2);//velocity works in z direction

//		this->Draw();
//	}

//	void applyGravity() {
//		//can be done better
//		//m_position -= QVector3D(0, +0.0610f, -0.0007);
//	}

//	QVector3D getPosition() override {
//		return m_position;
//	}

//	int vertexBufferSize() {
//		return 0;
//	}

//	int getLifetime() {
//		return lifetimer->elapsed();
//	}

//	~TexturedSphere1() {
//		delete lifetimer;
//		sphere_->~Sphere();
//		//delete sphere_;
//	}
//private:
//	GLuint vboId;
//	GLuint iboId;
//	Sphere* sphere_;
//	QVector3D m_direction;
//	QVector3D m_position;
//	QVector3D m_cameraPos;
//	QElapsedTimer* lifetimer;
//};

/*
class TexturedSphere : public BulletShapeData {
public:
	TexturedSphere(Sphere sphere, QVector3D direction, QVector3D origin) : m_direction(direction), origin(origin){}

	void Draw() override {}

	void update() {}

	void setZPosition(float zCoord) {}

	void applyGravity() {}

	QVector3D getPosition() override {}

	int vertexBufferSize() {}

	int getLifetime() {}

	~TexturedSphere() {}
private:
	GLuint vboId;
	GLuint iboId;
	Sphere* sphere_;
	QVector3D m_direction;
	QVector3D m_position;
	QVector3D origin;
	QElapsedTimer* lifetimer;
};
*/

//class TexturedSphere : public BulletShapeData {
//public:
//	TexturedSphere(Sphere sphere, QVector3D direction, QVector3D origin) : m_direction(direction), m_origin(origin) {
//		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

//		//icosahaedron sphere from the internet-> first google hit when looking for (OpenGL sphere)-> see Sphere.h / Sphere.cpp
//		this->sphere_ = new Sphere(sphere);
//		//generate and fill buffers with data
//		f->glGenBuffers(1, &vboId);
//		f->glBindBuffer(GL_ARRAY_BUFFER, vboId);
//		f->glBufferData(GL_ARRAY_BUFFER, sphere_->getInterleavedVertexSize(), sphere_->getInterleavedVertices(), GL_STATIC_DRAW);
//		f->glBindBuffer(GL_ARRAY_BUFFER, 0);
//		f->glGenBuffers(1, &iboId);
//		f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
//		f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_->getIndexSize(), sphere_->getIndices(), GL_STATIC_DRAW);

//		// unbind
//		f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

//		//Shift sphere bullet from the coordinate origin to the front of the camera
//		m_position = origin;

//		lifetimer = new QElapsedTimer();
//		lifetimer->start();
//	}

//	void Draw() override {
//		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
//		f->glBindBuffer(GL_ARRAY_BUFFER, vboId);//vertex buffer
//		f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);//index buffer
//		f->glEnableVertexAttribArray(0);
//		f->glEnableVertexAttribArray(1);

//		// set attrib arrays using glVertexAttribPointer()
//		int stride = sphere_->getInterleavedStride();
//		f->glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, 0);
//		f->glVertexAttribPointer(1, 2, GL_FLOAT, false, stride, (void*)(6 * sizeof(float)));//offset to texcords = 6 -> 3 position cords, 3 normal cords, then 2 texture cords

//		glDrawElements(GL_TRIANGLES, sphere_->getIndexCount(), GL_UNSIGNED_INT, (void*)0);

//		f->glDisableVertexAttribArray(0);
//		f->glDisableVertexAttribArray(1);
		
//		f->glBindBuffer(GL_ARRAY_BUFFER, 0);
//		f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);//index buffer

//		this->update();
//	}

//	void setZPosition(float zCoord) {

//	}

//	void update() {
	
//		m_position += m_direction;
//	}

//	void applyGravity() {
//		//gravity only works on the y axis
//		m_position -= QVector3D(0, +0.0610f, 0);
//	}

//	QVector3D getPosition() override {
//		return m_position;
//	}

//	int vertexBufferSize() {
//		return 0;
//	}

//	int getLifetime() {
//		return lifetimer->elapsed();
//	}

//	~TexturedSphere() {
//		delete lifetimer;
//		sphere_->~Sphere();
//		//delete sphere_;
//	}
//private:
//	GLuint vboId;
//	GLuint iboId;
//	Sphere* sphere_;
//	QVector3D m_direction;
//	QVector3D m_position;
//	QVector3D m_origin;
//	QElapsedTimer* lifetimer;
//};




class Ray : public ShapeData {
public:
	Ray(QVector<float> vertexData , QVector3D direction, QVector3D cameraPos) : m_direction(direction), m_cameraPos(cameraPos){
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.create();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
		lifetimer = new QElapsedTimer();
		lifetimer->start();
	}
	~Ray() {
		vertexesVBO.destroy();
		delete lifetimer;
	}
	void Draw () override {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_LINE, 0, 2);
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	int vertexBufferSize() override {
		return 0;
	}
protected:
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;

private:
	QVector3D m_direction;
	QVector3D m_position;
	QVector3D m_cameraPos;
	QElapsedTimer* lifetimer;
};


class Triangle : public ShapeData {
public:
	Triangle(QVector<float> vertexData) : vertexesRaw(vertexData) {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.create();
		vertexesVBO.bind();
		//tell OpenGl how the data is placed in the buffer(buffer layout)
		f->glEnableVertexAttribArray(0);	
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	~Triangle() {
		vertexesVBO.destroy();
	}

	void Draw() override {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);		
		f->glDrawArrays(GL_TRIANGLES, 0, 3);
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	int vertexBufferSize() override {
		return vertexesRaw.size() * sizeof(float);
	}
protected:
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
};





class BulletTriangle : public BulletShapeData {
public:
	BulletTriangle(QVector<float> vertexData, QVector3D direction) : vertexesRaw(vertexData), _direction(direction) {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.create();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
		velocity = 30; //30 floats per second		//s(depth) = dt(time elapsed since last update) * v

		position_new[0] = vertexesRaw[0];
		position_new[1] = vertexesRaw[1];
		position_new[2] = vertexesRaw[2];

		lifetimer = new QElapsedTimer();
		lifetimer->start();
	}

	BulletTriangle(QVector<float> vertexData) : vertexesRaw(vertexData) {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.create();
		vertexesVBO.bind();
		//tell OpenGl how the data is placed in the buffer(buffer layout)
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
		velocity = 30; //30 floats per second		//s(depth) = dt(time elapsed since last update) * v

		position_new[0] = vertexesRaw[0];
		position_new[1] = vertexesRaw[1];
		position_new[2] = vertexesRaw[2];

		lifetimer = new QElapsedTimer();
		lifetimer->start();
	}

	~BulletTriangle() {
		vertexesVBO.destroy();
		delete lifetimer;
	}
	
	void Draw() override {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		this->update();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_TRIANGLES, 0, 3);
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
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
		return vertexesRaw.size() * sizeof(float);
	}

	int getLifetime(){
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
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
	QElapsedTimer* lifetimer;
};

class Quad : public ShapeData {
public:
	Quad(QVector<float> vertexData) : vertexesRaw(vertexData) {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.create();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	~Quad() {
		vertexesVBO.destroy();
	}

	void Draw() override {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_TRIANGLES, 0, 6);
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	int vertexBufferSize() override {
		return vertexesRaw.size() * sizeof(float);
	}
protected:
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
};


class RQuad : public BulletShapeData {
public:
	RQuad(QVector<float> vertexData, QVector3D position, QVector3D direction) : vertexesRaw(vertexData),  m_position(position), m_direction(direction)  {

		//shift in front of camera
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.create();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
		
		lifetimer = new QElapsedTimer();
		lifetimer->start();
	}

	~RQuad() {
		vertexesVBO.destroy();
		delete lifetimer;
	}
	void Draw() override {
		this->update();

		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_QUADS, 0, 4);
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	void update() {
		m_position += m_direction;
	}

	QVector3D getPosition() override {
		return m_position;
	}

	int vertexBufferSize()  {
		return vertexesRaw.size() * sizeof(float);
	}

protected:
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;

private:
	QVector3D m_direction;
	QVector3D m_position;
	QVector3D m_cameraPos;
	QElapsedTimer* lifetimer;
};




class Cube : public ShapeData {
public:
	Cube(QVector<float> vertexData) : vertexesRaw(vertexData) {

		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.create();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}
	~Cube() {
		vertexesVBO.destroy();
	}

	void Draw() override {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_TRIANGLES, 0, 36);		//12 triangles each 3 vertices times 3 floats(3x3x12).
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	int vertexBufferSize() override {
		return vertexesRaw.size() * sizeof(float);
	}

protected:
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
};



class CubeMap : public ShapeData {
public:
	CubeMap(QVector<float> vertexData, unsigned int texID) : vertexesRaw(vertexData), textureID(texID) {

		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		//vertexesVAO.create();
		vertexesVBO.create();
		//vertexesVAO.bind();
		vertexesVBO.bind();
		
		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));	//gl buffer data
		f->glEnableVertexAttribArray(0);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}
	~CubeMap() {
		vertexesVBO.destroy();
	}

	void Draw() override {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		//vertexesVAO.bind();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 , nullptr);

		f->glActiveTexture(GL_TEXTURE0);
		f->glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		f->glDrawArrays(GL_TRIANGLES, 0, 36);		//12 triangles each 3 vertices times 3 floats(3x3x12).
		f->glDisableVertexAttribArray(0);

		vertexesVBO.release();
		//vertexesVAO.release();
	}

	int vertexBufferSize() override {
		return vertexesRaw.size() * sizeof(float);
	}

protected:
	QOpenGLBuffer vertexesVBO;
	//QOpenGLVertexArrayObject vertexesVAO;
	QVector<float> vertexesRaw;
	unsigned int textureID;
};


class BulletCube : public ShapeData {
public:
	BulletCube(QVector<float> vertexData) : vertexesRaw(vertexData) {

		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.create();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	void Draw() override {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_TRIANGLES, 0, 36);		//12 triangles each 3 vertices times 3 floats(3x3x12).
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	int vertexBufferSize() override {
		return vertexesRaw.size() * sizeof(float);
	}

	~BulletCube() {
		vertexesVBO.destroy();
	}
protected:
	bool drawed;
	int lifetime;
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
};



class TexturedQuad : public ShapeData {
	//implement texture binding when the vertexvbo is bound...also implement shader which can draw textures(sampler2D)..therefor implement shader system(maybe hardcoded reading of textfiles?)
public:
	TexturedQuad(QVector<float> vertexData) : vertexesRaw(vertexData) {

		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.create();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glEnableVertexAttribArray(1);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
		f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));
		
		f->glDisableVertexAttribArray(0);

		f->glDisableVertexAttribArray(1);


		vertexesVBO.release();
	}

	void Draw() override {

		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glEnableVertexAttribArray(1);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);	//Position Coordinates
		f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));	//2 Texture Coordinates

		f->glDrawArrays(GL_QUADS, 0, 4);

		f->glDisableVertexAttribArray(0);
		f->glDisableVertexAttribArray(1);
		vertexesVBO.release();
	}

	int vertexBufferSize() override {
		return vertexesRaw.size() * sizeof(float);
	}

protected:
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
};





class Point : public ShapeData {
public:
	Point(QVector<float> vertexData) : vertexesRaw(vertexData) {
	
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.create();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	void Draw() override {

		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_POINTS, 0, 1);
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	int vertexBufferSize() override {
		return vertexesRaw.size() * sizeof(float);
	}

	~Point() {
		vertexesVBO.destroy();
	}

protected:
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
};


class Quad_f : public ShapeData {
public:
	Quad_f(QVector<float> vertexData) : vertexesRaw(vertexData) {

		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.create();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		//tells OpenGl how the data in m_vao is placed
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		vertexesVBO.allocate(vertexesRaw.constData(), vertexesRaw.size() * sizeof(float));
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	~Quad_f() {
		vertexesVBO.destroy();
	}
	void Draw() override {

		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		vertexesVBO.bind();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		f->glDrawArrays(GL_QUADS, 0, 4);
		f->glDisableVertexAttribArray(0);
		vertexesVBO.release();
	}

	int vertexBufferSize() override {
		return vertexesRaw.size() * sizeof(float);
	}
protected:
	QOpenGLBuffer vertexesVBO;
	QVector<float> vertexesRaw;
};
