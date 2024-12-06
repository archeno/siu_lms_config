#include "mainwindow.h"
#include "Serial/serial.h"
#include <chrono>
#include <protocol/lms.h>
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QScreen>
#include <QSerialPortInfo>
const int StatusBarMsgShowTime = 5000;
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_serialOpenflag(false)
{
    QString test_str("test QString output");
    qDebug() << qPrintable(test_str);
    m_myserial = new Serial();
    m_TimerSendCycleMs = new QTimer();
    connect(m_TimerSendCycleMs, &QTimer::timeout, this, &MainWindow::SerialSendDataCycle);

    InitToolBar();
    initMainWindow();
    center();
    InitConnetction();
    connect(m_myserial, &Serial::comPortChanged, this, &MainWindow::fillComPort);
    QStringList baudStringlist;
    baudStringlist << "4800"
                   << "9600"
                   << "115200";
    m_combBaud->addItems(baudStringlist);
    QStringList comlist;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        comlist << info.portName();
    }
    m_combSerialPort->addItems(comlist);
    m_myserial->setPortName(m_combSerialPort->currentText());
    m_currentComName = m_combSerialPort->currentText();
    m_myserial->setBaudRate(m_combBaud->currentText().toInt());
    connect(m_toolbtnConnect, &QToolButton::clicked, this, &MainWindow::openSerial);
}
MainWindow::~MainWindow() {}

void MainWindow::Window1sUpdate()
{
    static bool inverseflag = false;
    if (m_serialOpenflag) {
        QString icon_name = inverseflag ? ":/images/green.png" : ":/images/gray.png";
        inverseflag = !inverseflag;
        m_toolbtnComStatus->setIcon(QIcon(icon_name));
    } else {
        m_toolbtnComStatus->setIcon(QIcon(":/images/red.png"));
    }
}

void MainWindow::UpdateStatusbar()
{
    m_baudRate = m_myserial->baudRate();
    m_comportName = m_myserial->portName();

    QString comStr = m_comportName + " " + QString::number(m_baudRate) + " "
                     + QString::number(m_myserial->dataBits()) + " "
                     + m_myserial->getSerialParityStr() + " " + m_myserial->getSerialStopBitsStr()
                     + " ";
    m_eventMsg->setText(comStr);
}

//static void setSpinBoxFormat(QSpinBox &spb)
//{
//    spb.setRange(0, 255);
//    spb.setPrefix("0x ");
//    spb.setDisplayIntegerBase(16);
//}
void MainWindow::InitToolBar()
{
    m_toolBar = new QToolBar();
    this->addToolBar(m_toolBar);
    m_statusbar = new QStatusBar();
    this->setStatusBar(m_statusbar);

    //toolbar
    m_labSerial = new QLabel("串口");
    m_labBaud = new QLabel("波特率");
    m_combSerialPort = new QComboBox();
    m_combBaud = new QComboBox();

    m_combSerialPort->setMinimumHeight(36);
    m_combBaud->setMinimumHeight(36);

    m_toolbtnComStatus = new QToolButton(this);
    m_toolbtnConnect = new QToolButton(this);
    m_toolbtnComStatus->setIcon(QIcon(":/images/red.png"));
    m_toolbtnComStatus->setText("通信指示");
    m_toolbtnComStatus->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_toolbtnConnect->setIcon(QIcon(":/images/connect.png"));
    m_toolbtnConnect->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_toolbtnConnect->setText("连接串口");

    m_timer500ms = new QTimer(this);
    m_timer500ms->start(500);

    m_toolBar->addWidget(m_labSerial);
    m_toolBar->addWidget(m_combSerialPort);
    m_toolBar->addWidget(m_labBaud);
    m_toolBar->addWidget(m_combBaud);
    m_toolBar->addWidget(m_toolbtnComStatus);
    m_toolBar->addWidget(m_toolbtnConnect);
    m_toolBar->addSeparator();

    //status bar
    m_eventMsg = new QLabel("");
    statusBar()->addPermanentWidget(m_eventMsg);

    //connect signals and slots
    connect(m_timer500ms, &QTimer::timeout, this, &MainWindow::Window1sUpdate);
    connect(m_combBaud, &QComboBox::currentTextChanged, this, [=](const QString &str) {
        m_myserial->setBaudRate(str.toInt());
    });
    connect(m_combSerialPort, &QComboBox::currentTextChanged, this, [=](const QString &str) {
        m_myserial->setPortName(str);
        m_currentComName = str;
        qDebug() << m_myserial->portName();
    });
    connect(m_myserial, &Serial::baudRateChanged, this, &MainWindow::UpdateStatusbar);
    connect(m_combSerialPort, &QComboBox::currentTextChanged, this, &MainWindow::UpdateStatusbar);
}

