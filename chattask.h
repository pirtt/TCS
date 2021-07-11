#ifndef CHATTASK_H
#define CHATTASK_H
#include <QThread>
#include <QTcpServer>

class TCSWin;
class ChatMsg;
class ChatTask : public QObject
{
    Q_OBJECT
public :
    ChatTask(ChatMsg* chat);
    ~ChatTask() override{}
public slots:
    void DoTask();
protected:
    virtual void Process();
protected:
    ChatMsg*    m_chat;
};
#endif // CHATTASK_H
