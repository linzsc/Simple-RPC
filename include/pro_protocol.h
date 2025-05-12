#pragma once
#include <string>
#include "rpc.pb.h"  // 引入 protobuf 生成的头文件

// RPCHeader 仍然可以保留为原始 struct，因为通常 Header 是非 protobuf 的固定格式。
struct RpcHeader {
    uint32_t magic = 0x12345678;
    uint32_t body_len;
    uint32_t msg_id;

    RpcHeader() = default;
    RpcHeader(uint32_t magic_, uint32_t body_len_, uint32_t msg_id_)
        : magic(magic_), body_len(body_len_), msg_id(msg_id_) {}
};
