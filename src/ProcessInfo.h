#pragma once
#include <string>
#include <sys/types.h>

// Represents a single process and its basic attributes
class ProcessInfo {
public:
    pid_t pid;
    std::string name;
    std::string state;

    // Constructors
    ProcessInfo() : pid(0), name(""), state("?") {}
    ProcessInfo(pid_t p, std::string n, std::string s) : pid(p), name(n), state(s) {}
};
