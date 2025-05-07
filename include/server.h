#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/thread_pool.hpp>
#include <iostream>
#include "rpc_protocol.h"
#include "service_router.h"     
#include "service_registry.h"     //服务注册
#include "zk_wrapper.h"          //zk库api封装
#include "service_provider.h"   //提供服务的具体方法
#include <queue>
using namespace boost::asio;
using namespace boost::asio::ip;

class RpcSession : public std::enable_shared_from_this<RpcSession> {
public:
    RpcSession(tcp::socket socket,Router & router);
    void start();
    void reset(tcp::socket socket, Router & router);
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


class RpcSessionPool {
    public:
        static RpcSessionPool& getInstance() {
            static RpcSessionPool instance;
            return instance;
        }
    
        std::shared_ptr<RpcSession> acquire(tcp::socket socket, Router &router) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!freeSessions_.empty()) {
                auto session = freeSessions_.front();
                freeSessions_.pop();
                session->reset(std::move(socket), router);
                return session;
            }
            return std::make_shared<RpcSession>(std::move(socket), router);
        }
    
        void release(std::shared_ptr<RpcSession> session) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (freeSessions_.size() < MAX_POOL_SIZE) {
                freeSessions_.push(session);
            }
        }
    
    private:
        RpcSessionPool() = default;
        ~RpcSessionPool() = default;
    
        std::queue<std::shared_ptr<RpcSession>> freeSessions_;
        std::mutex mutex_;
        static const int MAX_POOL_SIZE = 100;
    };