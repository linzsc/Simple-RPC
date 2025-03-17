#include "zk_wrapper.h"
#include <iostream>
#include <thread>
#include <chrono>

// 测试服务注册与发现
void testServiceRegistrationAndDiscovery(ZooKeeperWrapper& zk) {
    std::cout << "=== Testing Service Registration and Discovery ===\n";

    // 注册服务实例
    zk.registerService("CalculatorService", "127.0.0.1:12345");
    zk.registerService("CalculatorService", "127.0.0.1:12346");

    // 获取服务实例列表
    auto endpoints = zk.getServiceEndpoints("CalculatorService");
    std::cout << "Service endpoints:\n";
    for (const auto& endpoint : endpoints) {
        std::cout << " - " << endpoint << "\n";
    }

    // 预期结果：输出两个服务实例
    if (endpoints.size() != 2) {
        std::cerr << "Test failed: Expected 2 endpoints, got " << endpoints.size() << "\n";
        return;
    }
    std::cout << "Test passed!\n";
}

// 测试服务动态监听
void testServiceWatch(ZooKeeperWrapper& zk) {
    std::cout << "\n=== Testing Service Watch ===\n";

    // 监听服务变化
    zk.watchService("CalculatorService");

    // 模拟服务实例变化
    std::cout << "Adding a new service instance...\n";
    zk.registerService("CalculatorService", "127.0.0.1:12347");

    // 等待ZooKeeper通知
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 获取更新后的服务实例列表
    auto endpoints = zk.getServiceEndpoints("CalculatorService");
    std::cout << "Updated service endpoints:\n";
    for (const auto& endpoint : endpoints) {
        std::cout << " - " << endpoint << "\n";
    }

    // 预期结果：输出三个服务实例
    if (endpoints.size() != 3) {
        std::cerr << "Test failed: Expected 3 endpoints, got " << endpoints.size() << "\n";
        return;
    }
    std::cout << "Test passed!\n";
}

int main() {
    try {
        // 初始化ZooKeeper客户端
        ZooKeeperWrapper zk("127.0.0.1:2181");

        // 测试服务注册与发现
        testServiceRegistrationAndDiscovery(zk);

        // 测试服务动态监听
        testServiceWatch(zk);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
/*
g++ -o test_zookeeper test/test_zookeeper.cpp src/zk_wrapper.cpp -I include -lzookeeper_mt -std=c++11
g++ test_zookeeper.cpp src/zk_wrapper.cpp test/test_zookeeper.cpp -I include -o test_zookeeper -lzookeeper_mt
*/