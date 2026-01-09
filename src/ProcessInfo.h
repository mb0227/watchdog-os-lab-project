#pragma once
#include <string>
#include <sys/types.h>
#include <vector>

// Represents a single process and its basic attributes
class ProcessInfo {
public:
    pid_t pid;
    std::string name;
    std::string state;

    int ppid; // Parent PID
    
    // Attributes for resource calculation
    unsigned long long utime;
    unsigned long long stime;
    long rss; // Resident Set Size in bytes
    double cpu_percent;
    
    // I/O Stats
    unsigned long long io_read_bytes;
    unsigned long long io_write_bytes;
    unsigned long long io_read_speed; // bytes per second
    unsigned long long io_write_speed; // bytes per second
    
    // History for AI analysis (last N seconds)
    // We assume 1 update per second, so size=60 means 60 seconds.
    std::vector<double> cpu_history;
    std::vector<long> mem_history;

    // Constructors
    ProcessInfo() : pid(0), name(""), state("?"), ppid(0), utime(0), stime(0), rss(0), cpu_percent(0.0), 
                    io_read_bytes(0), io_write_bytes(0), io_read_speed(0), io_write_speed(0) {}
    
    ProcessInfo(pid_t p, std::string n, std::string s, int parent, unsigned long long u, unsigned long long st, long r,
                unsigned long long r_bytes = 0, unsigned long long w_bytes = 0) 
        : pid(p), name(n), state(s), ppid(parent), utime(u), stime(st), rss(r), cpu_percent(0.0),
          io_read_bytes(r_bytes), io_write_bytes(w_bytes), io_read_speed(0), io_write_speed(0) {}

    void add_history(double cpu, long mem) {
        cpu_history.push_back(cpu);
        mem_history.push_back(mem);
        
        // Keep last 60 seconds
        if (cpu_history.size() > 60) cpu_history.erase(cpu_history.begin());
        if (mem_history.size() > 60) mem_history.erase(mem_history.begin());
    }
};
