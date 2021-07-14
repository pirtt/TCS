#include "TCSWin.h"
#include "Log.h"
#include "ui_chat.h"
#include "ChatMsg.h"
#include "ChatTask.h"
#include <QTcpSocket>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QCheckBox>
#include <QPainter>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <sstream>

TCSWin &TCSWin::GetInstance()
{
    static TCSWin tcswin;
    return tcswin;
}
TCSWin::TCSWin(QWidget *parent) : QWidget(parent), m_configSetting(QCoreApplication::applicationDirPath() + "/tcs.ini", QSettings::IniFormat), m_chatui(new Ui::Chat)
{
    // 日志缓存空间
    m_logCnt = 0;
    for (int i = 0; i < 50; i++)
    {
        m_logview.push_back(std::make_shared<QListWidgetItem>());
    }

    // 最大并发聊天数
    for (unsigned int i = 0; i < 2 * std::thread::hardware_concurrency();)
    {
        auto chat = std::make_shared<ChatMsg>(this, i++);
        m_chatPool.push_back(chat);
        m_chatFQ.push(chat.get());
    }
    m_chatui->setupUi(this);
    // setWindowFlags(Qt::FramelessWindowHint);
    m_chatui->combo_msrp_fileacc->setCurrentIndex(MSRPSERVER);
    m_chatServerIndex = MSRPSERVER;
    m_chatui->combo_send_recv->setCurrentIndex(Method::UPLOAD);
    m_chatMethod = Method::UPLOAD;
    m_islogdebug = m_chatui->check_log->isChecked();
    m_chatui->scrollhis->setLayout(new QVBoxLayout());
    m_chatui->scrollhis->layout()->setSpacing(5);
    // 加载配置
    m_chatui->ipaddr_msrpserver->setText(m_configSetting.value("MsrpServerIP").toString());
    m_serverAddr[MSRPSERVER] = QHostAddress(m_chatui->ipaddr_msrpserver->text());
    m_chatui->ipaddr_fileaccess->setText(m_configSetting.value("FileAccessIP").toString());
    m_serverAddr[FILEACESS] = QHostAddress(m_chatui->ipaddr_fileaccess->text());
    m_chatui->ipaddr_local->setText(m_configSetting.value("LocalIP").toString());
    m_localAddr = QHostAddress(m_chatui->ipaddr_local->text());
    m_chatui->caller->setText(m_configSetting.value("Caller").toString());
    m_chatui->callee->setText(m_configSetting.value("Callee").toString());
    m_chatui->login_passwd->setText(m_configSetting.value("Passwd").toString());
    // tcp相关
    for (int i = 0; i < MAX_SERVER; i++)
    {
        m_serverConected[i] = false;
        m_serverSock[i] = new QTcpSocket(this);
        switch (i)
        {
        case MSRPSERVER:
            m_serverPort[i] = 7070;
            connect(m_serverSock[i], SIGNAL(readyRead()), this, SLOT(OnMsrpServerRead()));
            connect(m_serverSock[i], SIGNAL(bytesWritten(qint64)), this, SLOT(OnMsrpServerWrite(qint64)));
            connect(m_serverSock[i], SIGNAL(connected()), this, SLOT(OnMsrpServerConnected()));
            connect(m_serverSock[i], SIGNAL(disconnected()), this, SLOT(OnMsrpServerClosed()));
            connect(m_serverSock[i], SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnMsrpServerError(QAbstractSocket::SocketError)));
            break;
        case FILEACESS:
            m_serverPort[i] = 5050;
            connect(m_serverSock[i], SIGNAL(readyRead()), this, SLOT(OnFileAccsRead()));
            connect(m_serverSock[i], SIGNAL(bytesWritten(qint64)), this, SLOT(OnFileAccsWrite(qint64)));
            connect(m_serverSock[i], SIGNAL(connected()), this, SLOT(OnFileAccsConnected()));
            connect(m_serverSock[i], SIGNAL(disconnected()), this, SLOT(OnFileAccsClosed()));
            connect(m_serverSock[i], SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnFileAccsError(QAbstractSocket::SocketError)));
            break;
        default:
            break;
        }
        TryConnectServer(i);
    }

    m_serverConnectTimeID = startTimer(1000);
    m_server = new QTcpServer(this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(OnAccept()));
    m_server->listen(QHostAddress::Any, 9988);

    // 绘制统计
    m_sendStat = new QBarSet("send", this);
    m_recvStat = new QBarSet("recv", this);
    for (int i = 0; i < 5; i++)
    {
        *m_sendStat << 0;
        *m_recvStat << 0;
        m_statData[0][i] = 0;
        m_statData[1][i] = 0;
    }
    m_statSeries = new QBarSeries(this); //给每一列分配区域
    m_statSeries->append(m_sendStat);
    m_statSeries->append(m_recvStat);

    QChart *chart = new QChart();
    chart->addSeries(m_statSeries);                       //将serise添加到Char中
    chart->setTitle("recv and send msg Stat");            //char 的标题设置为
    chart->setAnimationOptions(QChart::SeriesAnimations); //动画在图表中启用
    QStringList categories;
    categories << "Text"
               << "Voice"
               << "File"
               << "Location"
               << "Emoji";
    QBarCategoryAxis *axisX = new QBarCategoryAxis(this);
    axisX->append(categories);              //设置X标签
    chart->addAxis(axisX, Qt::AlignBottom); //将系列标签放到底下
    m_statSeries->attachAxis(axisX);
    m_statYAxis = new QValueAxis(this);
    m_statYAxis->setRange(0,1000);
    chart->addAxis(m_statYAxis, Qt::AlignLeft);
    m_statSeries->attachAxis(m_statYAxis);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    m_chatui->tab_stat->layout()->addWidget(chartView);
}
TCSWin::~TCSWin()
{
    m_configSetting.sync();
    killTimer(m_serverConnectTimeID);
    m_server->close();
    m_logview.clear();
    m_chatPool.clear();
}

