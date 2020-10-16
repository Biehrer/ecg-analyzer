#include "ogl_plot_renderer_widget.h"

//#define DEBUG_INFO

#ifdef DEBUG_INFO
    #define DEBUG(msg) std::cout << msg << std::endl;
#else
    #define DEBUG(msg) do{} while(0)
#endif

QOpenGLPlotRendererWidget::~QOpenGLPlotRendererWidget() 
{
    _paint_update_timer->deleteLater();
    delete _paint_update_timer;
    makeCurrent();
    FontManager_C::Clean();
}

QOpenGLPlotRendererWidget::QOpenGLPlotRendererWidget(QWidget* parent)
    :
    _prog(),
    _projection_mat(),
    _model_mat(),
    _view_mat(),
    _MVP()
{
    // Attention: Do not use OpenGL-commands inside this constructor
    // => Instead use InizializeGl()
	_nearZ = 1.0;
	_farZ = 100.0;
    _projection_mat.setToIdentity();
    _model_mat.setToIdentity();
    _view_mat.setToIdentity();
    _MVP.setToIdentity();

    _framecounter = 0;
    _paint_update_timer = new QTimer();
    connect(_paint_update_timer, SIGNAL(timeout()), this, SLOT(update()));
}

// TODO: Move this into JonesPlot_C ? -> No because when visualization is a library, we would also need this!
void
QOpenGLPlotRendererWidget::OnNewChangeRequest(int plot_id,
    const OGLPlotProperty_TP& type,
    const QVariant& value)
{
    makeCurrent();

    switch ( type ) {
    case OGLPlotProperty_TP::ID:
        _model->Data()[plot_id]->SetID((value.toInt()));
        break;

    case OGLPlotProperty_TP::LABEL:
        _model->Data()[plot_id]->SetLabel(value.toString().toStdString());
        break;

    case OGLPlotProperty_TP::TIMERANGE:
        _model->Data()[plot_id]->SetTimerangeMs(value.toDouble());
        break;

    case OGLPlotProperty_TP::YMAX:
        _model->Data()[plot_id]->SetMaxValueYAxes(value.toDouble());
        break;

    case OGLPlotProperty_TP::YMIN:
        _model->Data()[plot_id]->SetMinValueYAxes(value.toDouble());
        break;

    case OGLPlotProperty_TP::MAJOR_TICK_X:
        _model->Data()[plot_id]->SetMajorTickValueXAxes(value.toDouble());
        break;

    case OGLPlotProperty_TP::MAJOR_TICK_Y:
        _model->Data()[plot_id]->SetMajorTickValueYAxes(value.toDouble());
        break;

    case OGLPlotProperty_TP::MINOR_TICK_X:
        _model->Data()[plot_id]->SetMinorTickValueXAxes(value.toDouble());
        break;

    case OGLPlotProperty_TP::MINOR_TICK_Y:
        _model->Data()[plot_id]->SetMinorTickValueYAxes(value.toDouble());
        break;
    }
}


const 
QMatrix4x4 
QOpenGLPlotRendererWidget::GetModelViewProjection() const
{
    return _MVP;
}


bool 
QOpenGLPlotRendererWidget::IsOpenGLInitialized() const
{
    return _ogl_initialized;
}


void 
QOpenGLPlotRendererWidget::SetPlotModel(PlotModel_C* model) 
{
    _model = model;
}

void 
QOpenGLPlotRendererWidget::StartPaint()
{
    _paint_update_timer->setInterval(30);
    _paint_update_timer->start();
}

void 
QOpenGLPlotRendererWidget::StopPaint()
{
    _paint_update_timer->stop();
}

void 
QOpenGLPlotRendererWidget::initializeGL()
{
    std::cout << "Start OpenGlPlotter by Jonas Biehrer" << std::endl;
    InitializeGLParameters();
    InitializeShaderProgramms();
    FontManager_C::AddFont(Font2D_TP::ARIAL);
    _ogl_initialized = true;
}


// This window is never resized. only JonesPlot.h is resized (the window this widget is placed in).
// If this event should be triggered, it needs to be passed to this widget from JonesPlot
void 
QOpenGLPlotRendererWidget::resizeGL(int width, int height)
{
    _projection_mat.setToIdentity();
    _view_mat.setToIdentity();
    _projection_mat.ortho(QRect(0, 0, width,height));
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glViewport(0, 0, width, height);
	this->update();
    _MVP = _projection_mat * _view_mat * _model_mat;
    // Send the new model view projection to the charts for correct text rendering 
    for ( auto& plot : _model->Data()) {
        plot->SetModelViewProjection(_MVP);
    }
}

void 
QOpenGLPlotRendererWidget::paintGL()
{
    //Timer_C timer("paintGL loop");
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClearColor(0.0f, 0.0f, 0.0f, 0.8f);
	f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ++_framecounter;
    _MVP = _projection_mat * _view_mat * _model_mat;

    //_prog.bind();
    //_prog.setUniformValue("u_MVP", *_MVP);
    //_prog.setUniformValue("point_scale", 2.0f);l
    //_prog.setUniformValue("u_Color", QVector3D(1.0f, 1.0f, 1.0f));
    _light_shader.bind();
    _light_shader.setUniformValue("u_MVP", _MVP);
    _light_shader.setUniformValue("point_scale", 2.0f);
    _light_shader.setUniformValue("u_object_color", QVector3D(1.0f, 1.0f, 1.0f));
    _light_shader.setUniformValue("u_light_color", QVector3D(1.0f, 1.0f, 1.0f));

    // Draw on screen 
    for ( const auto& plot : _model->constData() ) {
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
    //timer.StopNow();
}

void 
QOpenGLPlotRendererWidget::InitializeGLParameters()
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


bool
QOpenGLPlotRendererWidget::InitializeShaderProgramms()
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
    // "bind white texture trick"
    return success;
}

bool 
QOpenGLPlotRendererWidget::CreateShader(QOpenGLShaderProgram& shader, 
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
    // Set the uniforms in the order they are positioned inside the vector
    // The loop does set the uniforms like this:
    //shader.bindAttributeLocation("position", 0);
    //shader.bindAttributeLocation("vertexColor", 1);
    //...
    int position_idx = 0;
    for ( const auto& uniform_str : uniforms ) {
        shader.bindAttributeLocation(uniform_str, position_idx);
        ++position_idx;
    }

    shader.release();
    return success;
}