void MainWindow::initMainWindow()
{
    m_centralwidget = new QWidget(this);
    m_verticalLayout = new QVBoxLayout(m_centralwidget);

    m_verticalLayout->setSpacing(12);

    // 组号

    m_gbTop = new QGroupBox("目标终端选择");
    m_topHboxLayout = new QHBoxLayout();
    m_gbTop->setLayout(m_topHboxLayout);

    m_topHboxLayout->addWidget(new QLabel("组号:"));
    m_spbTargetGroupId = new QSpinBox();
    m_spbTargetGroupId->setPrefix("0x ");
    m_spbTargetGroupId->setDisplayIntegerBase(16);
    m_spbTargetGroupId->setRange(1, 0xff);
    m_spbTargetGroupId->setValue(0x01);
    m_topHboxLayout->addWidget(m_spbTargetGroupId);
    m_topHboxLayout->addSpacerItem(new QSpacerItem(40, 20));
    //段号
    m_topHboxLayout->addWidget(new QLabel("段号:"));
    m_spbTargetSegId = new QSpinBox();
    m_spbTargetSegId->setPrefix("0x ");
    m_spbTargetSegId->setDisplayIntegerBase(16);
    m_spbTargetSegId->setRange(0, 0xff);
    m_spbTargetSegId->setValue(0x10);
    m_topHboxLayout->addWidget(m_spbTargetSegId);
    m_topHboxLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

    //body
    m_container_body = new QWidget();
    m_layout_body = new QHBoxLayout();

    //left
    m_container_left = new QWidget();
    m_layout_left = new QVBoxLayout();
    m_container_left->setLayout(m_layout_left);

    //seg0
    m_gropSeg0 = new QGroupBox("编程数据段0");
    m_layoutVboxSeg0 = new QVBoxLayout();
    m_layoutFormSeg0 = new QFormLayout();
    m_spbSegInSeg0 = new QSpinBox();
    m_spbSegInSeg0->setPrefix("0x ");
    m_spbSegInSeg0->setRange(0, 255);
    m_spbSegInSeg0->setValue(0x10);
    m_spbSegInSeg0->setDisplayIntegerBase(16);

    m_layoutFormSeg0->addRow(new QLabel("控制段:"), m_spbSegInSeg0);

    for (int i = 0; i < 6; i++) {
        m_spbAddrInGrop[i] = new QSpinBox();
        m_spbAddrInGrop[i]->setRange(0, 255);
        m_spbAddrInGrop[i]->setValue(0);
        m_spbAddrInGrop[i]->setDisplayIntegerBase(16);
        m_spbAddrInGrop[i]->setPrefix("0x ");
        m_layoutFormSeg0->addRow(new QLabel(QString("%1组内地址:").arg(i + 1)), m_spbAddrInGrop[i]);
    }
    m_layoutVboxSeg0->addLayout(m_layoutFormSeg0);
    m_btnSeg0Download = new QPushButton("下载编程数据段0");
    m_btnSeg0Download->setObjectName("btnDownload0");
    m_layoutVboxSeg0->addWidget(m_btnSeg0Download);
    m_gropSeg0->setLayout(m_layoutVboxSeg0);
    m_layout_left->addWidget(m_gropSeg0);
    m_layout_left->addSpacing(40);

    //seg1
    m_gropSeg1 = new QGroupBox("编程数据段1");
    m_layoutVboxSeg1 = new QVBoxLayout();
    m_layoutFormSeg1 = new QFormLayout();
    m_cbSeg1PowerOnState = new QComboBox();

    m_cbSeg1PowerOnState->addItem("上电开灯", 0x40);
    m_cbSeg1PowerOnState->addItem("上电关灯", 0x60);

    //    m_spbSeg1PowerOn = new QSpinBox();
    //    m_spbSeg1PowerOn->setPrefix("0x ");
    //    m_spbSeg1PowerOn->setDisplayIntegerBase(16);
    //    m_spbSeg1PowerOn->setMaximum(255);
    //    m_spbSeg1PowerOn->setValue(0x40);

    m_spbSeg1TxDelay = new QDoubleSpinBox();
    m_spbSeg1TxDelay->setRange(0.0, 10.0);
    m_spbSeg1TxDelay->setSuffix(" ms");
    m_spbSeg1TxDelay->setDecimals(1);
    m_spbSeg1TxDelay->setSingleStep(0.1);
    m_spbSeg1TxDelay->setValue(0.5);

    m_layoutFormSeg1->addRow(new QLabel("上电状态"), m_cbSeg1PowerOnState);

    m_layoutFormSeg1->addRow(new QLabel("发送延时"), m_spbSeg1TxDelay);
    m_layoutVboxSeg1->addLayout(m_layoutFormSeg1);
    m_btnSeg1Download = new QPushButton("下载编程数据段1");
    m_btnSeg1Download->setObjectName("btnDownload1");
    m_layoutVboxSeg1->addWidget(m_btnSeg1Download);
    m_gropSeg1->setLayout(m_layoutVboxSeg1);
    m_layout_left->addWidget(m_gropSeg1);

    m_container_right = new QWidget();
    m_layout_right = new QVBoxLayout();
    m_container_right->setLayout(m_layout_right);

    //SS/Ls
    m_gbSs = new QGroupBox("巡检/控制命令");
    m_btnSsCmd = new QPushButton("巡检命令");
    m_laoutHBoxSs = new QHBoxLayout();
    m_laoutHBoxSs->addWidget(m_btnSsCmd);

    // 巡检命令发送周期

    m_gbSs->setLayout(m_laoutHBoxSs);

    m_laoutHBoxSs->addSpacerItem(new QSpacerItem(120, 20));
    //    m_gbLs = new QGroupBox("长指令控制");
    m_btnLsOn = new QPushButton("SIU上电/LMS开灯");
    m_btnLsOff = new QPushButton("SIU掉电/LMS关灯");
    m_btnLsOff->setObjectName("btnLsOff");
    //    m_layoutHboxLs = new QHBoxLayout();
    m_laoutHBoxSs->addWidget(m_btnLsOn);
    m_laoutHBoxSs->addSpacerItem(new QSpacerItem(60, 20));
    m_laoutHBoxSs->addWidget(m_btnLsOff);
    //    m_gbLs->setLayout(m_layoutHboxLs);

    m_layout_right->addWidget(m_gbSs);
    //    m_layout_right->addWidget(m_gbLs);

    //周期发送控制
    m_gpSendCtrl = new QGroupBox("发送周期控制");
    m_layoutHBoxSendCtrl = new QHBoxLayout();
    m_gpSendCtrl->setLayout(m_layoutHBoxSendCtrl);
    m_ckbSendCycleEnable = new QCheckBox("周期发送");
    m_spbSendCycleMs = new QSpinBox();
    m_spbSendCycleMs->setRange(0, 10000);
    m_spbSendCycleMs->setValue(1000);
    m_spbSendCycleMs->setSingleStep(100);
    m_SendCyclems = 1000;
    m_spbSendCycleMs->setSuffix("ms");
    m_layoutHBoxSendCtrl->addWidget(m_ckbSendCycleEnable);
    m_layoutHBoxSendCtrl->addSpacerItem(new QSpacerItem(20, 20));
    m_layoutHBoxSendCtrl->addWidget(m_spbSendCycleMs);
    m_layoutHBoxSendCtrl->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));
    m_layout_right->addWidget(m_gpSendCtrl);

    //接收区
    m_plainTextRecv = new QPlainTextEdit();

    m_plainTextRecv->setMinimumSize(700, 200);
    m_layout_right->addWidget(m_plainTextRecv);
    m_gbRecv = new QGroupBox("数据发送/接收显示");
    m_layoutVBoxRecv = new QVBoxLayout();
    m_layoutVBoxRecv->addWidget(m_plainTextRecv);
    m_btnClearRecv = new QPushButton("清空接收区");
    m_btnClearRecv->setObjectName("btnClear");
    m_layoutHBoxClear = new QHBoxLayout();
    m_layoutHBoxClear->addSpacerItem(new QSpacerItem(60, 40, QSizePolicy::Expanding));
    m_layoutHBoxClear->addWidget(m_btnClearRecv);
    m_layoutVBoxRecv->addLayout(m_layoutHBoxClear);
    m_gbRecv->setLayout(m_layoutVBoxRecv);
    m_layout_right->addWidget(m_gbRecv);

    m_layout_body->addWidget(m_container_left, 1);
    m_layout_body->addWidget(m_container_right, 4);

    m_container_body->setLayout(m_layout_body);
    m_verticalLayout->addWidget(m_gbTop);
    m_verticalLayout->addWidget(m_container_body);

    //    m_verticalLayout->setContentsMargins(20, 5, 10, 5); // 设置 m_verticalLayout 的边距
    //    m_layout_body->setContentsMargins(0, 10, 10, 10);    // 设置 m_layout_body 的边距
    this->setCentralWidget(m_centralwidget);
}

