#include "service_proxy.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>

using namespace std::chrono;

int main() {
    // 初始化日志和ZooKeeper
    Logger::getInstance().init("script", "./logs");
    ZooKeeperWrapper zk("127.0.0.1:2181", 3000);
    RoundRobinLoadBalancer roundRobinLB;

    // 创建服务代理
    ServiceProxy<int> add(zk, roundRobinLB, "CalculatorService");

    // 测试参数
    const int requestCount = 10000; // 总请求数
    const int concurrency = 10;   // 并发数

    // 记录开始时间
    auto start = steady_clock::now();

    // 使用线程池模拟并发
    std::vector<std::thread> threads;
    std::atomic<int> completedRequests(0); // 已完成的请求数

    auto executeRequests = [&](int count) {
        for (int i = 0; i < count; ++i) {
            try {
                int result = add.call("add", 10, 20);
                //std::cout << "Result: " << result << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
            completedRequests++;
        }
    };

    // 创建并发线程
    int requestsPerThread = requestCount / concurrency;
    for (int i = 0; i < concurrency; ++i) {
        threads.emplace_back(executeRequests, requestsPerThread);
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 记录结束时间
    auto end = steady_clock::now();
    duration<double> duration = end - start;

    // 计算QPS和其他指标
    double totalRequests = completedRequests.load();
    double totalTime = duration.count();
    double qps = totalRequests / totalTime;

    std::cout << "Total Requests: " << totalRequests << std::endl;
    std::cout << "Total Time: " << totalTime << " seconds" << std::endl;
    std::cout << "QPS: " << qps << std::endl;

    return 0;
}

/*
g++ -o test test/script.cpp  src/zk_wrapper.cpp  src/logger.cpp -I include -lzookeeper_mt -DTHREADED  -std=c++11 -lpthread -lglog
*/

/*
测试结果：

Total Requests: 10000
Total Time: 6.03765 seconds
QPS: 1656.27
*/