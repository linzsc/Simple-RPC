#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

class ServiceRegistry {
public:
    // 注册服务实例
    void registerService(const std::string& service_name, const std::string& endpoint) {
        std::lock_guard<std::mutex> lock(mutex_);
        services_[service_name].push_back(endpoint);
    }

    // 获取服务实例列表
    std::vector<std::string> getServiceEndpoints(const std::string& service_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        return services_[service_name];
    }

private:
    std::unordered_map<std::string, std::vector<std::string>> services_;
    std::mutex mutex_;
};