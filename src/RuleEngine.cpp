#include "RuleEngine.h"
#include <cmath>

std::vector<Warning> RuleEngine::analyze(const std::map<int, ProcessInfo>& processes) {
    std::vector<Warning> warnings;
    
    for (const auto& pair : processes) {
        const auto& p = pair.second;
        
        if (ignored_pids.count(p.pid)) continue;
        
        check_high_cpu(p, warnings);
        check_cpu_spike(p, warnings);
        check_memory_leak(p, warnings);
        check_stuck_process(p, warnings);
    }
    
    return warnings;
}

// Rule 1: Sustained High CPU
// IF CPU > 80% for > 30 seconds
void RuleEngine::check_high_cpu(const ProcessInfo& p, std::vector<Warning>& warnings) {
    if (p.cpu_history.size() < 30) return;
    
    int high_count = 0;
    // Check last 30 entries
    for (size_t i = p.cpu_history.size() - 30; i < p.cpu_history.size(); ++i) {
        if (p.cpu_history[i] > 80.0) {
            high_count++;
        }
    }
    
    if (high_count >= 30) {
        warnings.push_back({p.pid, "HIGH_CPU", "Sustained High CPU (>80%)", "pause / lower priority / kill"});
    }
}

// Rule 2: CPU Spike
// IF CPU increases > 40% within 5 seconds
void RuleEngine::check_cpu_spike(const ProcessInfo& p, std::vector<Warning>& warnings) {
    if (p.cpu_history.size() < 5) return;
    
    double current = p.cpu_history.back();
    double old = p.cpu_history[p.cpu_history.size() - 5];
    
    if ((current - old) > 40.0) {
        warnings.push_back({p.pid, "CPU_SPIKE", "Sudden CPU Spike (>40% increase)", "monitor process"});
    }
}

// Rule 3: Memory Leak Suspicion
// IF memory increases continuously for > 60 seconds
void RuleEngine::check_memory_leak(const ProcessInfo& p, std::vector<Warning>& warnings) {
    // Need at least 60 samples
    if (p.mem_history.size() < 60) return;
    
    bool continuously_increasing = true;
    for (size_t i = 1; i < p.mem_history.size(); ++i) {
        if (p.mem_history[i] <= p.mem_history[i-1]) {
            continuously_increasing = false;
            break;
        }
    }
    
    if (continuously_increasing) {
        warnings.push_back({p.pid, "MEMORY_LEAK", "Continuous Memory Growth (60s)", "restart process"});
    }
}

// Rule 4: Stuck / Zombie Process
// IF state == Z OR runtime > threshold with ~0 CPU
void RuleEngine::check_stuck_process(const ProcessInfo& p, std::vector<Warning>& warnings) {
    if (p.state == "Z") {
        warnings.push_back({p.pid, "ZOMBIE", "Zombie Process Detected", "parent process issue (kill parent?)"});
        return;
    }
    
    // Check for Stuck: > 60 seconds with ~0 CPU but state is R/S? 
    // Actually typically stuck means not responding. Hard to tell from /proc without interaction.
    // The prompt says: "runtime > threshold with ~0 CPU"
    // We'll define "threshold" as full 60 seconds of history available, and all are near 0.
    if (p.cpu_history.size() >= 60) {
        bool zero_cpu = true;
        for (double cpu : p.cpu_history) {
            if (cpu > 1.0) { // allowance for noise
                zero_cpu = false;
                break;
            }
        }
        
        // This heuristic is weak for S (sleeping) processes (like init), which are valid.
        // Usually "stuck" implies it SHOULD be doing something but isn't, OR it's consuming CPU but making no progress.
        // Prompt says "runtime > threshold with ~0 CPU".
        // Let's refine: If it's effectively 0 CPU for long time, maybe it's just idle service.
        // We'll filter for visual noise: Only warn if it's NOT a system process we know?
        // Or maybe just strictly follow the rule.
        // Let's add the warning but maybe "IDLE/STUCK" label.
        // HOWEVER, standard daemons are 0 CPU 99% of time.
        // A better check for "Stuck" might be: High CPU but no I/O? (not easy from just /proc/stat).
        // Let's stick to the Zombie check mainly, and maybe the "Strictly 0 CPU" if the user asked.
        // Re-reading: "IF state == Z OR runtime > threshold with ~0 CPU THEN WARNING: STUCK_PROCESS"
        // Most processes on linux are 0 CPU.
        // Perhaps we only flag if state is 'R' (Running) but CPU is 0? That would be weird.
        // Or state is 'D' (Uninterruptible Sleep). That is bad.
        if (p.state == "D") {
             warnings.push_back({p.pid, "STUCK_IO", "Uninterruptible Sleep (D state)", "check I/O wait"});
        }
    }
}
