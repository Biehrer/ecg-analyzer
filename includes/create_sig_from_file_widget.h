#ifndef CREATE_SIG_FROM_FILE_WIDGET_H
#define CREATE_SIG_FROM_FILE_WIDGET_H

#include "ui_create_sig_from_file_widget.h"

#include "../includes/signal_proc_lib/time_signal.h"

#include <QWidget>
#include <qfiledialog.h>

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

public:

public slots:
    void OnBtnSelectNLoad();

private:
    Ui::CreateSignalFromFileWidget_C *ui;
    template< typename DataType_TP>
    TimeSignal_C<DataType_TP> CreateSignal(const QString & filepath, SignalFileType_TP FileDataType_TP);
};

#endif // CREATE_SIG_FROM_FILE_WIDGET_H
