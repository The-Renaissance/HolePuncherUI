#ifndef SERIALCONFIGDIALOG_H
#define SERIALCONFIGDIALOG_H

#include <QDialog>
#include "serialconfig.h"

namespace Ui {
class SerialConfigDialog;
}

class SerialConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SerialConfigDialog(QWidget *parent = nullptr);
    ~SerialConfigDialog();

    SerialConfig getSerialConfig();

private slots:
    void validate();

private:
    Ui::SerialConfigDialog *ui;
    SerialConfig serialConfig;
};

#endif // SERIALCONFIGDIALOG_H
