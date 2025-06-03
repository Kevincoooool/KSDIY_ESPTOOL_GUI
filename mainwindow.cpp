
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QtSerialPort/QSerialPortInfo>

void Delay_MSec_Suspend(unsigned int msec)
{
    QTime _Timer = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < _Timer)
        ;
}
void msleep(quint32 msec)
{
    QEventLoop loop;                               // 定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit())); // 创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();                                   // 事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serial = new QSerialPort(this);

    // Fill baud box
    ui->baudBox->clear();
    ui->baudBox->addItem(QStringLiteral("1200"), QSerialPort::Baud1200);
    ui->baudBox->addItem(QStringLiteral("2400"), QSerialPort::Baud2400);
    ui->baudBox->addItem(QStringLiteral("4800"), QSerialPort::Baud4800);
    ui->baudBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudBox->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    ui->baudBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudBox->addItem(QStringLiteral("921600"), QSerialPort::Baud921600);
    ui->baudBox->addItem(QStringLiteral("2000000"), QSerialPort::Baud2000000);
    ui->baudBox->setCurrentIndex(9); // Default 115200

    ui->closeSerialBtn->setDisabled(true);
    ui->outputText->setReadOnly(true);

    loadPorts();
    keyFilePath = "";
    filePath = "";
    filePathApp = "";
    filePathBoot = "";
    addressApp = "0xf000";
    addressBoot = "0x1000";

    connect(ui->eraseBtn, &QPushButton::clicked,
            this, &MainWindow::eraseFlash);
    connect(ui->reloadBtn, &QPushButton::clicked,
            this, &MainWindow::loadPorts);
    connect(ui->browseBoot, &QPushButton::clicked,
            this, &MainWindow::browseBootFile);

    connect(ui->browseApp, &QPushButton::clicked,
            this, &MainWindow::browseAppFile);
    connect(ui->browseKeyBtn, &QPushButton::clicked,
            this, &MainWindow::browseKeyFile);
    connect(ui->burnBootBtn, &QPushButton::clicked,
            this, &MainWindow::burnBoot);
    connect(ui->burnAppBtn, &QPushButton::clicked,
            this, &MainWindow::burnApp);

    // Serial port setup
    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleSerialError);

    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    connect(ui->openSerialBtn, &QPushButton::clicked,
            this, &MainWindow::openSerialPort);
    connect(ui->closeSerialBtn, &QPushButton::clicked,
            this, &MainWindow::closeSerialPort);
    connect(ui->getDefaultMacBtn, &QPushButton::clicked,
            this, &MainWindow::getDefaultMac);
    connect(ui->getCustomMacBtn, &QPushButton::clicked,
            this, &MainWindow::getCustomMac);
    connect(ui->burnMacBtn, &QPushButton::clicked,
            this, &MainWindow::burnMac);
    connect(ui->burnKeyBtn, &QPushButton::clicked,
            this, &MainWindow::burnFlashKey);
    connect(ui->burnBlockBtn, &QPushButton::clicked,
            this, &MainWindow::burnFlashBlock);
    connect(ui->burnAllBtn, &QPushButton::clicked,
            this, &MainWindow::burnAll);

    //    connect(ui->resetBtn, &QPushButton::clicked,
    //            this, &MainWindow::resetBoard);
    // connect(ui->RTSCheckBox, &QCheckBox::stateChanged, [=](int state)
    //         {
    //     if(!serial->isOpen()) return ;
    //     if((Qt::CheckState)state == Qt::Checked) setRTS(true);
    //     else if((Qt::CheckState)state == Qt::Unchecked) setRTS(false); });
    connect(ui->resetBtn, &QPushButton::clicked, this, [=]()
            {
            setDTR(false);
            setRTS(true);
             msleep(500);
            setRTS(false); });
    // Fetch previous settings
    QCoreApplication::setOrganizationName("makerlab.mx");
    QCoreApplication::setOrganizationDomain("makerlab.mx");
    QCoreApplication::setApplicationName("Huafuu_Tool");
    QSettings settings;
    ui->appAddress->setText(addressApp);
    ui->bootAddress->setText(addressBoot);
    if (settings.contains("settings/port"))
    {
        int portIndex = settings.value("settings/port").toInt();
        ui->portBox->setCurrentIndex(portIndex);
    }

    if (settings.contains("settings/baud"))
    {
        int baudIndex = settings.value("settings/baud").toInt();
        ui->baudBox->setCurrentIndex(baudIndex);
    }

    if (settings.contains("settings/keyfile"))
    {
        keyFilePath = settings.value("settings/keyfile").toString();
        ui->keyFilePath->setText(keyFilePath);
    }
    if (settings.contains("settings/testFile_APP"))
    {
        filePathApp = settings.value("settings/testFile_APP").toString();
        ui->appFilePath->setText(filePathApp);
    }
    if (settings.contains("settings/testFile_BOOT"))
    {
        filePathBoot = settings.value("settings/testFile_BOOT").toString();
        ui->bootFilePath->setText(filePathBoot);
    }
    ui->setMacEdit->setText("00:00:00:00:00:01");
    connect(ui->portBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=](int index)
            {
                QSettings settings;
                settings.setValue("settings/port", index);
            });

    connect(ui->baudBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=](int index)
            {
                QSettings settings;
                settings.setValue("settings/baud", index);
            });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadPorts()
{
    ui->portBox->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->portBox->addItem(info.portName(), info.systemLocation());
    }
}

