#include "ogl_base_chart.h"

//#define DEBUG_INFO

#ifdef DEBUG_INFO
#define DEBUG(msg) std::cout << msg << std::endl;
#else
#define DEBUG(msg) do{} while(0)
#endif

OGLBaseChart_C::~OGLBaseChart_C()
{
    std::cout << "Destruct BaseChart_C " << std::endl;
    _y_axis_vbo.destroy();
    _x_axis_vbo.destroy();
    _bb_vbo.destroy();
    _surface_grid_vbo.destroy();
    delete _mutex;
}

OGLBaseChart_C::OGLBaseChart_C(const OGLChartGeometry_C& geometry,
                              const QObject* parent)
    :
    _x_axis_vbo(QOpenGLBuffer::VertexBuffer),
    _y_axis_vbo(QOpenGLBuffer::VertexBuffer),
    _bb_vbo(QOpenGLBuffer::VertexBuffer),
    _surface_grid_vbo(QOpenGLBuffer::VertexBuffer),
    _bounding_box(geometry),
    _plot_area(geometry),
    _parent_widget(parent)
{
    _mutex = new std::mutex();
    std::cout << "Construct BaseChart_C " << std::endl;
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
//
//OGLBaseChart_C::OGLBaseChart_C(OGLBaseChart_C && other)
//{ // move initialization
//    //std::lock_guard<std::mutex> lock(other.mtx);
//    //    value = std::move(other.value);
//    //    other.value = 0; 
//    _x_axis_vbo = std::move(other._x_axis_vbo);
//    other._x_axis_vbo = {};
//    _y_axis_vbo = std::move(other._y_axis_vbo);
//    other._y_axis_vbo = {};
//    _bb_vbo = std::move(other._bb_vbo);
//    other._bb_vbo = {};
//    _surface_grid_vbo = std::move(other._surface_grid_vbo);
//    other._surface_grid_vbo = {};
//    _num_of_surface_grid_positions = std::move(other._num_of_surface_grid_positions);
//    other._num_of_surface_grid_positions = 0;
//    _bounding_box = std::move(other._bounding_box);
//    other._bounding_box = {};
//    _plot_area = std::move(other._plot_area);
//    other._plot_area = {};
//    _lead_line_color = std::move(other._lead_line_color);
//    other._lead_line_color = {};
//    _series_color = std::move(other._series_color);
//    other._series_color = {};
//    _fiducial_marks_color = std::move(other._fiducial_marks_color);
//    other._fiducial_marks_color = {};
//    _bounding_box_color = std::move(other._bounding_box_color);
//    other._bounding_box_color = {};
//    _surface_grid_color = std::move(other._surface_grid_color);
//    other._surface_grid_color = {};
//    _axes_color = std::move(other._axes_color);
//    other._axes_color = {};
//    _text_color = std::move(other._text_color);
//    other._text_color = {};
//    _plot_axes = std::move(other._plot_axes);
//    other._plot_axes = {};
//    _chart_mvp = std::move(other._chart_mvp);
//    other._chart_mvp = {};
//    _parent_widget = std::move(other._parent_widget);
//    other._parent_widget = 0;
//    _label = std::move(other._label);
//    other._label = "";
//    _id = std::move(other._id);
//    other._id = -1;
//    _mutex = std::move(other._mutex);
//    other._mutex = 0;
//}
//
//
//
//OGLBaseChart_C & OGLBaseChart_C::operator=(OGLBaseChart_C && other)
//{ // move assignment
//    //std::lock(mtx, other.mtx);
//    //std::lock_guard<std::mutex> self_lock(mtx, std::adopt_lock);
//    //std::lock_guard<std::mutex> other_lock(other.mtx, std::adopt_lock);
//    //value = std::move(other.value);
//    //other.value = 0;
//
//    _x_axis_vbo = std::move(other._x_axis_vbo);
//    other._x_axis_vbo = {};
//    _y_axis_vbo = std::move(other._y_axis_vbo);
//    other._y_axis_vbo = {};
//    _bb_vbo = std::move(other._bb_vbo);
//    other._bb_vbo = {};
//    _surface_grid_vbo = std::move(other._surface_grid_vbo);
//    other._surface_grid_vbo = {};
//    _num_of_surface_grid_positions = std::move(other._num_of_surface_grid_positions);
//    other._num_of_surface_grid_positions = 0;
//    _bounding_box = std::move(other._bounding_box);
//    other._bounding_box = {};
//    _plot_area = std::move(other._plot_area);
//    other._plot_area = {};
//    _lead_line_color = std::move(other._lead_line_color);
//    other._lead_line_color = {};
//    _series_color = std::move(other._series_color);
//    other._series_color = {};
//    _fiducial_marks_color = std::move(other._fiducial_marks_color);
//    other._fiducial_marks_color = {};
//    _bounding_box_color = std::move(other._bounding_box_color);
//    other._bounding_box_color = {};
//    _surface_grid_color = std::move(other._surface_grid_color);
//    other._surface_grid_color = {};
//    _axes_color = std::move(other._axes_color);
//    other._axes_color = {};
//    _text_color = std::move(other._text_color);
//    other._text_color = {};
//    _plot_axes = std::move(other._plot_axes);
//    other._plot_axes = {};
//    _chart_mvp = std::move(other._chart_mvp);
//    other._chart_mvp = {};
//    _parent_widget = std::move(other._parent_widget);
//    other._parent_widget = 0;
//    _label = std::move(other._label);
//    other._label = "";
//    _id = std::move(other._id);
//    other._id = -1;
//    _mutex = std::move(other._mutex);
//    other._mutex = 0;
//    return *this;
//}

//
//OGLBaseChart_C::OGLBaseChart_C(const OGLBaseChart_C & other)
//{ // Copy initialization
//    //std::lock_guard<std::mutex> lock(other.mtx);
//    //value = other.value;
//    _x_axis_vbo = other._x_axis_vbo;
//    _y_axis_vbo = other._y_axis_vbo;
//    _bb_vbo = other._bb_vbo;
//    _surface_grid_vbo = other._surface_grid_vbo;
//    _num_of_surface_grid_positions = other._num_of_surface_grid_positions;
//    _bounding_box = other._bounding_box;
//    _plot_area = other._plot_area;
//    _lead_line_color = other._lead_line_color;
//    _series_color = other._series_color;
//    _fiducial_marks_color = other._fiducial_marks_color;
//    _bounding_box_color = other._bounding_box_color;
//    _surface_grid_color = other._surface_grid_color;
//    _axes_color = other._axes_color;
//    _text_color = other._text_color;
//    _plot_axes = other._plot_axes;
//    _chart_mvp = other._chart_mvp;
//    _parent_widget = other._parent_widget;
//    _label = other._label;
//    _id = other._id;
//    _mutex = other._mutex;
//    _in_init.store(other._in_init.load());
//
//    //_x_axis_vbo;
//    //_y_axis_vbo;
//    //_bb_vbo;
//    //_surface_grid_vbo;
//    //_num_of_surface_grid_positions;
//    //_bounding_box;
//    //_plot_area;
//    //_lead_line_color;
//    //_series_color;
//    //_fiducial_marks_color;
//    //_bounding_box_color;
//    //_surface_grid_color;
//    //_axes_color;
//    //_text_color;
//    //_plot_axes;
//    //_chart_mvp;
//    //_parent_widget;
//    //_label = "";
//    //_id;
//    //_in_init = true;
//    //_mutex;
//}
//
//OGLBaseChart_C & OGLBaseChart_C::operator=(const OGLBaseChart_C & other)
//{ // Copy assignment
//            /*{
//            std::lock(mtx, other.mtx);
//            std::lock_guard<std::mutex> self_lock(mtx, std::adopt_lock);
//            std::lock_guard<std::mutex> other_lock(other.mtx, std::adopt_lock);
//            value = other.value;
//            return *this;
//        }*/
//    _x_axis_vbo = other._x_axis_vbo;
//    _y_axis_vbo = other._y_axis_vbo;
//    _bb_vbo = other._bb_vbo;
//    _surface_grid_vbo = other._surface_grid_vbo;
//    _num_of_surface_grid_positions = other._num_of_surface_grid_positions;
//    _bounding_box = other._bounding_box;
//    _plot_area = other._plot_area;
//    _lead_line_color = other._lead_line_color;
//    _series_color = other._series_color;
//    _fiducial_marks_color = other._fiducial_marks_color;
//    _bounding_box_color = other._bounding_box_color;
//    _surface_grid_color = other._surface_grid_color;
//    _axes_color = other._axes_color;
//    _text_color = other._text_color;
//    _plot_axes = other._plot_axes;
//    _chart_mvp = other._chart_mvp;
//    _parent_widget = other._parent_widget;
//    _label = other._label;
//    _id = other._id;
//    _mutex = other._mutex; // just also point to the same mutex the old one shows   
//    _in_init.store(other._in_init.load());
//    return *this;
//}


void OGLBaseChart_C::InitializeAxesDescription(const QVector<float>& horizontal_grid_line_vertices,
                                               const QVector<float>& vertical_grid_line_vertices,
                                               float scale, 
                                               int time_range_ms, 
                                               double max_y_val,
                                               float maj_tick_x, 
                                               float maj_tick_y)
{
    // /2 -> to get the number of half the vertices (Point FROM, not point TO, because they are always equal )
    // /3 -> to get number of text fields 
    int num_y_textboxes = horizontal_grid_line_vertices.size() / 2 / 3; // y axes textboxes
    int num_x_textboxes = vertical_grid_line_vertices.size() / 2 / 3; // x axes textboxes
    std::cout << "num y textboxes:" << num_y_textboxes << std::endl;
    std::cout << "num x textboxes:" << num_x_textboxes << std::endl;

    int num_of_textboxes = num_y_textboxes + num_x_textboxes;


    ////std::cout << "Mutex locked? -> " << locked << std::endl;
    //std::cout << "clear() on plot_axes" << std::endl;

    //auto itr = _plot_axes.begin();
    //while ( itr != _plot_axes.end() ) {
    //        itr = _plot_axes.erase(itr);
    //}

    if ( !_plot_axes.empty() ) {
        // Remove the old ones first
        for ( auto textbox_it = _plot_axes.begin(); textbox_it != _plot_axes.end(); ++textbox_it ) {
            delete *textbox_it;
            //_plot_axes.erase(textbox_it);
        }
        _plot_axes.clear();
    }

    // Create new plot axes
    for (int num_axes = 0; num_axes < num_of_textboxes; ++num_axes) {
        _plot_axes.push_back(new OGLTextBox()); // Why is immediately the destructore called here? because the textbox is out of scope?
     }
    //_plot_axes.clear(); // leads to error, when the destructor is called, _VAO seems to be invalidated? Why? 
    //// Is it, because i made a copy of the plot via GetPlotptr() ? Do i need copy constructors?
    ////std::vector<OGLTextBox>().swap(_plot_axes);
    //// In here, the destructor is called
    
    //for ( int num_axes = 0; num_axes < num_of_textboxes; ++num_axes ) {
    //    _plot_axes.emplace_back(); // Why is immediately the destructore called here? because the textbox is out of scope?
    //}
    //std::cout << "resize() on plot_axes" << std::endl;
    //_plot_axes.resize(num_of_textboxes);  // CALLS COPY CONSTRUCTOR OF OGLTextbox -> must be implemented correctly or OGL state is messed up!!
    
    for ( auto& description : _plot_axes ) {
        description->Initialize(Font2D_TP::ARIAL);
        //description->Initialize(Font2D_TP::ARIAL);
    }

    // horizontal changing descriptions (y-axes)
    auto y_textboxes_it_end = _plot_axes.begin() + num_y_textboxes;
    // fine adjustment of text positioning via offet_x_S and offset_y_S:
    int offset_x_S = 3;
    int offset_y_S = -10;
    int vec_offset_idx = 1;
    auto current_y_description = max_y_val;

    int num_of_y_textboxes = 0;
    for ( auto y_textbox_it = _plot_axes.begin(); y_textbox_it != y_textboxes_it_end; ++y_textbox_it ) {
       /*int*/float pos_x = _bounding_box.GetLeftBottom()._x + offset_x_S;
        /*int*/float pos_y = horizontal_grid_line_vertices.at(vec_offset_idx) + offset_y_S;
        std::string text = std::to_string(static_cast<double>(current_y_description)) + " mV";
        std::cout << "set  y text #" << num_of_y_textboxes << std::endl;
        (*y_textbox_it)->SetText(text, pos_x, pos_y, scale);
        //y_textbox_it->SetText(text, pos_x, pos_y, scale);

        current_y_description -= maj_tick_y;
        vec_offset_idx += 6;
        ++num_of_y_textboxes;
    }
    assert(num_of_y_textboxes == num_y_textboxes);


    //int num_of_y_textboxes = 0;
    //for ( auto y_textbox_it = _plot_axes.begin(); y_textbox_it != y_textboxes_it_end; ++y_textbox_it ) {
    //   /*int*/float pos_x = _bounding_box.GetLeftBottom()._x + offset_x_S;
    //    /*int*/float pos_y = horizontal_grid_line_vertices.at(vec_offset_idx) + offset_y_S;
    //    std::string text = std::to_string(static_cast<double>(current_y_description)) + " mV";
    //    std::cout << "set  y text #" << num_of_y_textboxes << std::endl;
    //    y_textbox_it->SetText(text, pos_x, pos_y, scale);
    //    //y_textbox_it->SetText(text, pos_x, pos_y, scale);

    //    current_y_description -= maj_tick_y;
    //    vec_offset_idx += 6;
    //    ++num_of_y_textboxes;
    //}
    //assert(num_of_y_textboxes == num_y_textboxes);


    // offset to manually shift the textbox on the x-axis
    offset_x_S = 0;
    // offset to manually shift the textbox on the y-axis
    offset_y_S = 8;
    vec_offset_idx = 0;
    auto current_x_description = time_range_ms;
    int num_of_x_textboxes = 0;
    for ( auto x_textbox_it = y_textboxes_it_end; x_textbox_it != _plot_axes.end(); ++x_textbox_it ) {
        float /*int*/ pos_x = vertical_grid_line_vertices.at(vec_offset_idx) + offset_x_S;
        float /*int*/ pos_y = _plot_area.GetLeftTop()._y + offset_y_S;
        std::string text = std::to_string(static_cast<int>(current_x_description)) + " ms";
        (*x_textbox_it)->SetText(text, pos_x, pos_y, scale);
        std::cout << "set  x text #" << num_of_x_textboxes<<std::endl;

        current_x_description -= maj_tick_x;
        vec_offset_idx += 6;
        ++num_of_x_textboxes;
    }

    // Verify 
    for ( auto& description : _plot_axes ) {
        std::cout << "text set = " << description->IsTextSet() << std::endl;
    }

    assert(num_of_x_textboxes == num_x_textboxes);
    assert(_plot_axes.size() == num_of_x_textboxes + num_of_y_textboxes);
}


//void OGLBaseChart_C::InitializeAxesDescription(const QVector<float>& horizontal_grid_line_vertices,
//    const QVector<float>& vertical_grid_line_vertices,
//    float scale,
//    int time_range_ms,
//    double max_y_val,
//    float maj_tick_x,
//    float maj_tick_y)
//{
//    // /2 -> to get the number of half the vertices (Point FROM, not point TO, because they are always equal )
//    // /3 -> to get number of text fields 
//    int num_y_textboxes = horizontal_grid_line_vertices.size() / 2 / 3; // y axes textboxes
//    int num_x_textboxes = vertical_grid_line_vertices.size() / 2 / 3; // x axes textboxes
//    std::cout << "num y textboxes:" << num_y_textboxes << std::endl;
//    std::cout << "num x textboxes:" << num_x_textboxes << std::endl;
//
//    int num_of_textboxes = num_y_textboxes + num_x_textboxes;
//
//
//    ////std::cout << "Mutex locked? -> " << locked << std::endl;
//    //std::cout << "clear() on plot_axes" << std::endl;
//
//    //auto itr = _plot_axes.begin();
//    //while ( itr != _plot_axes.end() ) {
//    //        itr = _plot_axes.erase(itr);
//    //}
//
//    if ( !_plot_axes.empty() ) {
//
//    }
//    _plot_axes.clear(); // leads to error, when the destructor is called, _VAO seems to be invalidated? Why? 
//    //// Is it, because i made a copy of the plot via GetPlotptr() ? Do i need copy constructors?
//    ////std::vector<OGLTextBox>().swap(_plot_axes);
//    //// In here, the destructor is called
//
//    //for ( int num_axes = 0; num_axes < num_of_textboxes; ++num_axes ) {
//    //    _plot_axes.emplace_back(); // Why is immediately the destructore called here? because the textbox is out of scope?
//    //}
//    //std::cout << "resize() on plot_axes" << std::endl;
//    _plot_axes.resize(num_of_textboxes);  // CALLS COPY CONSTRUCTOR OF OGLTextbox -> must be implemented correctly or OGL state is messed up!!
//
//    for ( auto& description : _plot_axes ) {
//        description.Initialize(Font2D_TP::ARIAL);
//        //description->Initialize(Font2D_TP::ARIAL);
//    }
//
//    // horizontal changing descriptions (y-axes)
//    auto y_textboxes_it_end = _plot_axes.begin() + num_y_textboxes;
//    // fine adjustment of text positioning via offet_x_S and offset_y_S:
//    int offset_x_S = 3;
//    int offset_y_S = -10;
//    int vec_offset_idx = 1;
//    auto current_y_description = max_y_val;
//
//    int num_of_y_textboxes = 0;
//    for ( auto y_textbox_it = _plot_axes.begin(); y_textbox_it != y_textboxes_it_end; ++y_textbox_it ) {
//        /*int*/float pos_x = _bounding_box.GetLeftBottom()._x + offset_x_S;
//        /*int*/float pos_y = horizontal_grid_line_vertices.at(vec_offset_idx) + offset_y_S;
//        std::string text = std::to_string(static_cast<double>(current_y_description)) + " mV";
//        std::cout << "set  y text #" << num_of_y_textboxes << std::endl;
//        y_textbox_it->SetText(text, pos_x, pos_y, scale);
//        //y_textbox_it->SetText(text, pos_x, pos_y, scale);
//
//        current_y_description -= maj_tick_y;
//        vec_offset_idx += 6;
//        ++num_of_y_textboxes;
//    }
//    assert(num_of_y_textboxes == num_y_textboxes);
//
//
//    //int num_of_y_textboxes = 0;
//    //for ( auto y_textbox_it = _plot_axes.begin(); y_textbox_it != y_textboxes_it_end; ++y_textbox_it ) {
//    //   /*int*/float pos_x = _bounding_box.GetLeftBottom()._x + offset_x_S;
//    //    /*int*/float pos_y = horizontal_grid_line_vertices.at(vec_offset_idx) + offset_y_S;
//    //    std::string text = std::to_string(static_cast<double>(current_y_description)) + " mV";
//    //    std::cout << "set  y text #" << num_of_y_textboxes << std::endl;
//    //    y_textbox_it->SetText(text, pos_x, pos_y, scale);
//    //    //y_textbox_it->SetText(text, pos_x, pos_y, scale);
//
//    //    current_y_description -= maj_tick_y;
//    //    vec_offset_idx += 6;
//    //    ++num_of_y_textboxes;
//    //}
//    //assert(num_of_y_textboxes == num_y_textboxes);
//
//
//    // offset to manually shift the textbox on the x-axis
//    offset_x_S = 0;
//    // offset to manually shift the textbox on the y-axis
//    offset_y_S = 8;
//    vec_offset_idx = 0;
//    auto current_x_description = time_range_ms;
//    int num_of_x_textboxes = 0;
//    for ( auto x_textbox_it = y_textboxes_it_end; x_textbox_it != _plot_axes.end(); ++x_textbox_it ) {
//        float /*int*/ pos_x = vertical_grid_line_vertices.at(vec_offset_idx) + offset_x_S;
//        float /*int*/ pos_y = _plot_area.GetLeftTop()._y + offset_y_S;
//        std::string text = std::to_string(static_cast<int>(current_x_description)) + " ms";
//        x_textbox_it->SetText(text, pos_x, pos_y, scale);
//        std::cout << "set  x text #" << num_of_x_textboxes << std::endl;
//
//        current_x_description -= maj_tick_x;
//        vec_offset_idx += 6;
//        ++num_of_x_textboxes;
//    }
//
//    // Verify 
//    for ( auto& description : _plot_axes ) {
//        std::cout << "text set = " << description.IsTextSet() << std::endl;
//    }
//
//    assert(num_of_x_textboxes == num_x_textboxes);
//    assert(_plot_axes.size() == num_of_x_textboxes + num_of_y_textboxes);
//}


void 
OGLBaseChart_C::SetupAxes()
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
void 
OGLBaseChart_C::DrawSurfaceGrid(QOpenGLShaderProgram& shader)
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
OGLBaseChart_C::CreateSurfaceGrid(int x_major_tick_dist_ms, float y_major_tick_dist_unit, 
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
    combined_vertices.reserve(num_of_combined_verts); // preallocate memory
    for ( auto& horizontal_vert : horizontal_grid_vertices ) {
        combined_vertices.append(horizontal_vert);
    }
    for ( auto& vertical_vert : vertical_grid_vertices ) {
        combined_vertices.append(vertical_vert);
    }

    _num_of_surface_grid_positions = num_of_combined_verts / 3;

    // Destroy the old buffer
    _surface_grid_vbo.destroy();
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


//inline
void 
OGLBaseChart_C::DrawXYAxes(QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader)
{
    //_mutex->lock();
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

    // THIS IS HAPPENING; EVEN IF THE VAO VBO IS DESTRUCTED INSIDE CLEANUP(). Therefore we have this weird errors i think
    // -> We try to lock inside RenderText(), but the mutex is actually deleted...
    //std::cout << "axes size " << _plot_axes.size() << std::endl;
    if ( !_in_init.load() ){
        for ( /*const*/ auto& description : _plot_axes ) {
            // => Cherno 2 in 1 shader 
            // -> approach: bind a white 1x1 texture to use flat colors and bind a 1.0 color to just draw the texture, 
            // use a texture sampler, if not, just use the uniform color !
            description->RenderText(text_shader, _text_color, _chart_mvp);
        }
    }

    //_mutex->unlock();
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
    _bb_vbo.allocate(bb_vertices.constData(), bb_vertices.size() * /*static_cast<int>(*/sizeof(float)/*)*/);
    _bb_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    f->glDisableVertexAttribArray(0);
    _bb_vbo.release();
}



const std::string & OGLBaseChart_C::GetLabel()
{
    return _label;
}

unsigned int OGLBaseChart_C::GetID()
{
    return _id;
}
void OGLBaseChart_C::SetLabel(const std::string & label)
{
    _label = label;
}

void OGLBaseChart_C::SetID(unsigned int id)
{
    _id = id;
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

void OGLBaseChart_C::SetFiducialMarkColor(const QVector3D & color)
{
    _fiducial_marks_color = color;
}


void
OGLBaseChart_C::SetModelViewProjection(QMatrix4x4 model_view_projection)
{
    _chart_mvp = model_view_projection;
}

OGLChartGeometry_C
OGLBaseChart_C::GetBoundingBox()
{
    return _bounding_box;
}

OGLChartGeometry_C
OGLBaseChart_C::GetPlotSurfaceArea()
{
    return _plot_area;
}
