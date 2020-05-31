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

void 
SignalModel_C::AddSignal(const TimeSignal_C<SignalModelDataType_TP>& signal) 
{    
    // TODO copy constructor 
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

