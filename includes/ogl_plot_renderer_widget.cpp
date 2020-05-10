#include "includes/ogl_plot_renderer_widget.h"

//#define DEBUG_INFO

#ifdef DEBUG_INFO
    #define DEBUG(msg) std::cout << msg << std::endl;
#else
    #define DEBUG(msg) do{} while(0)
#endif


QOpenGLPlotRendererWidget::~QOpenGLPlotRendererWidget() {
    delete _projection_mat;
    delete _model_mat;
    delete _view_mat;
    delete _MVP;
	
    _paint_update_timer->deleteLater();
    delete _paint_update_timer;

    // Delete plots
    for ( int chart_idx = _plots.size() - 1; chart_idx >= 0; --chart_idx ) {
        delete _plots[chart_idx];
    }
}

QOpenGLPlotRendererWidget::QOpenGLPlotRendererWidget(QWidget* parent)
    :
      _prog()
{
    // Attention: DO NOT USE OPENGL COMMANDS INSIDE THE CONSTRUCTOR
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
}

void QOpenGLPlotRendererWidget::OnDataUpdateThreadFunction()
{
    Timestamp_TP timestamp;
    double pi = 3.1415026589; 
    double value_rad = 0;
    double data_value = 0;
   
    while(true)
    {
        timestamp.Now();
        value_rad = _pointcount * (2.0 * pi) / 360.0;
        data_value = 5.0 * std::sin(value_rad);
        for ( auto& plot : _plots ) {
            plot->AddDataTimestamp(data_value, timestamp);
        }
        ++_pointcount;
        //DEBUG("Thread added point (# " << _pointcount << "): " << data_value);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
}

void QOpenGLPlotRendererWidget::AddDataToAllPlots(float value_x, float value_y) 
{
    for ( auto& plot : _plots ) {
        plot->AddDataTimestamp(value_y, Timestamp_TP(value_x));
    }
}

const QMatrix4x4 QOpenGLPlotRendererWidget::GetModelViewProjection() const
{
    return *_MVP;
}


void QOpenGLPlotRendererWidget::InitializePlots(int number_of_plots) 
{
    DEBUG("initialize plots");

    // Chart properties
    int chart_buffer_size = 10000;
    int time_range_ms = 10000;
    float max_y_axis_value = 10.0f;
    float min_y_axis_value = -10.0f;

    // Calculate position of the charts
    int offset = SREENWIDTH / 6;
    int chart_width = SREENWIDTH - offset;
    int chart_height = SCREENHEIGHT / number_of_plots;
    // Chart is aligned at the left side of the screen
    int chart_pos_x = 0;

    int chart_to_chart_offset_S = 10;
    int chart_offset_from_origin_S = 4;

    // Create plots
    for ( int chart_idx = 0; chart_idx < number_of_plots; ++chart_idx ) {
        int chart_pos_y = (chart_height + chart_to_chart_offset_S) * chart_idx + chart_offset_from_origin_S; 
        OGLChartGeometry_C geometry(chart_pos_x, chart_pos_y, chart_width, chart_height);
        _plots.push_back( new OGLSweepChart_C(time_range_ms, chart_buffer_size, max_y_axis_value, min_y_axis_value, geometry, *this) );
        std::cout << "chart pos (idx=" << chart_idx << "): " << chart_pos_y << std::endl;
    }

    QVector3D series_color(0.0f, 1.0f, 0.0f);
    QVector3D axes_color(1.0f, 1.0f, 1.0f);
    QVector3D lead_line_color(1.0f, 0.01f, 0.0f);
    QVector3D surface_grid_color(static_cast<float>(235.0f/255.0f), 
                                 static_cast<float>(225.0f/255.0f), 
                                 static_cast<float>(27.0f/255.0f) );
    QVector3D bounding_box_color(1.0f, 1.0f, 1.0f);
    QVector3D text_color(1.0f, 1.0f, 1.0f);

    for ( auto& plot : _plots ) {
        // Setup colors
        plot->SetSeriesColor(series_color);
        plot->SetAxesColor(axes_color);
        plot->SetTextColor(text_color);
        plot->SetBoundingBoxColor(bounding_box_color);
        plot->SetLeadLineColor(lead_line_color);
        plot->SetSurfaceGridColor(surface_grid_color);
        // Set up axes
        plot->SetMajorTickValueXAxes(1000.0);
        plot->SetMajorTickValueYAxes(5.0);

        // Initialize
        plot->Initialize();
    }

    this->update();
}


void QOpenGLPlotRendererWidget::initializeGL()
{
    std::cout << "Start OpenGlPlotter by Jonas Biehrer" << std::endl;

    InitializeGLParameters();

    InitializeShaderProgramms();

    CreateLightSource();

    InitializePlots(1);

    _paint_update_timer->start();
}


// This window is never resized. only JonesPlot.h is resized (the window this widget is placed in).
// If this event should be triggered, it needs to be passed to this widget from JonesPlot
void QOpenGLPlotRendererWidget::resizeGL(int width, int height)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    _projection_mat->setToIdentity();
    _view_mat->setToIdentity();
    _projection_mat->ortho(QRect(0, 0, this->width(),this->height()));
	f->glViewport(0, 0, this->width(), this->height());
	this->update();

    // Alternative: Trigger a signal which is activated, when the viewport is resized
    // =>..Check whats better for performance
    *_MVP = *(_projection_mat) * *(_view_mat) * *(_model_mat);
    // Send the new model view projection to the charts for correct text rendering 
    for ( auto& plot : _plots ) {
        plot->SetModelViewProjection(*_MVP);
    }
}