void MainWindow::browseBootFile()
{
    QSettings settings;
    const QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        filePathBoot = fileName;
        ui->bootFilePath->setText(filePathBoot);
        settings.setValue("settings/testFile_BOOT", filePathBoot);
    }
}

void MainWindow::browseKeyFile()
{
    QSettings settings;
    const QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        keyFilePath = fileName;
        ui->keyFilePath->setText(keyFilePath);
        settings.setValue("settings/keyfile", keyFilePath);
    }
}
void MainWindow::browseAppFile()
{
    QSettings settings;
    const QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        filePathApp = fileName;
        ui->appFilePath->setText(filePathApp);
        settings.setValue("settings/testFile_APP", filePathApp);
    }
}
void MainWindow::setDTR(bool state)
{
    if (serial->isOpen())
        serial->setDataTerminalReady(state);
}

void MainWindow::setRTS(bool state)
{
    if (serial->isOpen())
        serial->setRequestToSend(state);
}
// 将 void MainWindow::erase_flash() 修改为
void MainWindow::eraseFlash()
{
    closeSerialPort();
    ui->outputText->clear();
    ui->outputText->appendPlainText("Erasing flash...\n");
    QString appPath = QCoreApplication::applicationDirPath();
    // QString program = appPath + "/esptool";
    QString program = "esptool.exe";

    qDebug() << "program =" << program;

    QStringList arguments;

    arguments << "-p"
              << ui->portBox->currentText()
              << "erase_flash";

    qDebug() << "arguments =" << arguments;
    //    startProcess(program, arguments, 2);

    // 改成我熟悉的

    process = new QProcess(this);

    connect(process, &QProcess::started, this, [=]()
            { setInputsDisabled(true); });

    connect(process, &QProcess::readyReadStandardError, this, [=]()
            {
            ui->outputText->insertPlainText(process->readAllStandardError());
            ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, &QProcess::readyReadStandardOutput, this, [=]()
            {
                ui->outputText->insertPlainText(process->readAllStandardOutput());
                ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
                ui->outputText->appendPlainText(QString("Finished with exit code ") + QString::number(exitCode));
                setInputsDisabled(false);
                if (exitCode != 0)
                {
                    ui->outputText->appendPlainText("\n擦除失败!!!!!!!!!!");

                    QMessageBox::critical(NULL, "擦除失败！", "擦除失败，请检查COM端口是否正确", QMessageBox::Ok, QMessageBox::Ok);
                }
                else
                {
                    ui->outputText->appendPlainText("\n擦除成功");

                    QMessageBox::information(NULL, "提示", "擦除成功", QMessageBox::Ok, QMessageBox::Ok);
                } });

    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError error)
            {
            ui->outputText->appendPlainText( QString("An error occurred: ") + error );
            setInputsDisabled(false); });

    process->start(program, arguments);
}
// 将 void MainWindow::Burn_BOOT() 修改为
void MainWindow::burnBoot()
{
    closeSerialPort();
    ui->outputText->clear();
    ui->outputText->appendPlainText("Uploading Test Firmware...\n");
    QString appPath = QCoreApplication::applicationDirPath();
    // QString program = appPath + "/esptool";
    QString program = "esptool.exe";

    qDebug() << "program =" << program;

    QStringList arguments;
    // QStringList arguments = "esptool.py -p COM473 -b 2000000 write_flash 0x30000" + testFilePath;
    // arguments << "-vv" << "-cd" << "ck" << "-cb" << ui->baudBox->currentText() << "-cp" << ui->portBox->currentData().toString() << "-cf" << testFilePath;
    arguments << "-b" << ui->baudBox->currentText() << "-p"
              << ui->portBox->currentText()
              << "--after"
              << "no_reset"
              << "--no-stub"
              << "write_flash" << ui->bootAddress->text() << filePathBoot;

    qDebug() << "arguments =" << arguments;
    // startProcess(program, arguments, true);
    // 改成我熟悉的

    process = new QProcess(this);

    connect(process, &QProcess::started, this, [=]()
            { setInputsDisabled(true); });

    connect(process, &QProcess::readyReadStandardError, this, [=]()
            {
            ui->outputText->insertPlainText(process->readAllStandardError());
            ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, &QProcess::readyReadStandardOutput, this, [=]()
            {
                ui->outputText->insertPlainText(process->readAllStandardOutput());
                ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
                ui->outputText->appendPlainText(QString("Finished with exit code ") + QString::number(exitCode));
                setInputsDisabled(false);
                if (exitCode != 0)
                {
                    ui->outputText->appendPlainText("\n烧录BOOT失败!!!!!!!!!!");

                    QMessageBox::critical(NULL, "烧录失败！", "烧录BOOT失败，请重试", QMessageBox::Ok, QMessageBox::Ok);
                }
                else
                {
                    ui->outputText->appendPlainText("\n烧录成功");

                    QMessageBox::information(NULL, "提示", "烧录成功！", QMessageBox::Ok, QMessageBox::Ok);
                    openSerialPort();
                    setDTR(false);
                    setRTS(true);
                    msleep(500);
                    setRTS(false);
                } });

    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError error)
            {
            ui->outputText->appendPlainText( QString("An error occurred: ") + error );
            setInputsDisabled(false); });

    process->start(program, arguments);
}

