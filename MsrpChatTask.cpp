//#include "MsrpChatTask.h"

//MsrpChatTask::MsrpChatTask(ChatMsg* win):p
//{

//}

//void MsrpChatTask::OnMsrpRead()
//{
//    qDebug()<<m_msrpSock->readAll();
//}

//void MsrpChatTask::OnMsrpWrite(qint64 bytes)
//{
//    qDebug()<<bytes;
//}

//void MsrpChatTask::OnMsrpConnected()
//{
//    qDebug()<<"m_msrpConnected";
//    m_msrpConnected = true;
//    m_total=100;
//    if(m_msrpState == WAIT_CONNECT)
//    {
//        pchatWin->m_processVal = 10;
//        m_msrpState = CONNECT;
//    }
//}

//void MsrpChatTask::OnMsrpClosed()
//{
//    qDebug()<<"OnMsrpClosed";
//    m_msrpConnected = false;
//    m_msrpState = FINISH;
//    if(pchatWin->m_processVal != 100)
//    {
//        pchatWin->m_processVal = 100;
//        pchatWin->m_errcode = 1;
//    }
//}

//void MsrpChatTask::OnMsrpError(QAbstractSocket::SocketError err)
//{
//    qDebug()<<"OnMsrpError"<<err;
//    pchatWin->m_errcode= err;
//    m_msrpConnected = false;
//    if(pchatWin->m_processVal != 100)
//    {
//        pchatWin->m_processVal = 100;
//    }
//}


//    qDebug()<<"OK";
//    m_msrpSock->connectToHost("127.0.0.1",9988);
//    m_msrpSock->waitForConnected();
//    m_total=1000;
//    m_doSize=0;
//    m_msrpState = CONNECT;
//    if(m_msrpState == CONNECT || m_msrpState == SEND)
//    {
//        for(int i=0;m_doSize < m_total;i++)
//        {
//            m_msrpSock->write("1111");
//            m_doSize += 10;
//            if(m_doSize<m_total)
//            {
//                pchatWin->m_processVal += static_cast<int>(90 * (m_doSize*1.0f/m_total));
//            }
//            std::this_thread::sleep_for(std::chrono::milliseconds(30));
//        }
//    }
//    qDebug()<<"END";
//    pchatWin->m_processVal=100;
//    m_msrpState = FINISH;
    //    m_msrpSock->close();
