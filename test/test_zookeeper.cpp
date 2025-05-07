#include "zk_wrapper.h"
#include <iostream>
#include <thread>
#include <chrono>

void testServiceRegistrationAndDiscovery(ZooKeeperWrapper& zk) {
    std::cout << "=== Testing Service Registration and Discovery ===\n";
    
    zk.registerService("CalculatorService", "127.0.0.1:12345");
    zk.registerService("CalculatorService", "127.0.0.1:12346");
    
    // 直接调用同步获取接口
    auto endpoints = zk.getServiceEndpoints("CalculatorService");

    std::cout << "Service endpoints:\n";
    for (const auto& endpoint : endpoints) {
        std::cout << " - " << endpoint << "\n";
    }

    if (endpoints.size() != 2) {
        std::cerr << "Test failed: Expected 2 endpoints, got " << endpoints.size() << "\n";
        return;
    }
    std::cout << "Test passed!\n";
}

void testServiceWatch(ZooKeeperWrapper& zk) {
    std::cout << "\n=== Testing Service Watch ===\n";

    // 注册观察者（注意：同步 watch 其实还是由 ZooKeeper 客户端异步触发的）
    zk.watchService("CalculatorService");

    std::cout << "Adding a new service instance...\n";
    zk.registerService("CalculatorService", "127.0.0.1:12347");

    // 这里可以适当等待一下，让 watch 回调有机会触发
    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto endpoints = zk.getServiceEndpoints("CalculatorService");
    std::cout << "Updated service endpoints:\n";
    for (const auto& endpoint : endpoints) {
        std::cout << " - " << endpoint << "\n";
    }

    if (endpoints.size() != 3) {
        std::cerr << "Test failed: Expected 3 endpoints, got " << endpoints.size() << "\n";
        return;
    }
    std::cout << "Test passed!\n";
}
void testServiceUnRegistration(ZooKeeperWrapper& zk) {
    std::cout << "\n=== Testing Service Unregistration ===\n";

    zk.unregisterService("CalculatorService", "127.0.0.1:12347");

    // 这里可以适当等待一下，让 watch 回调有机会触发
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto endpoints = zk.getServiceEndpoints("CalculatorService");
    std::cout << "Updated service endpoints:\n";
    for (const auto& endpoint : endpoints) {
        std::cout << " - " << endpoint << "\n";
    }

}
int main() {
    try {
        ZooKeeperWrapper zk("127.0.0.1:2181", 30000);

        testServiceRegistrationAndDiscovery(zk);
        testServiceWatch(zk);
        testServiceUnRegistration(zk);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
/*
g++ -o test_zk test/test_zookeeper.cpp src/zk_wrapper.cpp -I include -lzookeeper_mt -DTHREADED -lpthread -std=c++11
*/
