#ifndef LOAD_SIGNAL_DIALOG_H
#define LOAD_SIGNAL_DIALOG_H

#include "ui_loadsignaldialog.h"

// Projects includes
#include "create_sig_from_file_widget.h"
#include "signal_model.h"

// Qt includes
#include <QDialog>
#include <qfiledialog.h>

namespace Ui {
class LoadSignalDialog_C;
}

// The controller with a view ( tree view widget so we can display all channels)
class LoadSignalDialog_C : public QDialog
{
    Q_OBJECT

public:
    explicit LoadSignalDialog_C(QWidget *parent = 0);
    
    ~LoadSignalDialog_C();

public:
    void SetModel(SignalModel_C * model);

public slots:
    void OnBtnLoadFromDisk();

    void OnBtnCreateSerialport();

    void OnBtnOk();

    void OnBtnRemoveCurrent();

    void OnNewSignalCreated(TimeSignal_C<float> signal);

    void OnNewSignalCreated(TimeSignal_C<double> signal);

    void OnNewSignalCreated(TimeSignal_C<int> signal);

signals:
    void NewSignal(TimeSignal_C<int> signal);
    void NewSignal(TimeSignal_C<float> signal);
    void NewSignal(TimeSignal_C<double> signal);

private:
    Ui::LoadSignalDialog_C *ui;

    CreateSignalFromFileWidget_C* _signal_creator;

};

#endif // LOAD_SIGNAL_DIALOG_H
