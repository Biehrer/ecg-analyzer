#include "plot_model.h"


PlotModel_C::PlotModel_C( QObject* parent) 
    : QAbstractTableModel(parent)
{
    //QHash<int, QByteArray> roles;
    //roles[TypeRole] = "type";
    //roles[SizeRole] = "size";
    //setRoleNames(roles);
    _view_data.reserve(COLS * ROWS);
    _view_data.resize(COLS * ROWS);
    //_view_data[0] = "ID 0";
    //_view_data[1] = " 10000";
    //_view_data[2] = "10";
    //_view_data[3] = "-10";
    //_view_data[4] = " ID 1";
    //_view_data[5] = "10000";
    //_view_data[6] = " 10 ";
    //_view_data[7] = " -10";
}


// Sets the first row and determines the layout ( with #COLS items)
QVariant 
PlotModel_C::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role == Qt::DisplayRole && orientation == Qt::Horizontal ) {
        switch ( section ) {
        case 0:
            return QString("PlotID");
        case 1:
            return QString("Label");
        case 2:
            return QString("Time range (ms)");
        case 3:
            return QString("y-max");
        case 4:
            return QString("y-min");
        case 5:
            return QString("Maj-tick x-axes");
        case 6:
            return QString("Maj-tick y-axes");

        }
    }
    return QVariant();
}

Qt::ItemFlags PlotModel_C::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool PlotModel_C::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ( role == Qt::EditRole ) {
        //if ( !checkIndex(index) )
            //return false;
        //save value from editor to member m_gridData
        int row = index.row() - 1;
        _view_data[(row * COLS + index.column() )] = value.toString();
        //for presentation purposes only: build and emit a joined string
        //QString result;
        //for ( int row = 0; row < ROWS; row++ ) {
        //    for ( int col = 0; col < COLS; col++ )
        //        result += m_gridData[row][col] + ' ';
        //}
        //emit editCompleted(result);
        return true;
    }
    return false;
}

int PlotModel_C::rowCount(const QModelIndex & parent) const
{
    return ROWS;
    //return _plots.size();
}

int PlotModel_C::columnCount(const QModelIndex & parent) const
{
    return COLS;
}

QVariant 
PlotModel_C::data(const QModelIndex & index, int role) const 
{
    int row = index.row();
    int col = index.column();
   
    switch ( role ) {

    case Qt::DisplayRole:
        return QString(_view_data[(COLS * row + col)]);

        //case Qt::FontRole:
        //    if ( row == 0 && col == 0 ) { //change font only for cell(0,0)
        //        QFont boldFont;
        //        boldFont.setBold(true);
        //        return boldFont;
        //    }
        //    break;
    }
    return QVariant();
}


void 
PlotModel_C::RecreateData() 
{
    int number_of_plots = _plots.size();
    _view_data.reserve(COLS * number_of_plots);
    _view_data.resize(COLS * number_of_plots);

    int c_id = 0;
    for ( int r_id = 1; r_id <= number_of_plots; ++r_id ) {
            setData(createIndex(r_id, c_id), _plots[r_id-1]->GetID());
            QString label = QString::fromStdString(_plots[r_id-1]->GetLabel());
            setData(createIndex(r_id, c_id+1), QVariant(label));
            setData(createIndex(r_id, c_id+2), QVariant(_plots[r_id-1]->GetTimerangeMs()));
            setData(createIndex(r_id, c_id+3), QVariant(_plots[r_id-1]->GetMaxValueYAxes()));
            setData(createIndex(r_id, c_id+4), QVariant(_plots[r_id-1]->GetMinValueYAxes()));
            setData(createIndex(r_id, c_id+5), QVariant(_plots[r_id-1]->GetMajorTickValueXAxes()));
            setData(createIndex(r_id, c_id+6), QVariant(_plots[r_id-1]->GetMajorTickValueYAxes()));
    }
   
    QModelIndex topLeft = createIndex(0, 0);
    QModelIndex bottom_right = createIndex(number_of_plots, COLS);
    //emit a signal to make the view reread identified data
    emit dataChanged(topLeft, bottom_right, { Qt::DisplayRole });
}

void PlotModel_C::RemovePlot(unsigned int plot_id)
{
    for ( auto plot_it = _plots.begin(); plot_it < _plots.end(); ++plot_it ) {
        if ( (*plot_it)->GetID() == plot_id ) {
            plot_it = _plots.erase(plot_it);
        }
    }
}