void MainWindow::InitConnetction()
{
    connect(m_myserial, &QSerialPort::readyRead, [=]() {
        QByteArray data = m_myserial->readAll();
        // 获取当前时间戳，精确到微秒
        // 获取当前时间戳
        auto currentTimestamp = std::chrono::high_resolution_clock::now();

        // 计算发送到接收的时间间隔
        auto intervalNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                              currentTimestamp - m_lastSendTimestamp)
                              .count();
        auto intervalUs = intervalNs / 1000; // 微秒间隔
        auto intervalMs = intervalUs / 1000; // 毫秒间隔
        intervalUs = intervalUs % 1000;

        QString timestamp = QString("%1ms:%2us")
                                .arg(intervalMs, 5, 10, QLatin1Char('0'))
                                .arg(intervalUs, 3, 10, QLatin1Char('0'));

        // 输出高精度时间戳和接收的数据
        QByteArray HexData = data.toHex(' ');
        plainTextEditRecvUpdate(ContentTypeRecv, timestamp, HexData);
        //        m_plainTextRecv->appendPlainText("[" + timestamp + "] Rx: " + QString::fromUtf8(data));
    });
    connect(m_btnSeg0Download, &QPushButton::clicked, this, &MainWindow::onBtnSeg0DownloadClicked);
    connect(m_btnSeg1Download, &QPushButton::clicked, this, &MainWindow::onBtnSeg1DownloadClicked);
    connect(m_btnSsCmd, &QPushButton::clicked, this, &MainWindow::onBtnSsCmdClicked);
    connect(m_btnLsOn, &QPushButton::clicked, this, &MainWindow::onBtnLsOnCmdClicked);
    connect(m_btnLsOff, &QPushButton::clicked, this, &MainWindow::onBtnLsOffCmdClicked);
    connect(m_btnClearRecv, &QPushButton::clicked, [=]() { m_plainTextRecv->clear(); });
    connect(this, &MainWindow::errSerialNotOpened, this, &MainWindow::onSerialNotOpenedErr);
    connect(m_ckbSendCycleEnable, &QCheckBox::stateChanged, [&](int state) {
        if (!state) {
            m_TimerSendCycleMs->stop();
        }
    });

    connect(m_spbSendCycleMs, QOverload<int>::of(&QSpinBox::valueChanged), [&](int value) {
        m_SendCyclems = value;
        m_TimerSendCycleMs->setInterval(m_SendCyclems);
    });

    connect(m_spbTargetSegId,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &MainWindow::onTargetIdChanged);
    connect(m_spbTargetGroupId,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &MainWindow::onTargetIdChanged);
}
void MainWindow::plainTextEditRecvUpdate(ContentType type, QString timeStamp, const QByteArray &data)
{
    if (type == ContentTypeSend) {
        m_plainTextRecv->appendPlainText("[" + timeStamp + "] Tx: " + QString::fromUtf8(data));
    } else {
        m_plainTextRecv->appendPlainText("[" + timeStamp + "] Rx: " + QString::fromUtf8(data));
    }
}

