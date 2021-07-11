#ifndef TCSWIN_H
#define TCSWIN_H
#include <QWidget>
#include <QTcpServer>
#include <QTimerEvent>
#include <QTimer>
#include <QListWidgetItem>
#include <memory>
#include <queue>
#include <mutex>
#include <QThread>
#include <QSettings>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QValueAxis>
QT_CHARTS_USE_NAMESPACE
// UI 相关
namespace Ui
{
    class Chat;
}
class ChatTask;
class ChatMsg;
// TCS 主窗口
class TCSWin : public QWidget
{
    Q_OBJECT
public :
    static TCSWin& GetInstance();
    TCSWin(QWidget *parent = nullptr);
    ~TCSWin() override;
    void Log(const QColor &color, const QString &str);
    void OnChatFinished(ChatMsg *msg,int err);
protected slots:
    void OnAccept();
    void OnRead();
    void OnClosed();
    void OnMsrpServerRead();
    void OnMsrpServerWrite(qint64 bytes);
    void OnMsrpServerConnected();
    void OnMsrpServerClosed();
    void OnMsrpServerError(QAbstractSocket::SocketError err);
    void OnFileAccsRead();
    void OnFileAccsWrite(qint64 bytes);
    void OnFileAccsConnected();
    void OnFileAccsClosed();
    void OnFileAccsError(QAbstractSocket::SocketError err);
protected:
    void timerEvent(QTimerEvent *event) override;
    virtual void TryConnectServer(int index);
    virtual void OnServerRead(int index);
    virtual void OnServerWrite(int index,qint64 bytes);
    virtual void OnServerConnected(int index);
    virtual void OnServerClosed(int index);
    virtual void OnServerError(int index,QAbstractSocket::SocketError err);
private slots:
    void on_btn_send_clicked();
    void on_btn_open_clicked();
    void on_btn_vcard_clicked();
    void on_btn_emoji_clicked();
    void on_btn_voice_clicked();
    void on_btn_local_clicked();
    void on_check_log_stateChanged(int arg1);
    void on_btn_clean_clicked();
    void on_ipaddr_msrpserver_editingFinished();
    void on_ipaddr_fileaccess_editingFinished();
    void on_combo_send_recv_currentIndexChanged(int index);
    void on_ipaddr_local_editingFinished();
    void on_combo_msrp_fileacc_currentIndexChanged(int index);

    void on_caller_editingFinished();

    void on_callee_editingFinished();

    void on_login_passwd_editingFinished();

private:
    ChatMsg *AcquireChatMsg();
    void StartWithFile(const QString &fileName);
    void StartWithText(const QString &text);
    void StartChat(ChatMsg *msg);

private:
    enum Method
    {
        UPLOAD   = 0,
        DOWNLOAD = 1,
    };
    enum ServerIndex
    {
        MSRPSERVER = 0,
        FILEACESS,
        MAX_SERVER,
    };
    using LogView = std::vector<std::shared_ptr<QListWidgetItem>>;
    using ChatPool = std::vector<std::shared_ptr<ChatMsg>>;
    using ChatQueue = std::queue<ChatMsg *>;
    using StatData =  int[5];
    QSettings            m_configSetting;
    QTcpServer          *m_server;
    QHostAddress         m_localAddr;
    int                  m_serverConnectTimeID;
    QTcpSocket*          m_serverSock[MAX_SERVER];
    QHostAddress         m_serverAddr[MAX_SERVER];
    ushort               m_serverPort[MAX_SERVER];
    bool                 m_serverConected[MAX_SERVER];
    Ui::Chat            *m_chatui;
    LogView              m_logview;
    int                  m_logCnt;
    bool                 m_islogdebug;
    ChatPool             m_chatPool;
    ChatQueue            m_chatFQ;
    int                  m_chatMethod;
    int                  m_chatServerIndex;
    std::recursive_mutex m_chatFQMutex;
    QBarSet             *m_sendStat;
    QBarSet             *m_recvStat;
    QBarSeries          *m_statSeries;
    QValueAxis          *m_statYAxis;
    StatData             m_statData[2];
};
#endif // CHATMSG_H
