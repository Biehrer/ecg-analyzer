#include "load_signal_dialog.h"

LoadSignalDialog_C::LoadSignalDialog_C(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadSignalDialog_C)
{
    ui->setupUi(this);

    //this->setLayout(ui->_main_grid_layout)
    ui->_tree_view_loaded_signals->setLayout(ui->_tree_view_layout_horizontal);
    //ui.->setLayout(ui._grid_layout_page_signals);

    connect(ui->_btn_load_from_disk, SIGNAL(clicked()), this, SLOT(OnBtnLoadFromDisk()));
    connect(ui->_btn_load_serial_port, SIGNAL(clicked()), this, SLOT(OnBtnCreateSerialport()));
    connect(ui->_btn_cancel, SIGNAL(clicked()), this, SLOT(OnBtnLoadFromDisk()));
    connect(ui->_btn_ok, SIGNAL(clicked()), this, SLOT(OnBtnOk()));
    connect(ui->_btn_remove_signal, SIGNAL(clicked()), this, SLOT(OnBtnRemoveCurrent()));

    //_signal_creator = new CreateSignalFromFileWidget_C(this);
    //_signal_creator->hide();

}

LoadSignalDialog_C::~LoadSignalDialog_C()
{
    delete ui;
}

void LoadSignalDialog_C::OnBtnLoadFromDisk()
{
    _signal_creator = new CreateSignalFromFileWidget_C();
    _signal_creator->show();
    // Create a new dialog(D) in which we can specify:
    // via checkbox / radiobox:
    // - what datatype the signal should be loaded ( float, int, double )
    // - MIT signal file (phsyionet database) or G11 Data or CUSTOM Data(Todo for later, specify a custom format which can be loaded)
    //
    // Inside the new dialoue(D) we can open the file-dialog Afterr all this
    // specifications are defined and create the TimeSignal:C with this specifications
}

void LoadSignalDialog_C::OnBtnCreateSerialport()
{
}

void LoadSignalDialog_C::OnBtnOk()
{
}

void LoadSignalDialog_C::OnBtnRemoveCurrent()
{
}