void MainWindow::center()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenRect = screen->geometry();
        int x = (screenRect.width() - width()) / 2;
        int y = (screenRect.height() - height()) / 2;
        move(x, y); // 移动窗口到中心位置
    }
}

void MainWindow::SerailSendData(const QByteArray &data)
{
    m_myserial->write(data);
    m_lastSendTimestamp = std::chrono::high_resolution_clock::now(); // 记录发送时间
    QByteArray HexData = data.toHex(' ');
    QString timeStamp = "00000ms.000us";
    if (m_ckbSendCycleEnable->isChecked()) {
        m_SendData = data;
        m_TimerSendCycleMs->start(m_SendCyclems);
    }
    plainTextEditRecvUpdate(ContentTypeSend, timeStamp, HexData);
}

void MainWindow::SerialSendDataCycle()
{
    m_myserial->write(m_SendData);
    m_lastSendTimestamp = std::chrono::high_resolution_clock::now(); // 记录发送时间
    QByteArray HexData = m_SendData.toHex(' ');
    QString timeStamp = "00000ms.000us";
    plainTextEditRecvUpdate(ContentTypeSend, timeStamp, HexData);
}

void MainWindow::onSerialNotOpenedErr()
{
    QMessageBox::warning(this, "错误", "请先打开串口！");
}

