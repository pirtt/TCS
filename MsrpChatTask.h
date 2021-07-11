//#ifndef MSRPCHATTASK_H
//#define MSRPCHATTASK_H
//#include "ChatTask.h"

//class MsrpChatTask : public ChatTask
//{
//public:
//    MsrpChatTask(ChatMsg* win);
//protected slots:
//    void OnMsrpRead();
//    void OnMsrpWrite(qint64 bytes);
//    void OnMsrpConnected();
//    void OnMsrpClosed();
//    void OnMsrpError(QAbstractSocket::SocketError err);
//private:
//    enum Sate
//    {
//        CLOSED,
//        WAIT_CONNECT,
//        CONNECT,
//        SEND,
//        FINISH,
//    };
//    ChatMsg*             pchatWin;
//    QTcpSocket*          m_msrpSock;
//    QHostAddress         m_localAddr;
//    QHostAddress         m_remoteAddr;
//    ushort               m_localPort;
//    ushort               m_remotePort;
//    bool                 m_msrpConnected;
//    Sate                 m_msrpState;
//    int64_t              m_total;
//    int64_t              m_doSize;
//};

//#endif // MSRPCHATTASK_H
