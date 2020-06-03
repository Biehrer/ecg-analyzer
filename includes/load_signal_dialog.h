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
    void SetTreeViewModel(SignalModel_C * model);

public slots:
    void OnBtnLoadFromDisk();

    void OnBtnCreateSerialport();

    void OnBtnOk();

    void OnBtnRemoveCurrent();

    void OnNewSignalCreated(TimeSignal_C<float> signal);

    void OnNewSignalCreated(const TimeSignal_C<double> signal);

    void OnNewSignalCreated(const TimeSignal_C<int> signal);

signals:
    void NewSignal(const TimeSignal_C<int> signal);
    void NewSignal(const TimeSignal_C<float> signal);
    void NewSignal(const TimeSignal_C<double> signal);

    void RemoveSignalRequested(unsigned int);

private:
    void PreProcessSignalHeader(TimeSignal_C<float>& signal);

private:
    Ui::LoadSignalDialog_C *ui;

    CreateSignalFromFileWidget_C* _signal_from_file_factory;

    std::atomic<unsigned int> _current_signal_id = 0;
};

#endif // LOAD_SIGNAL_DIALOG_H