bool PlotModel_C::FastInitializePlots(int number_of_plots,
                                      int view_width, 
                                      int view_height,
                                      int time_range_ms,
                                      float max_y, 
                                      float min_y)
{
    DEBUG("initialize plots");

    // Chart properties
    RingBufferSize_TP chart_buffer_size = RingBufferSize_TP::Size65536;

    int offset_x = static_cast<double>(view_width) * 0.05;
    int offset_y = static_cast<double>(view_height) * 0.15;
    int chart_width = view_width - offset_x;
    int chart_height = (view_height - offset_y) / number_of_plots;

    // Chart is aligned at the left side of the screen
    int chart_pos_x = 10;
    int chart_to_chart_offset_S = 10;
    int chart_offset_from_origin_S = 4;

    // Create plots
    for( int chart_idx = 0; chart_idx < number_of_plots; ++chart_idx ) {

        int chart_pos_y = chart_idx * (chart_height + chart_to_chart_offset_S) +
                          chart_offset_from_origin_S;
        OGLChartGeometry_C geometry(chart_pos_x, chart_pos_y, chart_width, chart_height);
       _plots.push_back(new OGLSweepChart_C<ModelDataType_TP >(time_range_ms,
                                                    chart_buffer_size,
                                                    max_y,
                                                    min_y,
                                                    geometry,
                                                    *this));
    }

    QVector3D series_color(0.0f, 1.0f, 0.0f);
    QVector3D axes_color(1.0f, 1.0f, 1.0f);
    QVector3D lead_line_color(1.0f, 0.01f, 0.0f);
    QVector3D surface_grid_color(static_cast<float>(235.0f / 255.0f),
        static_cast<float>(225.0f / 255.0f),
        static_cast<float>(27.0f / 255.0f));
    QVector3D bounding_box_color(1.0f, 1.0f, 1.0f);
    QVector3D text_color(1.0f, 1.0f, 1.0f);

    unsigned int plot_idx = 0;
    for ( auto& plot : _plots ) {
        plot->SetID(plot_idx);
        plot->SetLabel("plot #" + std::to_string(plot_idx));
        ++plot_idx;
        // Setup colors
        plot->SetSeriesColor(series_color);
        plot->SetAxesColor(axes_color);
        plot->SetTextColor(text_color);
        plot->SetBoundingBoxColor(bounding_box_color);
        plot->SetLeadLineColor(lead_line_color);
        plot->SetSurfaceGridColor(surface_grid_color);
        // Set up axes
        plot->SetMajorTickValueXAxes(time_range_ms / 4);
        plot->SetMajorTickValueYAxes((max_y - min_y) / 4);
        // Set chart type
        plot->SetChartType(DrawingStyle_TP::LINE_SERIES);
        // Initialize
        plot->Initialize();
    }

    // update view
    RecreateData();
    
    //we identify the top left cell
    //QModelIndex topLeft = createIndex(0, 0);
    //QModelIndex bottom_right = createIndex(number_of_plots, COLS);
    //emit a signal to make the view reread identified data
    //emit dataChanged(topLeft, bottom_right, { Qt::DisplayRole });

    //this->data();
    return true;
}

OGLSweepChart_C<ModelDataType_TP>*
PlotModel_C::GetPlotPtr(unsigned int plot_idx)
{
    if ( plot_idx < _plots.size() ) {
        return _plots[plot_idx];
    }

    return nullptr;
}

OGLSweepChart_C<ModelDataType_TP>*
PlotModel_C::GetPlotPtr(const std::string & plot_label) {
    for ( const auto& plot : _plots ) {
        if ( plot_label == plot->GetLabel() ) {
            return plot;
        }
    }
    return nullptr;
}
const std::vector<OGLSweepChart_C<ModelDataType_TP>*>& 
PlotModel_C::constData() const
{
    return _plots;
}

std::vector<OGLSweepChart_C<ModelDataType_TP>*>& 
PlotModel_C::Data()
{
    return _plots;
}


// Inside PlotManager_C / PlotController_C 
void PlotModel_C::AddPlot( const PlotDescription_TP& plot_info)
{
    // Create plot
    _plots.push_back(new OGLSweepChart_C<ModelDataType_TP>(plot_info._time_range_ms,
                                                           plot_info._buffer_size, 
                                                           plot_info._max_y,
                                                           plot_info._min_y,
                                                           plot_info._geometry, 
                                                           *this));
    //++_number_of_plots;
    auto* plot = *(_plots.end() - 1);
    plot->SetLabel(plot_info._label);
    plot->SetID(plot_info._id);
    // Setup colors
    plot->SetSeriesColor(plot_info._colors._series);
    plot->SetAxesColor(plot_info._colors._axes);
    plot->SetTextColor(plot_info._colors._text);
    plot->SetBoundingBoxColor(plot_info._colors._bounding_box);
    plot->SetLeadLineColor(plot_info._colors._lead_line);
    plot->SetSurfaceGridColor(plot_info._colors._surface_grid);
    // Set up axes
    plot->SetMajorTickValueXAxes(plot_info._maj_tick_x);
    plot->SetMajorTickValueYAxes(plot_info._maj_tick_y);
    // Set chart type ( Point or Line series )
    plot->SetChartType(plot_info._chart_type);
    // Initialize
    plot->Initialize();
}

bool 
PlotModel_C::RemovePlot(const std::string & label)
{
    for ( auto plot_it = _plots.begin(); plot_it < _plots.end(); ++plot_it ) {
        if ( label == (*plot_it)->GetLabel() ) {
            // match
            _plots.erase(plot_it);
             return true;
        }
    }
     return false;
}
