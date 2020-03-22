#include "QOpenGLPlotter.h"
//coding convention : _var = member Variable
#include <math.h>
#include <time.h>
#include <thread>
#include <mutex>
#include <chrono>
#define SREENWIDTH 1680
#define SCREENHEIGHT 1050


QOpenGLPlotter::~QOpenGLPlotter() {
	delete m_projectionMat;
	delete m_modelMat;
	delete m_viewMat;
	delete m_MVP;
	
	delete update_timer;
	delete dataUpdate_timer;

    dataUpdate_timer->deleteLater();
	delete timestampsGen_ms;

	delete plot1;
	delete plot2;
	delete plot3;
	delete plot4;
	delete plot5;
	delete plot6;
}

QOpenGLPlotter::QOpenGLPlotter(QWidget* parent) : m_prog(){
	_nearZ = 1.0;
	_farZ = 100.0;
	cameraX = 0.0;
	cameraY = 0.0;
	cameraZ = 0.0;
	lookatX = 0.0;
	lookatY = 0.0;
	lookatZ = 1.0;
	//DO NOT USE OPENGL COMMANDS IN CONSTRUCTOR. First INITIALIZEGL needs to be executed to initialize OGL functions
	_framecounter = 0;

	//Chart parameters
	x_dist = 1;

	//timestampsGen_ms = new QElapsedTimer();
	//timestampsGen_ms->start();


	_pointcount = 0;

	m_projectionMat = new QMatrix4x4();
	m_modelMat = new QMatrix4x4();
	m_viewMat = new QMatrix4x4();
	m_MVP = new QMatrix4x4();

	m_projectionMat->setToIdentity();
	m_modelMat->setToIdentity();
	m_viewMat->setToIdentity();
	m_MVP->setToIdentity();

	update_timer = new QTimer();
	connect(update_timer, SIGNAL(timeout()), this, SLOT(update()));
	update_timer->setInterval(30);

	dataUpdate_timer = new QTimer();
	connect(dataUpdate_timer, SIGNAL(timeout()), this, SLOT(on_dataUpdate()));
	dataUpdate_timer->setInterval(1);



}

