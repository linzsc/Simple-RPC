#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <iostream>
#include "rpc_protocol.h"
#include "service_router.h"     
#include "service_registry.h"     //服务注册
#include "zk_wrapper.h"          //zk库api封装
#include "service_provider.h"   //提供服务的具体方法
using namespace boost::asio;
using namespace boost::asio::ip;
ZooKeeperWrapper zk("127.0.0.1:2181",3000);
class RpcSession : public std::enable_shared_from_this<RpcSession> {
public:
    RpcSession(tcp::socket socket,Router & router)
        : socket_(std::move(socket)),router_(router) {}
    
    void start() {
        readHeader();
    }

private:
    void readHeader() {
        auto self = shared_from_this();
        async_read(socket_, boost::asio::buffer(&header_, sizeof(RpcHeader)),
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
        async_read(socket_, boost::asio::buffer(body_buf_),
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
            std::cout<<"method_name:"<<req.method_name<<std::endl;
            std::cout<<"params:"<<req.params<<std::endl;

            // 处理请求
            nlohmann::json result = router_.dispatch(req.method_name, req.params);
            
            // 构造响应
            RpcResponse resp;
            resp.code = result["code"];
            resp.result = result["result"];

            //打印响应
            

            sendResponse(result);
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
        buffers.push_back(boost::asio::buffer(&resp_header, sizeof(RpcHeader)));
        buffers.push_back(boost::asio::buffer(data));
        
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

    Router& router_;
};

class RpcServer {
public:
    RpcServer(io_context& io, short port)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
        startAccept();
        
        //向zk注册服务
        ServiceRegistry registry(zk);
        registry.registerService("CalculatorService", "127.0.0.1:12345");
         
        router_= std::make_shared<Router>();
        //路由添加方法
        router_->resgister("add",  CalculatorService::add);

    }

private:
    void startAccept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    //打印连接信息
                    std::cout << "New connection from " << socket.remote_endpoint() << "\n";
                    

                    std::make_shared<RpcSession>(std::move(socket), *router_)->start();
                }
                startAccept();
            });
    }

    tcp::acceptor acceptor_;
    std::shared_ptr<Router>router_;
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
g++ -std=c++11 -o server src/server.cpp src/zk_wrapper.cpp -I include -lboost_system -lboost_thread -lzookeeper_mt -DTHREADED
*/