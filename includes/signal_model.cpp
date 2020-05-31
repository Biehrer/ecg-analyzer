#include "signal_model.h"

SignalModel_C::SignalModel_C(QObject* parent)
    : QAbstractTableModel(parent)
{
    //QHash<int, QByteArray> roles;
    //roles[TypeRole] = "type";
    //roles[SizeRole] = "size";
    //setRoleNames(roles);
}


QVariant
SignalModel_C::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role == Qt::DisplayRole && orientation == Qt::Horizontal ) {
        switch ( section ) {
        case 0:
            return QString("ID");
        case 1:
            return QString("Label");
        case 2:
            return QString("Time range (ms)");
        case 3:
            return QString("# channels");
        case 4:
            return QString("Datatype");
        }
    }
    return QVariant();
}

Qt::ItemFlags SignalModel_C::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

//
//bool SignalModel_C::setData(const QModelIndex &index, const QVariant &value, int role) {
//    if ( role == Qt::EditRole ) {
//
//        if ( !index.isValid() ) {
//            return false;
//        }
//
//        int row = index.row() - 1;
//
//        switch ( index.column() ) {
//        case OGLSignalProperty::SIGNAL_ID:
//            _signals[row]->SetID(value.toInt());
//            break;
//
//        case OGLPlotProperty::PLOT_LABEL:
//            _signals[row]->SetLabel(value.toString().toStdString());
//            break;
//
//        case OGLPlotProperty::PLOT_TIMERANGE:
//            _signals[row]->SetTimerangeMs(value.toDouble());
//            break;
//
//        case OGLPlotProperty::PLOT_YMAX:
//            _signals[row]->SetMaxValueYAxes(value.toFloat());
//            break;
//
//        case OGLPlotProperty::PLOT_YMIN:
//            _signals[row]->SetMinValueYAxes(value.toFloat());
//            break;
//
//        case OGLPlotProperty::PLOT_MAJTICK_X:
//            _signals[row]->SetMajorTickValueXAxes(value.toFloat());
//            break;
//
//        case OGLPlotProperty::PLOT_MAJTICK_Y:
//            _signals[row]->SetMajorTickValueYAxes(value.toFloat());
//            break;
//        }
//
//        return true;
//    }
//
//    return false;
//}

void 
SignalModel_C::AddSignal(const TimeSignal_C<SignalModelDataType_TP>& signal) 
{    
    // TODO implement copy constructor or initializing of a signal with another
    _signals.push_back(new TimeSignal_C<SignalModelDataType_TP>(signal));
    emit dataChanged(createIndex(0, 0), //  top left table index
                        createIndex(_signals.size(), SIGNAL_COLS), // bottom right table index
                        { Qt::DisplayRole });
}

int SignalModel_C::rowCount(const QModelIndex & parent) const
{
    return _signals.size();
}

int SignalModel_C::columnCount(const QModelIndex & parent) const {
    return SIGNAL_COLS;
}

QVariant
SignalModel_C::data(const QModelIndex & index, int role) const
{
    int row = index.row();
    int col = index.column();

    switch ( role ) 
    {
    case Qt::DisplayRole:

        switch ( col )
        {
        case OGLSignalProperty::SIGNAL_ID:
            return _signals[row]->GetID();

        case OGLSignalProperty::SIGNAL_LABEL:
            return QString::fromStdString(_signals[row]->GetLabel());

        case OGLSignalProperty::SIGNAL_TIMERANGE:
            return _signals[row]->GetTimerangeMs();

        case OGLSignalProperty::SIGNAL_NUM_CHANNELS:
            return _signals[row]->GetChannelCount();

        case OGLSignalProperty::SIGNAL_DATATYPE:
            return QString::fromStdString(_signals[row]->GetDatatype());

        }
    }
    return QVariant();
}


const std::vector<TimeSignal_C<SignalModelDataType_TP>*>&
SignalModel_C::Data()
{
    return _signals;
}

const std::vector<TimeSignal_C<SignalModelDataType_TP>*>&
SignalModel_C::constData() const
{
    return _signals;
}
//const 
//std::vector<TimeSignal_C<SignalModelDataType_TP>*>&
//SignalModel_C::constData() const
//{
//    return _signals;
//}



// Inside PlotManager_C / PlotController_C 
//void SignalModel_C::AddPlot(const PlotDescription_TP& plot_info)
//{
//    int number_of_plots = _plots.size();
//    // Create plot
//    _plots.push_back(new OGLSweepChart_C<ModelDataType_TP>(plot_info._time_range_ms,
//        plot_info._buffer_size,
//        plot_info._max_y,
//        plot_info._min_y,
//        plot_info._geometry,
//        *this));
//    //++_number_of_plots;
//    auto* plot = *(_plots.end() - 1);
//    plot->SetLabel(plot_info._label);
//    plot->SetID(plot_info._id);
//    // Setup colors
//    plot->SetSeriesColor(plot_info._colors._series);
//    plot->SetAxesColor(plot_info._colors._axes);
//    plot->SetTextColor(plot_info._colors._text);
//    plot->SetBoundingBoxColor(plot_info._colors._bounding_box);
//    plot->SetLeadLineColor(plot_info._colors._lead_line);
//    plot->SetSurfaceGridColor(plot_info._colors._surface_grid);
//    // Set up axes
//    plot->SetMajorTickValueXAxes(plot_info._maj_tick_x);
//    plot->SetMajorTickValueYAxes(plot_info._maj_tick_y);
//    // Set chart type ( Point or Line series )
//    plot->SetChartType(plot_info._chart_type);
//    // Initialize
//    plot->Initialize();
//
//    //emit a signal to make the view reread identified data
//    emit dataChanged(createIndex(number_of_plots, 0), //  top left table index
//        createIndex(number_of_plots + 1, COLS), // bottom right table index
//        { Qt::DisplayRole });
//}