void MainWindow::getDefaultMac()
{
    closeSerialPort();
    ui->outputText->clear();
    ui->outputText->appendPlainText("Getting MAC...\n");
    QString program = "esptool.exe";
    QStringList arguments;
    arguments << "-p" << ui->portBox->currentText() << "read_mac";

    // startProcess(program, arguments);

    // 改成我熟悉的

    process = new QProcess(this);

    connect(process, &QProcess::started, this, [=]()
            { setInputsDisabled(true); });

    connect(process, &QProcess::readyReadStandardError, this, [=]()
            {
            ui->outputText->insertPlainText(process->readAllStandardError());
            ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, &QProcess::readyReadStandardOutput, this, [=]()
            {
                ui->outputText->insertPlainText(process->readAllStandardOutput());
                ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
                ui->outputText->appendPlainText(QString("Finished with exit code ") + QString::number(exitCode));
                setInputsDisabled(false);
        if (exitCode != 0)
        {
            ui->outputText->appendPlainText("\n获取失败!!!!!!!!!!");

            QMessageBox::critical(NULL, "获取失败！", "获取失败，请重试", QMessageBox::Ok, QMessageBox::Ok);
        }
        else
        {
                // 添加成功提示
                ui->outputText->appendPlainText("获取成功");
                QMessageBox::information(NULL, "提示", "获取成功！", QMessageBox::Ok, QMessageBox::Ok);
                
                QString data = ui->outputText->toPlainText();
                qDebug() << "data =" << data;
                QString Frame_header = "\nMAC: ";   // 查找的字符串
                int a = data.indexOf(Frame_header); // 查找第一个5555aaaa，返回值为2
                qDebug() << "a =" << a;

                QString test = data.mid(a + 6, 17); // 按字节数，截取结果为"5555aaaa000000000"
                ui->defaultMacEdit->setText(test);

                setInputsDisabled(false); } });

    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError error)
            {
            ui->outputText->appendPlainText( QString("An error occurred: ") + error );
            setInputsDisabled(false); });

    process->start(program, arguments);
}

