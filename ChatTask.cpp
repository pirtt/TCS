#include "ChatTask.h"
#include "ChatMsg.h"
#include "TCSWin.h"
#include "Log.h"
#include <QString>

ChatTask::ChatTask(ChatMsg *chat) : m_chat(chat)
{
    //    m_msrpSock = new QTcpSocket(this);
    //    connect(m_msrpSock, SIGNAL(readyRead()), this, SLOT(OnMsrpRead()));
    //    connect(m_msrpSock, SIGNAL(bytesWritten(qint64)), this, SLOT(OnMsrpWrite(qint64)));
    //    connect(m_msrpSock, SIGNAL(connected()), this, SLOT(OnMsrpConnected()));
    //    connect(m_msrpSock, SIGNAL(disconnected()), this, SLOT(OnMsrpClosed()));
    //    connect(m_msrpSock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnMsrpError(QAbstractSocket::SocketError)));
    //    m_msrpConnected = false;
}

void ChatTask::DoTask()
{
    while (m_chat->m_processVal < 100 && !m_chat->m_errcode && m_chat->isUsed())
    {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        Process();
        m_chat->m_tcs->Log(Qt::black, m_chat->getMsgDesc() + ",DoTask:" + QString::number(m_chat->m_processVal));
    }
    qDebug() << m_chat->getMsgDesc() << " Task end!";
}

void ChatTask::Process()
{
    for (; m_chat->m_processVal < 100; m_chat->m_processVal += 20)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(100000));
    }
    m_chat->m_errcode = 0;
}
