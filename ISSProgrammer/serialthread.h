#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QThread>
#include <QMutex>
#include <QtSerialPort/QSerialPort>

class SerialThread : public QThread
{
    Q_OBJECT

public:
    SerialThread(QObject *parent = 0);
    ~SerialThread();

    void connect(const QString &portName);
    void disconnect();
    void write(const QByteArray &ba);

protected:
    void run() Q_DECL_OVERRIDE;

signals:
    void serialError(const QString &s);
    void serialTimeout(const QString &s);
    void serialData(const QByteArray &ba);

private:
    QString m_portName;
    QMutex m_mutex;
    QByteArray m_txBuf;
    QByteArray m_rxBuf;
    bool m_quit;
};

#endif // SERIALTHREAD_H
