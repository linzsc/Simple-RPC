

### **C++轻量级RPC框架技术路线**  
**目标**：基于Boost.Asio + ZooKeeper + JSON + glog，实现高并发、轻量级RPC框架，支持服务注册发现、负载均衡、动态代理和异步日志，可在单机开发环境完整运行。


### **一、技术选型与工具**
| 模块                | 技术方案                          | 理由                                                                 |
|---------------------|----------------------------------|----------------------------------------------------------------------|
| **网络通信**        | Boost.Asio                      | 跨平台异步IO，高效处理TCP长连接                                      |
| **序列化**          | nlohmann/json                   | 简单易用，支持复杂数据结构                                           |
| **服务注册发现**    | ZooKeeper（C客户端库）           | 分布式协调，支持动态服务发现和健康检测                                 |
| **日志系统**        | Google glog + 异步日志封装       | 高性能日志，支持日志分级和异步输出                                   |
| **负载均衡**        | 轮询/随机/一致性哈希             | 客户端集成策略，动态选择服务实例                                      |
| **动态代理**        | C++模板 + 宏定义                 | 零运行时开销，类型安全                                               |

---

### **二、项目架构设计**
```mermaid
graph TD
    A[客户端] -->|服务发现| B(ZooKeeper)
    B -->| 返回实例列表| A
    A -->|负载均衡| C[服务实例1]
    A -->|负载均衡| D[服务实例2]
    C -->|RPC调用| E[服务端]
    D -->|RPC调用| E
    E -->|日志记录| F[glog异步日志]
```

---

### **三、核心模块实现步骤**  
#### **1.基础RPC通信**
(1) **协议设计**  
   - 定义RPC协议头（魔数、消息ID、长度校验）  
   - 请求/响应体使用JSON序列化  
   ```cpp
   // rpc_protocol.h
   struct RpcHeader {
       uint32_t magic = 0x12345678;
       uint32_t body_len;
       uint32_t msg_id;
   };
   struct RpcRequest { ... }; // JSON序列化字段
   struct RpcResponse { ... };
   ```

(2) **网络通信（Boost.Asio）**  
   - 实现异步TCP服务器/客户端  
   - 解决粘包问题（Header+Body模式）  
   ```cpp
   // 服务端异步读取
   async_read(socket, buffer(&header, sizeof(header)), [this](...) {
       async_read(socket, buffer(body, header.body_len), handleRequest);
   });
   ```

(3) **集成glog日志**  
   - 封装异步日志输出类（避免阻塞主线程）  
   ```cpp
   class AsyncLogger {
   public:
       void log(const std::string& message) {
           std::lock_guard<std::mutex> lock(queue_mutex_);
           log_queue_.push(message);
           cond_.notify_one();
       }
   private:
       std::queue<std::string> log_queue_;
       std::mutex queue_mutex_;
       std::condition_variable cond_;
   };
   ```

---

#### **2.服务注册与发现**
(1) **ZooKeeper集成**  
   - 使用C客户端库（如`zookeeper`或`cpp-zookeeper`）  
   - 实现服务注册接口  
   ```cpp
   void registerService(const std::string& service_name, const std::string& endpoint) {
       zoo_create(zk_handle_, "/services/CalculatorService/node", endpoint.c_str(), 
           endpoint.size(), &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, nullptr, 0);
   }
   ```

(2) **服务发现接口**  
   - 监听ZooKeeper节点变化，动态更新服务列表  
   ```cpp
   void watchServices(zhandle_t* zh, int type, const std::string& path) {
       if (type == ZOO_CHILD_EVENT) {
           updateEndpointList(path);
       }
   }
   ```

---

#### **3.动态代理与负载均衡**
(1) **动态代理生成**  
   - 模板类封装远程调用，生成本地方法接口  
   ```cpp
   template <typename Service>
   class ServiceProxy : public Service {
   public:
       template <typename... Args>
       auto call(const std::string& method, Args... args) {
           RpcRequest req = buildRequest(method, args...);
           return client_.sendRequest(req);
       }
   };
   ```

(2) **负载均衡策略**  
   - 实现轮询、随机、一致性哈希策略  
   ```cpp
   class LoadBalancer {
   public:
       virtual std::string selectEndpoint(const std::vector<std::string>& endpoints) = 0;
   };
   ```

---

#### **4.性能优化**

(1) **线程池**  
   - 使用Boost.Asio的`io_context`线程池  
   ```cpp
   boost::asio::thread_pool pool(4); // 4线程
   ```

(2) **对象池**  
   - 使用RpcSession_pool对象池，减少会话的频繁创建和注销  
   ```cpp
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

   auto session = RpcSessionPool::getInstance().acquire(std::move(socket), *router_);
  
   ```

---

### **四、代码结构示例**
```
rpc-framework/
├── include/
│   ├── rpc_protocol.h         # 协议定义
│   ├── service_proxy.h        # 动态代理
│   ├── load_balancer.h        # 负载均衡策略
|   ├── service_dispatcher.h   #任务分发
|   ├── service_registry.h     #任务注册
│   └── logger.h               # 异步日志
├── src/
│   ├── server.cpp             # 服务端主逻辑
│   ├── client.cpp             # 客户端主逻辑
│   └── zk_wrapper.cpp         # ZooKeeper封装
├── test/
│   ├── test_serialize.cpp     # 单元测试
|   
│   └── benchmark.cpp          # 压测工具
```

---


### **五、项目优化手段与性能指标对比**

| 优化手段 | 总请求数 | 总时间（秒） | QPS  |
|:--------|:---------|:-------------|:-----|
| 单线程  | 10000    | 4.43475      | 2254.92 |
| 多线程  | 10000    | 3.71747      | 2690  |
| 对象池  | 10000    | 2.31242      | 4324.48 |

**测试环境：**  Lenovo-XiaoXinPro-13ARE-2020，CPU：AMD Ryzen 7 4800U
---

### **六、扩展功能（可选）**
1. **熔断与降级**：基于错误率自动屏蔽故障节点  
2. **TLS加密**：Boost.Asio集成OpenSSL实现安全通信  
3. **跨语言支持**：通过IDL生成其他语言客户端  

---


