#ifndef SERIAL_H
#define SERIAL_H
#include <QSerialPort>
#include <QTimer>
class Serial : public QSerialPort
{
    Q_OBJECT
public:
    Serial(QSerialPort *parent = nullptr);
    ~Serial();
    QString serialErrorCodeToString(int errcode);
    QString getSerialParityStr(void) const;
    QString getSerialStopBitsStr(void) const;

signals:
    void comPortChanged(QStringList const &comlist, bool, QString) const;

private:
    QStringList m_comList;
    QTimer *m_timerFreshComPort;
    void getAllPortInfo(void);
};

#endif // SERIAL_H
