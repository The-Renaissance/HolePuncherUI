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

private slots:
    void validate();
    void dataBitsChanged(int index);
    void stopBitsChanged(int index);
    void parityChanged(int index);

signals:
    void sendConfig(const SerialConfig& config);

private:
    Ui::SerialConfigDialog *ui;
    SerialConfig serialConfig;
};

#endif // SERIALCONFIGDIALOG_H
