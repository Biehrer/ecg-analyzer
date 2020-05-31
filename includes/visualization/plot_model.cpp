#include "plot_model.h"

PlotModel_C::PlotModel_C( QObject* parent) 
    : QAbstractTableModel(parent)
{
    //QHash<int, QByteArray> roles;
    //roles[TypeRole] = "type";
    //roles[SizeRole] = "size";
    //setRoleNames(roles);
}

enum OGLPlotProperty {

    PLOT_ID,
    PLOT_LABEL,
    PLOT_TIMERANGE,
    PLOT_YMAX,
    PLOT_YMIN,
    PLOT_MAJTICK_X,
    PLOT_MAJTICK_Y
};
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

        if ( !index.isValid() ) {
            return false;
        }

        int row = index.row() -1;

        switch ( index.column() ) {
            case OGLPlotProperty::PLOT_ID:
                _plots[row]->SetID(value.toInt() );
                break;

            case OGLPlotProperty::PLOT_LABEL:
                _plots[row]->SetLabel(value.toString().toStdString());
                break;

            case OGLPlotProperty::PLOT_TIMERANGE:
                _plots[row]->SetTimerangeMs(value.toDouble());
                break;

            case OGLPlotProperty::PLOT_YMAX:
                _plots[row]->SetMaxValueYAxes(value.toFloat());
                break;

            case OGLPlotProperty::PLOT_YMIN:
                _plots[row]->SetMinValueYAxes(value.toFloat());
                break;

            case OGLPlotProperty::PLOT_MAJTICK_X:
                _plots[row]->SetMajorTickValueXAxes(value.toFloat());
                break;

            case OGLPlotProperty::PLOT_MAJTICK_Y:
               _plots[row]->SetMajorTickValueYAxes(value.toFloat());
               break;
        }
        
         return true;
    }

    return false;
}

int PlotModel_C::rowCount(const QModelIndex & parent) const
{
    return ROWS;
    //std::cout << "row count " << _plots.size() << std::endl;
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

        switch ( col ) 
        {
        case OGLPlotProperty::PLOT_ID:
            return _plots[row]->GetID();

        case OGLPlotProperty::PLOT_LABEL:
            return QString::fromStdString(_plots[row]->GetLabel());

        case OGLPlotProperty::PLOT_TIMERANGE:
            return _plots[row]->GetTimerangeMs();

        case OGLPlotProperty::PLOT_YMAX:
            return _plots[row]->GetMaxValueYAxes();

        case OGLPlotProperty::PLOT_YMIN:
            return _plots[row]->GetMinValueYAxes();

        case OGLPlotProperty::PLOT_MAJTICK_X:
            return _plots[row]->GetMajorTickValueXAxes();

        case OGLPlotProperty::PLOT_MAJTICK_Y:
            return _plots[row]->GetMajorTickValueYAxes();
        }
        
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

    //emit a signal to make the view reread identified data
    emit dataChanged(createIndex(0, 0), //  top left table index
                     createIndex(number_of_plots, COLS), // bottom right table index
                     {Qt::DisplayRole});
}

// can cause locks -  but this is called from the ui thread so we dont care
void PlotModel_C::SetGain(const float gain) {
    //_sig_gain.store(gain);
    for ( auto& plot : _plots ) {
        plot->SetGain(gain);
    }
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

    unsigned int c_id = 0;
    unsigned int r_id = 1;

    for ( auto& plot : _plots ) {
        // modifyable variables (from user):
        setData(createIndex(r_id, c_id), r_id );
        QString label = QString("plot #" + QString::fromStdString(std::to_string(r_id)) );
        setData(createIndex(r_id, c_id + 1), QVariant(label));
        setData(createIndex(r_id, c_id + 2), QVariant(time_range_ms ));
        setData(createIndex(r_id, c_id + 3), QVariant(max_y));
        setData(createIndex(r_id, c_id + 4), QVariant(min_y));
        setData(createIndex(r_id, c_id + 5), QVariant(time_range_ms / 4));
        setData(createIndex(r_id, c_id + 6), QVariant((max_y - min_y) / 4));
        ++r_id;
        //plot->SetID(plot_idx);
        //plot->SetLabel("plot #" + std::to_string(plot_idx));
        //++plot_idx;
        // Set up axes
        //plot->SetMajorTickValueXAxes(time_range_ms / 4);
        //plot->SetMajorTickValueYAxes((max_y - min_y) / 4);
        // Non modifyale
        // Setup colors
        plot->SetSeriesColor(series_color);
        plot->SetAxesColor(axes_color);
        plot->SetTextColor(text_color);
        plot->SetBoundingBoxColor(bounding_box_color);
        plot->SetLeadLineColor(lead_line_color);
        plot->SetSurfaceGridColor(surface_grid_color);

        // Set chart type
        plot->SetChartType(DrawingStyle_TP::LINE_SERIES);
        // Initialize
        plot->Initialize();
    }

        //emit a signal to make the view reread identified data
    emit dataChanged(createIndex(0, 0), //  top left table index
                     createIndex(number_of_plots, COLS), // bottom right table index
                     { Qt::DisplayRole });
    //RecreateData();
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
    int number_of_plots = _plots.size();
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

    //emit a signal to make the view reread identified data
    emit dataChanged(createIndex(number_of_plots, 0), //  top left table index
                     createIndex(number_of_plots + 1, COLS), // bottom right table index
                     { Qt::DisplayRole });
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