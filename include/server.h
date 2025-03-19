#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/thread_pool.hpp>
#include <iostream>
#include "rpc_protocol.h"
#include "service_router.h"     
#include "service_registry.h"     //服务注册
#include "zk_wrapper.h"          //zk库api封装
#include "service_provider.h"   //提供服务的具体方法
using namespace boost::asio;
using namespace boost::asio::ip;

class RpcSession : public std::enable_shared_from_this<RpcSession> {
public:
    RpcSession(tcp::socket socket,Router & router);
    void start();

private:
    void readHeader();
    void readBody();
    void handleRequest();
    void sendResponse(const RpcResponse& resp);

    tcp::socket socket_;
    RpcHeader header_;
    std::string body_buf_;

    Router& router_;
};

class RpcServer {
public:
    RpcServer(io_context& io, short port);

private:
    void startAccept();

    tcp::acceptor acceptor_;
    std::shared_ptr<Router>router_;
};