void MainWindow::getCustomMac()
{
    closeSerialPort();
    ui->outputText->clear();
    ui->outputText->appendPlainText("Getting MAC...\n");
    QString program = "espefuse.exe";
    QStringList arguments;
    arguments << "-p" << ui->portBox->currentText() << "get_custom_mac";
    process = new QProcess(this);

    connect(process, &QProcess::started, this, [=]()
            { setInputsDisabled(true); });

    connect(process, &QProcess::readyReadStandardError, this, [=]()
            {
            ui->outputText->insertPlainText(process->readAllStandardError());
            ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, &QProcess::readyReadStandardOutput, this, [=]()
            {
                ui->outputText->insertPlainText(process->readAllStandardOutput());
                ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
        ui->outputText->appendPlainText(QString("Finished with exit code ") + QString::number(exitCode));
        setInputsDisabled(false);
        if (exitCode != 0)
        {
            ui->outputText->appendPlainText("\n获取失败!!!!!!!!!!");

            QMessageBox::critical(NULL, "获取失败！", "获取失败，请重试", QMessageBox::Ok, QMessageBox::Ok);
        }
        else
        {
            ui->outputText->appendPlainText("获取成功");

            QMessageBox::information(NULL, "提示", "获取成功！", QMessageBox::Ok, QMessageBox::Ok);

            QString data = ui->outputText->toPlainText();
            qDebug() << "data =" << data;
            QString Frame_header = "Custom MAC Address: "; // 查找的字符串
            int a = data.indexOf(Frame_header);            // 查找第一个，返回值
            qDebug() << "a =" << a;
            QString test = data.mid(a + 20, 17); // 按字节数，截取结果
            ui->customMacEdit->setText(test);

            setInputsDisabled(false);
        } });

    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError error)
            {
            ui->outputText->appendPlainText( QString("An error occurred: ") + error );
            setInputsDisabled(false); });

    process->start(program, arguments);
}
void MainWindow::burnFlashKey()
{
    closeSerialPort();
    ui->outputText->clear();
    ui->outputText->appendPlainText("开始烧录 key...\n");
    QString program = "espefuse.exe";
    QStringList arguments;
    arguments << "-p" << ui->portBox->currentText()
              << "burn_key"
              << "BLOCK_KEY1" << ui->keyFilePath->text() << "XTS_AES_128_KEY";
    process = new QProcess(this);

    connect(process, &QProcess::started, this, [=]()
            { setInputsDisabled(true); });

    connect(process, &QProcess::readyReadStandardError, this, [=]()
            {
        ui->outputText->insertPlainText(process->readAllStandardError());
        ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, &QProcess::readyReadStandardOutput, this, [=]()
            {
        ui->outputText->insertPlainText(process->readAllStandardOutput());
        ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
                ui->outputText->appendPlainText(QString("Finished with exit code ") + QString::number(exitCode));
                if (exitCode != 0)
                {
                    ui->outputText->appendPlainText("\n烧录KEY失败!!!!!!!!!!");

                    QMessageBox::critical(NULL, "烧录失败！", "烧录KEY失败，请重试", QMessageBox::Ok, QMessageBox::Ok);
                }
                else
                {
                    ui->outputText->appendPlainText("烧录KEY已成功完成");

                    QMessageBox::information(NULL, "提示", "烧录KEY已成功完成", QMessageBox::Ok, QMessageBox::Ok);
                }
                setInputsDisabled(false);
                process->write("BURN\n");
                process->write("BURN\n");
            });

    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError error)
            {
        ui->outputText->appendPlainText( QString("An error occurred: ") + error );
        setInputsDisabled(false); });

    process->start(program, arguments);
    //
    process->write("BURN\n");
    process->waitForFinished();
    burnFlashBlock();
}
void MainWindow::burnAll()
{
    closeSerialPort();
    ui->outputText->clear();
    ui->outputText->appendPlainText("开始烧录 key...\n");
    QString program = "espefuse.exe";
    QStringList arguments;
    arguments << "-p" << ui->portBox->currentText()
              << "burn_key"
              << "BLOCK_KEY1" << ui->keyFilePath->text() << "XTS_AES_128_KEY";
    //    startProcess(program, arguments);
    process = new QProcess(this);

    connect(process, &QProcess::started, this, [=]()
            { setInputsDisabled(true); });

    connect(process, &QProcess::readyReadStandardError, this, [=]()
            {
        ui->outputText->insertPlainText(process->readAllStandardError());
        ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, &QProcess::readyReadStandardOutput, this, [=]()
            {
        ui->outputText->insertPlainText(process->readAllStandardOutput());
        ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
                ui->outputText->appendPlainText(QString("Finished with exit code ") + QString::number(exitCode));
                if (exitCode != 0)
                {
                    ui->outputText->appendPlainText("\n烧录KEY失败!!!!!!!!!!");

                    QMessageBox::critical(NULL, "烧录失败！", "烧录KEY失败，请重试", QMessageBox::Ok, QMessageBox::Ok);
                }
                else
                {
                    ui->outputText->appendPlainText("烧录KEY已成功完成");

                    QMessageBox::information(NULL, "提示", "烧录KEY已成功完成", QMessageBox::Ok, QMessageBox::Ok);
                }
                setInputsDisabled(false);
                process->write("BURN\n");
                process->write("BURN\n");
            });

    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError error)
            {
        ui->outputText->appendPlainText( QString("An error occurred: ") + error );
        setInputsDisabled(false); });

    process->start(program, arguments);
    //
    process->write("BURN\n");
    process->waitForFinished();
    burnFlashBlock();

    burnBoot();
}
void MainWindow::burnFlashBlock()
{
    closeSerialPort();
    ui->outputText->clear();
    ui->outputText->appendPlainText("开始烧录 特征值...\n");
    QString program = "espefuse.exe";
    QStringList arguments;
    arguments << "-p" << ui->portBox->currentText()
              << "burn_block_data"
              << "BLOCK0"
              << "backup_key/blk0.bin"
              << "BLOCK3"
              << "backup_key/blk3.bin"
              << "BLOCK4"
              << "backup_key/blk4.bin"
              << "BLOCK5"
              << "backup_key/blk5.bin"
              << "BLOCK6"
              << "backup_key/blk6.bin"
              << "BLOCK7"
              << "backup_key/blk7.bin"
              << "BLOCK8"
              << "backup_key/blk8.bin"
              << "BLOCK9"
              << "backup_key/blk9.bin"
              << "BLOCK10"
              << "backup_key/blk10.bin"
              << "--force-write-always";
    //    startProcess(program, arguments);
    process = new QProcess(this);

    connect(process, &QProcess::started, this, [=]()
            { setInputsDisabled(true); });

    connect(process, &QProcess::readyReadStandardError, this, [=]()
            {
        ui->outputText->insertPlainText(process->readAllStandardError());
        ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, &QProcess::readyReadStandardOutput, this, [=]()
            {
        ui->outputText->insertPlainText(process->readAllStandardOutput());
        ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
                ui->outputText->appendPlainText(QString("Finished with exit code ") + QString::number(exitCode));
                if (exitCode != 0)
                {
                    ui->outputText->appendPlainText("\n烧录特征值失败!!!!!!!!!!");

                    QMessageBox::critical(NULL, "烧录失败！", "烧录特征值失败，请重试", QMessageBox::Ok, QMessageBox::Ok);
                }
                else
                {
                    ui->outputText->appendPlainText("烧录特征值已成功完成");

                    QMessageBox::information(NULL, "提示", "烧录特征值已成功完成", QMessageBox::Ok, QMessageBox::Ok);
                }
                setInputsDisabled(false);
                process->write("BURN\n");
                process->write("BURN\n");
            });

    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError error)
            {
        ui->outputText->appendPlainText( QString("An error occurred: ") + error );
        setInputsDisabled(false); });

    process->start(program, arguments);
    // process->waitForFinished();
    process->write("BURN\n");
    process->waitForFinished();
}

