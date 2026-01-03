#include <iostream>
#include <iomanip>
#include "ProcessScanner.h"

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include "ProcessScanner.h"

int main() {
    ProcessScanner scanner;
    
    std::cout << "Phase 2: CPU & Memory Calculation Test" << std::endl;
    std::cout << "Running 5 loops/samples..." << std::endl;
    
    for (int i = 0; i < 5; ++i) {
        auto processes = scanner.scan();
        
        // Clear screen (ANSI escape code)
        std::cout << "\033[2J\033[1;1H"; 
        
        std::cout << "Sample " << (i + 1) << "/5" << std::endl;
        std::cout << "---------------------------------------------------------" << std::endl;
        std::cout << std::left << std::setw(8) << "PID" 
                  << std::setw(25) << "NAME" 
                  << std::setw(8) << "STATE" 
                  << std::setw(10) << "CPU%" 
                  << std::setw(10) << "RSS(KB)" << std::endl;
        std::cout << "---------------------------------------------------------" << std::endl;

        // Print top 20 processes for check
        int count = 0;
        for (const auto& pair : processes) {
            const auto& p = pair.second;
            // Only show interesting ones or all
            // Simple filter: distinct from 0 cpu or just first 20
            if (count++ > 20) break; 
            
            std::cout << std::left << std::setw(8) << p.pid 
                      << std::setw(25) << p.name 
                      << std::setw(8) << p.state 
                      << std::setw(10) << std::fixed << std::setprecision(1) << p.cpu_percent
                      << std::setw(10) << p.rss << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
