#include "create_sig_from_file_widget.h"

CreateSignalFromFileWidget_C::CreateSignalFromFileWidget_C(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreateSignalFromFileWidget_C)
{
    ui->setupUi(this);

    connect(ui->_btn_select_n_load, SIGNAL(clicked()), this, SLOT(OnBtnSelectNLoad()));
}

CreateSignalFromFileWidget_C::~CreateSignalFromFileWidget_C() 
{
    delete ui;
}

void 
CreateSignalFromFileWidget_C::OnBtnSelectNLoad() 
{

    auto filepath = QFileDialog::getOpenFileName(this,
        tr("Open Signal"), "C:/", tr("Signal Files (*.dat *.hea)"));

    //if ( !QDir(filepath).exists() /*|| !QDir(filepath).isReadable()*/ ) {
    //    //QMessageBoxPrivate("Directory does not exist");
    //    return;
    //}

    SignalFileType_TP file_type;
    if ( ui->_radio_g11->isChecked() ) {
        // load g11 file
        file_type = SignalFileType_TP::G11;
    } else if (ui->_radio_phsyionet->isChecked() ) {
        // load physionet
        file_type = SignalFileType_TP::PHYSIONET;
    }

    SignalDataType_TP signal_datatype;
    if ( ui->_radio_double->isChecked() ) {
        signal_datatype = SignalDataType_TP::DOUBLE_TYPE;
        auto signal =  CreateSignal<double>(filepath, file_type);
        emit NewSignalCreated(signal);
    } else if ( ui->_radio_float->isChecked() ) {
        signal_datatype = SignalDataType_TP::FLOAT_TYPE;
        auto signal = CreateSignal<float>(filepath, file_type);
        emit NewSignalCreated(signal);
    } else if ( ui->_radio_int->isChecked() ) {
        signal_datatype = SignalDataType_TP::INT_TYPE;
        auto signal = CreateSignal<int>(filepath, file_type);
        emit NewSignalCreated(signal);
    }

    // default
    signal_datatype = SignalDataType_TP::FLOAT_TYPE;
    auto signal = CreateSignal<float>(filepath, file_type);
    emit NewSignalCreated(signal);
}

template<typename DataType_TP>
TimeSignal_C<DataType_TP>
CreateSignalFromFileWidget_C::CreateSignal(QString& filepath, 
    SignalFileType_TP file_type)
{
    TimeSignal_C<DataType_TP> signal;

    std::string path = filepath.toStdString();

    if ( file_type == SignalFileType_TP::PHYSIONET ) {
        for ( auto& letter : path ) {
            if ( letter == '/' ) {
                letter = '\\';
            }
        }
        signal.LoadFromMITFileFormat(path);

    } else if ( file_type == SignalFileType_TP::G11 ) {
        // Not tested yet
        for ( auto& letter : path ) {
            if ( letter == '/' ) {
                letter = '//';
            }
        }        
        signal.ReadG11Data(path);
    }

    // TODO MOVE CTOR for TimeSignal_C
       return signal;
}