#include "ui_msg.h"
#include "ChatMsg.h"
#include "ChatTask.h"
#include "TCSWin.h"
#include "Log.h"

ChatMsg::ChatMsg(TCSWin *tcs, size_t index) : m_index(index), m_msg(new Ui::Msg), m_tcs(tcs)
{
    m_msg->setupUi(this);
    m_msg->process_send->setMaximum(100);
    m_msg->img_view->setVisible(false);
    m_errcode = Err::OK;
    m_isused = false;
    m_task = new ChatTask(this);
    m_msgThread = new QThread(this);
    m_task->moveToThread(m_msgThread);
    connect(this, SIGNAL(run()), m_task, SLOT(DoTask()));
}
ChatMsg::~ChatMsg()
{
    m_msgThread->terminate();
    killTimer(m_msgchecktimer);
    killTimer(m_msgtimer);
}

void ChatMsg::setMsgType(MsgType type)
{
    m_msgType = type;
}
void ChatMsg::setText(const QString &str)
{
    m_msg->img_view->setVisible(false);
    m_msg->text_content->setVisible(true);
    m_msg->text_content->setText(str);
}
void ChatMsg::setFileName(const QString &str) { m_msg->lab_filename->setText(str); }
void ChatMsg::setIcon(const QIcon &icon)
{
    m_msg->img_view->setVisible(true);
    m_msg->text_content->setVisible(false);
    m_msg->img_view->setIcon(icon);
}
void ChatMsg::setProcess(int value)
{
    m_msg->process_send->setValue(value > 100 ? 100 : value);
}
void ChatMsg::reset()
{
    setVisible(false);
    setProcess(0);
    m_msgThread->terminate();
    m_processVal = 0;
    m_errcode = Err::OK;
    m_isused = true;
}

void ChatMsg::start(ChatMsg::TransferType transType, ChatMsg::MsgType msgType, int timeout)
{
    m_msgTransFerType = transType;
    m_msgType = msgType;
    m_msgtimer = QWidget::startTimer(timeout, Qt::TimerType::PreciseTimer);
    m_msgchecktimer = QWidget::startTimer(100);
    m_msgThread->start();
    emit run();
}
bool ChatMsg::isUsed() const { return m_isused; }

const QString ChatMsg::getMsgDesc() const
{
    return QString("index=") + QString::number(m_index);
}

int ChatMsg::getChatType()
{
    return m_msgType;
}
void ChatMsg::paintEvent(QPaintEvent *) {}
void ChatMsg::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_msgtimer)
    {
        m_errcode = Err::TIME_OUT;
        m_processVal.store(100);
        setProcess(m_processVal);
    }
    else if (event->timerId() == m_msgchecktimer)
    {
        if (m_isused)
        {
            setProcess(m_processVal);
        }
    }
}
void ChatMsg::on_process_send_valueChanged(int value)
{
    if (value >= 100 && m_isused)
    {
        m_isused = false;
        killTimer(m_msgchecktimer);
        killTimer(m_msgtimer);
        m_msgThread->exit(0);
        m_tcs->OnChatFinished(this, m_errcode);
    }
}
