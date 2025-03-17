#pragma once
#include "client.h"
#include "zk_wrapper.h"
#include "load_balance.h"

template <typename T>
class ServiceProxy : public Service {
public:
    ServiceProxy(ZooKeeperWrapper& zk, LoadBalancer& lb,std::string& Path)
        : zk_(zk), lb_(lb), ServicePath(Path){}

    template <typename... Args>
    T call(const std::string& method, Args... args) {
        // 获取服务实例列表
        auto endpoints = zk_.getServiceEndpoints(ServicePath);
        if (endpoints.empty()) {
            throw std::runtime_error("No available service instances");
        }
        
        // 选择服务实例
        std::string endpoint = lb_.selectEndpoint(endpoints);
        size_t pos = endpoint.find(':');
        std::string host = endpoint.substr(0, pos);
        short port = std::stoi(endpoint.substr(pos + 1));
        
        // 创建RPC客户端
        RpcClient client(io_, host, port);
        
        // 构造请求
        RpcRequest req;
        req.service_name = typeid(Service).name();
        req.method_name = method;
        req.params = {args...};
        
        return client.call(req);
    }
    void setServicePath(const std::string& path) {
        ServicePath = path;
    }

private:
    std:: string ServicePath; 
    boost::asio::io_context io_;
    ZooKeeperWrapper& zk_;
    LoadBalancer& lb_;
};