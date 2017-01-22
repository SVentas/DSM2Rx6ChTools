#include "serialthread.h"

#include <QtSerialPort/QSerialPort>
#include <QDebug>

#define SERIAL_WRITE_TIMEOUT_MS         20
#define SERIAL_READ_TIMEOUT_MS          20
#define SERIAL_READ_TIMEOUT_EXTRA_MS    10
#define SERIAL_DISCONNECT_TIMEOUT_MS    5000

QT_USE_NAMESPACE

SerialThread::SerialThread(QObject *parent) :
    QThread(parent),
    m_quit(false)
{
    // Empty;
}

SerialThread::~SerialThread()
{
    if (isRunning()) {
        disconnect();
    }
}

void SerialThread::connect(const QString &portName)
{
    m_mutex.lock();
    m_portName = portName;
    m_quit = false;
    m_txBuf.clear();
    m_rxBuf.clear();
    m_mutex.unlock();

    if (!isRunning()) {
        start();
    }
}

void SerialThread::disconnect()
{
    m_mutex.lock();
    m_quit = true;
    m_mutex.unlock();

    if (!wait(SERIAL_DISCONNECT_TIMEOUT_MS)) {
        qDebug() << "Failed to terminate serial thread!";
    }
}

void SerialThread::run()
{
    QSerialPort serial;

    serial.setPortName(m_portName);
    serial.setBaudRate(57600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    if (!serial.open(QIODevice::ReadWrite)) {
        qDebug() << "Connection failed!";
        emit this->serialError(tr("Can't open %1, error code %2. %3.")
            .arg(m_portName).arg(serial.error()).arg(serial.errorString()));
        return;
    }

    qDebug() << "Serial Thread is ready...";

    while (!m_quit) {
        /* Protect shared resources while thread is working. */
        m_mutex.lock();

        if (m_txBuf.size() > 0) {
            qint64 bytesWritten = serial.write(m_txBuf);
            if (serial.waitForBytesWritten(SERIAL_WRITE_TIMEOUT_MS)) {
                m_txBuf.remove(0, bytesWritten);
            } else {
                qDebug() << "Write request timeout!";

                /* Unlock resources and exit. */
                m_mutex.unlock();
                emit this->serialTimeout(tr("Write request timeout!"));
                break;
            }
        }

        /* Release shared resources. */
        m_mutex.unlock();

        if (serial.waitForReadyRead(SERIAL_READ_TIMEOUT_MS)) {
            m_rxBuf += serial.readAll();
            while (serial.waitForReadyRead(SERIAL_READ_TIMEOUT_EXTRA_MS)) {
                m_rxBuf += serial.readAll();
            }

            QByteArray msg;
            while (m_rxBuf.size() >= 4)
            {
                msg.append(m_rxBuf.at(0));
                msg.append(m_rxBuf.at(1));
                msg.append(m_rxBuf.at(2));
                msg.append(m_rxBuf.at(3));

                m_rxBuf.remove(0, 4);

                emit this->serialData(msg);
            }
        }
    }

    serial.close();
    qDebug() << "Serial Thread is terminated.";
}

void SerialThread::write(const QByteArray &ba)
{
    m_mutex.lock();
    m_txBuf.append(ba);
    m_mutex.unlock();
}