void TCSWin::OnChatFinished(ChatMsg *msg, int err)
{
    if (err != 0)
    {
        log_error(msg->getMsgDesc().toStdString().data() << ",err=" << err);
    }
    else
    {
        log_debug(msg->getMsgDesc().toStdString().data() << ",err=" << err);
    }
    msg->setVisible(false);
    m_chatui->scrollhis->layout()->removeWidget(msg);
    std::lock_guard<std::recursive_mutex> lk(m_chatFQMutex);
    m_chatFQ.push(msg);
}
void TCSWin::Log(const QColor &color, const QString &str)
{
    if (!m_islogdebug && color == Qt::black)
    {
        return;
    }
    auto index = static_cast<unsigned int>(m_logCnt++) % m_logview.size();
    if (index == 0)
    {
        for (auto iter : m_logview)
        {
            iter->setText("");
            iter->setTextColor(Qt::black);
        }
    }
    auto item = m_logview[index];
    item->setTextColor(color);
    item->setText(str);
    m_chatui->loglist->insertItem(m_logCnt++, item.get());
}

ChatMsg *TCSWin::AcquireChatMsg()
{
    ChatMsg *ret = nullptr;
    std::lock_guard<std::recursive_mutex> lk(m_chatFQMutex);
    if (!m_chatFQ.empty())
    {
        ret = m_chatFQ.front();
        m_chatFQ.pop();
        ret->reset();
    }
    return ret;
}

void TCSWin::StartWithFile(const QString &fileName)
{
    int beginindex = m_chatServerIndex;
    int endindex = m_chatServerIndex;
    if (m_chatServerIndex == 2)
    {
        beginindex = MSRPSERVER;
        endindex = FILEACESS;
    }
    for (; beginindex <= endindex; ++beginindex)
    {
        auto chat = AcquireChatMsg();
        if (chat == nullptr)
        {
            log_error("no empty chatmsg to send,wait try again !");
            continue;
        }
        ++m_statData[0][ChatMsg::FILE];
        QFileInfo file_info(fileName);
        QFileIconProvider icon_provider;
        QIcon icon = icon_provider.icon(file_info);
        chat->setIcon(icon);
        chat->setFileName(fileName);
        chat->start(static_cast<ChatMsg::TransferType>(beginindex), ChatMsg::FILE, 10000);
        StartChat(chat);
    }
}

void TCSWin::StartWithText(const QString &text)
{
    int beginindex = m_chatServerIndex;
    int endindex = m_chatServerIndex;
    if (m_chatServerIndex == 2)
    {
        beginindex = MSRPSERVER;
        endindex = FILEACESS;
    }
    for (; beginindex <= endindex; ++beginindex)
    {
        auto chat = AcquireChatMsg();
        if (chat == nullptr)
        {
            log_error("no empty chatmsg to send,wait try again !");
            continue;
        }
        ++m_statData[0][ChatMsg::TEXT];
        chat->setText(text);
        chat->start(static_cast<ChatMsg::TransferType>(beginindex), ChatMsg::TEXT, 10000);
        StartChat(chat);
    }
}

