#include <stdio.h>
#include <memory>
#include <iostream>
#include <vector>
#include <mutex>
#include <queue>
#include <unordered_map>

using namespace std;

class SessionHome;
class Session
{
public:
    static const uint8_t INVALID_THREADID = 0xFF;
    enum class SessionSate
    {
        GET_SESSION,
        ACTION,
        WAIT_CLOSE,
        CLOSED,
    };
public:
    const size_t GetSessionID()const
    {
        return m_sessionID;
    }
    SessionSate GetSessionSate()const
    {
        return m_sessionState;
    }
protected:
    void SetThreadID(uint8_t ID)
    {
        m_threadID = ID;
    }
    void GoHome()
    {
        m_sessionState = SessionSate::CLOSED;
        m_home.Back(m_sessionID);
    }
    void Reset()
    {
        m_sessionState = SessionSate::GET_SESSION;
        m_threadID = INVALID_THREADID;
    }
private:
    friend class SessionHome;
    explicit Session(SessionHome& home, uint32_t sessionID) :
        m_home(home),
        m_sessionID(sessionID)
    {
    }
    SessionHome&   m_home;
    const uint32_t m_sessionID;
    SessionSate    m_sessionState;
    uint8_t        m_threadID;
};

class SessionHome
{
public:
    SessionHome() :m_maxSession(1000), m_sessionSeed(0), m_isInited(false)
    {
    }
    void Init(const uint32_t& initSize, const uint32_t& maxSize)
    {
        std::lock_guard<std::mutex> safe_lock(m_pool_mutex);
        if (!m_isInited)
        {
            m_isInited = true;
            if (initSize > 0)
            {
                m_maxSession = maxSize > initSize ? maxSize : initSize;
                for (uint32_t s = 0; s < initSize; s++)
                {
                    MakeSession();
                }
            }
        }
    }
    std::shared_ptr<Session> Get()
    {
        std::lock_guard<std::mutex> safe_lock(m_pool_mutex);
        std::shared_ptr<Session> session;
         do 
         {
             if (!m_freeSessions.empty())
             {
                 session = m_freeSessions.front();
                 m_freeSessions.pop();
                 m_usedSessions.emplace(session->GetSessionID(), session);
                 break;
             }
             MakeSession();
         } while (true);
        return session;
    }
    std::shared_ptr<Session> FindSession(const uint32_t sessionID) const
    {
        std::lock_guard<std::mutex> safe_lock(m_pool_mutex);
        std::shared_ptr<Session> session;
        auto iterS = m_usedSessions.find(sessionID);
        if (iterS != m_usedSessions.end())
        {
            session = iterS->second;
        }
        return session;
    }
    size_t GetAllSessionSize() const
    {
        return static_cast<size_t>(m_freeSessions.size() + m_usedSessions.size());
    }
    size_t GetUsedSessionSize() const
    {
        return m_usedSessions.size();
    }
private:
    void Back(const uint32_t sessionID)
    {
        std::lock_guard<std::mutex> safe_lock(m_pool_mutex);
        if (m_usedSessions.find(sessionID) != m_usedSessions.end())
        {
            std::shared_ptr<Session> session = m_usedSessions[sessionID];
            m_usedSessions.erase(sessionID);
            m_freeSessions.push(session);
        }
    }
    void MakeSession()
    {
        try
        {
            std::shared_ptr<Session> session = std::make_shared<Session>(*this, m_sessionSeed++);
            m_freeSessions.push(session);
        }
        catch (...)
        {
            // RUN_ERROR("....");
        }
    }
private:
    friend class Session;
    using FreeSessionsType = std::queue<std::shared_ptr<Session>>;
    using UsedSessionsType = std::unordered_map<uint32_t, std::shared_ptr<Session>>;
    FreeSessionsType       m_freeSessions;
    UsedSessionsType       m_usedSessions;
    uint32_t               m_maxSession;
    uint32_t               m_sessionSeed;
    bool                   m_isInited;
    mutable std::mutex     m_pool_mutex;
};


int main()
{


    getchar();

    return 0;
}