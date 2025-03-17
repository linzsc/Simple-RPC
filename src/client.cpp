#include <boost/asio.hpp>
#include <iostream>
#include "rpc_protocol.h"

using namespace boost::asio;
using namespace boost::asio::ip;

class RpcClient {
public:
    RpcClient(io_context& io, const std::string& host, short port)
        : socket_(io), resolver_(io) {
        connect(host, std::to_string(port));
    }

    nlohmann::json call(const RpcRequest& req) {
        // 发送请求
        std::string req_body = nlohmann::json(req).dump();
        RpcHeader header{0x12345678, static_cast<uint32_t>(req_body.size()), next_msg_id_++};
        
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(buffer(&header, sizeof(header)));
        buffers.push_back(buffer(req_body));
        write(socket_, buffers);
        
        // 读取响应头
        RpcHeader resp_header;
        read(socket_, buffer(&resp_header, sizeof(resp_header)));
        
        if (resp_header.magic != 0x12345678) {
            throw std::runtime_error("Invalid response magic");
        }
        
        // 读取响应体
        std::string resp_body(resp_header.body_len, '\0');
        read(socket_, buffer(resp_body));
        
        return nlohmann::json::parse(resp_body).get<RpcResponse>().result;
    }

private:
    void connect(const std::string& host, const std::string& port) {
        auto endpoints = resolver_.resolve(host, port);
        boost::asio::connect(socket_, endpoints);
    }

    tcp::socket socket_;
    tcp::resolver resolver_;
    uint32_t next_msg_id_ = 1;
};


/*
ServiceProxy<CalculatorService> proxy(client);
*/
int main() {
    try {
        
        io_context io;
        RpcClient client(io, "127.0.0.1", 12345);
        // 测试调用
        RpcRequest req;
        req.service_name = "CalculatorService";
        req.method_name = "add";
        req.params = {4, 5};
            
        auto result = client.call(req);
        std::cout << "4 + 5 = " << result.get<int>() << "\n"; 
        
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << "\n";
    }
    return 0;
}