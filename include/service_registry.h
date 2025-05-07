#pragma once
#include <string>
#include <mutex>
#include "zk_wrapper.h"

class ServiceRegistry {
public:
    // 构造函数
    ServiceRegistry(ZooKeeperWrapper& zk):zk_(zk) {
    }

    // 析构函数
    ~ServiceRegistry() {
    }

    // 注册服务到 ZooKeeper
    void registerService(const std::string& service_name, const std::string& endpoint) {

        // 确保服务路径存在
        std::string path = "/services/" + service_name;
        zk_.ensurePathExists(path);

        // 注册服务实例
        zk_.registerService(service_name, endpoint);
    }
    //服务下线
    void unregisterService(const std::string& service_name, const std::string& endpoint) {

        // 注销服务实例
        zk_.unregisterService(service_name, endpoint);
    }

private:
    // ZooKeeperWrapper 实例
    ZooKeeperWrapper& zk_;
};