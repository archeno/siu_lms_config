#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Serial/serial.h>
#include <chrono>
#include <QAction>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QModbusClient>
#include <QModbusDataUnit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    enum ContentType { ContentTypeSend = 0, ContentTypeRecv };
    enum SerialCmdType {
        CMD_SS = 0,
        CMD_LS_ON,
        CMD_LS_OFF,
    };

private:
    QWidget *m_centralwidget;
    QVBoxLayout *m_verticalLayout;

    QSpacerItem *m_horizontalSpacer_2;

    QGridLayout *m_gridLayoutProtocolArea;

    QHBoxLayout *m_topHboxLayout;

    QGroupBox *m_gbTop;
    QWidget *m_widgetTop;
    QVBoxLayout *m_layoutVBoxTop;
    QSpinBox *m_spbTargetGroupId; //组号
    QSpinBox *m_spbTargetSegId;   // ID 号
    QWidget *m_container_body;
    QWidget *m_container_left;
    QWidget *m_container_right;
    QHBoxLayout *m_layout_body;
    QVBoxLayout *m_layout_left;
    QVBoxLayout *m_layout_right;
    QGroupBox *m_gropSeg0;
    QGroupBox *m_gropSeg1;
    QVBoxLayout *m_layoutVboxSeg0;
    QVBoxLayout *m_layoutVboxSeg1;
    QFormLayout *m_layoutFormSeg0;
    QFormLayout *m_layoutFormSeg1;
    QSpinBox *m_spbAddrInGrop[6]; //组内地址
    QSpinBox *m_spbSegInSeg0;
    QPushButton *m_btnSeg0Download;

    QComboBox *m_cbSeg1PowerOnState;
    QDoubleSpinBox *m_spbSeg1TxDelay;
    QPushButton *m_btnSeg1Download;

    //同步命令
    QGroupBox *m_gbSs;
    QPushButton *m_btnSsCmd;
    QGroupBox *m_gpSendCtrl;
    QHBoxLayout *m_layoutHBoxSendCtrl;
    QCheckBox *m_ckbSendCycleEnable; //周期发送使能
    QSpinBox *m_spbSendCycleMs;      //巡检指令周期发送间隔
    QTimer *m_TimerSendCycleMs;
    QHBoxLayout *m_laoutHBoxSs;

    QGroupBox *m_gbLs;
    QPushButton *m_btnLsOn;
    QPushButton *m_btnLsOff;

    QHBoxLayout *m_layoutHboxLs;

    QPlainTextEdit *m_plainTextRecv;
    QGroupBox *m_gbRecv;
    QVBoxLayout *m_layoutVBoxRecv;
    QHBoxLayout *m_layoutHBoxClear;
    QPushButton *m_btnClearRecv;

    QStatusBar *m_statusbar;
    QToolBar *m_toolBar;
    QLabel *m_labSerial;
    QLabel *m_labBaud;
    QComboBox *m_combSerialPort;
    QComboBox *m_combBaud;
    QToolButton *m_toolbtnConnect;
    QToolButton *m_toolbtnComStatus;

    Serial *m_myserial;
    QTimer *m_timer500ms;
    QLabel *m_errMsg;
    QLabel *m_eventMsg;
    qint32 m_baudRate;
    QString m_comportName;
    SerialCmdType m_cmd_type;
    QString m_currentComName;

    bool m_serialOpenflag;
    QByteArray m_SendData;
    int m_SendCyclems;
    std::chrono::high_resolution_clock::time_point m_lastSendTimestamp; // 记录上次发送的时间戳

    void InitToolBar(void);
    void initMainWindow(void);
    void InitConnetction(void);
    void plainTextEditRecvUpdate(ContentType type, QString timeStamp, const QByteArray &data);
    void center();
    void SerailSendData(const QByteArray &data);
    void SerialSendDataCycle();

signals:
    void errSerialNotOpened();
private slots:
    void onSerialNotOpenedErr();
    void onBtnSeg0DownloadClicked();
    void onBtnSeg1DownloadClicked();
    void onBtnSsCmdClicked();
    void onBtnLsOnCmdClicked();
    void onBtnLsOffCmdClicked();

    void Window1sUpdate();
    void UpdateStatusbar();
    void openSerial();
    void onTargetIdChanged();
    //    void onReadReady();

    void fillComPort(QStringList const &list, bool addFlag, QString changedComName);
};
#endif // MAINWINDOW_H
