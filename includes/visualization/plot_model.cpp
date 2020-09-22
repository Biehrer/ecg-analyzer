#include "plot_model.h"

PlotModel_C::PlotModel_C(QObject* parent)
    : QAbstractTableModel(parent)
{
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

        int row = index.row() - 1;

        if ( row < 0 ) {
            std::cout << "some fix was needed. row = " << row << std::endl;
            row = 0;
            //throw std::runtime_error("Something wrong with the index");
        }

        switch ( index.column() ) {
        case OGLPlotProperty::PLOT_ID:
            _plots[row]->SetID(value.toInt());
            break;

        case OGLPlotProperty::PLOT_LABEL:
            _plots[row]->SetLabel(value.toString().toStdString());
            break;

        case OGLPlotProperty::PLOT_TIMERANGE:
            _plots[row]->SetTimerangeMs(value.toDouble());
            break;

        case OGLPlotProperty::PLOT_YMAX:
            std::cout << "change plot y max val (plot_id = " << row << ")" << std::endl;
            _plots[row]->SetMaxValueYAxes(value.toDouble());
            break;

        case OGLPlotProperty::PLOT_YMIN:
            _plots[row]->SetMinValueYAxes(value.toDouble());
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
    return _plots.size();
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


// can cause locks -  but this is called from the ui thread so we dont care
void PlotModel_C::SetGain(const float gain) {
    //_sig_gain.store(gain);
    for ( auto& plot : _plots ) {
        plot->SetGain(gain);
    }
}

void PlotModel_C::ClearPlotSurfaces()
{
    for ( auto& plot : _plots ) {
        plot->Clear();
    }
}

void PlotModel_C::RemovePlot(unsigned int plot_id)
{
    for ( auto plot_it = _plots.begin(); plot_it < _plots.end(); ++plot_it ) {
        if ( (*plot_it)->GetID() == plot_id ) {
            // beginRemoveRows()
            plot_it = _plots.erase(plot_it);
            // endRemoveRows()
        }
    }
}

bool PlotModel_C::FastInitializePlots(int number_of_plots,
    int view_width,
    int view_height,
    int time_range_ms,
    const std::vector<std::pair<ModelDataType_TP, ModelDataType_TP>>& y_ranges)
{
    DEBUG("initialize plots");

    // Chart properties
    //RingBufferSize_TP chart_buffer_size = RingBufferSize_TP::Size65536;
    RingBufferSize_TP chart_buffer_size = RingBufferSize_TP::Size1048576;

    if ( y_ranges.size() != number_of_plots ) {
        throw std::invalid_argument("Y-Ranges vector is not correct");
    }

    int offset_x = static_cast<double>(view_width) * 0.05;
    int offset_y = static_cast<double>(view_height) * 0.15;
    int chart_width = view_width - offset_x;
    int chart_height = (view_height - offset_y) / number_of_plots;

    // Chart is aligned at the left side of the screen
    int chart_pos_x = 10;
    int chart_to_chart_offset_S = 10;
    int chart_offset_from_origin_S = 4;

    // Create plots
    for ( int chart_idx = 0; chart_idx < number_of_plots; ++chart_idx ) {

        int chart_pos_y = chart_idx * (chart_height + chart_to_chart_offset_S) +
            chart_offset_from_origin_S;
        OGLChartGeometry_C geometry(chart_pos_x, chart_pos_y, chart_width, chart_height);
        _plots.push_back(new OGLSweepChart_C<ModelDataType_TP >(time_range_ms,
            chart_buffer_size,
            /*max_y*/y_ranges[chart_idx].second,
            /*min_y*/y_ranges[chart_idx].first,
            geometry,
            *this));
    }

    QVector3D series_color(0.0f, 1.0f, 0.0f); // green
    QVector3D axes_color(1.0f, 1.0f, 1.0f); //white
    QVector3D lead_line_color(1.0f, 0.01f, 0.0f); // red
    QVector3D surface_grid_color(static_cast<float>(235.0f / 255.0f),   //yellow-ish
        static_cast<float>(225.0f / 255.0f),
        static_cast<float>(27.0f / 255.0f));
    QVector3D bounding_box_color(1.0f, 1.0f, 1.0f); // white
    QVector3D text_color(1.0f, 1.0f, 1.0f); // white
    QVector3D fiducial_mark_color(0.0f, 0.0f, 1.0f); // blue

    unsigned int c_id = 0;
    unsigned int r_id = 1;

    for ( auto& plot : _plots ) {
        //beginInsertRows(QModelIndex(), _plots.size(), _plots.size() + 1);
        beginInsertRows(QModelIndex(), r_id - 1, r_id);
        // modifyable variables (from user):
        setData(createIndex(r_id, c_id), r_id-1);
        QString label = QString("plot #" + QString::fromStdString(std::to_string(r_id)));
        setData(createIndex(r_id, c_id + 1), QVariant(label));
        setData(createIndex(r_id, c_id + 2), QVariant(time_range_ms));
        //setData(createIndex(r_id, c_id + 3), QVariant(/*max_y*/static_cast<double>(y_ranges[r_id-1].second)));
        //setData(createIndex(r_id, c_id + 4), QVariant(/*min_y*/static_cast<double>(y_ranges[r_id-1].first)));
        plot->SetMaxValueYAxes(static_cast<double>(y_ranges[r_id - 1].second));
        plot->SetMinValueYAxes(static_cast<double>(y_ranges[r_id - 1].first));
        // Divide through 4 to create 4 horizontal and 4 vertical lines 
        setData(createIndex(r_id, c_id + 5), QVariant(time_range_ms / 4));
        setData(createIndex(r_id, c_id + 6), QVariant((static_cast<double>(y_ranges[r_id - 1].second - y_ranges[r_id - 1].first)) / 4));
        endInsertRows();
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
        plot->SetFiducialMarkColor(fiducial_mark_color);

        // Set chart type
        plot->SetChartType(DrawingStyle_TP::LINE_SERIES);
        // Initialize
        plot->Initialize();
    }

    //emit a signal to make the view reread identified data
    emit dataChanged(createIndex(0, 0), //  top left table index
        createIndex(number_of_plots, COLS), // bottom right table index
        { Qt::DisplayRole });
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


void PlotModel_C::AddPlot(const PlotDescription_TP& plot_info)
{
    int number_of_plots = _plots.size();
    // Create plot
    _plots.push_back(new OGLSweepChart_C<ModelDataType_TP>(plot_info._time_range_ms,
        plot_info._input_buffer_size,
        plot_info._max_y,
        plot_info._min_y,
        plot_info._geometry,
        *this));
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

void
PlotModel_C::AddPlot(OGLSweepChart_C<ModelDataType_TP>& plot)
{

    int number_of_plots = _plots.size();
    // Create plot
    _plots.push_back(new OGLSweepChart_C<ModelDataType_TP>(static_cast<int>(plot.GetTimerangeMs()),
        plot.GetInputBufferSize(),
        plot.GetMaxValueYAxes(),
        plot.GetMinValueYAxes(),
        plot.GetBoundingBox(),
        *this));

    auto* plot_new = *(_plots.end() - 1);
    plot_new->SetLabel(plot.GetLabel());
    plot_new->SetID(plot.GetID());
    // Set up axes
    plot_new->SetMajorTickValueXAxes(plot.GetMajorTickValueXAxes());
    plot_new->SetMajorTickValueYAxes(plot.GetMajorTickValueYAxes());
    // Set chart type ( Point or Line series )
    plot_new->SetChartType(plot.GetChartType());
    // Initialize
    plot_new->Initialize();

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
