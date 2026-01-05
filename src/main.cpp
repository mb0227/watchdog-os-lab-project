#include <iostream>
#include <iomanip>
#include "ProcessScanner.h"

#include <iostream>
#include <thread>
#include <chrono>
#include "ProcessScanner.h"
#include "TUI.h"

#include "CommandParser.h"
#include "ProcessController.h"

int main() {
    ProcessScanner scanner;
    TUI tui;
    CommandParser parser;
    ProcessController controller;
    
    try {
        tui.init();
    } catch (...) {
        std::cerr << "Failed to initialize ncurses." << std::endl;
        return 1;
    }

    int ticks = 0;
    std::map<int, ProcessInfo> processes;

    bool running = true;
    while (running) {
        // 1. Scan at 1s interval (approx 10 ticks of 100ms)
        if (ticks == 0) {
            processes = scanner.scan();
        }
        
        if (++ticks >= 10) ticks = 0;

        // 2. Render
        tui.draw(processes);

        // 3. Input Handling
        std::string cmd_str = tui.get_input();
        if (!cmd_str.empty()) {
            Command cmd = parser.parse(cmd_str);
            
            if (cmd.type == CommandType::QUIT) {
                running = false;
            } else if (cmd.type == CommandType::KILL) {
                if (!cmd.args.empty()) {
                    int pid = std::stoi(cmd.args[0]);
                    controller.killProcess(pid);
                    tui.set_message("Killed PID " + cmd.args[0]);
                }
            } else if (cmd.type == CommandType::PAUSE) {
               if (!cmd.args.empty()) {
                    int pid = std::stoi(cmd.args[0]);
                    controller.pauseProcess(pid);
                    tui.set_message("Paused PID " + cmd.args[0]);
                }
            } else if (cmd.type == CommandType::RESUME) {
               if (!cmd.args.empty()) {
                    int pid = std::stoi(cmd.args[0]);
                    controller.resumeProcess(pid);
                    tui.set_message("Resumed PID " + cmd.args[0]);
                }
            } else if (cmd.type == CommandType::RUN) {
               if (!cmd.args.empty()) {
                    // Reconstruct command line? Or separate args
                    // Simple run: first arg is cmd, rest args
                    // ProcessController::runProcess takes a string and splits it again.
                    // Let's pass the raw substring or reconstruct.
                    // For now, reconstruct simple space separated
                    std::string full_cmd = cmd.args[0];
                    for (size_t i = 1; i < cmd.args.size(); ++i) full_cmd += " " + cmd.args[i];
                    controller.runProcess(full_cmd);
                    tui.set_message("Ran: " + full_cmd);
               }
            } else if (cmd.type == CommandType::PRIORITY) {
                 if (cmd.args.size() >= 2) {
                    int pid = std::stoi(cmd.args[0]);
                    int val = std::stoi(cmd.args[1]);
                    controller.setPriority(pid, val);
                    tui.set_message("Reniced PID " + cmd.args[0]);
                 }
            } else {
                tui.set_message("Unknown command: " + cmd_str);
            }
        }

        // 4. Sleep (refresh rate)
        napms(100); // Reduced sleep to make typing responsive. 
        // Logic loop should technically trigger scan every 1s, but render/input loop faster.
        // For simplicity, we just loop fast and scan every time? 
        // No, scanning /proc every 100ms is heavy.
        // Let's optimize: Check time delta for scan, but UI update fast.
        // Or simpler for student project: 
        // Switch getch to non-blocking (already done in TUI::init with nodelay).
        // If we sleep 100ms, input will feel laggy if we type fast.
        // Better: Scan every 10 loops.
        static int ticks = 0;
        if (++ticks >= 10) {
             // scan happens at start of loop
             ticks = 0;
        } else {
            // Skip scan, just draw (for input feedback)
            // But we need 'processes' for draw.
            // We can cache them.
        }
    }

    tui.close();
    return 0;
}
