#pragma once
#include <zookeeper/zookeeper.h>
#include <string>
#include <vector>
#include <mutex>

class ZooKeeperWrapper {
public:
    ZooKeeperWrapper(const std::string& hosts, int timeout);
    ~ZooKeeperWrapper();

    void ensurePathExists(const std::string& path);
    void registerService(const std::string& service_name, const std::string& endpoint);
    std::vector<std::string> getServiceEndpoints(const std::string& service_name);
    void watchService(const std::string& service_name);

    // 同步调用中 watcher 只用于日志通知（watch 仍然由 ZooKeeper 异步触发）
    static void watcher(zhandle_t* zh, int type, int state, const char* path, void* context);

private:
    zhandle_t* zk_handle_;
    std::mutex mutex_;
    std::vector<std::string> endpoints_;
};
