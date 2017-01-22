#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>

#define NORMAL_SEQUENCE

#define NUM_BLOCKS      (128)
#define BLOCK_SIZE      (64)

unsigned char programData[NUM_BLOCKS][BLOCK_SIZE];
unsigned char securityData[BLOCK_SIZE];
unsigned char blockCounter = 0x00;
bool fSecurityAddrAcquired = false;
unsigned short securityAddr = 0x0000;
bool fChecksumAddrAcquired = false;
unsigned short checksumAddr = 0x0000;
unsigned short programChecksum = 0x0000;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_SerialDeviceList(new QComboBox),
    m_fConnected(false),
    m_fDataLoaded(false)
{
    ui->setupUi(this);

    m_SerialDeviceList->setMinimumWidth(250);
    m_SerialDeviceList->setEnabled(false);
    ui->mainToolBar->insertWidget(ui->actionConnect, m_SerialDeviceList);
    ui->mainToolBar->insertSeparator(ui->actionConnect);

    FillPortsInfo();

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(fileOpen()));
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(serialConnect()));
    connect(ui->actionFlash, SIGNAL(triggered()), this, SLOT(targetFlash()));

    connect(&m_serialThread, SIGNAL(serialError(QString)), this, SLOT(serialError(QString)));
    connect(&m_serialThread, SIGNAL(serialTimeout(QString)), this, SLOT(serialTimeout(QString)));
    connect(&m_serialThread, SIGNAL(serialData(QByteArray)), this, SLOT(serialData(QByteArray)));
}

MainWindow::~MainWindow()
{
    if (m_fConnected) {
        serialConnect();
    }
    delete ui;
}

/**
 * @brief MainWindow::FillPortsInfo
 */
void MainWindow::FillPortsInfo()
{
    m_SerialDeviceList->clear();
    if (QSerialPortInfo::availablePorts().count() == 0) {
        m_SerialDeviceList->addItem(tr("Serial port not detected"), "None");
    } else {
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
            m_SerialDeviceList->addItem(info.description() + ' ' + '(' + info.portName() + ')', info.portName());
        }
        m_SerialDeviceList->setEnabled(true);
        ui->actionConnect->setEnabled(true);
    }
}

/**
 * @brief MainWindow::HandleConnection
 */
void MainWindow::serialConnect()
{
    if (m_fConnected) {
        m_serialThread.disconnect();
        ui->actionConnect->setText(tr("Connect"));
        ui->actionFlash->setEnabled(false);
        m_SerialDeviceList->setEnabled(true);
        ui->statusBar->showMessage(tr("Disconnected from: %1").arg(m_SerialDeviceList->currentText()));
        m_fConnected = false;
    } else {
        m_serialThread.connect(m_SerialDeviceList->currentData().toString());
        ui->actionConnect->setText(tr("Disconnect"));
        if (m_fDataLoaded)
            ui->actionFlash->setEnabled(true);
        m_SerialDeviceList->setEnabled(false);
        ui->statusBar->showMessage(tr("Connected to: %1").arg(m_SerialDeviceList->currentText()));
        m_fConnected = true;
    }
}

/**
 * @brief MainWindow::serialPortError
 * @param s - error string;
 */
void MainWindow::serialError(const QString &s)
{
    if (m_fConnected) {
        ui->actionConnect->setText(tr("Connect"));
        ui->actionFlash->setEnabled(false);
        m_SerialDeviceList->setEnabled(true);
        m_fConnected = false;
        ui->statusBar->showMessage(s);
    }
}

/**
 * @brief MainWindow::serialPortTimeout
 * @param s - error string;
 */
void MainWindow::serialTimeout(const QString &s)
{
    if (m_fConnected) {
        ui->actionConnect->setText(tr("Connect"));
        ui->actionFlash->setEnabled(false);
        m_SerialDeviceList->setEnabled(true);
        m_fConnected = false;
        ui->statusBar->showMessage(s);
    }
}

/**
 * @brief MainWindow::serialData
 * @param s - data string;
 */
