#pragma once

// Project includes

// Qt includes
#include <qobject.h>
#include <QAbstractTableModel>
#include <qvariant.h>

const int COLS = 7;
const int ROWS = 2;

class PlotModel_C : public QAbstractTableModel
{
    Q_OBJECT

public:
    PlotModel_C( QObject *parent = nullptr);
    
    //!
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    
    //!
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    //!
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    
    //!
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    
    //! Adds a plot to the view
    //! Writes PlotDescription_TP to _data as string -> implement to String method 
    void AddPlot(PlotDescription_TP description);

    void RecreateData();

    //! Removes a plot from the view
    void RemovePlot(unsigned int plot_id);
    
    void RemovePlot(const std::string& plot_label);

    //bool PlotModel_C::setData(const QModelIndex &index, const QVariant &value, int role);

private:
    //!
    //QOpenGLPlotRendererWidget* _data = nullptr;

    std::vector<PlotDescription_TP> _raw_data;

    QVector<QString> _data;
};