void TCSWin::StartChat(ChatMsg *msg)
{
    m_chatui->scrollhis->layout()->addWidget(msg);
    msg->setVisible(true);
}
void TCSWin::OnAccept()
{
    QTcpSocket *aa = m_server->nextPendingConnection();
    connect(aa, SIGNAL(readyRead()), this, SLOT(OnRead()));
    connect(aa, SIGNAL(disconnected()), this, SLOT(OnClosed()));
}
void TCSWin::OnRead()
{
    QTcpSocket *tcp = dynamic_cast<QTcpSocket *>(sender());
    log_debug(tcp->readAll().toStdString().c_str());
    Q_UNUSED(tcp);
}
void TCSWin::OnClosed() { log_debug("closed"); }

void TCSWin::OnMsrpServerRead()
{
    return OnServerRead(MSRPSERVER);
}

void TCSWin::OnMsrpServerWrite(qint64 bytes)
{
    return OnServerWrite(MSRPSERVER, bytes);
}

void TCSWin::OnMsrpServerConnected()
{
    m_chatui->msrpserver_check->setChecked(true);
    return OnServerConnected(MSRPSERVER);
}

void TCSWin::OnMsrpServerClosed()
{
    m_chatui->msrpserver_check->setChecked(false);
    return OnServerClosed(MSRPSERVER);
}

void TCSWin::OnMsrpServerError(QAbstractSocket::SocketError err)
{
    return OnServerError(MSRPSERVER, err);
}

void TCSWin::OnFileAccsRead()
{
    return OnServerRead(FILEACESS);
}
void TCSWin::OnFileAccsWrite(qint64 bytes)
{
    return OnServerWrite(FILEACESS, bytes);
}

void TCSWin::OnFileAccsConnected()
{
    m_chatui->fileaccess_check->setChecked(true);
    return OnServerConnected(FILEACESS);
}

void TCSWin::OnFileAccsClosed()
{
    m_chatui->fileaccess_check->setChecked(false);
    return OnServerClosed(FILEACESS);
}

void TCSWin::OnFileAccsError(QAbstractSocket::SocketError err)
{
    return OnServerError(FILEACESS, err);
}
void TCSWin::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_serverConnectTimeID)
    {
        for (int i = 0; i < ServerIndex::MAX_SERVER; i++)
        {
            if (!m_serverConected[i])
            {
                TryConnectServer(i);
            }
        }
        for (int i = 0; i < 5; i++)
        {
            m_sendStat->insert(i,++m_statData[0][i]);
            m_recvStat->insert(i,++m_statData[1][i]);
            m_statSeries->insert(0,m_sendStat);
            m_statSeries->insert(1,m_recvStat);
            m_statYAxis->setRange(0,100);
        }
    }
}

void TCSWin::OnServerRead(int index)
{
    Q_UNUSED(index);
}

void TCSWin::OnServerWrite(int index, qint64 bytes)
{
    Q_UNUSED(index);
    Q_UNUSED(bytes);
}

void TCSWin::OnServerConnected(int index)
{
    m_serverConected[index] = true;
    log_debug(m_serverAddr[index].toString().toStdString().c_str() << ":" << m_serverPort[index] << " connected!");
}

void TCSWin::OnServerClosed(int index)
{
    m_serverConected[index] = false;
    log_warn(m_serverAddr[index].toString().toStdString().c_str() << ":" << m_serverPort[index] << " disconnected!");
}

void TCSWin::OnServerError(int index, QAbstractSocket::SocketError err)
{
    log_error(m_serverAddr[index].toString().toStdString().c_str() << ":" << m_serverPort[index] << " err=" << err);
}
void TCSWin::TryConnectServer(int index)
{
    if (index < ServerIndex::MAX_SERVER)
    {
        log_debug("try connect " << m_serverAddr[index].toString().toStdString().c_str() << ":" << m_serverPort[index]);
        m_serverSock[index]->abort();
        m_serverSock[index]->bind(m_localAddr);
        m_serverSock[index]->connectToHost(m_serverAddr[index], m_serverPort[index]);
    }
}
void TCSWin::on_btn_send_clicked()
{
    StartWithText(m_chatui->edit_input->toPlainText());
    m_chatui->edit_input->clear();
}
void TCSWin::on_btn_open_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "[TCS] Select File ", QDir::homePath(), tr("*.*"));
    if (!fileName.isEmpty())
    {
        StartWithFile(fileName);
    }
}
void TCSWin::on_btn_vcard_clicked() {}
void TCSWin::on_btn_emoji_clicked() {}
void TCSWin::on_btn_voice_clicked() {}
void TCSWin::on_btn_local_clicked() {}
void TCSWin::on_check_log_stateChanged(int arg1)
{

    Q_UNUSED(arg1);
    m_islogdebug = m_chatui->check_log->isChecked();
}
void TCSWin::on_btn_clean_clicked()
{
    for (auto iter : m_logview)
    {
        iter->setText("");
        iter->setTextColor(Qt::black);
    }
    m_logCnt = 0;
}

