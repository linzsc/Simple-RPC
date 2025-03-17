#include <zookeeper/zookeeper.h>
#include <iostream>

int main() {
    zoo_set_debug_level(ZOO_LOG_LEVEL_INFO);
    std::cout << "ZooKeeper客户端库版本: " << zookeeper_version() << std::endl;
    return 0;
}