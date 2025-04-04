#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialworker.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , configDialog(this)
{
    ui->setupUi(this);

    defaultStyle = ui->label->styleSheet();

    SerialWorker *worker = new SerialWorker;
    worker->moveToThread(&serialThread);
    connect(&serialThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(ui->configButton, &QPushButton::clicked, &configDialog, &SerialConfigDialog::show);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(ui->stopButton, &QPushButton::clicked, this, [this](){serialThread.requestInterruption(); ui->statusLabel->setText(QStringLiteral("Stopping punching operation..."));});
    connect(ui->fileButton, &QPushButton::clicked, this, &MainWindow::openFileDialog);
    // Both editing the input box and selecting a file via "Browse" chooses the file
    connect(ui->filePathLineEdit, &QLineEdit::editingFinished, this, [this](){auto name = ui->filePathLineEdit->text();
            if (!name.isEmpty()) filename = name;});

    // connect(worker, &SerialWorker::serialError, this, &MainWindow::onSerialError);
    connect(worker, &SerialWorker::error, this, &MainWindow::onGenericError);
    connect(worker, &SerialWorker::finished, this, &MainWindow::onPunchFinish);
    connect(this, &MainWindow::startWorker, worker, &SerialWorker::start);
    connect(worker, &SerialWorker::stopped, this, &MainWindow::onPunchStop);

    serialThread.start();
}

MainWindow::~MainWindow()
{
    serialThread.quit();
    serialThread.wait();
    delete ui;
}

void MainWindow::onSerialError(QSerialPort::SerialPortError err)
{
    static const QString errText[] =
    {
        "No error occured",
        "Serial port does not exist",
        "Permission error",
        "Open error",
        "Error when attempting to write data",
        "Error when attempting to read data",
        "Resource error, check connection",
        "An operation that is unsupported has been attempted",
        "Unknown error",
        "Timeout",
        "Serial port was not open",
    };
    ui->statusLabel->setText("Error: " + errText[err]);
    ui->statusLabel->setStyleSheet("QLabel {color: red}");
    ui->startButton->setEnabled(true);
}

void MainWindow::onGenericError(const QString& err)
{
    ui->statusLabel->setText("Error!");
    ui->statusLabel->setStyleSheet("QLabel {color: red}");
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    QMessageBox::critical(this, "Error", err);
}

void MainWindow::onPunchFinish()
{
    ui->statusLabel->setText("Ready to connect");
    ui->statusLabel->setStyleSheet(defaultStyle);
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

void MainWindow::openFileDialog()
{
    QString filename = QFileDialog::getOpenFileName(this,
                "Open SVG Image", QString(), "Images (*.svg)");
    if (!filename.isEmpty()) {
        ui->filePathLineEdit->setText(filename);
        this->filename = filename;
    }
}

void MainWindow::onStartClicked()
{
    ui->statusLabel->setText("Punch in progress...");
    ui->statusLabel->setStyleSheet(defaultStyle);
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    emit startWorker(filename, configDialog.getSerialConfig());
}

void MainWindow::onPunchStop()
{
    ui->statusLabel->setText("Operation stopped.");
    ui->statusLabel->setStyleSheet("QLabel {color: red}");
    ui->stopButton->setEnabled(false);
    ui->startButton->setEnabled(true);
}
