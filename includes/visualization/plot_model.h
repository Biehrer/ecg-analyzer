#pragma once

// Project includes
#include "ogl_sweep_chart.h"

// Qt includes
#include <QAbstractTableModel>
#include <qobject.h>
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

enum OGLPlotProperty_TP {
    PLOT_ID,
    PLOT_LABEL,
    PLOT_TIMERANGE,
    PLOT_YMAX,
    PLOT_YMIN,
    PLOT_MAJTICK_X,
    PLOT_MAJTICK_Y,
    PLOT_NOT_DEFINED
};
// Register, to make it work with signals & slots
Q_DECLARE_METATYPE(OGLPlotProperty_TP);

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
    QVector3D _fiducial_mark_color = QVector3D(0.0f, 0.0f, 1.0f); // blue
};

//! All the info required to initialize a plot
struct PlotDescription_TP
{
    std::string _label = "unnamed";
    OGLChartGeometry_C _geometry;
    PlotColors_TP _colors;
    DrawingStyle_TP _chart_type = DrawingStyle_TP::LINE_SERIES;
    RingBufferSize_TP _input_buffer_size = RingBufferSize_TP::Size32768;

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
    ~PlotModel_C();

public:
    /**************
    * End Testing
    ***************/
    //! Removes a plot from the view
    void RemovePlot(unsigned int plot_id);

    bool RemovePlot(const std::string& plot_label);

    void AddPlot(const PlotDescription_TP & plot_info);

    // Creates a copy of the plot in the argument and adds it to the plot-screen
    void AddPlot(/*const*/ OGLSweepChart_C<ModelDataType_TP>& plot);

    unsigned int GetNumberOfPlots();

    //! Fast initialization of plots in a horizontal layout (shared timerange and max/min y values)
    //! Creates OGLSweepCharts
    bool InitializePlots(int number_of_plots,
        int view_width,
        int view_height,
        int time_range_ms,
        const std::vector<std::pair<ModelDataType_TP, ModelDataType_TP>>& y_ranges);

    OGLSweepChart_C<ModelDataType_TP>* GetPlotPtr(unsigned int plot_idx);

    OGLSweepChart_C<ModelDataType_TP>* GetPlotPtr(const std::string & plot_label);

    //! Adds a plot to the view
    //! Writes PlotDescription_TP to _data as string -> implement to String method 
    //void AddPlot(PlotDescription_TP description);

    void SetGain(const float gain);

    void ClearPlotSurfaces();

    std::vector<OGLSweepChart_C<ModelDataType_TP >*>& Data();
    const std::vector<OGLSweepChart_C<ModelDataType_TP >*>& constData() const;
    
    //std::list<OGLSweepChart_C<ModelDataType_TP >*>& Data();
    //const std::list<OGLSweepChart_C<ModelDataType_TP >*>& constData() const;

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


signals:
    void NewChangeRequest(int plot_id, const OGLPlotProperty_TP& type, const QVariant& value);

private:
    //! the data this model manages
    //std::list<OGLSweepChart_C<ModelDataType_TP >*>* _plots;
    std::vector<OGLSweepChart_C<ModelDataType_TP >*> _plots;

    std::atomic<float> _sig_gain;
};