void MainWindow::serialData(const QByteArray &ba)
{
    QByteArray cmd;
    char cmdID;
    char cmdTrm;
    char argID;
    char argTrm;
    static unsigned char blockCntr = 0x00;

    cmdID  = ba.at(0);
    cmdTrm = ba.at(1);

    if ((cmdID < 'a') || (cmdID > 'z') || (cmdTrm != '\n')) {
        ui->plainTextEdit->appendPlainText("Unrecognized responce received!");
        return;
    }

    switch (cmdID) {
    case 'a':
        argID  = ba.at(2);
        argTrm = ba.at(3);
        if ((argID == '0') && (argTrm == '\n')) {
            ui->plainTextEdit->appendPlainText("Target Reset Initialization Completed. Silicon ID verified!");

            cmd = "b\n";
            m_serialThread.write(cmd);
        } else {
            ui->plainTextEdit->appendPlainText("Target Reset Initialization failed! Silicon ID Verification Failed!");
        }
        break;

    case 'b':
        argID  = ba.at(2);
        argTrm = ba.at(3);
        if ((argID == '0') && (argTrm == '\n')) {
            ui->plainTextEdit->appendPlainText("Target Bulk Erase Completed!\r\nProgramming");
#if defined(NORMAL_SEQUENCE)
            cmd = "c\n";
            m_serialThread.write(cmd);

            blockCntr = 0x00;
            cmd.setRawData((const char *)programData[blockCntr++], BLOCK_SIZE);
            m_serialThread.write(cmd);
#else
            cmd = "e\n";
            m_serialThread.write(cmd);

            cmd.setRawData((const char *)securityData, BLOCK_SIZE);
            m_serialThread.write(cmd);
#endif
        } else {
            ui->plainTextEdit->appendPlainText("Target Bulk Erase Failed!");
        }
        break;

    case 'c':
        argID  = ba.at(2);
        argTrm = ba.at(3);
        if ((argID == '+') && (argTrm == '\n') && (blockCntr < NUM_BLOCKS)) {
            if ((blockCntr % 2) == 0)
                ui->plainTextEdit->insertPlainText(".");
            cmd.setRawData((const char *)programData[blockCntr++], BLOCK_SIZE);
            m_serialThread.write(cmd);
        } else if ((argID == '0') && (argTrm == '\n')) {
            ui->plainTextEdit->insertPlainText("Complete!");

            ui->plainTextEdit->appendPlainText("Verifying");

            cmd = "d\n";
            m_serialThread.write(cmd);

            blockCntr = 0x00;
            cmd.setRawData((const char *)programData[blockCntr++], BLOCK_SIZE);
            m_serialThread.write(cmd);
        } else {
            ui->plainTextEdit->insertPlainText("Failed!");
        }
        break;

    case 'd':
        argID  = ba.at(2);
        argTrm = ba.at(3);
        if ((argID == '+') && (argTrm == '\n') && (blockCntr < NUM_BLOCKS)) {
            if ((blockCntr % 2) == 0)
                ui->plainTextEdit->insertPlainText(".");
            cmd.setRawData((const char *)programData[blockCntr++], BLOCK_SIZE);
            m_serialThread.write(cmd);
        } else if ((argID == '0') && (argTrm == '\n')) {
            ui->plainTextEdit->insertPlainText("Verified!");

            cmd = "e\n";
            m_serialThread.write(cmd);

            cmd.setRawData((const char *)securityData, BLOCK_SIZE);
            m_serialThread.write(cmd);
        } else {
            ui->plainTextEdit->insertPlainText("Failed!");
        }
        break;

    case 'e':
        argID  = ba.at(2);
        argTrm = ba.at(3);
        if ((argID == '0') && (argTrm == '\n')) {
            ui->plainTextEdit->appendPlainText("Program Security Settings Stored!");

            cmd = "f\n";
            m_serialThread.write(cmd);

            cmd.setRawData((const char *)securityData, BLOCK_SIZE);
            m_serialThread.write(cmd);
        } else {
            ui->plainTextEdit->appendPlainText("Programming Security Data Failed!");
        }
        break;

    case 'f':
        argID  = ba.at(2);
        argTrm = ba.at(3);
        if ((argID == '0') && (argTrm == '\n')) {
            ui->plainTextEdit->appendPlainText("Program Security Settings Verified!");

            cmd = "g\n";
            m_serialThread.write(cmd);
        } else {
            ui->plainTextEdit->appendPlainText("Program Security Data Incorrect!");
        }
        break;

    case 'g':
        argID  = ba.at(2);
        argTrm = ba.at(3);
        if ((argID == '0') && (argTrm == '\n')) {
            ui->plainTextEdit->appendPlainText("Checksum Verified!");

            cmd = "x\n";
            m_serialThread.write(cmd);
        } else {
            ui->plainTextEdit->appendPlainText("Checksum Failed!");
            qDebug() << cmdID << argID;
        }
        break;

    case 'x':
        argID  = ba.at(2);
        argTrm = ba.at(3);
        if (argID == '0' && argTrm == '\n') {
            ui->plainTextEdit->appendPlainText("Target ReStarted!");
        }
        break;

    default:
        ui->plainTextEdit->appendPlainText("Unrecognized command received!\r\n");
    }
}

/**
 * @brief MainWindow::targetFlash
 */
void MainWindow::targetFlash()
{
    QByteArray cmd;

    cmd = "a\n";
    m_serialThread.write(cmd);
}

/**
 * @brief MainWindow::hexDataProcess
 * @param line
 * @return
 */
