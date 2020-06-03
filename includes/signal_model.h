#pragma once

// Project includes
#include "../includes/signal_proc_lib/time_signal.h"

// Qt includes
#include <qobject.h>
#include <QAbstractTableModel>
#include <qvariant.h>
#include <qvector3d.h>
#include <qvector.h>
#include <qstring.h>

// STL includes
#include <vector>
#include <string>
#include <atomic>

enum OGLSignalProperty {
    SIGNAL_ID,
    SIGNAL_LABEL,
    SIGNAL_TIMERANGE,
    SIGNAL_NUM_CHANNELS,
    SIGNAL_DATATYPE
};

using SignalModelDataType_TP = float;

class SignalModel_C : public QAbstractTableModel
{
    Q_OBJECT

public:
    SignalModel_C(QObject *parent = nullptr);

public:

    //!
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    //bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    //!
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    //!
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    //!
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    //
    const std::vector<TimeSignal_C<SignalModelDataType_TP>*>& Data();

    const std::vector<TimeSignal_C<SignalModelDataType_TP>*>& constData() const;

    void RemoveSignal(unsigned int id);

    void RemoveSignal(const std::string& label);
    
public slots:
    void AddSignal(const TimeSignal_C<SignalModelDataType_TP>& signal);
    //void AddSignal(const TimeSignal_C<float>& signal);
    //void AddSignal(const TimeSignal_C<int>& signal);
    //void AddSignal(const TimeSignal_C<double>& signal);

private:
    //! the data this model manages
    std::vector<TimeSignal_C<SignalModelDataType_TP>*> _signals;

    const int SIGNAL_COLS = 5;
};

