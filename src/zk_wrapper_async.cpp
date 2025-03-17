#include "zk_wrapper_async.h"
#include <iostream>
#include <vector>
#include <algorithm>

ZooKeeperWrapper::ZooKeeperWrapper(const std::string& hosts, int timeout) {
    zk_handle_ = zookeeper_init(hosts.c_str(), watcher, timeout, nullptr, this, 0);
    if (!zk_handle_) {
        throw std::runtime_error("Failed to connect to ZooKeeper");
    }
}

ZooKeeperWrapper::~ZooKeeperWrapper() {
    zookeeper_close(zk_handle_);
}

void ZooKeeperWrapper::ensurePathExists(const std::string& path) {
    std::string currentPath;
    size_t pos = 0;
    while (1) {
        pos = path.find('/', pos + 1);
        currentPath = path.substr(0, pos);
        std::cout<<"currentpath : "<<currentPath<<std::endl;
        if (currentPath.empty()) continue;

        // 使用普通函数而非 lambda 表达式，避免捕获问题
        zoo_aexists(zk_handle_, currentPath.c_str(), 0, existsCallback, this);
        if(pos==std::string::npos){
            break;
        }
    }
}

// 静态成员函数作为回调
void ZooKeeperWrapper::existsCallback(int rc, const struct Stat* stat, const void* ctx) {
    auto wrapper = reinterpret_cast<ZooKeeperWrapper*>(const_cast<void*>(ctx));
    if (rc == ZNONODE) {
        std::cout<<"不存在!"<<std::endl;
        // 创建节点的回调也使用普通函数
        zoo_acreate(wrapper->zk_handle_, wrapper->currentPath_.c_str(), "", 0,
                    &ZOO_OPEN_ACL_UNSAFE, 0, createCallback, wrapper);
    }
}

// 静态成员函数作为创建节点的回调
void ZooKeeperWrapper::createCallback(int rc, const char* path, const void* ctx) {
    auto wrapper = reinterpret_cast<ZooKeeperWrapper*>(const_cast<void*>(ctx));
    if (rc != ZOK && rc != ZNODEEXISTS) {
        std::cerr << "Failed to create path: " << wrapper->currentPath_
                  << ", error: " << zerror(rc) << "\n";
    }
}
static void createNodeCallback(int rc, const char* path, const void* ctx) {
    auto wrapper = reinterpret_cast<ZooKeeperWrapper*>(const_cast<void*>(ctx));
    if (rc != ZOK) {
        std::cerr << "Failed to create node: " << zerror(rc) << "\n";
    }
}

void ZooKeeperWrapper::registerService(const std::string& service_name, const std::string& endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string basePath = "/services/" + service_name;
    ensurePathExists(basePath);

    std::string path = basePath + "/" + endpoint;
    std::cout<<"Path :"<<path<<std::endl;
    zoo_acreate(zk_handle_, path.c_str(), "", 0, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, createNodeCallback, this);
}



std::vector<std::string> ZooKeeperWrapper::getServiceEndpoints(
    const std::string& service_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> endpoints;
    std::string path = "/services/" + service_name;

    zoo_awget_children(zk_handle_, path.c_str(), watcher, this, [](int rc, const String_vector* strings, const void* ctx) {
        auto wrapper = reinterpret_cast<ZooKeeperWrapper*>(const_cast<void*>(ctx));
        if (rc == ZOK) {
            std::cout<<"获取成功:"<<strings->count<<std::endl;
            for (int i = 0; i < strings->count; ++i) {
                wrapper->endpoints_.push_back(strings->data[i]);
            }
        } else {
            std::cerr << "Failed to get children: " << zerror(rc) << "\n";
        }
    }, this);
    std::cout<<"endpoints_:"<<endpoints_.size()<<std::endl;
    return endpoints;
}

void ZooKeeperWrapper::watchService(const std::string& service_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string path = "/services/" + service_name;

    zoo_awget_children(zk_handle_, path.c_str(), watcher, this, [](int rc, const String_vector* strings, const void* ctx) {
        auto wrapper = reinterpret_cast<ZooKeeperWrapper*>(const_cast<void*>(ctx));
        if (rc == ZOK) {
            wrapper->endpoints_.clear();
            for (int i = 0; i < strings->count; ++i) {
                wrapper->endpoints_.push_back(strings->data[i]);
            }
        } else {
            std::cerr << "Failed to watch children: " << zerror(rc) << "\n";
        }
    }, this);
}

void ZooKeeperWrapper::watcher(zhandle_t* zh, int type, int state,
                              const char* path, void* context) {
    if (state == ZOO_CONNECTED_STATE && type == ZOO_CHILD_EVENT) {
        auto wrapper = reinterpret_cast<ZooKeeperWrapper*>(context);
        zoo_awget_children(zh, path, watcher, context, [](int rc, const String_vector* strings, const void* ctx) {
            auto wrapper = reinterpret_cast<ZooKeeperWrapper*>(const_cast<void*>(ctx));
            if (rc == ZOK) {
                wrapper->endpoints_.clear();
                for (int i = 0; i < strings->count; ++i) {
                    wrapper->endpoints_.push_back(strings->data[i]);
                }
                std::cout << "Service list updated!\n";
            }
        }, context);
    }
}