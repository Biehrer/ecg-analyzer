#pragma once

// Project includes
//#include "ogl_plot_renderer_widget.h"
#include "ogl_sweep_chart.h"

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

const int COLS = 7;
const int ROWS = 2;

using ModelDataType_TP = float;

//! Definition of plot colors
struct PlotColors_TP {
    QVector3D _series = QVector3D(0.0f, 1.0f, 0.0f);
    QVector3D _axes = QVector3D(1.0f, 1.0f, 1.0f);
    QVector3D _lead_line = QVector3D(1.0f, 0.01f, 0.0f);
    QVector3D _surface_grid = QVector3D(static_cast<float>(235.0f / 255.0f),
                                        static_cast<float>(225.0f / 255.0f),
                                        static_cast<float>(27.0f / 255.0f));
    QVector3D _text = QVector3D(1.0f, 1.0f, 1.0f);
    QVector3D _bounding_box = QVector3D(1.0f, 1.0f, 1.0f);
};

//! All the info required to initialize a plot
struct PlotDescription_TP
{
    std::string _label = "unnamed";
    OGLChartGeometry_C _geometry;
    PlotColors_TP _colors;
    DrawingStyle_TP _chart_type = DrawingStyle_TP::LINE_SERIES;
    RingBufferSize_TP _buffer_size = RingBufferSize_TP::Size32768;

    unsigned int _id = 0;
    int _time_range_ms = 1000.0;
    ModelDataType_TP _min_y = -5;
    ModelDataType_TP _max_y = 5;
    ModelDataType_TP _maj_tick_x = _time_range_ms / 4.0;
    ModelDataType_TP _maj_tick_y = (_max_y - _min_y) / 4.0;

    QVector<QString> ToPlotModelString()
    { // 7 x QStrings
        QVector<QString> string_data;
        string_data.push_back(QString::fromStdString(std::to_string(_id)));
        string_data.push_back(QString::fromStdString(_label));

        string_data.push_back(QString::fromStdString(std::to_string(_time_range_ms)));
        string_data.push_back(QString::fromStdString(std::to_string(_max_y)));
        string_data.push_back(QString::fromStdString(std::to_string(_min_y)));

        string_data.push_back(QString::fromStdString(std::to_string(_maj_tick_x)));
        string_data.push_back(QString::fromStdString(std::to_string(_maj_tick_y)));

        return string_data;
    }
};

class PlotModel_C : public QAbstractTableModel
{
    Q_OBJECT

public:
    PlotModel_C( QObject *parent = nullptr);

public:

    //! Removes a plot from the view
    void RemovePlot(unsigned int plot_id);

    bool RemovePlot(const std::string& plot_label);

    void AddPlot(const PlotDescription_TP & plot_info);

    //! Fast initialization of plots in a horizontal layout (shared timerange and max/min y values)
    //! Creates OGLSweepCharts
    bool FastInitializePlots(int number_of_plots,
        int view_width,
        int view_height,
        int time_range_ms,
        const std::vector<std::pair<ModelDataType_TP, ModelDataType_TP>>& y_ranges);

    OGLSweepChart_C<ModelDataType_TP>* GetPlotPtr(unsigned int plot_idx);

    OGLSweepChart_C<ModelDataType_TP>* GetPlotPtr(const std::string & plot_label);

    std::vector<OGLSweepChart_C<ModelDataType_TP >*>& Data();

    const std::vector<OGLSweepChart_C<ModelDataType_TP >*>& constData() const;
    
    //!
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    
    //!
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    //!
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    
    //!
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    
    //! Adds a plot to the view
    //! Writes PlotDescription_TP to _data as string -> implement to String method 
    //void AddPlot(PlotDescription_TP description);
    
    void SetGain(const float gain);

private:
    //! the data this model manages
    std::vector<OGLSweepChart_C<ModelDataType_TP >*> _plots;

    std::atomic<float> _sig_gain;
};
