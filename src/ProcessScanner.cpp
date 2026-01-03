#include "ProcessScanner.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

#include <unistd.h> // for sysconf

unsigned long long ProcessScanner::get_system_time() {
    std::ifstream stat_file("/proc/stat");
    if (!stat_file.is_open()) return 0;

    std::string line;
    std::getline(stat_file, line);
    // line is like: cpu  2255 34 2290 22625563 ...
    
    std::istringstream iss(line);
    std::string label;
    iss >> label; // "cpu"

    unsigned long long user, nice, system, idle, iowait = 0, irq = 0, softirq = 0, steal = 0;
    iss >> user >> nice >> system >> idle;
    if (iss >> iowait >> irq >> softirq >> steal) {
        // We have extended fields
    }
    
    // Total system time = sum of all these
    // Actually for CPU usage % calculation we usually care about (User + Nice + System + Idle + ...)
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

std::map<int, ProcessInfo> ProcessScanner::scan() {
    std::map<int, ProcessInfo> current_processes;
    unsigned long long current_system_time = get_system_time();

    // stored in /proc
    std::string proc_dir = "/proc";

    if (!fs::exists(proc_dir)) return current_processes;

    long page_size_kb = sysconf(_SC_PAGESIZE) / 1024; // Convert to KB for display or keep bytes

    try {
        for (const auto& entry : fs::directory_iterator(proc_dir)) {
            if (!entry.is_directory()) continue;

            std::string filename = entry.path().filename().string();
            if (!std::all_of(filename.begin(), filename.end(), ::isdigit)) {
                continue;
            }

            int pid = std::stoi(filename);
            
            std::string stat_path = entry.path().string() + "/stat";
            std::ifstream stat_file(stat_path);
            
            if (stat_file.is_open()) {
                std::string line;
                if (std::getline(stat_file, line)) {
                    size_t first_paren = line.find('(');
                    size_t last_paren = line.rfind(')');
                    
                    if (first_paren != std::string::npos && last_paren != std::string::npos && last_paren > first_paren) {
                        std::string name = line.substr(first_paren + 1, last_paren - first_paren - 1);
                        
                        // Rest of the line after ')'
                        std::istringstream iss(line.substr(last_paren + 1));
                        std::string state_str;
                        iss >> state_str; // Field 3: state

                        int ppid, pgrp, session, tty_nr, tpgid;
                        unsigned int flags;
                        unsigned long minflt, cminflt, majflt, cmajflt;
                        unsigned long long utime, stime;
                        long cutime, cstime, priority, nice, num_threads, itrealvalue;
                        unsigned long long starttime;
                        unsigned long vsize;
                        long rss;

                        // Fields 4-24 (based on man 5 proc)
                        // 4   5    6        7      8      9      10       11      12       
                        iss >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt;
                        // 14     15     16      17
                        iss >> utime >> stime >> cutime >> cstime;
                        // 18       19    20           21           22         23     24
                        iss >> priority >> nice >> num_threads >> itrealvalue >> starttime >> vsize >> rss;

                        // rss is in pages, convert to KB
                        long rss_kb = rss * page_size_kb;

                        ProcessInfo p(pid, name, state_str, utime, stime, rss_kb);

                        // Calculate CPU usage
                        if (prev_processes.count(pid) && prev_system_time > 0) {
                            unsigned long long total_time = utime + stime;
                            unsigned long long prev_total_time = prev_processes[pid].utime + prev_processes[pid].stime;
                            
                            unsigned long long proc_delta = total_time - prev_total_time;
                            unsigned long long sys_delta = current_system_time - prev_system_time;

                            if (sys_delta > 0) {
                                // Multiply by number of cores? 
                                // Top essentially shows 100% per core. If we want total system usage normalize by cores.
                                // Standard `top` behavior: single process can be 100%. The prompt implies simple similar-to-top.
                                // We'll stick to 100% * proc_delta / sys_delta * cpu_count or just plain.
                                // Actually, /proc/stat lines are sum of all CPUs. 
                                // So (proc_delta / sys_delta) gives % of total available execution time across all cores.
                                // To get "top" style (where 4 cores = 400%), we need to multiply by num_cpus.
                                // Let's just do single-core normalized for now (0-100% total system load) for simplicity 
                                // OR better: sys_curr - sys_prev is the total time elapsed * num_cpus.
                                // Wait, no. /proc/stat "cpu" line is the aggregation of all cores.
                                // So sys_delta IS (wall_time * num_cpus).
                                // So (proc_delta / sys_delta) * 100 is the % of TOTAL capacity used.
                                // If we want 100% max per core, we multiply by num_cpus.
                                // Let's start with % of total system capacity (0-100% total).
                                p.cpu_percent = 100.0 * proc_delta / sys_delta;
                                
                                // Determine number of CPUs to scale to "top" style?
                                // Let's simplify: Standard prompt asks for "CPU %". Users usually expect 0-100% total or per core.
                                // Let's stick to simple: proc_delta / sys_delta * cpu_count.
                                // Actually, let's just count processors.
                                static long num_processors = sysconf(_SC_NPROCESSORS_ONLN);
                                p.cpu_percent *= num_processors;
                            }
                        }

                        current_processes[pid] = p;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
    }

    prev_processes = current_processes;
    prev_system_time = current_system_time;

    return current_processes;
}
