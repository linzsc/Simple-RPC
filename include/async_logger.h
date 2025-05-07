#ifndef LOGGER_H
#define LOGGER_H

#include <glog/logging.h>
#include <string>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void init(const std::string& program_name, const std::string& log_dir);
    void log(const std::string& message, google::LogSeverity severity, const char* file, int line);
    void shutdown();

private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void backgroundWriter();

private:
    std::queue<std::pair<std::string, google::LogSeverity>> async_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::thread background_thread_;
    bool stop_background_ = false;
};

#define LOG_INFO(message) \ 
    Logger::getInstance().log(message, google::GLOG_INFO, __FILE__, __LINE__)

#define LOG_WARNING(message) \ 
    Logger::getInstance().log(message, google::GLOG_WARNING, __FILE__, __LINE__)

#define LOG_ERROR(message) \ 
    Logger::getInstance().log(message, google::GLOG_ERROR, __FILE__, __LINE__)

#define LOG_FATAL(message) \ 
    Logger::getInstance().log(message, google::GLOG_FATAL, __FILE__, __LINE__)

#endif // LOGGER_H