#include "logger.h"
#include <iostream>

int main() {
    // 初始化日志系统
    Logger::getInstance().init("test1_program", "./logs");

    // 测试不同级别的日志
    LOG_INFO("This is an info message.");
    LOG_WARNING("This is a warning message.");
    LOG_ERROR("This is an error message.");
    //LOG_FATAL("This is a fatal message.");

    // 关闭日志系统
    Logger::getInstance().shutdown();
    std::cout << "Logger shutdown." << std::endl;
    return 0;
}

/*
g++ -o test_logger test/test_logger.cpp src/logger.cpp -I include -lglog -lpthread -std=c++11
*/