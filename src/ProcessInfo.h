#pragma once
#include <string>
#include <sys/types.h>

// Represents a single process and its basic attributes
class ProcessInfo {
public:
    pid_t pid;
    std::string name;
    std::string state;

    // Attributes for resource calculation
    unsigned long long utime;
    unsigned long long stime;
    long rss; // Resident Set Size in bytes
    double cpu_percent;

    // Constructors
    ProcessInfo() : pid(0), name(""), state("?"), utime(0), stime(0), rss(0), cpu_percent(0.0) {}
    ProcessInfo(pid_t p, std::string n, std::string s, unsigned long long u = 0, unsigned long long st = 0, long r = 0) 
        : pid(p), name(n), state(s), utime(u), stime(st), rss(r), cpu_percent(0.0) {}
};
