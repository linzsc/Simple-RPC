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
    
        static void watcher(zhandle_t* zh, int type, int state, const char* path, void* context);
    
    private:
        static void existsCallback(int rc, const struct Stat* stat, const void* ctx);
        static void createCallback(int rc, const char* path, const void* ctx);
        static void getChildrenCallback(int rc, const String_vector* strings, const void* ctx);
    
        zhandle_t* zk_handle_;
        std::mutex mutex_;
        std::vector<std::string> endpoints_;
        std::string currentPath_;
    };