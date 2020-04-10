#include "QOpenGLPlotWidget.h"

#include <qglobal.h>
#include <qglobalstatic.h>

#include <chrono>
#include <iostream>
//#define DEBUG_INFO

#ifdef DEBUG_INFO
    #define DEBUG(msg) std::cout << msg << std::endl;
#else
    #define DEBUG(msg) do{} while(0)
#endif


QOpenGLPlotWidget::~QOpenGLPlotWidget() {
    delete _projection_mat;
    delete _model_mat;
    delete _view_mat;
    delete _MVP;
	
    _paint_update_timer->deleteLater();
    delete _paint_update_timer;

    _data_update_timer->deleteLater();
	delete _data_update_timer;
    
    // Delete plots
    for ( int chart_idx = _plots.size() - 1; chart_idx >= 0; --chart_idx ) {
        delete _plots[chart_idx];
    }
}

QOpenGLPlotWidget::QOpenGLPlotWidget(QWidget* parent)
    :
      _prog()
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

    _paint_update_timer = new QTimer();
    connect(_paint_update_timer, SIGNAL(timeout()), this, SLOT(update()));
    _paint_update_timer->setInterval(30);

    double data_gen_frequency_hz = 1000.0;
    double data_gen_frequency_s = 1.0 / data_gen_frequency_hz;
    double data_gren_frequency_ms = data_gen_frequency_s * 1000.0;

    _data_update_timer = new QTimer();
    connect(_data_update_timer, SIGNAL(timeout()), this, SLOT(OnDataUpdate()));
    _data_update_timer->setInterval(data_gren_frequency_ms);
    //_data_update_timer->start(data_gren_frequency_ms);
}

void QOpenGLPlotWidget::OnDataUpdateThreadFunction()
{
    double pi = 3.1415026589;
    double data_val_static = 5;

    Timestamp_TP timestamp;
    
    while(true){
        timestamp.Now();
        // duplicate for testing
        double val_in_radians = _pointcount * (2.0 * pi) / 360.0;
        double data_value = 5.0 * std::sin(val_in_radians);
        for ( auto& plot : _plots ) {
            plot->AddDataToSeries(data_value, _pointcount);
            //plot->AddData(data_value, timestamp);
        }
        ++_pointcount;
        //DEBUG("Thread added point (# " << _pointcount << "): " << data_value);
        std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
}

void QOpenGLPlotWidget::OnDataUpdate()
{
    double pi = 3.1415026589;
    double val_in_radians = _pointcount * (2.0 * pi) / 360.0;
    float data_value = 10.0 * std::sin(val_in_radians);
    for ( auto& plot : _plots ) {
        // assume that _pointcount is an incrementing value with the unit milliseconds
        plot->AddDataToSeries(data_value, _pointcount);
    }
    DEBUG("Added data value (# " << _pointcount << "): " << data_value);
	_pointcount++;	
}



void QOpenGLPlotWidget::InitializePlots(int number_of_plots)
{
    // This value equals the displayable milliseconds
    int max_point_count = 2000;

    int screenwidth_fraction = SREENWIDTH / 6;
    int chart_width = SREENWIDTH - screenwidth_fraction;
    int chart_height = SCREENHEIGHT / number_of_plots;
    // Chart is aligned at the right side of the screen
    int chart_pos_x = 0;

    std::cout << "initialize plots: " << std::endl;
    int chart_to_chart_offset_S = 10;
    // origin = _screenpos_x/y
    int chart_offset_from_origin_S = 4;

    for ( int chart_idx = 0; chart_idx < number_of_plots; ++chart_idx ) {
        
        int chart_pos_y = (chart_height + chart_to_chart_offset_S) * chart_idx + chart_offset_from_origin_S; 
        _plots.push_back(new OGLChart_C(max_point_count, chart_pos_x, chart_pos_y, chart_width, chart_height, *this));

        std::cout << "chart pos (idx=" << chart_idx << "): " << chart_pos_y << std::endl;
    }

    _paint_update_timer->start();
}


void QOpenGLPlotWidget::initializeGL()
{
    std::cout << "Start OpenGlPlotter by Jonas Biehrer" << std::endl;

    InitializeGLParameters();

    InitializeShaderProgramms();

    InitializePlots(2);

    CreateLightSource();
}

void QOpenGLPlotWidget::resizeGL(int width, int height)
{
    _projection_mat->setToIdentity();
    _view_mat->setToIdentity();
    // This window is never resized. only JonesPlot.h is resized.
    // If this event should be triggered, it needs to be passed to this widget.
    _projection_mat->ortho(QRect(0, 0, this->width(),this->height()));

    //_projection_mat->ortho(QRect(-(this->width() / 2), -(this->height()/2), this->width(), this->height() ));

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	f->glViewport(0, 0, this->width(), this->height());
	this->update();
}

void QOpenGLPlotWidget::paintGL()
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClearColor(0.0f, 0.0f, 0.0f, 0.8f);
	f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ++_framecounter;

    *_MVP = *(_projection_mat) * *(_view_mat) * *(_model_mat);

    //_prog.bind();
    //_prog.setUniformValue("u_MVP", *_MVP);
    //_prog.setUniformValue("point_scale", 2.0f);
    //_prog.setUniformValue("u_Color", QVector3D(1.0f, 1.0f, 1.0f));

    _light_shader.bind();
    _light_shader.setUniformValue("u_MVP", *_MVP);
    _light_shader.setUniformValue("point_scale", 2.0f);
    _light_shader.setUniformValue("u_object_color", QVector3D(1.0f, 1.0f, 1.0f));
    _light_shader.setUniformValue("u_light_color", QVector3D(0.0f, 1.0f, 1.0f));

    //float color_counter = 2;
    for ( const auto& plot : _plots ) {
        //float red_val = std::sinf(color_counter * 10.0);
        //float blue_val = std::cosf(color_counter * 10.0);
        //_prog.setUniformValue("u_Color", QVector3D(red_val, blue_val, 1.0f));
        //++color_counter;
        plot->Draw();
    }

    //_prog.release();

    //_light_shader.bind();
    //_light_shader.setUniformValue("u_MVP", *_MVP);
    //_light_shader.setUniformValue("point_scale", 2.0f);
    //_light_shader.setUniformValue("u_object_color", QVector3D(1.0f, 1.0f, 1.0f));
    //_light_shader.setUniformValue("u_light_color", QVector3D(1.0f, 0.0f, 1.0f));
    //_light_source.Draw();
    _light_shader.release();
}

