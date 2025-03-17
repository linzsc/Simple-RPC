#include <boost/asio.hpp>
#include <iostream>
#include "rpc_protocol.h"
#include "service_dispatcher.h"

using namespace boost::asio;
using namespace boost::asio::ip;

class RpcSession : public std::enable_shared_from_this<RpcSession> {
public:
    RpcSession(tcp::socket socket, ServiceDispatcher& dispatcher)
        : socket_(std::move(socket)), dispatcher_(dispatcher) {}
    
    void start() {
        readHeader();
    }

private:
    void readHeader() {
        auto self = shared_from_this();
        async_read(socket_, buffer(&header_, sizeof(RpcHeader)),
            [this, self](boost::system::error_code ec, size_t) {
                if (ec) {
                    std::cerr << "Read header error: " << ec.message() << "\n";
                    return;
                }
                if (header_.magic != 0x12345678) {
                    std::cerr << "Invalid magic number\n";
                    return;
                }
                readBody();
            });
    }

    void readBody() {
        auto self = shared_from_this();
        body_buf_.resize(header_.body_len);
        async_read(socket_, buffer(body_buf_),
            [this, self](boost::system::error_code ec, size_t) {
                if (ec) {
                    std::cerr << "Read body error: " << ec.message() << "\n";
                    return;
                }
                handleRequest();
                readHeader(); // 继续处理下一个请求
            });
    }

    void handleRequest() {
        try {
            // 反序列化请求
            auto j = nlohmann::json::parse(body_buf_);
            RpcRequest req = j.get<RpcRequest>();
            
            // 处理请求
            auto result = dispatcher_.dispatch(req.service_name, req.method_name, req.params);
            
            // 构造响应
            RpcResponse resp;
            resp.result = result;
            sendResponse(resp);
        } catch (const std::exception& e) {
            std::cerr << "Handle request error: " << e.what() << "\n";
        }
    }

    void sendResponse(const RpcResponse& resp) {
        auto self = shared_from_this();
        std::string data = nlohmann::json(resp).dump();
        
        // 构造响应头
        RpcHeader resp_header;
        resp_header.magic = 0x12345678;
        resp_header.body_len = data.size(); 
        resp_header.msg_id = header_.msg_id; // 复用请求的msg_id
        
        // 异步发送头部+数据
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(buffer(&resp_header, sizeof(RpcHeader)));
        buffers.push_back(buffer(data));
        
        async_write(socket_, buffers,
            [this, self](boost::system::error_code ec, size_t) {
                if (ec) {
                    std::cerr << "Send response error: " << ec.message() << "\n";
                }
            });
    }

    tcp::socket socket_;
    RpcHeader header_;
    std::string body_buf_;
    ServiceDispatcher& dispatcher_;
};

class RpcServer {
public:
    RpcServer(io_context& io, short port)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port)),
          dispatcher_(std::make_shared<ServiceDispatcher>()) {
        startAccept();
        
        
        // 注册示例服务
        dispatcher_->registerService("CalculatorService", 
            [](const std::string& method, const nlohmann::json& params) {
                if (method == "add") {
                    return params[0].get<int>() + params[1].get<int>();
                }
                throw std::runtime_error("Method not found");
            });
    }

private:
    void startAccept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    //打印连接信息
                    std::cout << "New connection from " << socket.remote_endpoint() << "\n";
                    

                    std::make_shared<RpcSession>(std::move(socket), *dispatcher_)->start();
                }
                startAccept();
            });
    }

    tcp::acceptor acceptor_;
    std::shared_ptr<ServiceDispatcher> dispatcher_;
};

int main() {
    try {
        io_context io;
        RpcServer server(io, 12345);
        std::cout << "Server running on port 12345\n";
        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Server exception: " << e.what() << "\n";
    }
    return 0;
}
/*
g++ -std=c++11 -o server src/server.cpp -I include -lboost_system -lboost_thread -lnlohmann_json
*/