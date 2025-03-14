#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "serialconfigdialog.h"
#include <QMainWindow>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSerialError(QSerialPort::SerialPortError err);
    void onGenericError(const QString& err);
    void openFileDialog();
    void onPunchFinish();
    void onStartClicked();

signals:
    void startWorker(const QString& filename, const SerialConfig& config);

private:
    Ui::MainWindow *ui;
    SerialConfigDialog configDialog;
    QThread serialThread;
    QString filename;
};
#endif // MAINWINDOW_H
