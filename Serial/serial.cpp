#include "Serial/serial.h"
#include <QDebug>
#include <QSerialPortInfo>

Serial::Serial(QSerialPort *parent) : QSerialPort(parent)
{
    m_timerFreshComPort = new QTimer(this);
    m_timerFreshComPort->start(500);
    connect(m_timerFreshComPort, &QTimer::timeout, this, &Serial::getAllPortInfo);
    // qDebug() << "helloworld";
}

Serial::~Serial() {}

QString Serial::serialErrorCodeToString(int errcode)
{
    QString errStr;
    errStr = "errcode: " + QString::number(errcode);
    switch (errcode) {
    case QSerialPort::DeviceNotFoundError: {
        errStr += "DeviceNotFoundError";
        break;
    }

    case QSerialPort::PermissionError: {
        errStr += "PermissionError";
        break;
    }
    case QSerialPort::OpenError: {
        errStr += "OpenError";
        break;
    }
    case QSerialPort::NotOpenError: {
        errStr += "NotOpenError";
        break;
    }
    case QSerialPort::FramingError: {
        errStr += "FramingError";
        break;
    }
    default:
        break;
    }
    return errStr;
}

QString Serial::getSerialParityStr() const
{
    QString str = "unkonw parity";
    switch (this->parity()) {
    case QSerialPort::NoParity: {
        str = "None";
        break;
    }
    case QSerialPort::EvenParity: {
        str = "Even";
        break;
    }
    case QSerialPort::OddParity: {
        str = "Odd";
        break;
    }
    default:
        break;
    }
    return str;
}

//QSerialPort::OneStop
//    QSerialPort::OneAndHalfStop
//        QSerialPort::TwoStop
//            QSerialPort::UnknownStopBits

QString Serial::getSerialStopBitsStr() const
{
    QString str = "unkonw StopBits";
    switch (this->stopBits()) {
    case QSerialPort::OneStop: {
        str = "1";
        break;
    }
    case QSerialPort::OneAndHalfStop: {
        str = "1.5";
        break;
    }
    case QSerialPort::TwoStop: {
        str = "2";
        break;
    }
    default:
        break;
    }
    return str;
}

void Serial::getAllPortInfo()
{
    static QStringList comlist_last;
    QStringList comlist;
    int comlistCount, comlistLastCount;
    int index;
    QString changedComName;

    bool comAddFlag = true;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        comlist << info.portName();
    }
    comlist.sort();
    comlistCount = comlist.count();
    comlistLastCount = comlist_last.count();
    if (comlist_last != comlist) {
        if (comlistCount > comlistLastCount) {
            comAddFlag = true;
            for (index = 0; index < comlistLastCount; index++) {
                if (!comlist_last.contains(comlist.at(index)))
                    break;
            }
            changedComName = comlist[index];
        } else {
            comAddFlag = false;
            for (index = 0; index < comlistCount; index++) {
                if (!comlist.contains(comlist_last.at(index)))
                    break;
            }
            changedComName = comlist_last[index];
        }
        emit(comPortChanged(comlist, comAddFlag, changedComName));
        comlist_last = comlist;
    }

    //        qDebug() << comlist;
}
