#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QMainWindow>

#include "serialthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void FillPortsInfo();
    bool hexDataProcess(const QByteArray &line);
    bool load(const QString &f);

private slots:
    void fileOpen();
    void serialConnect();
    void targetFlash();
    void serialError(const QString &s);
    void serialTimeout(const QString &s);
    void serialData(const QByteArray &ba);


private:
    Ui::MainWindow *ui;
    QComboBox *m_SerialDeviceList;
    SerialThread m_serialThread;
    bool m_fConnected;
    bool m_fDataLoaded;
};

#endif // MAINWINDOW_H
