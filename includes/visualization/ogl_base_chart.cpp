#include "ogl_base_chart.h"

//#define DEBUG_INFO

#ifdef DEBUG_INFO
#define DEBUG(msg) std::cout << msg << std::endl;
#else
#define DEBUG(msg) do{} while(0)
#endif

OGLBaseChart_C::~OGLBaseChart_C() {
    _y_axis_vbo.destroy();
    _x_axis_vbo.destroy();
    _bb_vbo.destroy();
    _surface_grid_vbo.destroy();
}

void OGLBaseChart_C::SetLabel(const std::string & label){
    _label = label;
}

void OGLBaseChart_C::SetID(unsigned int id)
{
    _id = id;
}


OGLBaseChart_C::OGLBaseChart_C(const OGLChartGeometry_C& geometry,
                              const QObject& parent)
    :
    _x_axis_vbo(QOpenGLBuffer::VertexBuffer),
    _y_axis_vbo(QOpenGLBuffer::VertexBuffer),
    _bb_vbo(QOpenGLBuffer::VertexBuffer),
    _surface_grid_vbo(QOpenGLBuffer::VertexBuffer),
    _bounding_box(geometry),
    _plot_area(geometry),
    _parent_widget(parent)
{
    // Set the plot area just a little bit smaller than the bounding box
    // must not exceed 40 % of the bounding box height or width or calculations will fail 
    PositionType_TP offset = 30; 

    auto left_btm = _bounding_box.GetLeftBottom();
    _plot_area.SetLeftBottom(left_btm + Position3D_TC<PositionType_TP>(offset, offset, 0));

    auto left_top = _bounding_box.GetLeftTop();
    _plot_area.SetLeftTop(left_top + Position3D_TC<PositionType_TP>(offset, -offset, 0));

    auto right_btm = _bounding_box.GetRightBottom();
    _plot_area.SetRightBottom(right_btm + Position3D_TC<PositionType_TP>(-offset, offset, 0));

    auto right_top = _bounding_box.GetRightTop();
    _plot_area.SetRightTop(right_top + Position3D_TC<PositionType_TP>(-offset, -offset, 0));
}


void OGLBaseChart_C::InitializeAxesDescription(const QVector<float>& horizontal_grid_vertices,
                                               const QVector<float>& vertical_grid_vertices,
                                               float scale, int time_range_ms, float max_y_val,
                                               float maj_tick_x, float maj_tick_y)
{
    // /2 -> to get the number of half the vertices (Point FROM, not point TO, because they are always equal )
    // /3 -> to get number of text fields 
    int num_of_horizontal_desc = horizontal_grid_vertices.size() / 2 / 3;
    int num_of_vertical_desc = vertical_grid_vertices.size() / 2 / 3;
    int num_of_descriptions = num_of_horizontal_desc + num_of_vertical_desc;

    _plot_axes.reserve(num_of_descriptions);
    _plot_axes.resize(num_of_descriptions);
    for ( auto& description : _plot_axes ) {
        description.Initialize(Font2D_TP::ARIAL);
    }

    // horizontal changing descriptions (y-axes)
    auto plot_axes_horizontal_end = _plot_axes.begin() + num_of_horizontal_desc;
    // fine adjustment of text positioning via offet_x_S and offset_y_S:
    int offset_x_S = 3;
    int offset_y_S = -10;
    int vec_offset_idx = 1;
    auto current_y_description = max_y_val;

    for ( auto plot_desc_y_it = _plot_axes.begin(); plot_desc_y_it != plot_axes_horizontal_end; ++plot_desc_y_it ) {
       
        // for the first half: 
        // Start with the first
        int pos_x = _bounding_box.GetLeftBottom()._x + offset_x_S;
        int pos_y = horizontal_grid_vertices.at(vec_offset_idx) + offset_y_S;
        std::string text = std::to_string(static_cast<int>(current_y_description)) + " mV";

        plot_desc_y_it->SetText(text, pos_x, pos_y, scale);

        current_y_description -= maj_tick_y;
        vec_offset_idx += 6;
    }

    // vertical changing descriptions (x-axes)
    offset_x_S = 0;
    offset_y_S = 8;
    vec_offset_idx = 0;
    auto current_x_description = time_range_ms;

    for ( auto plot_desc_x_it = plot_axes_horizontal_end; plot_desc_x_it != _plot_axes.end(); ++plot_desc_x_it ) {
        int pos_x = vertical_grid_vertices.at(vec_offset_idx) + offset_x_S;
        int pos_y = _plot_area.GetLeftTop()._y + offset_y_S;
        std::string text = std::to_string(static_cast<int>(current_x_description)) + " ms";

        plot_desc_x_it->SetText(text, pos_x, pos_y, scale);

        current_x_description -= maj_tick_x;
        vec_offset_idx += 6;
    }
}

void OGLBaseChart_C::SetAxesColor(const QVector3D& color)
{
    _axes_color = color;
}


void OGLBaseChart_C::SetTextColor(const QVector3D& color)
{
    _text_color = color;
}


void OGLBaseChart_C::SetSeriesColor(const QVector3D& color)
{
    _series_color = color;
}


void OGLBaseChart_C::SetBoundingBoxColor(const QVector3D& color)
{
    _bounding_box_color = color;
}


void OGLBaseChart_C::SetSurfaceGridColor(const QVector3D& color)
{
    _surface_grid_color = color;
}


void OGLBaseChart_C::SetModelViewProjection(QMatrix4x4 model_view_projection)
{
    _chart_mvp = model_view_projection;
}


