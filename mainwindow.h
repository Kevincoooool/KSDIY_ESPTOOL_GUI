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
    
    // 规范化函数命名，使用驼峰式
    void burnApp();
    void burnBoot();
    void getDefaultMac();
    void getCustomMac();
    void resetBoard();
    void burnFlashKey();
    void burnFlashBlock();
    void burnAll();
    void burnMac();
    void eraseFlash();
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
    
    // 规范化变量命名，使用驼峰式
    QString filePath;
    QString keyFilePath;
    QString filePathApp;
    QString filePathBoot;
    QString addressApp;
    QString addressBoot;
};

#endif // MAINWINDOW_H
