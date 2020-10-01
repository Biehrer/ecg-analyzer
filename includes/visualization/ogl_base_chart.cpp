#include "ogl_base_chart.h"

//#define DEBUG_INFO

#ifdef DEBUG_INFO
#define DEBUG(msg) std::cout << msg << std::endl;
#else
#define DEBUG(msg) do{} while(0)
#endif

OGLBaseChart_C::~OGLBaseChart_C()
{
    _bb_vbo.destroy();
    _surface_grid_vbo.destroy();
}

OGLBaseChart_C::OGLBaseChart_C(const OGLChartGeometry_C& geometry,
                              const QObject* parent)
    :
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

void OGLBaseChart_C::InitializeAxesDescription(const QVector<float>& horizontal_grid_line_vertices,
                                               const QVector<float>& vertical_grid_line_vertices,
                                               float scale, 
                                               int time_range_ms, 
                                               double max_y_val,
                                               float maj_tick_x, 
                                               float maj_tick_y) // TODO: Pass struct with all of this data ! Too much arguments
{
    // /2 -> to get the number of half the vertices (Point FROM, not point TO, because they are always equal )
    // /3 -> to get number of text fields 
    int num_y_textboxes = horizontal_grid_line_vertices.size() / 2 / 3; // y axes textboxes
    int num_x_textboxes = vertical_grid_line_vertices.size() / 2 / 3; // x axes textboxes

    int num_of_textboxes = num_y_textboxes + num_x_textboxes;

    _plot_axes.clear(); 
    for ( int num_axes = 0; num_axes < num_of_textboxes; ++num_axes ) {
        _plot_axes.emplace_back();
    }

    // horizontal changing descriptions (y-axes):
    // Vars for fine adjustment of text positioning(via offet_x_S and offset_y_S)
    int offset_x_S = 3;
    int offset_y_S = -10;
    int vec_offset_idx = 1;

    auto current_y_description = max_y_val;
    auto y_textboxes_it_end = _plot_axes.begin() + num_y_textboxes;
    for ( auto y_textbox_it = _plot_axes.begin(); y_textbox_it != y_textboxes_it_end; ++y_textbox_it ) {
        float pos_x = _bounding_box.GetLeftBottom()._x + offset_x_S;
        float pos_y = horizontal_grid_line_vertices.at(vec_offset_idx) + offset_y_S;
        std::string text = std::to_string(static_cast<double>(current_y_description)) + " mV";
        y_textbox_it->SetText(text, pos_x, pos_y, scale, Font2D_TP::ARIAL);
        current_y_description -= maj_tick_y;
        vec_offset_idx += 6;
    }

    // Vertical changing descriptions (x-axes):
    // offset to manually shift the textbox on the x-axis
    offset_x_S = 0;
    // offset to manually shift the textbox on the y-axis
    offset_y_S = 8;
    vec_offset_idx = 0;
    auto current_x_description = time_range_ms;
    for ( auto x_textbox_it = y_textboxes_it_end; x_textbox_it != _plot_axes.end(); ++x_textbox_it ) {
        float pos_x = vertical_grid_line_vertices.at(vec_offset_idx) + offset_x_S;
        float pos_y = _plot_area.GetLeftTop()._y + offset_y_S;
        std::string text = std::to_string(static_cast<int>(current_x_description)) + " ms";
        x_textbox_it->SetText(text, pos_x, pos_y, scale, Font2D_TP::ARIAL);

        current_x_description -= maj_tick_x;
        vec_offset_idx += 6;
    }

}


//inline
void 
OGLBaseChart_C::DrawSurfaceGrid(QOpenGLShaderProgram& shader)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    shader.bind();
    shader.setUniformValue("u_object_color", _surface_grid_color);
    _surface_grid_vbo.bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glDrawArrays(GL_LINES, 0, _num_of_grid_positions);
    f->glDisableVertexAttribArray(0);
    _surface_grid_vbo.release();

}

