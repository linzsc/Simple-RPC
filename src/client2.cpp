#include "service_proxy.h"
#include <iostream>
#include <unistd.h>

//


int main() {
    Logger::getInstance().init("client2", "./logs");

    ZooKeeperWrapper zk("127.0.0.1:2181", 3000);
    //zk.registerService("CalculatorService", "127.0.0.1:12345");
    RoundRobinLoadBalancer roundRobinLB;
    // 创建服务代理,只需要绑定zk、负载均衡器、设置服务路径即可
    ServiceProxy<int> add(zk, roundRobinLB,"CalculatorService");
    

    // 调用远程服务
    try {
        int result = add.call("add", 10, 20);
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
/*
g++ -o client2 src/client2.cpp  src/zk_wrapper.cpp  src/logger.cpp -I include -lzookeeper_mt -DTHREADED  -std=c++11 -lpthread 
*/