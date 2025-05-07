#include "async_logger.h"
#include <thread>
#include <chrono>
#include <iostream>
int main() {
    Logger::getInstance().init("async_logger_test", "./log");

    const int num_threads = 4;
    const int logs_per_thread = 2500000; // 总共1000万条日志

    std::vector<std::thread> threads;
    auto start =std::chrono::system_clock::now();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i]() {
            for (int j = 0; j < logs_per_thread; ++j) {
                LOG_INFO("Thread " + std::to_string(i) + " - log number " + std::to_string(j));
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
    auto mid=std::chrono::system_clock::now();
    Logger::getInstance().shutdown();
    auto end=std::chrono::system_clock::now();
    auto submit= std::chrono::duration_cast<std::chrono::milliseconds>(mid-start).count();
    auto flush=std::chrono::duration_cast<std::chrono::milliseconds>(end-mid).count();
    std::cout<<"submit: "<<submit<<"ms, flush: "<<flush<<"ms"<<std::endl;
    return 0;
}
//g++ -o test_async_logger test/test_async.cpp src/async_logger.cpp -I include -lglog -lpthread -std=c++11

//submit=123ms
//flush=1941ms

//1867=ms

//1000万条，4个线程