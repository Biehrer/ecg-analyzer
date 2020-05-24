#pragma once

#include "../includes/visualization/ogl_plot_renderer_widget.h"

#include <qobject.h>
#include <QAbstractTableModel>
#include <qvariant.h>

class PlotModel_C : public QAbstractTableModel
{
    Q_OBJECT

public:
    PlotModel_C( QObject *parent = nullptr);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void SetDataPtr(QOpenGLPlotRendererWidget* data);

private:
    QOpenGLPlotRendererWidget* _data = nullptr;
};
