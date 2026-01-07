#pragma once
#include <string>
#include <vector>
#include <map>
#include "ProcessInfo.h"

struct Warning {
    int pid;
    std::string type;
    std::string message;
    std::string suggestion;
};

class RuleEngine {
public:
    std::vector<Warning> analyze(const std::map<int, ProcessInfo>& processes);
    
private:
    void check_high_cpu(const ProcessInfo& p, std::vector<Warning>& warnings);
    void check_cpu_spike(const ProcessInfo& p, std::vector<Warning>& warnings);
    void check_memory_leak(const ProcessInfo& p, std::vector<Warning>& warnings);
    void check_stuck_process(const ProcessInfo& p, std::vector<Warning>& warnings);
};
