#ifndef SERVICE_DISPATCHER_H
#define SERVICE_DISPATCHER_H

#include <functional>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>

class ServiceDispatcher {
public:
    // 服务处理函数类型
    using Handler = std::function<nlohmann::json(const std::string&, const nlohmann::json&)>;

    // 注册服务
    void registerService(const std::string& service_name, Handler handler) {
        services_[service_name] = handler;
    }

    // 分发请求
    nlohmann::json dispatch(const std::string& service_name, const std::string& method_name, const nlohmann::json& params) {
        auto it = services_.find(service_name);
        if (it != services_.end()) {
            try {
                return it->second(method_name, params);
            } catch (const std::exception& e) {
                return {{"code", 500}, {"result", e.what()}};
            }
        }
        return {{"code", 404}, {"result", "Service not found"}};
    }

private:
    std::unordered_map<std::string, Handler> services_;
};

#endif // SERVICE_DISPATCHER_H