#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
//使用json库模仿protobuf

// RPC协议头（固定12字节）
struct RpcHeader {
    uint32_t magic = 0x12345678;  // 魔数校验
    uint32_t body_len;            // Body长度
    uint32_t msg_id;              // 消息ID（用于匹配请求响应）
    RpcHeader(){
        
    };
    RpcHeader(uint32_t magic_,uint32_t body_len_, uint32_t msg_id_ ): magic(magic_), body_len(body_len_), msg_id(){

    };
};

// 请求消息体（JSON格式）
struct RpcRequest {
    std::string service_name;    // 服务名
    std::string method_name;     // 方法名
    nlohmann::json params;       // 参数列表
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RpcRequest, service_name, method_name, params)
};

// 响应消息体（JSON格式）
struct RpcResponse {
    int code;                // 错误码
    nlohmann::json result;       // 返回值
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RpcResponse, code, result)
};

