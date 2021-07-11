#ifndef LOG_H
#define LOG_H
// TCS主窗口可以使用此日志记录
#define log_debug(X)                                     \
    {                                                    \
        std::ostringstream oss;                          \
        oss << __FILE__ << ":" << __LINE__ << ">>" << X; \
        Log(Qt::black, oss.str().c_str());               \
    }
#define log_error(X)                                     \
    {                                                    \
        std::ostringstream oss;                          \
        oss << __FILE__ << ":" << __LINE__ << ">>" << X; \
        Log(Qt::red, oss.str().c_str());                 \
    }
#define log_warn(X)                                      \
    {                                                    \
        std::ostringstream oss;                          \
        oss << __FILE__ << ":" << __LINE__ << ">>" << X; \
        Log(Qt::blue, oss.str().c_str());                \
    }
#endif // LOG_H