void OGLBaseChart_C::SetupAxes()
{
    const auto axes_vertices = ChartShapes_C<float>::MakesAxesVertices(_plot_area, 5.0); 
    auto& x_axis_vertices = axes_vertices._x_axis_vertices;
    auto& y_axis_vertices = axes_vertices._y_axis_vertices;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    
    _x_axis_vbo.create();
    _x_axis_vbo.bind();
    f->glEnableVertexAttribArray(0);
    // 3 positions for x and y and z data coordinates
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _x_axis_vbo.allocate(x_axis_vertices.constData(), x_axis_vertices.size() * static_cast<int>(sizeof(float)));
    _x_axis_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    f->glDisableVertexAttribArray(0);
    _x_axis_vbo.release();

    _y_axis_vbo.create();
    _y_axis_vbo.bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _y_axis_vbo.allocate(y_axis_vertices.constData(), y_axis_vertices.size() * static_cast<int>(sizeof(float)));
    _y_axis_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    f->glDisableVertexAttribArray(0);
    _y_axis_vbo.release();
}

//inline
void OGLBaseChart_C::DrawSurfaceGrid(QOpenGLShaderProgram& shader)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    shader.bind();
    shader.setUniformValue("u_object_color", _surface_grid_color);
    _surface_grid_vbo.bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glDrawArrays(GL_LINES, 0, _num_of_surface_grid_positions);
    f->glDisableVertexAttribArray(0);
    _surface_grid_vbo.release();
}


std::pair<QVector<float>, QVector<float>>
OGLBaseChart_C::CreateSurfaceGrid(int x_major_tick_dist_ms, int y_major_tick_dist_unit, 
                                  int time_range_ms, float max_y, float min_y)
{
    auto surface_grid_vertices =
        ChartShapes_C<float>::CreateSurfaceGridVertices(_plot_area,
                                                        time_range_ms,
                                                        max_y,
                                                        min_y,
                                                        x_major_tick_dist_ms,
                                                        y_major_tick_dist_unit);

    auto& horizontal_grid_vertices = surface_grid_vertices.first;
    auto& vertical_grid_vertices = surface_grid_vertices.second;

    QVector<float> combined_vertices;
    int num_of_combined_verts = horizontal_grid_vertices.size() + vertical_grid_vertices.size();
    combined_vertices.reserve(num_of_combined_verts);
    for ( auto& horizontal_vert : horizontal_grid_vertices ) {
        combined_vertices.append(horizontal_vert);
    }
    for ( auto& vertical_vert : vertical_grid_vertices ) {
        combined_vertices.append(vertical_vert);
    }

    _num_of_surface_grid_positions = num_of_combined_verts / 3;

    // Create VBO
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // Setup OGL Chart buffer - empty 
    _surface_grid_vbo.create();
    _surface_grid_vbo.bind();
    f->glEnableVertexAttribArray(0);
    // 3 positions for x and y and z data coordinates
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _surface_grid_vbo.allocate(combined_vertices.constData(), combined_vertices.size() * sizeof(float));
    _surface_grid_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    f->glDisableVertexAttribArray(0);
    _surface_grid_vbo.release();

    return surface_grid_vertices;
}


const std::string & OGLBaseChart_C::GetLabel()
{
    return _label;
}

unsigned int OGLBaseChart_C::GetID()
{
    return _id;
}


//inline
void OGLBaseChart_C::DrawXYAxes(QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    shader.bind();
    shader.setUniformValue("u_object_color", _axes_color);
    //   _y_axis_vbo.bind();
    //   f->glEnableVertexAttribArray(0);
    //   f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    //   f->glDrawArrays(GL_TRIANGLES, 0, 6);
    //   f->glDisableVertexAttribArray(0);
    //   _y_axis_vbo.release();
    //   _x_axis_vbo.bind();
    // f->glEnableVertexAttribArray(0);
    // f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    // f->glDrawArrays(GL_TRIANGLES, 0, 6);
    // f->glDisableVertexAttribArray(0);
    //   _x_axis_vbo.release();

    // Draw the axes descriptions
    for ( const auto& description : _plot_axes ) {
        // => Cherno 2 in 1 shader 
        // -> approach: bind a white 1x1 texture to use flat colors and bind a 1.0 color to just draw the texture, 
        // use a texture sampler, if not, just use the uniform color !
        description.RenderText(text_shader, _text_color, _chart_mvp);
    }
}

//inline
void OGLBaseChart_C::DrawBoundingBox(QOpenGLShaderProgram& shader)
{
    auto* f = QOpenGLContext::currentContext()->functions();
    shader.bind();
    shader.setUniformValue("u_object_color", _bounding_box_color);
    _bb_vbo.bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glDrawArrays(GL_LINES, 0, 8);
    f->glDisableVertexAttribArray(0);
    _bb_vbo.release();
}

void OGLBaseChart_C::CreateBoundingBox()
{
    auto bb_vertices = ChartShapes_C<float>::MakeBoundingBoxVertices(_bounding_box);
    // Setup vbo
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    _bb_vbo.create();
    _bb_vbo.bind();
    f->glEnableVertexAttribArray(0);
    // 3 positions for x and y and z data coordinates
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _bb_vbo.allocate(bb_vertices.constData(), bb_vertices.size() * static_cast<int>(sizeof(float)));
    _bb_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    f->glDisableVertexAttribArray(0);
    _bb_vbo.release();
}