void QOpenGLPlotter::dataThreadFunc() {
	while(true){
	//duplicate for testing
	double pi_ = 3.1415026589;
	double val_in_radians = _pointcount * (2 * pi_) / 360;
	double data_value = 10 * std::sin(val_in_radians);
	double data_value_cos = 10 * std::cos(val_in_radians);

	plot1->addData(data_value, _pointcount); //assume pointcount is in ms
	plot2->addData(data_value, _pointcount); //assume pointcount is in ms
	plot3->addData(data_value, _pointcount); //assume pointcount is in ms
	plot4->addData(data_value_cos, _pointcount); //assume pointcount is in ms
	plot5->addData(data_value, _pointcount); //assume pointcount is in ms
	plot6->addData(data_value_cos, _pointcount); //assume pointcount is in ms

	//std::cout << timestampsGen_ms->elapsed() << "\n\r";
	_pointcount++;
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
}


void QOpenGLPlotter::on_dataUpdate() {
	//plot1->addData(std::rand() % 10, x_dist *_pointcount);

	double pi_ = 3.1415026589;
	double val_in_radians = _pointcount * (2 * pi_) / 360;
	double data_value = 10 * std::sin(val_in_radians);

	plot1->addData(data_value, _pointcount); //assume pointcount is in ms
	plot2->addData(data_value, _pointcount); //assume pointcount is in ms
//	plot3->addData(data_value, _pointcount); //assume pointcount is in ms

	//std::cout << timestampsGen_ms->elapsed() << "\n\r";
	_pointcount++;
	
}



void QOpenGLPlotter::initializeGL() {
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	//initialize OGL functions before any other OpenGL call !
	f->initializeOpenGLFunctions();

	//Check if  OS is able to scale the width of drawn lines(rays) (with standard ogl 'GL_LINES' flag)
	std::cout << "Start OpenGlPlotter by Jonas Biehrer" << "\n\r";
	std::cout << "maximum OGL line width on this operating system" << "\n\r";
	QString min;
	QString max;
	GLfloat linerange[2];

//    this->context()->currentContext()->versionFunctions()->set
	f->glGetFloatv(GL_LINE_WIDTH_RANGE, linerange);
	min = linerange[0];
	max = linerange[1];

	std::cout << "minimal width: " << min.toFloat() << "\n\r";
	std::cout << "maximal width: " << max.toFloat() << "\n\r";

    std::cout << "maximum OGL point size on this operating system" << "\n\r";

    GLint point_range[2];
    f->glGetIntegerv(GL_ALIASED_POINT_SIZE_RANGE, point_range);

    std::cout << "minimal width: " << point_range[0] << "\n\r";
    std::cout << "maximal width: " << point_range[1] << "\n\r";

	//updates ogl scene
	/*
	auto format = QSurfaceFormat::defaultFormat();
	format.setSwapInterval(0);
	format.setVersion(3, 3);
	format.setProfile(QSurfaceFormat::CoreProfile);

	QSurfaceFormat::setDefaultFormat(format);
	*/
	//OpenGL Settings
	f->glEnable(GL_TEXTURE_2D_ARRAY);
	f->glEnable(GL_TEXTURE_2D);
	f->glEnable(GL_TEXTURE_CUBE_MAP);
	f->glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	//Enable Depth Testing to allow overlapping objects
	f->glEnable(GL_DEPTH);
	f->glEnable(GL_DEPTH_TEST);
	f->glDepthFunc(GL_LESS);

	//Enable Blending to create transparency 
	f->glEnable(GL_BLEND);
	f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	f->glDisable(GL_BLEND);

	//f->glEnable(GL_CULL_FACE);
	f->glEnable(GL_POINT_SMOOTH);
	f->glEnable(GL_POINT_SIZE);
	f->glEnable(GL_PROGRAM_POINT_SIZE);

//    this->context()->versionFunctions<QOpenGLFunctions_3_1>()->versionProfile().setVersion(3,3)>;

//     this->context()->versionFunctions<QOpenGLFunctions_3_1>()->;
//    auto funcs =
//    if (!funcs) {
//        throw std::runtime_error("Could not obtain required OpenGL context version");
//    }
	//Get OpenGL Version from OS
	char *p = (char*)f->glGetString(GL_VERSION);
	std::cout << "using OpenGl Version: " << p << "\n\r";


	initShaders();
	//plot1 = new OGLChart(100000, 0, (this->height() / 2) + 10, SREENWIDTH, this->height() / 2);//space for 10000 Points (each point consists of 3 floats)
	/*
	plot1 = new OGLChart(10000, 0, (this->height() / 6) * 1 + 10, SREENWIDTH, this->height() / 6, 1);//space for 10000 Points (each point consists of 3 floats)
	plot2 = new OGLChart(10000, 0, (this->height() / 6) * 2 + 10, SREENWIDTH, this->height() / 6, 2);//space for 10000 Points (each point consists of 3 floats)
	plot3 = new OGLChart(10000, 0, (this->height() / 6) * 3 + 10, SREENWIDTH, this->height() / 6, 3);//space for 10000 Points (each point consists of 3 floats)
	plot4 = new OGLChart(10000, 0, (this->height() / 6) * 4 + 10, SREENWIDTH, this->height() / 6, 4);//space for 10000 Points (each point consists of 3 floats)
	plot5 = new OGLChart(10000, 0, (this->height() / 6) * 5 + 10, SREENWIDTH, this->height() / 6, 5);//space for 10000 Points (each point consists of 3 floats)
	plot6 = new OGLChart(10000, 0, (this->height() / 6) * 6 + 10, SREENWIDTH, this->height() / 6, 6);
	*/
	plot1 = new OGLChart(10000, 0, (SCREENHEIGHT / 6) * 1 + 10, SREENWIDTH, SCREENHEIGHT / 6, 1);//space for 10000 Points (each point consists of 3 floats)
	plot2 = new OGLChart(10000, 0, (SCREENHEIGHT / 6) * 2 + 10, SREENWIDTH, SCREENHEIGHT / 6, 2);//space for 10000 Points (each point consists of 3 floats)
	plot3 = new OGLChart(10000, 0, (SCREENHEIGHT / 6) * 3 + 10, SREENWIDTH, SCREENHEIGHT / 6, 3);//space for 10000 Points (each point consists of 3 floats)
	plot4 = new OGLChart(10000, 0, (SCREENHEIGHT / 6) * 4 + 10, SREENWIDTH, SCREENHEIGHT / 6, 4);//space for 10000 Points (each point consists of 3 floats)
	plot5 = new OGLChart(10000, 0, (SCREENHEIGHT / 6) * 5 + 10, SREENWIDTH, SCREENHEIGHT / 6, 5);//space for 10000 Points (each point consists of 3 floats)
	plot6 = new OGLChart(10000, 0, (SCREENHEIGHT / 6) * 6 + 10, SREENWIDTH, SCREENHEIGHT / 6, 6);

	update_timer->start();
	//dataUpdate_timer->start();
}

//is called when the window is resized
void QOpenGLPlotter::resizeGL(int width, int height){
	//execute when window gets resized
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	m_projectionMat->setToIdentity();
	m_viewMat->setToIdentity();
	
	float height_ = this->height();
	float width_ = this->width();

	float aspect = width_ / height_;
	m_projectionMat->ortho(QRect(0, 0, width_, height_));
	//m_projectionMat->ortho(-width_, width_, -height_, height_, _nearZ, _farZ);
	//recalculate projection matrix
	//m_projectionMat->perspective(_fov, aspect, nearZ, farZ);
	//m_viewMat->lookAt(QVector3D(cameraX, cameraY, cameraZ), QVector3D(lookatX, lookatY, lookatZ), QVector3D(0, 1, 0));
	//using namespace std::chrono_literals;
	/*
	static int p_count = 0;
	std::thread t([](OGLChart& ref) {
		//		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
				//std::this_thread::sleep_for
		while (true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

			double pi_ = 3.1415026589;
			double val_in_radians = p_count * (2 * pi_) / 360;
			double data_value = 10 * std::sin(val_in_radians);

			ref.addData(data_value, p_count); //assume pointcount is in ms
		//	plot2->addData(data_value, _pointcount); //assume pointcount is in ms
		//	plot3->addData(data_value, _pointcount); //assume pointcount is in ms

			//std::cout << timestampsGen_ms->elapsed() << "\n\r";
			p_count++;
		}

	}, *plot1);
	*/
	f->glViewport(0, 0, this->width(), this->height());
	this->update();
}


bool QOpenGLPlotter::initShaders() {

	//initializes all shaders used in this program
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	std::cout << "\n\r";
	std::cout << "Shader Compiling Error Log:" << "\n\r";
	std::cout << "Standard Shader error log: ";
	std::cout << "\n\r";

	QString path(QDir::currentPath());
    //path = QDir::toNativeSeparators(path);	//changes (forward)slashes to backslashes

	std::cout << "filepath to shaders: " << path.toStdString() << "\n\r";

	bool success = false;
    success = m_prog.addShaderFromSourceFile(QOpenGLShader::Vertex, QString(path + "//Resources//shaders//vertex.vsh"));

    QString errorLog = m_prog.log();
    std::cout << "Vertex Shader sucess?: " << success << "\n\r";
    std::cout << &errorLog;
    std::cout << "\n\r";

    if(!success){
        throw::std::runtime_error("Error while readingv shader");
    }


    success = m_prog.addShaderFromSourceFile(QOpenGLShader::Fragment, QString(path + "//Resources//shaders//fragment.fsh"));

    errorLog = m_prog.log();
    std::cout << "Fragment Shader sucess?: " << success << "\n\r";
    std::cout << &errorLog;
    std::cout << "\n\r";

    if(!success){
        throw::std::runtime_error("Error while reading shader");
    }


	success = m_prog.link();
	errorLog = m_prog.log();
	std::cout <<"linkinkg success?: "<< success << "\n\r" << "shader programm linking errors: ";
	std::cout << &errorLog;
	std::cout << "\n\r";

	m_prog.bind();
	m_prog.bindAttributeLocation("position", 0);
	m_prog.bindAttributeLocation("vertexColor", 1);
	m_prog.release();

	return success;
}

 void QOpenGLPlotter::mouseMoveEvent(QMouseEvent* evt) {
	 float x  = evt->x();
	 float y = evt->y();

	 plot1->addData(x, y);
}


 void QOpenGLPlotter::mousePressEvent(QMouseEvent* evt) {
	 float x = evt->x();
	 float y = evt->y();

	 plot1->addData(x, y);
 }


void QOpenGLPlotter::paintGL(){
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glClearColor(1.0f, 0.0f, 0.0f, 0.5f);

	f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_framecounter++;

	*m_MVP = *(m_projectionMat) * *(m_viewMat) * *(m_modelMat);

	m_prog.bind();

	m_prog.setUniformValue("u_MVP", *m_MVP);
    m_prog.setUniformValue("point_scale", 10.0f);
	m_prog.setUniformValue("u_Color", QVector3D(1.0f, 1.0f, 1.0f));

	plot1->Draw();
	plot2->Draw();
	plot3->Draw();
	plot4->Draw();
	plot5->Draw();
	plot6->Draw();

    ShapeGenerator shapegen;
//    auto triang_vertices = shapegen.makeTriangleAtPos_(1.0f, 1.0f, 1.0f, 1000.0f);
//    Triangle triang(triang_vertices);
//    triang.Draw();

    auto point_vertices = shapegen.makePointAtPos_(10.0f, 10.0f, 1.0f);
    Point pt(point_vertices);
    pt.Draw();

	m_prog.release();
}
