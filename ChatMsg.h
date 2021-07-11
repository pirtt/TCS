#ifndef CHATMSG_H
#define CHATMSG_H
#include <QWidget>
#include <QTimerEvent>
#include <QTimer>
#include <memory>
#include <QThread>

namespace Ui
{
    class Msg;
}
class TCSWin;
class ChatTask;
class ChatMsg : public QWidget
{
    Q_OBJECT
public :
    enum Err
    {
        OK = 0,
        TIME_OUT,
        DISCONNECT,
        SOCKET_ERROR,
    };
    enum MsgType
    {
        TEXT =0,
        VOICE,
        FILE,
        EMOJI,
        LACATION,
    };
    enum TransferType
    {
        MSRP=0,
        MSRPS,
        HTTP,
        HTTPS,
    };
    ChatMsg(TCSWin *tcs, size_t index);
    ~ChatMsg() override;
    void setMsgType(MsgType type);
    void setText(const QString &str);
    void setFileName(const QString &str);
    void setIcon(const QIcon &icon);
    void reset();
    void start(TransferType transType,MsgType msgType,int timeout = 10000);
    bool isUsed() const;
    const QString getMsgDesc() const;
    int getChatType();
protected:
    friend class ChatTask;
    void setProcess(int value);
    void paintEvent(QPaintEvent *) override;
    void timerEvent(QTimerEvent *event) override;
protected:
    const size_t     m_index;           // 消息唯一索引
    Ui::Msg         *m_msg;             // UI显示
    MsgType          m_msgType;         // 消息类型
    TransferType     m_msgTransFerType; // 传输类型
    int              m_msgtimer;        // 消息总超时定时器
    int              m_msgchecktimer;   // UI刷新定时器
    QThread         *m_msgThread;       // 消息处理线程
    ChatTask        *m_task;            // 消息处理器
    std::atomic_int  m_processVal;      // 处理进度值
    bool             m_isused;          // 是否正在使用中
    int              m_errcode;         // 消息处理错误码
    TCSWin          *m_tcs;             // 控制窗口
private slots:
    void on_process_send_valueChanged(int value);
signals:
    void run();
};

#endif // CHATMSG_H
