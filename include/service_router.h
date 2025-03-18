#ifndef SERVICE_DISPATCHER_H
#define SERVICE_DISPATCHER_H

#include <functional>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>

class Router{
    using Handler = std::function<nlohmann::json(const nlohmann::json&)>;
public:
    void resgister(const std::string& method_name, Handler handler){
        router[method_name] = handler;
        std::cout<<method_name + " 已经注册！！"<<std::endl;
    }
    nlohmann::json dispatch(const std::string& method_name, const nlohmann::json& params){
        auto it = router.find(method_name);
        if (it != router.end()) {
            try {
                return it->second(params);
            } catch (const std::exception& e) {
                return {{"code", 500}, {"result", e.what()}};
            }
        }
        else{
            return {{"code", 404}, {"result", "Service not found"}};
        }
    }
private:
    std::unordered_map<std::string, Handler> router;
};

#endif