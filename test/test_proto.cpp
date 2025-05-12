#include <iostream>
#include <string>
#include "../include/rpc.pb.h"  // 引入你生成的 protobuf 头文件

int main() {
    // 初始化 protobuf（只在使用 protobuf 的程序中首次运行时需要）
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // 构造请求消息 RpcRequest
    rpc::RpcRequest request;
    request.set_service_name("UserService");
    request.set_method_name("Login");
    request.set_params(R"({"username": "alice", "password": "123456"})");

    // 序列化到字符串
    std::string serialized_request;
    if (!request.SerializeToString(&serialized_request)) {
        std::cerr << "Failed to serialize RpcRequest." << std::endl;
        return 1;
    }

    std::cout << "Serialized RpcRequest size: " << serialized_request.size() << " bytes" << std::endl;

    // 反序列化回 RpcRequest 对象
    rpc::RpcRequest parsed_request;
    if (!parsed_request.ParseFromString(serialized_request)) {
        std::cerr << "Failed to parse RpcRequest." << std::endl;
        return 1;
    }

    std::cout << "\nParsed RpcRequest:" << std::endl;
    std::cout << "Service: " << parsed_request.service_name() << std::endl;
    std::cout << "Method: " << parsed_request.method_name() << std::endl;
    std::cout << "Params: " << parsed_request.params() << std::endl;

    // 模拟服务器构造响应消息 RpcResponse
    rpc::RpcResponse response;
    response.set_code(0); // 0 表示成功
    response.set_result(R"({"user_id": 1001, "token": "abcdefg123456"})");

    // 序列化响应
    std::string serialized_response;
    if (!response.SerializeToString(&serialized_response)) {
        std::cerr << "Failed to serialize RpcResponse." << std::endl;
        return 1;
    }

    // 反序列化回 RpcResponse
    rpc::RpcResponse parsed_response;
    if (!parsed_response.ParseFromString(serialized_response)) {
        std::cerr << "Failed to parse RpcResponse." << std::endl;
        return 1;
    }

    std::cout << "\nParsed RpcResponse:" << std::endl;
    std::cout << "Code: " << parsed_response.code() << std::endl;
    std::cout << "Result: " << parsed_response.result() << std::endl;

    // 清理 protobuf 内部资源（可选）
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