std::pair<QVector<float>, QVector<float>>
OGLBaseChart_C::CreateSurfaceGrid(int x_major_tick_dist_ms, 
                                  float y_major_tick_dist_unit,
                                  float x_minor_tick_dist_ms,
                                  float y_minor_tick_dist_unit,
                                  int time_range_ms, 
                                  float max_y, 
                                  float min_y)
{
    /////////////////////////////
    // Major Tick vertices
    /////////////////////////////

    // These are also used for the axes text labels
    auto grid_maj_tick_verts =
        ChartShapes_C<float>::CreateMajTickSurfaceGridVertices(_plot_area,
                                                            time_range_ms,
                                                            max_y, 
                                                            min_y,
                                                            x_major_tick_dist_ms,
                                                            y_major_tick_dist_unit);

    const auto& horizontal_maj_grid_vertices = grid_maj_tick_verts.first;
    const auto& vertical_maj_grid_vertices = grid_maj_tick_verts.second;
    /////////////////////////////
    // Minor Tick vertices
    ///////////////////////////// 
    auto grid_minor_tick_verts =
        ChartShapes_C<float>::CreateMinorTickSurfaceGridVertices(_plot_area,
            time_range_ms,
            max_y,
            min_y,
            x_minor_tick_dist_ms,
            y_minor_tick_dist_unit);

    const auto& horizontal_minor_grid_vertices = grid_minor_tick_verts.first;
    const auto& vertical_minor_grid_vertices = grid_minor_tick_verts.second;

    // Create one vector of all vertices
    QVector<float> combined_grid_verts;
    int num_of_combined_grid_verts = horizontal_maj_grid_vertices.size() + vertical_maj_grid_vertices.size() +
                                     horizontal_minor_grid_vertices.size() + vertical_minor_grid_vertices.size();

    combined_grid_verts.reserve(num_of_combined_grid_verts); 

    for ( auto& horizontal_maj_vert : horizontal_maj_grid_vertices ) {
        combined_grid_verts.append(horizontal_maj_vert);
    }
    for ( auto& vertical_maj_vert : vertical_maj_grid_vertices ) {
        combined_grid_verts.append(vertical_maj_vert);
    }

    for ( auto& horizontal_min_vert : horizontal_minor_grid_vertices ) {
        combined_grid_verts.append(horizontal_min_vert);
    }
    for ( auto& vertical_min_vert : vertical_minor_grid_vertices ) {
        combined_grid_verts.append(vertical_min_vert);
    }

    _num_of_grid_positions = num_of_combined_grid_verts / 3;

    // Destroy the old buffer
    _surface_grid_vbo.destroy();
    // Setup OGL Chart buffer - empty 
    _surface_grid_vbo.create();
    _surface_grid_vbo.bind();
    // Create VBO
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glEnableVertexAttribArray(0);
    // 3 positions for x and y and z data coordinates
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _surface_grid_vbo.allocate(combined_grid_verts.constData(), combined_grid_verts.size() * sizeof(float));
    _surface_grid_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    f->glDisableVertexAttribArray(0);
    _surface_grid_vbo.release();

    // Return maj tick vertices to for the plot labels
    return grid_maj_tick_verts;
}


//inline
void 
OGLBaseChart_C::DrawTextLabels(QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    shader.bind();
    shader.setUniformValue("u_object_color", _axes_color);

    // Draw the plot label
    _plot_label.RenderText(text_shader, _text_color, _chart_mvp);

    // Draw the axes descriptions
    for ( /*const*/ auto& description : _plot_axes ) {
        // => Cherno 2 in 1 shader 
        // -> approach: bind a white 1x1 texture to use flat colors and bind a 1.0 color to just draw the texture, 
        // use a texture sampler, if not, just use the uniform color !
        description.RenderText(text_shader, _text_color, _chart_mvp);
    }
}

//inline
void 
OGLBaseChart_C::DrawBoundingBox(QOpenGLShaderProgram& shader)
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

void 
OGLBaseChart_C::CreateBoundingBox()
{
    auto bb_vertices = ChartShapes_C<float>::MakeBoundingBoxVertices(_bounding_box);
    // Setup vbo
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    _bb_vbo.create();
    _bb_vbo.bind();
    f->glEnableVertexAttribArray(0);
    // 3 positions for x and y and z data coordinates
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _bb_vbo.allocate(bb_vertices.constData(), bb_vertices.size() * sizeof(float));
    _bb_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    f->glDisableVertexAttribArray(0);
    _bb_vbo.release();
}



const 
std::string&
OGLBaseChart_C::GetLabel()
{
    return _label;
}

unsigned int 
OGLBaseChart_C::GetID()
{
    return _id;
}

void 
OGLBaseChart_C::SetLabel(const std::string & label)
{
    _label = label;
    _plot_label.SetText(label,
                        _bounding_box.GetLeftBottom()._x,
                        _bounding_box.GetLeftBottom()._y,
                        0.35f,
                        Font2D_TP::ARIAL);

}

void 
OGLBaseChart_C::SetID(unsigned int id)
{
    _id = id;
}

void 
OGLBaseChart_C::SetAxesColor(const QVector3D& color)
{
    _axes_color = color;
}


void 
OGLBaseChart_C::SetTextColor(const QVector3D& color)
{
    _text_color = color;
}


void 
OGLBaseChart_C::SetSeriesColor(const QVector3D& color)
{
    _series_color = color;
}


void 
OGLBaseChart_C::SetBoundingBoxColor(const QVector3D& color)
{
    _bounding_box_color = color;
}


void 
OGLBaseChart_C::SetSurfaceGridColor(const QVector3D& color)
{
    _surface_grid_color = color;
}

void 
OGLBaseChart_C::SetFiducialMarkColor(const QVector3D & color)
{
    _fiducial_marks_color = color;
}


void
OGLBaseChart_C::SetModelViewProjection(const QMatrix4x4& model_view_projection)
{
    _chart_mvp = model_view_projection;
}

const OGLChartGeometry_C&
OGLBaseChart_C::GetBoundingBox()
{
    return _bounding_box;
}

const OGLChartGeometry_C&
OGLBaseChart_C::GetPlotSurfaceArea()
{
    return _plot_area;
}