void QOpenGLPlotRendererWidget::paintGL()
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
    _light_shader.setUniformValue("u_light_color", QVector3D(1.0f, 1.0f, 1.0f));

    for ( const auto& plot : _plots ) {
        plot->Draw(_light_shader, _text_shader);
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

void QOpenGLPlotRendererWidget::InitializeGLParameters()
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

void QOpenGLPlotRendererWidget::CreateLightSource()
{
    float light_source_x_pos = 0.0f;
    float light_source_y_pos = 0.0f;
    float light_source_z_pos = -1.5f;
    float cube_size = this->width();

    ShapeGenerator_C shape_gen;
    auto light_source_vertices = shape_gen.makeQuadAtPos_(light_source_x_pos, 
                                                          light_source_y_pos, 
                                                          light_source_z_pos, 
                                                          cube_size);
    _light_source.CreateVBO(light_source_vertices);
}

bool QOpenGLPlotRendererWidget::InitializeShaderProgramms()
{
    std::cout << std::endl << "Shader Compiling Error Log:" << std::endl
     << "Standard Shader error log: " << std::endl;

    QString path_of_executable( QDir::currentPath() );
    std::cout << "expected filepath to shaders (make sure it exists): "
        << path_of_executable.toStdString() << std::endl;

    QString path_of_shader_dir = path_of_executable + "//Resources//shaders//";

    // Standard color shader
    std::vector<QString> std_shader_uniforms;
    std_shader_uniforms.push_back("position");
    std_shader_uniforms.push_back("vertexColor");
    bool success = CreateShader(_prog,
                                QString(path_of_shader_dir + "vertex.vsh"),
                                QString(path_of_shader_dir + "fragment.fsh"),
                                std_shader_uniforms);

    // Light shader
    std::vector<QString> light_shader_uniforms;
    // Must be pushed in the right order ! 
    light_shader_uniforms.push_back("position");
    light_shader_uniforms.push_back("vertexColor");
    success = CreateShader(_light_shader,
        QString(path_of_shader_dir + "vertex.vsh"),
        QString(path_of_shader_dir + "fragment_light.fsh"),
        light_shader_uniforms);

    // 2D Light shader 
    std::vector<QString> light_2d_shader_uniforms;
    // Must be pushed in the right order ! 
    light_2d_shader_uniforms.push_back("position");
    light_2d_shader_uniforms.push_back("vertexColor");
    success = CreateShader(_light_2d_shader,
        QString(path_of_shader_dir + "vertex_light_2d.vsh"),
        QString(path_of_shader_dir + "fragment_light.fsh"),
        light_2d_shader_uniforms);

    // Text shader
    std::vector<QString> text_shader_uniforms;
    // Must be pushed in the right order ! 
    text_shader_uniforms.push_back("vertex");

    success = CreateShader(_text_shader,
        QString(path_of_shader_dir + "vertex_text.vsh"),
        QString(path_of_shader_dir + "fragment_text.fsh"), 
        text_shader_uniforms);

    // ToDo: 
    // Two in One shader for text shading (requires texture sampler 2D) and standard color shading

    return success;
}

bool QOpenGLPlotRendererWidget::CreateShader(QOpenGLShaderProgram& shader, 
                                     QString& vertex_path, 
                                     QString& fragment_path,
                                     std::vector<QString>& uniforms)
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
    // Set the uniforms in the order they are positioned inside the vector
    int position_idx = 0;
    for ( const auto& uniform_str : uniforms ) {
        shader.bindAttributeLocation(uniform_str, position_idx);
        ++position_idx;
    }

    shader.release();
    return success;
}



void QOpenGLPlotRendererWidget::mouseMoveEvent(QMouseEvent* evt) 
{
    float x  = evt->x();
    float y = evt->y();
}

void QOpenGLPlotRendererWidget::mousePressEvent(QMouseEvent* evt) 
{
    float x = evt->x();
    float y = evt->y();
}