#pragma once
#include "client.h"
#include "zk_wrapper.h"
#include "load_balance.h"
#include "logger.h"
template <typename T>
class ServiceProxy{
public:
    ServiceProxy(ZooKeeperWrapper& zk, LoadBalancer& lb,std::string Path)
        : zk_(zk), lb_(lb), ServicePath(Path){
        
    }

    template <typename... Args>
    T call(const std::string& method, Args... args) {
        // 获取服务实例列表
        auto endpoints = zk_.getServiceEndpoints(ServicePath);
        for(auto endpoint_1 : endpoints){
            LOG_INFO(endpoint_1);
        }
        
        if (endpoints.empty()) {
            throw std::runtime_error("No available service instances");
        }
        
        // 选择服务实例
        std::string endpoint = lb_.selectEndpoint(endpoints);
        LOG_INFO(endpoint);
        size_t pos = endpoint.find(':');
        std::string host = endpoint.substr(0, pos);
        short port = std::stoi(endpoint.substr(pos + 1));

        // 创建RPC客户端
        
        
        // 构造请求
        RpcRequest req;
        req.service_name = ServicePath;
        req.method_name = method;
        req.params = {args...};
        
       
        LOG_INFO(nlohmann::json(req).dump());
        RpcClient client(io_, host, port);

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