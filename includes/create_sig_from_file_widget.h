#ifndef CREATE_SIG_FROM_FILE_WIDGET_H
#define CREATE_SIG_FROM_FILE_WIDGET_H

#include "ui_create_sig_from_file_widget.h"

#include "../includes/signal_proc_lib/time_signal.h"

#include <QWidget>
#include <qfiledialog.h>
#include <qgroupbox.h>
enum SignalFileType_TP {
    G11,
    PHYSIONET
};

enum SignalDataType_TP {
    INT_TYPE,
    FLOAT_TYPE,
    DOUBLE_TYPE
};

namespace Ui {
class CreateSignalFromFileWidget_C;
}

class CreateSignalFromFileWidget_C : public QWidget
{
    Q_OBJECT

public:
    explicit CreateSignalFromFileWidget_C(QWidget *parent = 0);
    ~CreateSignalFromFileWidget_C();

public slots:
    void OnBtnSelectNLoad();

signals:
    void NewSignalCreated(TimeSignal_C<float> signal);
    void NewSignalCreated(TimeSignal_C<double> signal);
    void NewSignalCreated(TimeSignal_C<int> signal);

private:
    template< typename DataType_TP>
    TimeSignal_C<DataType_TP> CreateSignal(QString & filepath, SignalFileType_TP FileDataType_TP);


private:
    Ui::CreateSignalFromFileWidget_C *ui;

};

#endif // CREATE_SIG_FROM_FILE_WIDGET_H
