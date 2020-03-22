//#include "ShapeData.h"

/*
int ShapeData::vertexBufferSize()
{
	return vertexesRaw.size() * sizeof(float);
}
*/
/*
void ShapeData::Draw() {

	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	vertexesVBO.bind();

	f->glEnableVertexAttribArray(0);
	f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	f->glDrawArrays(GL_TRIANGLES, 0, 3);

	f->glDisableVertexAttribArray(0);
	vertexesVBO.release();
}
*/

/*
ShapeData::ShapeData(QVector<float> vertexData) : vertexesRaw(vertexData), vertexesVBO(QOpenGLBuffer::VertexBuffer) {

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
*/
/*
ShapeData::~ShapeData() {


}
*/


/*
class Cube : public ShapeData {

public:
protected:
private:

	

};
*/
/*
int ShapeData::vertexBufferSize()
{
	return 0;
}

void ShapeData::Draw()
{
}
*/