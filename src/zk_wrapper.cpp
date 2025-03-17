#include "zk_wrapper.h"
#include <iostream>

ZooKeeperWrapper::ZooKeeperWrapper(const std::string& hosts, int timeout) {
    zk_handle_ = zookeeper_init(hosts.c_str(), watcher, timeout, nullptr, this, 0);
    if (!zk_handle_) {
        throw std::runtime_error("Failed to connect to ZooKeeper");
    }
}

ZooKeeperWrapper::~ZooKeeperWrapper() {
    zookeeper_close(zk_handle_);
}

void ZooKeeperWrapper::registerService(const std::string& service_name,
                                      const std::string& endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string path = "/services/" + service_name + "/" + endpoint;
    zoo_acreate(zk_handle_, path.c_str(), "", 0, &ZOO_OPEN_ACL_UNSAFE,
                ZOO_EPHEMERAL, [](int rc, const char* path, const void* context) {
                    if (rc != ZOK) {
                        std::cerr << "Failed to create node: " << zerror(rc) << "\n";
                    }
                }, nullptr);
}

std::vector<std::string> ZooKeeperWrapper::getServiceEndpoints(
    const std::string& service_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> endpoints;
    std::string path = "/services/" + service_name;
    String_vector children;
    zoo_awget_children(zk_handle_, path.c_str(), watcher, this, [](int rc, const String_vector* strings, const void* context) {
        auto wrapper = reinterpret_cast<ZooKeeperWrapper*>(const_cast<void*>(context));
        if (rc == ZOK) {
            for (int i = 0; i < strings->count; ++i) {
                wrapper->endpoints_.push_back(strings->data[i]);
            }
        } else {
            std::cerr << "Failed to get children: " << zerror(rc) << "\n";
        }
    }, this);
    return endpoints;
}

void ZooKeeperWrapper::watchService(const std::string& service_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string path = "/services/" + service_name;
    String_vector children;
    zoo_awget_children(zk_handle_, path.c_str(), watcher, this, [](int rc, const String_vector* strings, const void* context) {
        if (rc == ZOK) {
            std::cout << "Service list updated!\n";
        } else {
            std::cerr << "Failed to watch children: " << zerror(rc) << "\n";
        }
    }, this);
}

void ZooKeeperWrapper::watcher(zhandle_t* zh, int type, int state,
                              const char* path, void* context) {
    if (type == ZOO_CHILD_EVENT) {
        auto wrapper = reinterpret_cast<ZooKeeperWrapper*>(context);
        std::cout << "Service list updated!\n";
    }
}