#include "ProcessScanner.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

std::map<int, ProcessInfo> ProcessScanner::scan() {
    std::map<int, ProcessInfo> processes;

    // stored in /proc
    std::string proc_dir = "/proc";

    if (!fs::exists(proc_dir)) {
        // Fallback or error if not on Linux/WSL (e.g. running on plain Windows)
        // For now, we return empty map or could throw. 
        // But since this is a student lab, we assume /proc exists.
        return processes;
    }

    try {
        for (const auto& entry : fs::directory_iterator(proc_dir)) {
            if (!entry.is_directory()) continue;

            // Check if directory name is a number (PID)
            std::string filename = entry.path().filename().string();
            if (!std::all_of(filename.begin(), filename.end(), ::isdigit)) {
                continue;
            }

            int pid = std::stoi(filename);
            
            // Parse /proc/[pid]/stat
            // Format: pid (comm) state ...
            std::string stat_path = entry.path().string() + "/stat";
            std::ifstream stat_file(stat_path);
            
            if (stat_file.is_open()) {
                std::string line;
                if (std::getline(stat_file, line)) {
                    // We need to parse: 123 (name with spaces) S
                    
                    size_t first_paren = line.find('(');
                    size_t last_paren = line.rfind(')');
                    
                    if (first_paren != std::string::npos && last_paren != std::string::npos && last_paren > first_paren) {
                        // Extract Name
                        std::string name = line.substr(first_paren + 1, last_paren - first_paren - 1);
                        
                        // Extract State
                        // The state is the character after the last parenthesis and a space
                        // e.g. ") S"
                        std::string state = "?";
                        if (last_paren + 2 < line.length()) {
                            state = std::string(1, line[last_paren + 2]);
                        }
                        
                        processes[pid] = ProcessInfo(pid, name, state);
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        // Silent catch for permission errors on some /proc entries
    }

    return processes;
}
