#include "service_proxy.h"
#include <iostream>
#include <unistd.h>

//
ZooKeeperWrapper zk("127.0.0.1:2181", 3000);
RoundRobinLoadBalancer roundRobinLB;
int main() {

    // 创建服务代理,只需要绑定zk、负载均衡器、设置服务路径即可
    ServiceProxy<int> add(zk, roundRobinLB,"/CalculatorService/");
    

    // 调用远程服务
    try {
        int result = add.call("add", 10, 20);
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}