#ifndef LIST_VIEW_DIALOG_H
#define LIST_VIEW_DIALOG_H

#include "ui_list_view_dialog.h"

// Project includes
#include "signal_model.h"

// Qt includes
#include <qdialogbuttonbox.h>
#include <QDialog>

namespace Ui {
class ListViewDialog_C;
}

class ListViewDialog_C : public QDialog
{
    Q_OBJECT

public:
    explicit ListViewDialog_C(QWidget *parent = 0);
    ~ListViewDialog_C();

    // Public access methods
public:
    void SetSignalModel(SignalModel_C* model);

public slots:
    void OnButtonOK();

    void OnButtonCancel();

signals:
    void SignalSelected(unsigned int);

private:
    Ui::ListViewDialog_C *ui;

    QPushButton* _button_ok;
    
    QPushButton* _button_cancel;
};

#endif // LIST_VIEW_DIALOG_H
