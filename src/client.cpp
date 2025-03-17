#include "client.h"
#include <iostream>
#include <unistd.h>
int main() {
    try {
        
        io_context io;
        RpcClient client(io, "127.0.0.1", 12345);
        // 测试调用
        RpcRequest req;
        req.service_name = "CalculatorService";
        req.method_name = "add";
        req.params = {4, 5};
        while(1){
            auto result = client.call(req);
            std::cout << "4 + 5 = " << result.get<int>() << "\n"; 
            result.clear();
            sleep(1);
        }   
        
        
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << "\n";
    }
    return 0;
}