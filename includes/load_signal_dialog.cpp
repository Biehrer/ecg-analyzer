#include "load_signal_dialog.h"

LoadSignalDialog_C::LoadSignalDialog_C(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadSignalDialog_C)
{
    ui->setupUi(this);
    // Connect buttons
    connect(ui->_btn_load_from_disk, SIGNAL(clicked()), this, SLOT(OnBtnLoadFromDisk()));
    connect(ui->_btn_load_serial_port, SIGNAL(clicked()), this, SLOT(OnBtnCreateSerialport()));
    connect(ui->_btn_cancel, SIGNAL(clicked()), this, SLOT(OnBtnLoadFromDisk()));
    connect(ui->_btn_ok, SIGNAL(clicked()), this, SLOT(OnBtnOk()));
    connect(ui->_btn_remove_signal, SIGNAL(clicked()), this, SLOT(OnBtnRemoveCurrent()));

    // Create and connect file reader widget
    _signal_from_file_factory = new CreateSignalFromFileWidget_C();
    connect(_signal_from_file_factory, SIGNAL(NewSignalCreated(TimeSignal_C<float>)), 
        this, SLOT(OnNewSignalCreated(TimeSignal_C<float>)), Qt::ConnectionType::UniqueConnection);

    connect(_signal_from_file_factory, SIGNAL(NewSignalCreated(TimeSignal_C<int>)), 
        this, SLOT(OnNewSignalCreated(TimeSignal_C<int>)), Qt::ConnectionType::UniqueConnection);
    
    connect(_signal_from_file_factory, SIGNAL(NewSignalCreated(TimeSignal_C<double>)), 
        this, SLOT(OnNewSignalCreated(TimeSignal_C<double>)), Qt::ConnectionType::UniqueConnection);
    // Create and connect Serial port signal factory ... 
    // ...
    // ui->_tree_view_loaded_signals
}

void 
LoadSignalDialog_C::SetTreeViewModel(SignalModel_C* model) 
{
    // Todo: create a tree model (different model approach) so we can give some info about all the channels as child items
    ui->_tree_view_loaded_signals->setModel(model);
}

LoadSignalDialog_C::~LoadSignalDialog_C()
{
    delete ui;
    delete _signal_from_file_factory;
}

void LoadSignalDialog_C::OnBtnLoadFromDisk()
{
    _signal_from_file_factory->show();
}

void LoadSignalDialog_C::OnBtnCreateSerialport()
{
}

void LoadSignalDialog_C::OnBtnOk()
{
    _signal_from_file_factory->hide();
}

void 
LoadSignalDialog_C::OnBtnRemoveCurrent()
{
    // emit signal to jones_plot_app so the signal can be removed
    emit RemoveSignalRequested( ui->_tree_view_loaded_signals->currentIndex().row() );
}

void 
LoadSignalDialog_C::OnNewSignalCreated(TimeSignal_C<float> signal)
{
    _signal_from_file_factory->hide();
    PreProcessSignalHeader(signal);
    emit NewSignal(signal);
}

void
LoadSignalDialog_C::OnNewSignalCreated(const TimeSignal_C<int> signal)
{
    _signal_from_file_factory->hide();
    emit NewSignal(signal);
}

void
LoadSignalDialog_C::OnNewSignalCreated(const TimeSignal_C<double> signal)
{
    _signal_from_file_factory->hide();
    emit NewSignal(signal);
}

void LoadSignalDialog_C::PreProcessSignalHeader(TimeSignal_C<float>& signal) 
{
    signal.SetID( _current_signal_id.load() );
    signal.SetLabel("signal #" + std::to_string(_current_signal_id.load()));
    _current_signal_id.store(_current_signal_id.load() + 1);
}