void MainWindow::burnMac()
{
    closeSerialPort();
    ui->outputText->clear();
    ui->outputText->appendPlainText("Burn Mac...\n");
    //    QString program = "/usr/local/bin/esptool.py"; burn_key BLOCK_KEY1 key.bin XTS_AES_128_KEY
    QString program = "espefuse.exe";
    QStringList arguments;
    //    arguments  << "-p" << ui->portBox->currentText()
    //              << "burn_key"<< "BLOCK_KEY1"<< "key.bin"<< "XTS_AES_128_KEY";
    arguments << "-p" << ui->portBox->currentText()
              << "burn_custom_mac" << ui->setMacEdit->text();

    //    startProcess(program, arguments);
    process = new QProcess(this);

    connect(process, &QProcess::started, this, [=]()
            { setInputsDisabled(true); });

    connect(process, &QProcess::readyReadStandardError, this, [=]()
            {
        ui->outputText->insertPlainText(process->readAllStandardError());
        ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, &QProcess::readyReadStandardOutput, this, [=]()
            {
        ui->outputText->insertPlainText(process->readAllStandardOutput());
        ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
        ui->outputText->appendPlainText(QString("Finished with exit code ") + QString::number(exitCode));
        setInputsDisabled(false);
        if (exitCode != 0)
        {
            ui->outputText->appendPlainText("\n烧录失败!!!!!!!!!!");

            QMessageBox::critical(NULL, "烧录失败！", "烧录失败，请重试!", QMessageBox::Ok, QMessageBox::Ok);
        }
        else
        {
        setInputsDisabled(false);
        process->write("BURN\n"); 
         ui->outputText->appendPlainText("\n烧录成功!!!!!!!!!!");

            QMessageBox::critical(NULL, "烧录成功", "烧录成功!", QMessageBox::Ok, QMessageBox::Ok);
        } });

    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError error)
            {
        ui->outputText->appendPlainText( QString("An error occurred: ") + error );
        setInputsDisabled(false); });

    process->start(program, arguments);
    // process->waitForFinished();
    process->write("BURN\n");
}
void MainWindow::resetBoard()
{
    // closeSerialPort();
    ui->outputText->clear();
    QString program = "esptool.exe";
    QStringList arguments;
    arguments << "-b" << ui->baudBox->currentText() << "-p" << ui->portBox->currentText() << "--after"
              << "hard_reset"
              << "read_mac";

    startProcess(program, arguments, 3);

    // openSerialPort();
}
void MainWindow::burnApp()
{
    closeSerialPort();
    ui->outputText->clear();
    ui->outputText->appendPlainText("Uploading Test Firmware...\n");
    QString appPath = QCoreApplication::applicationDirPath();
    // QString program = appPath + "/esptool";
    QString program = "esptool.exe";

    qDebug() << "program =" << program;

    QStringList arguments;
    // QStringList arguments = "esptool.py -p COM473 -b 2000000 write_flash 0x30000" + testFilePath;
    // arguments << "-vv" << "-cd" << "ck" << "-cb" << ui->baudBox->currentText() << "-cp" << ui->portBox->currentData().toString() << "-cf" << testFilePath;
    arguments << "-b" << ui->baudBox->currentText() << "-p"
              << ui->portBox->currentText() << "--before"
              << "default_reset"
              << "--after"
              << "no_reset"
              << "--no-stub"
              << "write_flash" << ui->appAddress->text() << filePathApp;

    qDebug() << "arguments =" << arguments;
    // startProcess(program, arguments, 4);
    // 改成我熟悉的

    process = new QProcess(this);

    connect(process, &QProcess::started, this, [=]()
            { setInputsDisabled(true); });

    connect(process, &QProcess::readyReadStandardError, this, [=]()
            {
            ui->outputText->insertPlainText(process->readAllStandardError());
            ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, &QProcess::readyReadStandardOutput, this, [=]()
            {
                ui->outputText->insertPlainText(process->readAllStandardOutput());
                ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
                ui->outputText->appendPlainText(QString("Finished with exit code ") + QString::number(exitCode));
                setInputsDisabled(false);
                if (exitCode != 0)
                {
                    ui->outputText->appendPlainText("\n烧录APP失败!!!!!!!!!!");

                    QMessageBox::critical(NULL, "烧录失败！", "烧录APP失败，请重试", QMessageBox::Ok, QMessageBox::Ok);
                }
                else
                {
                    ui->outputText->appendPlainText("\n烧录成功");

                    QMessageBox::information(NULL, "提示", "烧录成功！", QMessageBox::Ok, QMessageBox::Ok);
                    openSerialPort();
                    setDTR(false);
                    setRTS(true);
                     msleep(500);
                    setRTS(false);
                } });

    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError error)
            {
            ui->outputText->appendPlainText( QString("An error occurred: ") + error );
            setInputsDisabled(false); });

    process->start(program, arguments);
}

