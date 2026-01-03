#pragma once
#include <map>
#include <vector>
#include <string>
#include "ProcessInfo.h"

class ProcessScanner {
public:
    // scan() reads /proc and returns a map of PID -> ProcessInfo
    std::map<int, ProcessInfo> scan();

private:
    std::map<int, ProcessInfo> prev_processes;
    unsigned long long prev_system_time = 0;
    
    // Helper to get total system CPU time from /proc/stat
    unsigned long long get_system_time();
};
