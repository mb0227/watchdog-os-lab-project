#pragma once
#include <map>
#include <vector>
#include <string>
#include "ProcessInfo.h"

class ProcessScanner {
public:
    // scan() reads /proc and returns a map of PID -> ProcessInfo
    std::map<int, ProcessInfo> scan();
};
