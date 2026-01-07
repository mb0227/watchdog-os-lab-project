#include "ProcessController.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/resource.h>
#include <vector>
#include <sstream>
#include <cstring>
#include <iostream>

void ProcessController::killProcess(int pid) {
    if (pid <= 0) return;
    kill(pid, SIGTERM);
}

void ProcessController::pauseProcess(int pid) {
    if (pid <= 0) return;
    kill(pid, SIGSTOP);
}

void ProcessController::resumeProcess(int pid) {
    if (pid <= 0) return;
    kill(pid, SIGCONT);
}

void ProcessController::setPriority(int pid, int value) {
    if (pid <= 0) return;
    setpriority(PRIO_PROCESS, pid, value);
}

void ProcessController::runProcess(const std::string& command) {
    if (command.empty()) return;

    // Parse command into args for execvp
    std::istringstream iss(command);
    std::string token;
    std::vector<std::string> args;
    while (iss >> token) {
        args.push_back(token);
    }

    if (args.empty()) return;

    std::vector<char*> c_args;
    for (const auto& arg : args) {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0) {
        // Child
        execvp(c_args[0], c_args.data());
        // If execvp returns, it failed
        exit(1);
    } 
    // Parent returns immediately
}

void ProcessController::reapZombies() {
    // Non-blocking wait to clean up zombies
    while (waitpid(-1, nullptr, WNOHANG) > 0) {
        // Keep reaping until no more dead children
    }
}
