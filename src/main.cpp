#include <iostream>
#include <iomanip>
#include "ProcessScanner.h"

int main() {
    ProcessScanner scanner;
    
    std::cout << "Phase 1: Process Discovery Test" << std::endl;
    std::cout << "Scanning /proc filesystem..." << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    std::cout << std::left << std::setw(8) << "PID" 
              << std::setw(25) << "NAME" 
              << std::setw(8) << "STATE" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;

    auto processes = scanner.scan();
    
    if (processes.empty()) {
        std::cout << "No processes found! (Are you running on Linux/WSL?)" << std::endl;
    }

    for (const auto& pair : processes) {
        const auto& p = pair.second;
        std::cout << std::left << std::setw(8) << p.pid 
                  << std::setw(25) << p.name 
                  << std::setw(8) << p.state << std::endl;
    }

    return 0;
}
