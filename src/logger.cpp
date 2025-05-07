#include "logger.h"
#include <glog/logging.h>
#include <iostream>
#include <sys/stat.h>
Logger::Logger() {
}

Logger::~Logger() {
    
    
}

void Logger::init(const std::string& program_name, const std::string& log_dir) {
    // 确保日志目录存在
    if (mkdir(log_dir.c_str(), 0755) == -1) {
        if (errno != EEXIST) {
            std::cerr << "Failed to create log directory: " << log_dir << std::endl;
            exit(1);
        }
    }

    // 初始化 Google glog
    google::InitGoogleLogging(program_name.c_str());
    std::cout << "Google glog initialized." << std::endl;
    // 设置日志路径
    std::string info_log = log_dir + "/info_";
    std::string warning_log = log_dir + "/warning_";
    std::string error_log = log_dir + "/error_";
    std::string fatal_log = log_dir + "/fatal_";

    google::SetLogDestination(google::INFO, info_log.c_str());
    google::SetLogDestination(google::WARNING, warning_log.c_str());
    google::SetLogDestination(google::ERROR, error_log.c_str());
    google::SetLogDestination(google::GLOG_FATAL, fatal_log.c_str());

    // 打印日志路径（调试用）
    /*
    std::cout << "Log paths:" << std::endl;
    std::cout << "INFO: " << info_log << std::endl;
    std::cout << "WARNING: " << warning_log << std::endl;
    std::cout << "ERROR: " << error_log << std::endl;
    std::cout << "FATAL: " << fatal_log << std::endl;
    */
   
    FLAGS_logtostderr = false;
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;
    FLAGS_log_prefix = true;
    FLAGS_logbufsecs = 0;
}

void Logger::log(const std::string& message, google::LogSeverity severity, const char* file, int line) {
    std::lock_guard<std::mutex> lock(log_mutex_);

    // 根据日志级别输出
    switch (severity) {
        case google::GLOG_INFO:
            LOG(INFO) << message;
            break;
        case google::GLOG_WARNING:
            LOG(WARNING) << message;
            break;
        case google::GLOG_ERROR:
            LOG(ERROR) << message;
            break;
        case google::GLOG_FATAL:
            LOG(FATAL) << message;
            break;
        default:
            LOG(INFO) << message;
            break;
    }
}

void Logger::shutdown() {
    if (google::IsGoogleLoggingInitialized()) {
        google::ShutdownGoogleLogging();
    }
}