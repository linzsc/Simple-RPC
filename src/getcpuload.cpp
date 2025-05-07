#include "getcpuload.h"

double GetCPULoad::getCPULoad() {
    // 获取第一次CPU状态
    std::ifstream file1("/proc/stat");
    std::string line1;
    std::getline(file1, line1);
    std::istringstream ss1(line1);

    std::string cpu;
    long user1, nice1, system1, idle1, iowait1, irq1, softirq1, steal1;
    ss1 >> cpu >> user1 >> nice1 >> system1 >> idle1 >> iowait1 >> irq1 >> softirq1 >> steal1;
    
    // 等待1秒
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 获取第二次CPU状态
    std::ifstream file2("/proc/stat");
    std::string line2;
    std::getline(file2, line2);
    std::istringstream ss2(line2);

    long user2, nice2, system2, idle2, iowait2, irq2, softirq2, steal2;
    ss2 >> cpu >> user2 >> nice2 >> system2 >> idle2 >> iowait2 >> irq2 >> softirq2 >> steal2;

    // 计算两次之间的增量
    long total1 = user1 + nice1 + system1 + idle1 + iowait1 + irq1 + softirq1 + steal1;
    long total2 = user2 + nice2 + system2 + idle2 + iowait2 + irq2 + softirq2 + steal2;
    long total = total2 - total1;
    long idle = idle2 - idle1;

    // 计算CPU负载
    double cpuLoad = 100.0 * (total - idle) / total;
    return cpuLoad;
}



