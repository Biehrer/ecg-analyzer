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
    delete _projection_mat;
    delete _model_mat;
    delete _view_mat;
    delete _MVP;
	
    delete paint_update_timer;
    dataUpdate_timer->deleteLater();

	delete dataUpdate_timer;

	delete plot1;
	delete plot2;
	delete plot3;
	delete plot4;
	delete plot5;
	delete plot6;
}

QOpenGLPlotter::QOpenGLPlotter(QWidget* parent)
    :
      m_prog()
{
    // Setup opengl parameters
    // DO NOT USE OPENGL COMMANDS INSIDE THE CONSTRUCTOR

	_nearZ = 1.0;
	_farZ = 100.0;

    _projection_mat = new QMatrix4x4();
    _model_mat = new QMatrix4x4();
    _view_mat = new QMatrix4x4();
    _MVP = new QMatrix4x4();

    _projection_mat->setToIdentity();
    _model_mat->setToIdentity();
    _view_mat->setToIdentity();
    _MVP->setToIdentity();

    _framecounter = 0;
    _pointcount = 0;

    paint_update_timer = new QTimer();
    connect(paint_update_timer, SIGNAL(timeout()), this, SLOT(update()));
    paint_update_timer->setInterval(30);

    dataUpdate_timer = new QTimer();
    connect(dataUpdate_timer, SIGNAL(timeout()), this, SLOT(on_dataUpdate()));
    dataUpdate_timer->setInterval(1);
    dataUpdate_timer->start(1);
}

void QOpenGLPlotter::dataThreadFunc()
{
	while(true){
        // duplicate for testing
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

        _pointcount++;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
}


void QOpenGLPlotter::on_dataUpdate()
{
	double pi_ = 3.1415026589;
	double val_in_radians = _pointcount * (2 * pi_) / 360;
	double data_value = 10 * std::sin(val_in_radians);

	plot1->addData(data_value, _pointcount); //assume pointcount is in ms
	plot2->addData(data_value, _pointcount); //assume pointcount is in ms

	_pointcount++;	
}



void QOpenGLPlotter::initializeGL()
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // Initialize OGL functions before any other OpenGL call
	f->initializeOpenGLFunctions();

    std::cout << "Start OpenGlPlotter by Jonas Biehrer" << std::endl;

    //Get OpenGL Version from OS
    char *p = (char*)f->glGetString(GL_VERSION);
    std::cout << "using OpenGl Version: " << p << std::endl;

    //Check if  OS is able to scale the width of drawn lines(rays) (with standard ogl 'GL_LINES' flag)
    std::cout << "maximum OGL line width on this operating system" << std::endl;
    GLfloat linerange[2];
	f->glGetFloatv(GL_LINE_WIDTH_RANGE, linerange);
    std::cout << "minimal width: " << linerange[0] << std::endl;
    std::cout << "maximal width: " << linerange[1] << std::endl;

    std::cout << "maximum OGL point size on this operating system" << std::endl;
    GLint point_range[2];
    f->glGetIntegerv(GL_ALIASED_POINT_SIZE_RANGE, point_range);
    std::cout << "minimal size: " << point_range[0] << std::endl;
    std::cout << "maximal size: " << point_range[1] << std::endl;

    // OpenGL Settings
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


    InitializeShaderProgramms();
//    plot1 = new OGLChart(10000, 0, (this->height() / 6) * 1 + 10, SREENWIDTH, this->height() / 6, 1); //space for 10000 Points (each point consists of 3 floats)
//    plot2 = new OGLChart(10000, 0, (this->height() / 6) * 2 + 10, SREENWIDTH, this->height() / 6, 2); //space for 10000 Points (each point consists of 3 floats)
//    plot3 = new OGLChart(10000, 0, (this->height() / 6) * 3 + 10, SREENWIDTH, this->height() / 6, 3); //space for 10000 Points (each point consists of 3 floats)
//    plot4 = new OGLChart(10000, 0, (this->height() / 6) * 4 + 10, SREENWIDTH, this->height() / 6, 4); //space for 10000 Points (each point consists of 3 floats)
//    plot5 = new OGLChart(10000, 0, (this->height() / 6) * 5 + 10, SREENWIDTH, this->height() / 6, 5); //space for 10000 Points (each point consists of 3 floats)
//	plot6 = new OGLChart(10000, 0, (this->height() / 6) * 6 + 10, SREENWIDTH, this->height() / 6, 6);

    int screenwidth_fraction = SREENWIDTH / 6;
    plot1 = new OGLChart(10000, 0, (SCREENHEIGHT / 6) * 1 + 10, SREENWIDTH - screenwidth_fraction, SCREENHEIGHT / 6);//space for 10000 Points (each point consists of 3 floats)
    plot2 = new OGLChart(10000, 0, (SCREENHEIGHT / 6) * 2 + 10, SREENWIDTH - screenwidth_fraction, SCREENHEIGHT / 6);//space for 10000 Points (each point consists of 3 floats)
    plot3 = new OGLChart(10000, 0, (SCREENHEIGHT / 6) * 3 + 10, SREENWIDTH - screenwidth_fraction, SCREENHEIGHT / 6);//space for 10000 Points (each point consists of 3 floats)
    plot4 = new OGLChart(10000, 0, (SCREENHEIGHT / 6) * 4 + 10, SREENWIDTH - screenwidth_fraction, SCREENHEIGHT / 6);//space for 10000 Points (each point consists of 3 floats)
    plot5 = new OGLChart(10000, 0, (SCREENHEIGHT / 6) * 5 + 10, SREENWIDTH - screenwidth_fraction, SCREENHEIGHT / 6);//space for 10000 Points (each point consists of 3 floats)
    plot6 = new OGLChart(10000, 0, (SCREENHEIGHT / 6) * 6 + 10, SREENWIDTH - screenwidth_fraction, SCREENHEIGHT / 6);

    paint_update_timer->start();
}

