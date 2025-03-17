#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <stdexcept>
class LoadBalancer {
public:
    virtual std::string selectEndpoint(const std::vector<std::string>& endpoints) = 0;
};

// 轮询策略
class RoundRobinLoadBalancer : public LoadBalancer {
public:
    std::string selectEndpoint(const std::vector<std::string>& endpoints) override {
        if (endpoints.empty()) {
            throw std::runtime_error("No available endpoints");
        }
        current_index_ = (current_index_ + 1) % endpoints.size();
        return endpoints[current_index_];
    }

private:
    size_t current_index_ = 0;
};

// 随机策略
class RandomLoadBalancer : public LoadBalancer {
public:
    std::string selectEndpoint(const std::vector<std::string>& endpoints) override {
        if (endpoints.empty()) {
            throw std::runtime_error("No available endpoints");
        }
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, endpoints.size() - 1);
        return endpoints[dis(gen)];
    }
};

// 加权轮询策略
class WeightedRoundRobinLoadBalancer : public LoadBalancer {
public:
    WeightedRoundRobinLoadBalancer(const std::vector<int>& weights) {
        for (size_t i = 0; i < weights.size(); ++i) {
            total_weight_ += weights[i];
            weighted_endpoints_.push_back({weights[i], i});
        }
    }

    std::string selectEndpoint(const std::vector<std::string>& endpoints) override {
        if (endpoints.empty()) {
            throw std::runtime_error("No available endpoints");
        }
        current_weight_ = (current_weight_ + 1) % total_weight_;
        for (const auto& [weight, index] : weighted_endpoints_) {
            if (current_weight_ < weight) {
                return endpoints[index];
            }
            current_weight_ -= weight;
        }
        return endpoints[0];
    }

private:
    int total_weight_ = 0;
    int current_weight_ = 0;
    std::vector<std::pair<int, size_t>> weighted_endpoints_;
};