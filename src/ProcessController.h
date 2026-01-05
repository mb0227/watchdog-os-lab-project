#pragma once
#include <string>

class ProcessController {
public:
    void killProcess(int pid);
    void pauseProcess(int pid);
    void resumeProcess(int pid);
    void setPriority(int pid, int value);
    void runProcess(const std::string& command);
};
