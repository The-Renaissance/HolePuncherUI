#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "serialconfigdialog.h"
#include <QMainWindow>

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
    void on_configButton_clicked();

private:
    Ui::MainWindow *ui;
    SerialConfigDialog configDialog;
};
#endif // MAINWINDOW_H
