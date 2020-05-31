#include "load_signal_dialog.h"

LoadSignalDialog_C::LoadSignalDialog_C(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadSignalDialog_C)
{
    ui->setupUi(this);


    connect(ui->_btn_load_from_disk, SIGNAL(clicked()), this, SLOT(OnBtnLoadFromDisk()));
    connect(ui->_btn_load_serial_port, SIGNAL(clicked()), this, SLOT(OnBtnCreateSerialport()));
    connect(ui->_btn_cancel, SIGNAL(clicked()), this, SLOT(OnBtnLoadFromDisk()));
    connect(ui->_btn_ok, SIGNAL(clicked()), this, SLOT(OnBtnOk()));
    connect(ui->_btn_remove_signal, SIGNAL(clicked()), this, SLOT(OnBtnRemoveCurrent()));

    _signal_creator = new CreateSignalFromFileWidget_C();
    connect(_signal_creator, SIGNAL(NewSignalCreated(TimeSignal_C<float>)), this, SLOT(OnNewSignalCreated(TimeSignal_C<float>)));
    connect(_signal_creator, SIGNAL(NewSignalCreated(TimeSignal_C<int>)), this, SLOT(OnNewSignalCreated(TimeSignal_C<int>)));
    connect(_signal_creator, SIGNAL(NewSignalCreated(TimeSignal_C<double>)), this, SLOT(OnNewSignalCreated(TimeSignal_C<double>)));
}


void 
LoadSignalDialog_C::SetModel(SignalModel_C* model) 
{
    ui->_tree_view_loaded_signals->setModel(model);
}

LoadSignalDialog_C::~LoadSignalDialog_C()
{
    delete ui;
    delete _signal_creator;
}

void LoadSignalDialog_C::OnBtnLoadFromDisk()
{
    _signal_creator->show();
}

void LoadSignalDialog_C::OnBtnCreateSerialport()
{
}

void LoadSignalDialog_C::OnBtnOk()
{
    _signal_creator->hide();
}

void 
LoadSignalDialog_C::OnBtnRemoveCurrent()
{
}

void 
LoadSignalDialog_C::OnNewSignalCreated(TimeSignal_C<float> signal)
{
    std::cout << "created float signal" << std::endl;
    _signal_creator->hide();
    emit NewSignal(signal);
}

void
LoadSignalDialog_C::OnNewSignalCreated(TimeSignal_C<int> signal)
{
    std::cout << "created int signal" << std::endl;
    _signal_creator->hide();
    emit NewSignal(signal);
}
void
LoadSignalDialog_C::OnNewSignalCreated(TimeSignal_C<double> signal)
{
    std::cout << "created double signal" << std::endl;
    _signal_creator->hide();
    emit NewSignal(signal);
}
