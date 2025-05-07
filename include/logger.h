#ifndef LOGGER_H
#define LOGGER_H

#include <glog/logging.h>
#include <string>
#include <mutex>



class Logger {
public:
    // 单例模式
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // 初始化日志系统
    void init(const std::string& program_name, const std::string& log_dir);

    // 日志输出接口
    void log(const std::string& message, google::LogSeverity severity, const char* file, int line);

    // 关闭日志系统
    void shutdown();

private:
    Logger();
    ~Logger();

    // 禁止拷贝和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    bool is_shutdown_;
    std::mutex log_mutex_; // 日志锁
};

// 日志宏定义
#define LOG_INFO(message) \
    Logger::getInstance().log(message, google::GLOG_INFO, __FILE__, __LINE__)

#define LOG_WARNING(message) \
    Logger::getInstance().log(message, google::GLOG_WARNING, __FILE__, __LINE__)

#define LOG_ERROR(message) \
    Logger::getInstance().log(message, google::GLOG_ERROR, __FILE__, __LINE__)

#define LOG_FATAL(message) \
    Logger::getInstance().log(message, google::GLOG_FATAL, __FILE__, __LINE__)

#endif // LOGGER_H