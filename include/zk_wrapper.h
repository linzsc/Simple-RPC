#pragma once
#include <zookeeper/zookeeper.h>
#include <string>
#include <vector>
#include <mutex>

class ZooKeeperWrapper {
public:
    ZooKeeperWrapper(const std::string& hosts, int timeout = 30000);
    ~ZooKeeperWrapper();

    // 注册服务
    void registerService(const std::string& service_name, 
                       const std::string& endpoint);

    // 获取服务实例列表
    std::vector<std::string> getServiceEndpoints(const std::string& service_name);

    // 监听服务变化
    void watchService(const std::string& service_name);

private:
    static void watcher(zhandle_t* zh, int type, int state, 
                       const char* path, void* context);

    zhandle_t* zk_handle_;
    std::mutex mutex_;
    std::string hosts_;
    std::vector<std::string> endpoints_;
};