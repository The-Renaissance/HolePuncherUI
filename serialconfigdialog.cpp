#include "serialconfigdialog.h"
#include "ui_serialconfigdialog.h"
#include <QMessageBox>

SerialConfigDialog::SerialConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SerialConfigDialog)
{
    ui->setupUi(this);
    ui->dataBitsComboBox->addItems(
        {
            "5 bits",
            "6 bits",
            "7 bits",
            "8 bits",
        });
    ui->stopBitsComboBox->addItems(
        {
            "1 bit",
            "1.5 bits",
            "2 bits",
        });
    ui->parityComboBox->addItems(
        {
            "No Parity",
            "Even",
            "Odd",
            "Space",
            "Mark",
        });
    ui->flowControlComboBox->addItems(
        {
            "No flow control",
            "RTS/CTS",
            "XON/XOFF",
        });
    QValidator *validator = new QIntValidator(0, 115200, this);
    ui->baudRateLineEdit->setValidator(validator);
    connect(this, &SerialConfigDialog::accepted, this, &SerialConfigDialog::validate);
}

SerialConfigDialog::~SerialConfigDialog()
{
    delete ui;
}

// Make sure to validate the input parameters after clicking "OK"
void SerialConfigDialog::validate()
{
    bool ok = false;
    qint32 baudrate = ui->baudRateLineEdit->text().toInt(&ok);
    if (!ok)
    {
        QMessageBox::warning(this, "Warning", "Baud rate is not a valid number.");
        return;
    }
    serialConfig.baudrate = baudrate;
    serialConfig.name = ui->portLocationLineEdit->text();
    constexpr QSerialPort::DataBits dataBitOptions[] =
        {
            QSerialPort::DataBits::Data5,
            QSerialPort::DataBits::Data6,
            QSerialPort::DataBits::Data7,
            QSerialPort::DataBits::Data8,
        };
    int index = ui->dataBitsComboBox->currentIndex();
    if (index >= 0) serialConfig.databits = dataBitOptions[index];
    constexpr QSerialPort::StopBits stopBitOptions[] =
        {
            QSerialPort::StopBits::OneStop,
            QSerialPort::StopBits::OneAndHalfStop,
            QSerialPort::StopBits::TwoStop,
        };
    index = ui->dataBitsComboBox->currentIndex();
    if (index >= 0) serialConfig.stopbits = stopBitOptions[index];
    constexpr QSerialPort::Parity parityOptions[] =
        {
            QSerialPort::Parity::NoParity,
            QSerialPort::Parity::EvenParity,
            QSerialPort::Parity::OddParity,
            QSerialPort::Parity::SpaceParity,
            QSerialPort::Parity::MarkParity,
        };
    index = ui->parityComboBox->currentIndex();
    if (index >= 0) serialConfig.parity = parityOptions[index];
    constexpr QSerialPort::FlowControl flowControlOptions[] =
        {
            QSerialPort::FlowControl::NoFlowControl,
            QSerialPort::FlowControl::HardwareControl,
            QSerialPort::FlowControl::SoftwareControl,
        };
    index = ui->flowControlComboBox->currentIndex();
    if (index >= 0) serialConfig.flowControl = flowControlOptions[index];
}

SerialConfig SerialConfigDialog::getSerialConfig()
{
    return serialConfig;
}