//is called when the window is resized
void QOpenGLPlotter::resizeGL(int width, int height){
	//execute when window gets resized
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    _projection_mat->setToIdentity();
    _view_mat->setToIdentity();

    _projection_mat->ortho(QRect(0, 0, this->width(),this->height()));

	f->glViewport(0, 0, this->width(), this->height());
	this->update();
}


bool QOpenGLPlotter::InitializeShaderProgramms()
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    std::cout << std::endl;
    std::cout << "Shader Compiling Error Log:" << std::endl;
	std::cout << "Standard Shader error log: ";
    std::cout << std::endl;

    QString path_of_executable(QDir::currentPath());
    std::cout << "expected filepath to shaders (make sure it exists): "
              << path_of_executable.toStdString() << std::endl;

	bool success = false;
    success = m_prog.addShaderFromSourceFile(QOpenGLShader::Vertex, QString(path_of_executable + "//Resources//shaders//vertex.vsh"));

    QString errorLog = m_prog.log();
    std::cout << "Vertex Shader sucess?: " << success << std::endl;
    std::cout << &errorLog;
    std::cout << std::endl;

    if( !success ){
        throw::std::runtime_error("Error while readingv shader");
    }

    success = m_prog.addShaderFromSourceFile(QOpenGLShader::Fragment, QString(path_of_executable + "//Resources//shaders//fragment.fsh"));

    errorLog = m_prog.log();
    std::cout << "Fragment Shader sucess?: " << success << std::endl;
    std::cout << &errorLog;
    std::cout << std::endl;

    if( !success ){
        throw::std::runtime_error("Error while reading shader");
    }

	success = m_prog.link();
	errorLog = m_prog.log();
    std::cout <<"linkinkg success?: "<< success << std::endl << "shader programm linking errors: ";
	std::cout << &errorLog;
    std::cout << std::endl;

	m_prog.bind();
	m_prog.bindAttributeLocation("position", 0);
	m_prog.bindAttributeLocation("vertexColor", 1);
	m_prog.release();

	return success;
}


void QOpenGLPlotter::paintGL(){
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glClearColor(1.0f, 0.0f, 0.0f, 0.5f);
	f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_framecounter++;

    *_MVP = *(_projection_mat) * *(_view_mat) * *(_model_mat);

	m_prog.bind();

    m_prog.setUniformValue("u_MVP", *_MVP);
    m_prog.setUniformValue("point_scale", 2.0f);
	m_prog.setUniformValue("u_Color", QVector3D(1.0f, 1.0f, 1.0f));

	plot1->Draw();
	plot2->Draw();
	plot3->Draw();
	plot4->Draw();
	plot5->Draw();
	plot6->Draw();

	m_prog.release();
}


void QOpenGLPlotter::mouseMoveEvent(QMouseEvent* evt) {
    float x  = evt->x();
    float y = evt->y();
   // plot1->addData(x, y);
}


void QOpenGLPlotter::mousePressEvent(QMouseEvent* evt) {
    float x = evt->x();
    float y = evt->y();
   // plot1->addData(x, y);
}
