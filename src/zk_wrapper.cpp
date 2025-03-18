#include "zk_wrapper.h"
#include <iostream>
#include <sstream>
#include <vector>

ZooKeeperWrapper::ZooKeeperWrapper(const std::string& hosts, int timeout) {
    
    zk_handle_ = zookeeper_init(hosts.c_str(), watcher, timeout, nullptr, nullptr, 0);
    if (!zk_handle_) {
        throw std::runtime_error("Failed to connect to ZooKeeper");
    }
}

ZooKeeperWrapper::~ZooKeeperWrapper() {
    zookeeper_close(zk_handle_);
}

void ZooKeeperWrapper::ensurePathExists(const std::string& path) {
    size_t pos = 0; 
    std::cout<<"whole path :"<<path<<std::endl; 
    
    while ((pos = path.find('/', pos + 1)) ) {
        std::string subPath = path.substr(0, pos);
        std::cout<<"cur path :"<<subPath<<std::endl; 
        if (subPath.empty())
            continue;
        struct Stat stat;
        int rc = zoo_exists(zk_handle_, subPath.c_str(), 0, &stat);
        std::cout<<"zoo_exists, rc:"<<rc<<std::endl;
        if (rc == ZNONODE) {
            char buffer[512];
            int buffer_len = sizeof(buffer);
            rc = zoo_create(zk_handle_, subPath.c_str(), "", 0,
                            &ZOO_OPEN_ACL_UNSAFE, 0, buffer, buffer_len);
            if (rc != ZOK && rc != ZNODEEXISTS) {
                std::cerr << "Failed to create path: " << subPath
                          << ", error: " << zerror(rc) << "\n";
            } else {
                std::cout << "Created path: " << subPath << "\n";
            }
        }
        if(pos==std::string::npos){
            break;
        }
    }
}

void ZooKeeperWrapper::registerService(const std::string& service_name, const std::string& endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string basePath = "/services/" + service_name;
    // 确保所有父节点存在
    ensurePathExists(basePath);
    std::cout<<"endpoint: "<<endpoint<<std::endl;
    std::string path = basePath + "/" + endpoint;
    char buffer[512];
    int buffer_len = sizeof(buffer);

    // 创建临时节点
    //int rc = zoo_create(zk_handle_, path.c_str(), "", 0, &ZOO_OPEN_ACL_UNSAFE,
    //                    ZOO_EPHEMERAL, buffer, buffer_len);
    // 创建持久节点
    int rc = zoo_create(zk_handle_, path.c_str(), "", 0, &ZOO_OPEN_ACL_UNSAFE,
                        ZOO_PERSISTENT, buffer, buffer_len);
    std::cout<<rc<<std::endl;
    if (rc != ZOK && rc != ZNODEEXISTS) {
        std::cerr << "Failed to create node: " << zerror(rc) << "\n";
    } else {
        std::cout << "Registered service at: " << path << "\n";
    }
}

std::vector<std::string> ZooKeeperWrapper::getServiceEndpoints(const std::string& service_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> endpoints;
    std::string path = "/services/" + service_name;
    struct String_vector strings;
    int rc = zoo_get_children(zk_handle_, path.c_str(), 0, &strings);
    if (rc == ZOK) {
        for (int i = 0; i < strings.count; ++i) {
            endpoints.push_back(strings.data[i]);
        }
        deallocate_String_vector(&strings);
    } else {
        std::cerr << "Failed to get children: " << zerror(rc) << "\n";
    }
    return endpoints;
}

void ZooKeeperWrapper::watchService(const std::string& service_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string path = "/services/" + service_name;
    // 第三个参数设为 1 表示启用监控，监控事件将由 zookeeper_init 时指定的默认 watcher 回调处理
    struct String_vector strings;
    int rc = zoo_get_children(zk_handle_, path.c_str(), 1, &strings);
    if (rc == ZOK) {
        endpoints_.clear();
        for (int i = 0; i < strings.count; ++i) {
            endpoints_.push_back(strings.data[i]);
        }
        deallocate_String_vector(&strings);
    } else {
        std::cerr << "Failed to watch children: " << zerror(rc) << "\n";
    }
}


void ZooKeeperWrapper::watcher(zhandle_t* zh, int type, int state, const char* path, void* context) {
    if (state == ZOO_CONNECTED_STATE && type == ZOO_CHILD_EVENT) {
        std::cout << "Watch triggered: changes detected at path " << path << "\n";
    }
}


void ZooKeeperWrapper::disconnect(){
    zookeeper_close(zk_handle_);
}

void ZooKeeperWrapper::unregisterService(const std::string& service_name, const std::string& endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string path = "/services/" + service_name + "/" + endpoint;
    int rc = zoo_delete(zk_handle_, path.c_str(), -1);
    if (rc != ZOK) {
        std::cerr << "Failed to delete node: " << zerror(rc) << "\n";
    }
    else{
        std::cout << "Unregistered service at: " << path << "\n";
    }
}