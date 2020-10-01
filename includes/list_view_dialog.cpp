#include "list_view_dialog.h"

ListViewDialog_C::ListViewDialog_C(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ListViewDialog_C)
{
    ui->setupUi(this);
    _button_ok = ui->_button_box->button(QDialogButtonBox::FirstButton);
    _button_cancel = ui->_button_box->button(QDialogButtonBox::LastButton);

    //ui->_button_box->
    connect(ui->_button_box, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->_button_box, SIGNAL(rejected()), this, SLOT(reject()));
    //connect(, SIGNAL(clicked()), this, SLOT(OnButtonOK()));
    //connect(ui->_button_box->button(QDialogButtonBox::LastButton), SIGNAL(clicked()), this, SLOT(OnButtonCancel()));
}

ListViewDialog_C::~ListViewDialog_C()
{
    delete ui;
}

void ListViewDialog_C::SetSignalModel(SignalModel_C * model)
{
    ui->_list_view->setModel(model);
    connect(ui->_button_box, SIGNAL(accepted()), this, SLOT(OnButtonOK()));
}

void ListViewDialog_C::OnButtonCancel()
{
    std::cout << "going out" << std::endl;
}

void ListViewDialog_C::OnButtonOK()
{
    emit SignalSelected( ui->_list_view->currentIndex().row() );
}
