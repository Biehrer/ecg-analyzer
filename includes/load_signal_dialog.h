#ifndef LOAD_SIGNAL_DIALOG_H
#define LOAD_SIGNAL_DIALOG_H

#include "ui_loadsignaldialog.h"

// Projects includes
#include "create_sig_from_file_widget.h"

// Qt includes
#include <QDialog>
#include <qfiledialog.h>

namespace Ui {
class LoadSignalDialog_C;
}

class LoadSignalDialog_C : public QDialog
{
    Q_OBJECT

public:
    explicit LoadSignalDialog_C(QWidget *parent = 0);
    ~LoadSignalDialog_C();

private:
    Ui::LoadSignalDialog_C *ui;

    CreateSignalFromFileWidget_C* _signal_creator;

public slots:
    void OnBtnLoadFromDisk();

    void OnBtnCreateSerialport();

    void OnBtnOk();

    void OnBtnRemoveCurrent();
};

#endif // LOAD_SIGNAL_DIALOG_H