bool MainWindow::hexDataProcess(const QByteArray &line)
{
    QString str;
    bool ok;
    unsigned char dataSize;
    unsigned short dataAddr;
    unsigned char dataType;
    unsigned char checksum;
    unsigned char i;
    unsigned char dataByte;
    unsigned char dataChecksum;

    if (line[0] != ':') {
        qDebug() << "Not an Intel hex file.";
        return false;
    }

    str.append(line[1]);
    str.append(line[2]);

    dataSize = str.toInt(&ok, 16);
    if (!ok) {
        qDebug() << "dataSize hex2int failed.";
        return false;
    }

    str.clear();
    str.append(line[3]);
    str.append(line[4]);
    str.append(line[5]);
    str.append(line[6]);

    dataAddr = str.toInt(&ok, 16);
    if (!ok) {
        qDebug() << "dataAddr hex2int failed.";
        return false;
    }

    str.clear();
    str.append(line[7]);
    str.append(line[8]);

    dataType = str.toInt(&ok, 16);
    if (!ok) {
        qDebug() << "dataType hex2int failed.";
        return false;
    }

    checksum  = dataSize + dataType;
    checksum += (unsigned char)(dataAddr);
    checksum += (unsigned char)(dataAddr >> 8);

    if ((dataSize == BLOCK_SIZE) && (dataType == 0x00) && (blockCounter < NUM_BLOCKS) && (!fSecurityAddrAcquired) && (!fChecksumAddrAcquired)) {
        for (i = 0; i < BLOCK_SIZE; i++) {
            str.clear();
            str.append(line[9  + i * 2]);
            str.append(line[10 + i * 2]);

            dataByte = str.toInt(&ok, 16);
            if (!ok) {
                qDebug() << "dataByte hex2int failed.";
                return false;
            }
            checksum += dataByte;
            programData[blockCounter][i] = dataByte;
        }
        blockCounter++;
    } else if ((dataSize == BLOCK_SIZE) && (dataType == 0x00) && (blockCounter == NUM_BLOCKS) && (fSecurityAddrAcquired) && (!fChecksumAddrAcquired)) {
        for (i = 0; i < BLOCK_SIZE; i++) {
            str.clear();
            str.append(line[ 9 + i * 2]);
            str.append(line[10 + i * 2]);

            dataByte = str.toInt(&ok, 16);
            if (!ok) {
                qDebug() << "dataByte hex2int failed.";
                return false;
            }
            checksum += dataByte;
            securityData[i] = dataByte;
        }
    } else if ((dataSize == 0x02) && (dataType == 0x04) && (blockCounter == NUM_BLOCKS) && (!fSecurityAddrAcquired) && (!fChecksumAddrAcquired)) {
        str.clear();
        str.append(line[9]);
        str.append(line[10]);
        str.append(line[11]);
        str.append(line[12]);

        securityAddr = str.toInt(&ok, 16);
        if (!ok) {
            qDebug() << "securityAddr hex2int failed.";
            return false;
        }

        fSecurityAddrAcquired = true;

        checksum += (unsigned char)(securityAddr);
        checksum += (unsigned char)(securityAddr >> 8);
    } else if ((dataSize == 0x02) && (dataType == 0x04) && (blockCounter == NUM_BLOCKS) && (fSecurityAddrAcquired) && (!fChecksumAddrAcquired)) {
        str.clear();
        str.append(line[9]);
        str.append(line[10]);
        str.append(line[11]);
        str.append(line[12]);

        checksumAddr = str.toInt(&ok, 16);
        if (!ok) {
            qDebug() << "checksumAddr hex2int failed.";
            return false;
        }

        fChecksumAddrAcquired = true;

        checksum += (unsigned char)(checksumAddr);
        checksum += (unsigned char)(checksumAddr >> 8);
    } else if ((dataSize == 0x02) && (dataType == 0x00) && (blockCounter == NUM_BLOCKS) && (fSecurityAddrAcquired) && (fChecksumAddrAcquired)) {
        str.clear();
        str.append(line[9]);
        str.append(line[10]);
        str.append(line[11]);
        str.append(line[12]);

        programChecksum = str.toInt(&ok, 16);
        if (!ok) {
            qDebug() << "programChecksum hex2int failed.";
            return false;
        }

        checksum += (unsigned char)(programChecksum);
        checksum += (unsigned char)(programChecksum >> 8);
    } else if ((dataSize == 0x00) && (dataType == 0x01) && (blockCounter == NUM_BLOCKS) && (fSecurityAddrAcquired) && (fChecksumAddrAcquired)) {
        qDebug() << "The End of File.";
    } else {
        qDebug() << "The file is corrupted.";
        return false;
    }

    str.clear();
    str.append(line[9 + dataSize * 2]);
    str.append(line[10 + dataSize * 2]);

    dataChecksum = str.toInt(&ok, 16);
    if (!ok) {
        qDebug() << "dataChecksum hex2int failed.";
        return false;
    }

    checksum = (~checksum) + 0x01;

    if (dataChecksum != checksum) {
        qDebug() << "Checksum error.";
        return false;
    }

    return true;
}

bool MainWindow::load(const QString &f)
{
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    blockCounter = 0x00;
    fSecurityAddrAcquired = false;
    fChecksumAddrAcquired = false;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!hexDataProcess(line))
            return false;
    }

    return true;
}

void MainWindow::fileOpen()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Open File..."), QString(), tr("HEX-Files (*.hex);;All Files (*)"));
    if (!fn.isEmpty())
        m_fDataLoaded = load(fn);
    if (m_fDataLoaded) {
        ui->statusBar->showMessage("File loaded successfully!");
        if (m_fConnected) {
            ui->actionFlash->setEnabled(true);
        }
    } else {
        ui->statusBar->showMessage("File is corrupted!");
        ui->actionFlash->setEnabled(false);
    }
}
