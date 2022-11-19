#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtSerialPort/QSerialPort>
#include <QProcess>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void loadPorts();
    void browseBootFile();
    void browseAppFile();
    void browseKeyFile();

void Burn_APP();
    void Burn_BOOT();
    void doGetDefMac();
     void doGetCusMac();
    void Reset_Board();
    void Burn_flash_Key();
    void Burn_flash_Block();
void Burn_All();
    void Burn_Mac();
 void erase_flash();
    void closeSerialPort();
    void openSerialPort();
    void readData();
    void setDTR(bool state);
    void setRTS(bool state);

private:
    Ui::MainWindow *ui;

    void setInputsDisabled(bool disabled);
    void startProcess(QString program, QStringList arguments, uint8_t isTest);

    void handleSerialError(QSerialPort::SerialPortError error);

    QSerialPort *serial;
    QProcess *process;
    QString filePath;
    QString keyfilePath;

    QString FilePath_APP;
    QString FilePath_BOOT;
    QString testAddress_APP;
    QString testAddress_BOOT;

};

#endif // MAINWINDOW_H
