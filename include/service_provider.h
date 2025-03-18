#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
class CalculatorService{
public:

    static json add(const json& params) {
    // 检查params是否是一个数组，并且包含两个数字
    if (params.is_array() && params.size() == 2 && params[0].is_number() && params[1].is_number()) {
        int a = params[0].get<int>();
        int b = params[1].get<int>();
        //std::cout<<json{a+b}<<std::endl;
        //std::cout<<json{{"error", "Invalid parameters"}}<<std::endl;
        /*
        [30]
        {"error":"Invalid parameters"}
        */
        return  {{"code", 0}, {"result", a+b}};
    } else {
        // 如果参数不符合要求，返回一个错误信息
        return  {{"code", 200}, {"result", "Invalid parameters"}};
    }
}

    static nlohmann::json  sub(const std::string& a,const std::string& b){
        int num1 = std::stoi(a);
        int num2 = std::stoi(b);
        return num1-num2;
        
    }

};

