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
    QValidator *validator = new QIntValidator(0, 115200, this);
    ui->baudRateLineEdit->setValidator(validator);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SerialConfigDialog::validate);
    connect(this, &SerialConfigDialog::accepted, this, [this](){emit sendConfig(serialConfig);});
    connect(ui->dataBitsComboBox, &QComboBox::currentIndexChanged, this, &SerialConfigDialog::dataBitsChanged);
    connect(ui->stopBitsComboBox, &QComboBox::currentIndexChanged, this, &SerialConfigDialog::stopBitsChanged);
    connect(ui->parityComboBox, &QComboBox::currentIndexChanged, this, &SerialConfigDialog::parityChanged);
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
    accept();
}

void SerialConfigDialog::dataBitsChanged(int index)
{
    constexpr QSerialPort::DataBits dataBitOptions[] =
    {
        QSerialPort::DataBits::Data5,
        QSerialPort::DataBits::Data6,
        QSerialPort::DataBits::Data7,
        QSerialPort::DataBits::Data8,
    };
    if (index < 0) return;
    serialConfig.databits = dataBitOptions[index];
}

void SerialConfigDialog::stopBitsChanged(int index)
{
    constexpr QSerialPort::StopBits dataBitOptions[] =
    {
        QSerialPort::StopBits::OneStop,
        QSerialPort::StopBits::OneAndHalfStop,
        QSerialPort::StopBits::TwoStop,
    };
    if (index < 0) return;
    serialConfig.stopbits = dataBitOptions[index];
}

void SerialConfigDialog::parityChanged(int index)
{
    constexpr QSerialPort::Parity parityOptions[] =
    {
        QSerialPort::Parity::NoParity,
        QSerialPort::Parity::EvenParity,
        QSerialPort::Parity::OddParity,
        QSerialPort::Parity::SpaceParity,
        QSerialPort::Parity::MarkParity,
    };
    if (index < 0) return;
    serialConfig.parity = parityOptions[index];
}