void TCSWin::on_ipaddr_msrpserver_editingFinished()
{
    const QString &host = m_chatui->ipaddr_msrpserver->text();
    QHostAddress addr;
    if (!addr.setAddress(host))
    {
        log_error(host.toStdString().c_str() << " is invalid");
        m_chatui->ipaddr_msrpserver->setStyleSheet("color:red");
        return;
    }
    m_chatui->ipaddr_msrpserver->setStyleSheet("");
    if (addr != m_serverAddr[MSRPSERVER])
    {
        m_serverAddr[MSRPSERVER] = addr;
        m_configSetting.setValue("MsrpServerIP", addr.toString());
        m_configSetting.sync();
        TryConnectServer(MSRPSERVER);
    }
}

void TCSWin::on_ipaddr_fileaccess_editingFinished()
{
    const QString &host = m_chatui->ipaddr_fileaccess->text();
    QHostAddress addr;
    if (!addr.setAddress(host))
    {
        log_error(host.toStdString().c_str() << " is invalid");
        m_chatui->ipaddr_fileaccess->setStyleSheet("color:red");
        return;
    }
    m_chatui->ipaddr_fileaccess->setStyleSheet("");
    if (addr != m_serverAddr[FILEACESS])
    {
        m_serverAddr[FILEACESS] = addr;
        m_configSetting.setValue("FileAccessIP", addr.toString());
        m_configSetting.sync();
        TryConnectServer(FILEACESS);
    }
}

void TCSWin::on_ipaddr_local_editingFinished()
{
    const QString &host = m_chatui->ipaddr_msrpserver->text();
    QHostAddress addr;
    if (!addr.setAddress(host))
    {
        log_error(host.toStdString().c_str() << " is invalid");
        m_chatui->ipaddr_local->setStyleSheet("color:red");
        return;
    }
    m_chatui->ipaddr_local->setStyleSheet("");
    if (addr != m_localAddr)
    {
        m_localAddr = addr;
        m_configSetting.setValue("LocalIP", m_localAddr.toString());
        m_configSetting.sync();
        for (int i = 0; i < ServerIndex::MAX_SERVER; i++)
        {
            TryConnectServer(i);
        }
    }
}

void TCSWin::on_combo_send_recv_currentIndexChanged(int index)
{
    m_chatMethod = index;
    switch (m_chatMethod)
    {
    case Method::DOWNLOAD:
        m_chatui->btn_open->setEnabled(false);
        m_chatui->btn_emoji->setEnabled(false);
        m_chatui->btn_local->setEnabled(false);
        m_chatui->btn_vcard->setEnabled(false);
        m_chatui->btn_voice->setEnabled(false);
        break;
    case Method::UPLOAD:
    default:
        m_chatui->btn_open->setEnabled(true);
        m_chatui->btn_emoji->setEnabled(true);
        m_chatui->btn_local->setEnabled(true);
        m_chatui->btn_vcard->setEnabled(true);
        m_chatui->btn_voice->setEnabled(true);
        m_chatMethod = Method::UPLOAD;
        break;
    };
}
void TCSWin::on_combo_msrp_fileacc_currentIndexChanged(int index)
{
    m_chatServerIndex = index;
}

void TCSWin::on_caller_editingFinished()
{
    m_configSetting.setValue("Caller", m_chatui->caller->text());
    m_configSetting.sync();
}

void TCSWin::on_callee_editingFinished()
{
    m_configSetting.setValue("Callee", m_chatui->caller->text());
    m_configSetting.sync();
}

void TCSWin::on_login_passwd_editingFinished()
{
    m_configSetting.setValue("Passwd", m_chatui->login_passwd->text());
    m_configSetting.sync();
}
