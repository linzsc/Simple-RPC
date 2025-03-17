#include "rpc_protocol.h"
#include <iostream>

int main() {
    // 序列化请求
    RpcRequest req;
    req.service_name = "CalculatorService";
    req.method_name = "add";
    req.params = {1, 2};
    
    std::string data = nlohmann::json(req).dump();
    std::cout << "Serialized Request: " << data << std::endl;

    // 反序列化请求
    auto req2 = nlohmann::json::parse(data).get<RpcRequest>();
    std::cout << "Deserialized Service: " << req2.service_name << std::endl;
    std::cout << "Deserialized Method: " << req2.method_name << std::endl;
    for(int i=0;i<req.params.size();i++){
        std::cout << "Deserialized Params: " << req2.params[i] << std::endl;
    }
    //std::cout << "Deserialized Params: " << req2.params[0] << ", " << req2.params[1] << std::endl;

    return 0;
}