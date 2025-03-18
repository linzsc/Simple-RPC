#include "server.h"
#include <iomanip>
ZooKeeperWrapper zk("127.0.0.1:2181",3000);

RpcSession::RpcSession(tcp::socket socket, Router &router)
    : socket_(std::move(socket)), router_(router) {}

void RpcSession::start()
{
    readHeader();
}

void RpcSession::readHeader()
{
    auto self = shared_from_this();
    async_read(socket_, boost::asio::buffer(&header_, sizeof(RpcHeader)),
               [this, self](boost::system::error_code ec, size_t)
               {
                std::cout<<"readHeader"<<std::endl;
                //std::cout << std::hex << header_.magic << std::endl;
                std::cout<<header_.body_len<<std::endl;
                std::cout<<header_.msg_id<<std::endl;
                   if (ec)
                   {
                       std::cerr << "Read header error: " << ec.message() << "\n";
                       return;
                   }
                   if (header_.magic != 0x12345678)
                   {
                       std::cerr << "Invalid magic number\n";
                       return;
                   }
                   readBody();
               });
}

void RpcSession::readBody()
{
    auto self = shared_from_this();
    body_buf_.resize(header_.body_len);
    async_read(socket_, boost::asio::buffer(body_buf_),
               [this, self](boost::system::error_code ec, size_t)
               {
                   if (ec)
                   {
                       std::cerr << "Read body error: " << ec.message() << "\n";
                       return;
                   }
                   handleRequest();
                   readHeader(); // 继续处理下一个请求
               });
}

void RpcSession::handleRequest()
{
    try
    {
        std::cout << "handleRequest" << std::endl;
        //std::cout<<body_buf_<<std::endl;
        // 反序列化请求
        auto j = nlohmann::json::parse(body_buf_);
        RpcRequest req = j.get<RpcRequest>();
        std::cout << "method_name:" << req.method_name << std::endl;
        std::cout << "params:" << req.params << std::endl;

        // 处理请求
        nlohmann::json result = router_.dispatch(req.method_name, req.params);

        // 构造响应
        RpcResponse resp;
        resp.code = result["code"];
        resp.result = result["result"];

        // 打印响应

        sendResponse(result);
    }
    catch (const std::exception &e)
    {
        //std::cout<<body_buf_<<std::endl;
        std::cerr << "Handle request error: " << e.what() << "\n";
    }
}

void RpcSession::sendResponse(const RpcResponse &resp)
{
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
                [this, self](boost::system::error_code ec, size_t)
                {
                    if (ec)
                    {
                        std::cerr << "Send response error: " << ec.message() << "\n";
                    }
                });
}

RpcServer::RpcServer(io_context &io, short port)
    : acceptor_(io, tcp::endpoint(tcp::v4(), port))
{
    startAccept();

    // 向zk注册服务
    ServiceRegistry registry(zk);
    registry.registerService("CalculatorService", "127.0.0.1:12345");

    router_ = std::make_shared<Router>();
    // 路由添加方法
    router_->resgister("add", CalculatorService::add);
}

void RpcServer::startAccept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                // 打印连接信息
                std::cout << "New connection from " << socket.remote_endpoint() << "\n";

                std::make_shared<RpcSession>(std::move(socket), *router_)->start();
            }
            startAccept();
        });
}

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