/**
 * @brief MainWindow::onBtnSeg0DownloadClicked
 */
void MainWindow::onBtnSeg0DownloadClicked()
{
    QByteArray data;
    uint8_t seg0_data[9];

    if (!m_myserial->isOpen()) {
        emit(errSerialNotOpened());
        return;
    }

    int groupid = m_spbTargetGroupId->value();
    int segid = m_spbTargetSegId->value();
    int Synccmd = GenSyncFrame(groupid, LONG_CMD);
    data.append(Synccmd); //
    data.append(segid);
    uint8_t buf[2];
    buf[0] = segid;
    buf[1] = ProgData0_Order << 4;
    data.append(ProgData0_Order << 4 | crc4_itu(buf, 2));

    seg0_data[0] = m_spbSegInSeg0->value();
    seg0_data[1] = 0x00;
    for (int i = 0; i < 6; i++) {
        seg0_data[i + 2] = m_spbAddrInGrop[i]->value();
    }
    seg0_data[8] = 0x00;
    uint16_t crc16 = crc16_modbus(seg0_data, 9);
    for (int i = 0; i < 9; i++) {
        data.append(seg0_data[i]);
    }
    data.append(crc16 & 0xff);
    data.append(crc16 >> 8);

    SerailSendData(data);
}

void MainWindow::onBtnSeg1DownloadClicked()
{
    if (!m_myserial->isOpen()) {
        emit(errSerialNotOpened());
        return;
    }
    QByteArray data;
    uint8_t seg1_data[8] = {
        0x40, //PowerOnorder;???/??λ??LMS/SIU????	????1??PowerOnorder=0x80=SIU,??2??PowerOnorder=0x40=LMS????????????,PowerOnorder=0x41=LMS???????????
        0x01,
        0x01,
        42,
        40, //Tp_TxDelayNum
        0x13,
        0xff,
        0x07,
    };
    int groupid = m_spbTargetGroupId->value();
    int segid = m_spbTargetSegId->value();
    int Synccmd = GenSyncFrame(groupid, LONG_CMD);
    data.append(Synccmd); //
    data.append(segid);
    uint8_t buf[2];
    buf[0] = segid;
    buf[1] = ProgData1_Order << 4;
    data.append(ProgData1_Order << 4 | crc4_itu(buf, 2));

    int powerOnState = m_cbSeg1PowerOnState->currentData().toInt();
    seg1_data[0] = powerOnState;
    seg1_data[4] = m_spbSeg1TxDelay->value() * 10;
    for (int i = 0; i < 8; i++) {
        data.append(seg1_data[i]);
    }
    uint16_t crc16 = crc16_modbus(seg1_data, 8);
    data.append(crc16 & 0xff);
    data.append(crc16 >> 8);
    SerailSendData(data);
}

void MainWindow::onBtnSsCmdClicked()
{
    if (!m_myserial->isOpen()) {
        emit(errSerialNotOpened());
        return;
    }
    m_cmd_type = CMD_SS;
    int groupid = m_spbTargetGroupId->value();
    int Synccmd = GenSyncFrame(groupid, SHORT_CMD);
    QByteArray data;
    data.append(Synccmd);
    SerailSendData(data);
}

void MainWindow::onBtnLsOnCmdClicked()
{
    if (!m_myserial->isOpen()) {
        emit(errSerialNotOpened());
        return;
    }
    m_cmd_type = CMD_LS_ON;
    int groupid = m_spbTargetGroupId->value();
    int segid = m_spbTargetSegId->value();
    int Synccmd = GenSyncFrame(groupid, LONG_CMD);
    QByteArray data;
    data.append(Synccmd);
    data.append(segid);
    uint8_t buf[2];
    buf[0] = segid;
    buf[1] = CloseLamp_Order << 4;
    data.append(CloseLamp_Order << 4 | crc4_itu(buf, 2));

    SerailSendData(data);
}

