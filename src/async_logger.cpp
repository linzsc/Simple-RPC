#include "async_logger.h"
#include <sys/stat.h>
#include <iostream>

Logger::Logger() {}

Logger::~Logger() {}

void Logger::init(const std::string& program_name, const std::string& log_dir) {
    if (mkdir(log_dir.c_str(), 0755) == -1) {
        if (errno != EEXIST) {
            std::cerr << "Failed to create log directory: " << log_dir << std::endl;
            exit(1);
        }
    }

    google::InitGoogleLogging(program_name.c_str());
    std::cout << "Google glog initialized." << std::endl;

    std::string info_log = log_dir + "/info_";
    std::string warning_log = log_dir + "/warning_";
    std::string error_log = log_dir + "/error_";
    std::string fatal_log = log_dir + "/fatal_";

    google::SetLogDestination(google::INFO, info_log.c_str());
    google::SetLogDestination(google::WARNING, warning_log.c_str());
    google::SetLogDestination(google::ERROR, error_log.c_str());
    google::SetLogDestination(google::GLOG_FATAL, fatal_log.c_str());

    FLAGS_logtostderr = false;
    FLAGS_alsologtostderr = false;
    FLAGS_colorlogtostderr = true;
    FLAGS_log_prefix = true;
    FLAGS_logbufsecs = 2;

    stop_background_ = false;
    background_thread_ = std::thread([this]() { this->backgroundWriter(); });
}

void Logger::log(const std::string& message, google::LogSeverity severity, const char* file, int line) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        async_queue_.emplace(message, severity);
    }
    queue_cv_.notify_one();
}

void Logger::backgroundWriter() {
    while (!stop_background_ || !async_queue_.empty()) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_cv_.wait(lock, [this] { return !async_queue_.empty() || stop_background_; });

        while (!async_queue_.empty()) {
            auto [msg, severity] = std::move(async_queue_.front());
            async_queue_.pop();
            lock.unlock();

            switch (severity) {
                case google::GLOG_INFO:
                    LOG(INFO) << msg;
                    break;
                case google::GLOG_WARNING:
                    LOG(WARNING) << msg;
                    break;
                case google::GLOG_ERROR:
                    LOG(ERROR) << msg;
                    break;
                case google::GLOG_FATAL:
                    LOG(FATAL) << msg;
                    break;
                default:
                    LOG(INFO) << msg;
                    break;
            }

            lock.lock();
        }
    }
}

void Logger::shutdown() {
    if (google::IsGoogleLoggingInitialized()) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            stop_background_ = true;
        }
        queue_cv_.notify_all();
        if (background_thread_.joinable()) {
            background_thread_.join();
        }
        google::ShutdownGoogleLogging();
    }
}