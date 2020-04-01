#include "QOpenGLPlotWidget.h"

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
    for ( int chart_idx = _plots.size() - 1; chart_idx > 0; --chart_idx ) {
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

    _data_update_timer = new QTimer();
    connect(_data_update_timer, SIGNAL(timeout()), this, SLOT(on_dataUpdate()));
    _data_update_timer->setInterval(1);
    _data_update_timer->start(1);
}

void QOpenGLPlotWidget::dataThreadFunc()
{
    while(true){
        // duplicate for testing
        double pi_ = 3.1415026589;
        double val_in_radians = _pointcount * (2 * pi_) / 360;
        double data_value = 10 * std::sin(val_in_radians);
        double data_value_cos = 10 * std::cos(val_in_radians);

        for ( auto& plot : _plots ) {
            plot->AddDataToSeries(data_value, _pointcount);
        }

        _pointcount++;
        DEBUG("Thread added point (# " << _pointcount << "): " << data_value);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}


void QOpenGLPlotWidget::on_dataUpdate()
{
    double pi_ = 3.1415026589;
    double val_in_radians = _pointcount * (2.0 * pi_) / 360.0;
    float data_value = 10.0 * std::sin(val_in_radians);

    DEBUG("Added data value (# " << _pointcount << "): " << data_value);
    for ( auto& plot : _plots ) {
        // assume that _pointcount is an incrementing value with the unit milliseconds
        plot->AddDataToSeries(data_value, _pointcount);
    }
	_pointcount++;	
}



void QOpenGLPlotWidget::initializeGL()
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // Initialize OGL functions before any other OpenGL call
	f->initializeOpenGLFunctions();

    std::cout << "Start OpenGlPlotter by Jonas Biehrer" << std::endl;

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

    InitializeShaderProgramms();

    int number_of_plots = 2;

    ++number_of_plots;
    int screenwidth_fraction = SREENWIDTH / 6;
    int chart_width = SREENWIDTH - screenwidth_fraction;
    int chart_height = SCREENHEIGHT / number_of_plots;

    int max_point_count = 10000;

    // Chart is aligned at the right side of the screen
    int chart_pos_x = 0;

    int chart_to_chart_offset_S = 10;

    for ( int chart_idx = 1; chart_idx < number_of_plots; ++chart_idx) {

        //if ( chart_idx % 2 == 0 ) {
        //    chart_to_chart_offset_S = 30;
        //}
        //else {
        //    chart_to_chart_offset_S = 0;
        //}

        int chart_pos_y = chart_height * chart_idx + chart_to_chart_offset_S;
        _plots.push_back(new OGLChart_C(max_point_count, chart_pos_x, chart_pos_y, chart_width, chart_height));
    }
    _paint_update_timer->start();
}

void QOpenGLPlotWidget::resizeGL(int width, int height){
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    _projection_mat->setToIdentity();
    _view_mat->setToIdentity();

    // This window is never resized. only JonesPlot.h is resized.
    // If this event should be triggered, it needs to be passed to this widget.
    _projection_mat->ortho(QRect(0, 0, this->width(),this->height()));

	f->glViewport(0, 0, this->width(), this->height());
	this->update();
}


bool QOpenGLPlotWidget::InitializeShaderProgramms()
{
    std::cout << std::endl;
    std::cout << "Shader Compiling Error Log:" << std::endl;
	std::cout << "Standard Shader error log: ";
    std::cout << std::endl;

    QString path_of_executable(QDir::currentPath());
    std::cout << "expected filepath to shaders (make sure it exists): "
              << path_of_executable.toStdString() << std::endl;

	bool success = false;
    success = _prog.addShaderFromSourceFile(QOpenGLShader::Vertex, QString(path_of_executable + "//Resources//shaders//vertex.vsh"));

    QString errorLog = _prog.log();
    std::cout << "Vertex Shader sucess?: " << success << std::endl;
    std::cout << &errorLog;
    std::cout << std::endl;

    if( !success ){
        throw::std::runtime_error("Error while readingv shader");
    }

    success = _prog.addShaderFromSourceFile(QOpenGLShader::Fragment, QString(path_of_executable + "//Resources//shaders//fragment.fsh"));

    errorLog = _prog.log();
    std::cout << "Fragment Shader sucess?: " << success << std::endl;
    std::cout << &errorLog;
    std::cout << std::endl;

    if( !success ){
        throw::std::runtime_error("Error while reading shader");
    }

    success = _prog.link();
    errorLog = _prog.log();
    std::cout <<"linkinkg success?: "<< success << std::endl << "shader programm linking errors: ";
	std::cout << &errorLog;
    std::cout << std::endl;

    _prog.bind();
    _prog.bindAttributeLocation("position", 0);
    _prog.bindAttributeLocation("vertexColor", 1);
    _prog.release();

	return success;
}


void QOpenGLPlotWidget::paintGL(){
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glClearColor(1.0f, 0.0f, 0.0f, 0.5f);
	f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ++_framecounter;

    *_MVP = *(_projection_mat) * *(_view_mat) * *(_model_mat);

    _prog.bind();

    _prog.setUniformValue("u_MVP", *_MVP);
    _prog.setUniformValue("point_scale", 2.0f);
    _prog.setUniformValue("u_Color", QVector3D(1.0f, 1.0f, 1.0f));

    for ( const auto& plot : _plots ) {
        plot->Draw();
    }

    _prog.release();
}


void QOpenGLPlotWidget::mouseMoveEvent(QMouseEvent* evt) {
    float x  = evt->x();
    float y = evt->y();
}


void QOpenGLPlotWidget::mousePressEvent(QMouseEvent* evt) {
    float x = evt->x();
    float y = evt->y();
}