void QOpenGLPlotWidget::InitializeGLParameters()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // Initialize OGL functions before any other OpenGL call
    f->initializeOpenGLFunctions();
    // Get OpenGL Version from OS
    char *p = (char*)f->glGetString(GL_VERSION);
    std::cout << "using OpenGl Version: " << p << std::endl;

    // Check if  OS is able to scale the width of drawn lines(rays) (with standard ogl 'GL_LINES' flag)
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

    // Enable Depth Testing to allow overlapping objects
    f->glEnable(GL_DEPTH);
    f->glEnable(GL_DEPTH_TEST);
    f->glDepthFunc(GL_LESS);

    // Enable Blending to create transparency
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    f->glDisable(GL_BLEND);

    // f->glEnable(GL_CULL_FACE);
    f->glEnable(GL_POINT_SMOOTH);
    f->glEnable(GL_POINT_SIZE);
    f->glEnable(GL_PROGRAM_POINT_SIZE);
}

void QOpenGLPlotWidget::CreateLightSource()
{
    float light_source_x_pos = 0.0f;
    float light_source_y_pos = 0.0f;
    float light_source_z_pos = -1.5f;
    float cube_size = this->width();

    ShapeGenerator_C shape_gen;
    auto light_source_vertices = shape_gen.makeQuadAtPos_(light_source_x_pos, light_source_y_pos, light_source_z_pos, cube_size);
    _light_source.CreateVBO(light_source_vertices);
}

