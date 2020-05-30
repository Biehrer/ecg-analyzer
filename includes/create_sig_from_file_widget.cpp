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
CreateSignalFromFileWidget_C::OnBtnSelectNLoad() {

    auto filepath = QFileDialog::getOpenFileName(this,
        tr("Open Signal"), "c/", tr("Signal Files (*.dat *.hea)"));

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
    }
    else if ( ui->_radio_float->isChecked() ) {
        signal_datatype = SignalDataType_TP::FLOAT_TYPE;
        auto signal = CreateSignal<float>(filepath, file_type);

    }
    else if ( ui->_radio_int->isChecked() ) {
        signal_datatype = SignalDataType_TP::INT_TYPE;
        auto signal = CreateSignal<int>(filepath, file_type);
    }

    // Store the signals?

}



template<typename DataType_TP>
TimeSignal_C<DataType_TP>
CreateSignalFromFileWidget_C::CreateSignal(const QString& filepath, 
    SignalFileType_TP file_type)
{
    TimeSignal_C<DataType_TP> signal;

    if ( file_type == SignalFileType_TP::PHYSIONET ) {
        signal.ReadG11Data(filepath.toStdString());

    } else if ( file_type == SignalFileType_TP::G11 ) {
        signal.LoadFromMITFileFormat(filepath.toStdString());
    }
    // TODO MOVE CTOR
       return signal;
}