void MainWindow::onBtnLsOffCmdClicked()
{
    if (!m_myserial->isOpen()) {
        emit(errSerialNotOpened());
        return;
    }
    m_cmd_type = CMD_LS_OFF;
    int groupid = m_spbTargetGroupId->value();
    int segid = m_spbTargetSegId->value();
    int Synccmd = GenSyncFrame(groupid, LONG_CMD);
    QByteArray data;
    data.append(Synccmd);
    data.append(segid);
    uint8_t buf[2];
    buf[0] = segid;
    buf[1] = OpenLamp_Order << 4;
    data.append(OpenLamp_Order << 4 | crc4_itu(buf, 2));

    SerailSendData(data);
}

void MainWindow::openSerial()
{
    //init serial parameters
    if (!m_serialOpenflag) {
        if (m_myserial->open(QIODevice::ReadWrite)) {
            m_serialOpenflag = true;
            m_toolbtnConnect->setText("断开连接");
            m_toolbtnConnect->setIcon(QIcon(":/images/disconnect.png"));
            statusBar()->showMessage("串口已连接", StatusBarMsgShowTime);
            m_combSerialPort->setEnabled(false);
            m_combBaud->setEnabled(false);
        } else {
            statusBar()->showMessage(tr("Connect failed: ") + m_myserial->errorString(), 5000);
        }
    } else {
        m_serialOpenflag = false;
        m_myserial->close();
        m_combSerialPort->setEnabled(true);
        m_combBaud->setEnabled(true);
        m_toolbtnConnect->setText("连接串口");
        m_toolbtnConnect->setIcon(QIcon(":/images/connect.png"));
        statusBar()->showMessage("串口已断开", StatusBarMsgShowTime);
    }
}

void MainWindow::onTargetIdChanged()
{
    int groupid, segid, Synccmd;
    QByteArray data;

    uint8_t buf[2];

    if (m_TimerSendCycleMs->isActive()) {
        groupid = m_spbTargetGroupId->value();
        m_SendData.clear();
        if (m_cmd_type == CMD_SS) {
            Synccmd = GenSyncFrame(groupid, SHORT_CMD);
            m_SendData.append(Synccmd);
        } else if (m_cmd_type == CMD_LS_ON) {
            Synccmd = GenSyncFrame(groupid, LONG_CMD);
            segid = m_spbTargetSegId->value();
            buf[0] = segid;
            buf[1] = CloseLamp_Order << 4;
            m_SendData.append(Synccmd);
            m_SendData.append(segid);
            m_SendData.append(CloseLamp_Order << 4 | crc4_itu(buf, 2));
        } else if (m_cmd_type == CMD_LS_OFF) {
            segid = m_spbTargetSegId->value();
            Synccmd = GenSyncFrame(groupid, LONG_CMD);
            buf[0] = segid;
            buf[1] = OpenLamp_Order << 4;
            m_SendData.append(Synccmd);
            m_SendData.append(segid);
            m_SendData.append(OpenLamp_Order << 4 | crc4_itu(buf, 2));
        }
    }
}

void MainWindow::fillComPort(QStringList const &list, bool addFlag, QString changedComName)
{
    static bool firstrun = true;
    int count = m_combSerialPort->count();
    qDebug() << count;
    //    int new_count = list.count();
    //    for (int i = 0; i < count; i++) {
    //        m_combSerialPort->removeItem(0);

    QString msg;
    msg = addFlag ? "新增" : "移除";
    msg += changedComName;

    if (firstrun) {
        firstrun = false;
    } else {
        statusBar()->showMessage(msg, StatusBarMsgShowTime);
    }
    if (!addFlag) //移除当前串口，则断开串口
    {
        if (changedComName == m_currentComName) {
            //断开连接
            m_serialOpenflag = false;
            m_myserial->close();
            m_combSerialPort->setEnabled(true);
            m_combBaud->setEnabled(true);
            m_toolbtnConnect->setText("连接串口");
            m_toolbtnConnect->setIcon(QIcon(":/images/connect.png"));
            statusBar()->showMessage("串口已断开", StatusBarMsgShowTime);
        }
    }
    if (m_serialOpenflag)
        return;
    m_combSerialPort->clear();
    m_combSerialPort->addItems(list);
    m_combSerialPort->setCurrentText(changedComName);
}
