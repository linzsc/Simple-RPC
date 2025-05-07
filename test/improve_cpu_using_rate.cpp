#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

using namespace std;

int main() {
    int num_threads = 16;
    int num_iterations = 1000000000;

    vector<thread> threads;
    for (int i = 0; i < num_threads; i++) {
        threads.push_back(thread([i, num_iterations]() {
            while(1){
                // Do some work
                int x = 2*3;
            }
            
        }));
    }
    for (auto& t : threads) {
        t.join();
    }
    return 0;
}
//g++ -std=c++11 -pthread ./test/improve_cpu_using_rate.cpp -o improve_cpu_using_rate