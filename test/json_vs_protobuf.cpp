#include <iostream>
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>
#include "../include/rpc.pb.h"  // Protobuf 生成的头文件

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

// JSON版本的结构体
struct JsonRpcRequest {
    std::string service_name;
    std::string method_name;
    json params;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(JsonRpcRequest, service_name, method_name, params)
};

int main() {
    // 准备测试数据
    JsonRpcRequest json_req;
    json_req.service_name = "CalculatorService";
    json_req.method_name = "add";
    json_req.params = {1, 2};

    rpc::RpcRequest proto_req;
    proto_req.set_service_name("CalculatorService");
    proto_req.set_method_name("add");
    proto_req.set_params(R"([1,2])");

    const int N = 10000000;  // 循环次数（测试更明显）

    // ------------------ JSON 测试 ------------------
    auto start_json = high_resolution_clock::now();
    std::string json_data;
    for (int i = 0; i < N; ++i) {
        json_data = json(json_req).dump();
        auto parsed = json::parse(json_data).get<JsonRpcRequest>();
    }
    auto end_json = high_resolution_clock::now();
    auto duration_json = duration_cast<milliseconds>(end_json - start_json);
    std::cout << "JSON time: " << duration_json.count() << " ms" << std::endl;
    std::cout << "JSON serialized size: " << json_data.size() << " bytes" << std::endl;

    // ------------------ Protobuf 测试 ------------------
    std::string proto_data;
    auto start_proto = high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        proto_req.SerializeToString(&proto_data);
        rpc::RpcRequest parsed_req;
        parsed_req.ParseFromString(proto_data);
    }
    auto end_proto = high_resolution_clock::now();
    auto duration_proto = duration_cast<milliseconds>(end_proto - start_proto);
    std::cout << "Protobuf time: " << duration_proto.count() << " ms" << std::endl;
    std::cout << "Protobuf serialized size: " << proto_data.size() << " bytes" << std::endl;

    return 0;
}
//g++ json_vs_protobuf.cpp rpc.pb.cc -o json_vs_protobuf -lprotobuf -std=c++17 -O2
/*
测试结果：
次数：1000000

JSON time: 2723 ms
JSON serialized size: 71 bytes
Protobuf time: 271 ms
Protobuf serialized size: 31 bytes


次数：10000000
JSON time: 28329 ms
JSON serialized size: 71 bytes
Protobuf time: 2754 ms
Protobuf serialized size: 31 bytes
*/