void MainWindow::setInputsDisabled(bool disabled)
{
    ui->eraseBtn->setDisabled(disabled);
    ui->baudBox->setDisabled(disabled);
    ui->portBox->setDisabled(disabled);
    ui->reloadBtn->setDisabled(disabled);
    ui->burnBootBtn->setDisabled(disabled);
    ui->browseBoot->setDisabled(disabled);
    ui->browseApp->setDisabled(disabled);
    ui->burnAppBtn->setDisabled(disabled);
}

void MainWindow::startProcess(QString program, QStringList arguments, uint8_t isTest)
{
    process = new QProcess(this);

    connect(process, &QProcess::started, this, [=]()
            { setInputsDisabled(true); });

    connect(process, &QProcess::readyReadStandardError, this, [=]()
            {
        ui->outputText->insertPlainText(process->readAllStandardError());
        ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, &QProcess::readyReadStandardOutput, this, [=]()
            {
        ui->outputText->insertPlainText(process->readAllStandardOutput());
        ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum()); });

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
        ui->outputText->appendPlainText(QString("Finished with exit code ") + QString::number(exitCode));
        setInputsDisabled(false);
        if(isTest ==1)
        {
            ui->outputText->appendPlainText("\nTest Firmware upload finished. Press 'Open Serial' and press the config button on the Device (if any) to start testing.");
        }
        else if(isTest ==2)
        {
            ui->outputText->appendPlainText("\n擦除flash完成，请开始烧录KEY.bin");
        }
        else if(isTest ==3)
        {
            ui->outputText->appendPlainText("\n重启板子");

        }
        else if(isTest ==4)
        {
            ui->outputText->appendPlainText("\n烧录APP完成，请重启板子启动等待加密完成后重启\n");
        } });

    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError error)
            {
        ui->outputText->appendPlainText( QString("An error occurred: ") + error );
        setInputsDisabled(false); });

    process->start(program, arguments);
}

void MainWindow::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
    {
        serial->close();
        ui->openSerialBtn->setDisabled(false);
        ui->closeSerialBtn->setDisabled(true);
        ui->outputText->appendPlainText("__________________________________________________________\nSerial port closed.");
    }
}

void MainWindow::openSerialPort()
{
    serial->setPortName(ui->portBox->currentText());
    // serial->setBaudRate(ui->baudBox->currentData().toInt());
    serial->setBaudRate(115200);

    // serial->setDataBits(p.dataBits);
    // serial->setParity(p.parity);
    // serial->setStopBits(p.stopBits);
    // serial->setFlowControl(p.flowControl);
    if (serial->open(QIODevice::ReadWrite))
    {
        ui->openSerialBtn->setDisabled(true);
        ui->closeSerialBtn->setDisabled(false);
        ui->outputText->appendPlainText("\nSerial port open:\n__________________________________________________________\n");
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), serial->errorString());
        ui->outputText->appendPlainText(tr("Open error"));
    }
}

void MainWindow::readData()
{
    QByteArray data = serial->readAll();
    ui->outputText->insertPlainText(data);
    ui->outputText->verticalScrollBar()->setValue(ui->outputText->verticalScrollBar()->maximum());
}
