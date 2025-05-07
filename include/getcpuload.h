#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>


// 整体流程：服务器启动->查询当前CPU负载->返回CPU负载->携带负载进行服务注册
// fork子进程->子进程查询当前CPU负载->返回CPU负载->子进程休眠，每隔1分钟查询一次->向zookeeper更新节点的负载信息

class GetCPULoad{

public:
    double getCPULoad();

};