bool QOpenGLPlotWidget::InitializeShaderProgramms()
{
    std::cout << std::endl;
    std::cout << "Shader Compiling Error Log:" << std::endl;
    std::cout << "Standard Shader error log: ";
    std::cout << std::endl;

    QString path_of_executable( QDir::currentPath() );
    std::cout << "expected filepath to shaders (make sure it exists): "
        << path_of_executable.toStdString() << std::endl;

    QString path_of_shader_dir = path_of_executable + "//Resources//shaders//";

    // Standard color shader
    std::vector<QString> std_shader_uniforms;
    std_shader_uniforms.push_back("position");
    std_shader_uniforms.push_back("vertexColor");
    bool result = CreateShader(_prog,
        QString(path_of_shader_dir + "vertex.vsh"),
        QString(path_of_shader_dir + "fragment.fsh"),
        std_shader_uniforms);

    //if ( !result ) {
    //    throw std::runtime_error("Error while linking std shader");
    //}

    //bool success = false;
    //success = _prog.addShaderFromSourceFile(QOpenGLShader::Vertex, QString(path_of_shader_dir +"vertex.vsh"));

    //QString errorLog = _prog.log();
    //std::cout << "Vertex Shader sucess?: " << success << std::endl;
    //std::cout << &errorLog;
    //std::cout << std::endl;

    //if ( !success ) {
    //    throw::std::runtime_error("Error while readingv shader");
    //}

    //success = _prog.addShaderFromSourceFile(QOpenGLShader::Fragment, QString(path_of_shader_dir + "fragment.fsh"));

    //errorLog = _prog.log();
    //std::cout << "Fragment Shader sucess?: " << success << std::endl;
    //std::cout << &errorLog;
    //std::cout << std::endl;

    //if ( !success ) {
    //    throw::std::runtime_error("Error while reading shader");
    //}

    //success = _prog.link();
    //errorLog = _prog.log();
    //std::cout << "linkinkg success?: " << success << std::endl << "shader programm linking errors: ";
    //std::cout << &errorLog;
    //std::cout << std::endl;

    //_prog.bind();
    //_prog.bindAttributeLocation("position", 0);
    //_prog.bindAttributeLocation("vertexColor", 1);
    //_prog.release();

    // Light shader
    std::vector<QString> light_shader_uniforms;
    // Must be pushed in the right order ! 
    light_shader_uniforms.push_back("position");
    light_shader_uniforms.push_back("vertexColor");

    result = CreateShader(_light_shader,
        QString(path_of_shader_dir + "vertex.vsh"),
        QString(path_of_shader_dir + "fragment_light.fsh"), light_shader_uniforms);

    /*success = _light_shader.addShaderFromSourceFile(QOpenGLShader::Vertex, QString(path_of_shader_dir + "vertex.vsh"));
    errorLog = _light_shader.log();
    std::cout << "Light vertex shader sucess?: " << success << std::endl;
    std::cout << &errorLog;
    std::cout << std::endl;
    if ( !success ) {
        throw::std::runtime_error("Error while reading light vertex shader");
    }

    success = _light_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, QString(path_of_shader_dir + "fragment_light.fsh"));
    errorLog = _light_shader.log();
    std::cout << "Light fragment shader sucess?: " << success << std::endl;
    std::cout << &errorLog;
    std::cout << std::endl;
    if ( !success ) {
        throw::std::runtime_error("Error while reading light fragment shader");
    }

    success = _light_shader.link();
    errorLog = _light_shader.log();
    std::cout << "linkinkg success?: " << success << std::endl << "shader programm linking errors: ";
    std::cout << &errorLog;
    std::cout << std::endl;

    _light_shader.bind();
    _light_shader.bindAttributeLocation("position", 0);
    _light_shader.bindAttributeLocation("vertexColor", 1);
    _light_shader.release();*/

    return result;
}

bool QOpenGLPlotWidget::CreateShader(QOpenGLShaderProgram& shader, QString vertex_path, QString fragment_path, std::vector<QString>& uniforms)
{   
    bool success = false;
    success = shader.addShaderFromSourceFile(QOpenGLShader::Vertex, vertex_path);

    QString errorLog = shader.log();
    std::cout << "Vertex Shader sucess?: " << success << std::endl;
    std::cout << &errorLog;
    std::cout << std::endl;

    if ( !success ) {
        throw::std::runtime_error("Error while readingv shader");
    }

    success = shader.addShaderFromSourceFile(QOpenGLShader::Fragment, fragment_path);

    errorLog = shader.log();
    std::cout << "Fragment Shader sucess?: " << success << std::endl;
    std::cout << &errorLog;
    std::cout << std::endl;

    if ( !success ) {
        throw::std::runtime_error("Error while reading shader");
    }

    success = shader.link();
    errorLog = shader.log();
    std::cout << "linkinkg success?: " << success << std::endl << "shader programm linking errors: ";
    std::cout << &errorLog;
    std::cout << std::endl;

    shader.bind();
    //shader.bindAttributeLocation("position", 0);
    //shader.bindAttributeLocation("vertexColor", 1);

    // Set the uniforms in the order they are inside the vector. Alternative to manual uniform
    int position_idx = 0;
    for ( const auto& uniform_str : uniforms ) {
        shader.bindAttributeLocation(uniform_str, position_idx);
        ++position_idx;
    }

    shader.release();

    return success;
}



void QOpenGLPlotWidget::mouseMoveEvent(QMouseEvent* evt) 
{
    float x  = evt->x();
    float y = evt->y();
}

void QOpenGLPlotWidget::mousePressEvent(QMouseEvent* evt) 
{
    float x = evt->x();
    float y = evt->y();
}
