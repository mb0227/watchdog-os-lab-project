#include "TUI.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

void TUI::init() {
    initscr();              // Start ncurses mode
    cbreak();               // Line buffering disabled
    noecho();               // Don't echo while we do getch
    curs_set(0);            // Invisible cursor
    nodelay(stdscr, TRUE);  // Non-blocking input
    start_color();          // Enable colors
    
    // Define some colors pairs
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // Default
    init_pair(2, COLOR_BLACK, COLOR_CYAN);  // Header
    init_pair(3, COLOR_GREEN, COLOR_BLACK); // Running
    init_pair(4, COLOR_RED, COLOR_BLACK);   // Warning
}

void TUI::close() {
    endwin();
}

void TUI::draw(const std::map<int, ProcessInfo>& processes) {
    erase();

    // 1. Calculate stats & Filter/Sort preparation
    std::vector<ProcessInfo> visible_procs;
    double total_cpu = 0.0;
    long total_mem_kb = 0;

    for (const auto& pair : processes) {
        const auto& p = pair.second;
        total_cpu += p.cpu_percent;
        total_mem_kb += p.rss;

        // Filtering
        if (filter_text.empty() || p.name.find(filter_text) != std::string::npos) {
            visible_procs.push_back(p);
        }
    }

    // Sorting
    std::sort(visible_procs.begin(), visible_procs.end(), [this](const ProcessInfo& a, const ProcessInfo& b) {
        if (current_sort == SortMode::CPU) return a.cpu_percent > b.cpu_percent;
        if (current_sort == SortMode::MEM) return a.rss > b.rss;
        return a.pid < b.pid; // Default PID ascending
    });

    draw_header(visible_procs.size(), total_cpu, total_mem_kb);
    
    if (tree_mode) {
        draw_tree(processes);
    } else {
        // Draw Table from sorted vector
        int row = 2;
        int max_rows = LINES - 4;
        
        attron(A_BOLD);
        mvprintw(row++, 0, "%-6s %-20s %-6s %-6s %-8s %-8s %-8s", "PID", "NAME", "ST", "CPU%", "MEM", "READ/s", "WRITE/s");
        attroff(A_BOLD);
    
        for (const auto& p : visible_procs) {
            if (row >= max_rows + 2) break;
            
            if (p.state == "R") attron(COLOR_PAIR(3));
            
            // Format speeds (B/s, KB/s)
            std::string r_spd = std::to_string(p.io_read_speed);
            std::string w_spd = std::to_string(p.io_write_speed);
            if (p.io_read_speed > 1024) r_spd = std::to_string(p.io_read_speed / 1024) + "K";
            if (p.io_write_speed > 1024) w_spd = std::to_string(p.io_write_speed / 1024) + "K";

            mvprintw(row++, 0, "%-6d %-20s %-6s %-6.1f %-8ld %-8s %-8s", 
                p.pid, 
                p.name.substr(0, 20).c_str(), 
                p.state.c_str(), 
                p.cpu_percent, 
                p.rss,
                r_spd.c_str(),
                w_spd.c_str());
                
            if (p.state == "R") attroff(COLOR_PAIR(3));
        }
    }

    draw_footer();
    refresh();
}

// Simple recursive function to draw tree
void draw_tree_recursive(int pid, const std::map<int, std::vector<int>>& children, const std::map<int, ProcessInfo>& processes, int level, int& row, int max_rows) {
    if (row >= max_rows) return;
    if (processes.find(pid) == processes.end()) return; // Should not happen if children map built correctly

    const auto& p = processes.at(pid);
    
    // Indentation
    std::string indent = "";
    for(int i=0; i<level; i++) indent += "  ";
    
    std::string disp_name = indent + "|- " + p.name + " (" + std::to_string(pid) + ")";
    
    mvprintw(row++, 0, "%-60s [%s]", disp_name.substr(0, 60).c_str(), p.state.c_str());
    
    if (children.count(pid)) {
        for (int child_pid : children.at(pid)) {
            draw_tree_recursive(child_pid, children, processes, level + 1, row, max_rows);
        }
    }
}

void TUI::draw_tree(const std::map<int, ProcessInfo>& processes) {
    // 1. Build hierarchy
    std::map<int, std::vector<int>> children;
    std::vector<int> roots;
    
    for (const auto& pair : processes) {
        int pid = pair.first;
        int ppid = pair.second.ppid;
        
        if (processes.count(ppid)) {
            children[ppid].push_back(pid);
        } else {
            // If parent not in our list (e.g. kthreadd or we missed it), treat as root
            roots.push_back(pid);
        }
    }
    
    // 2. Draw
    int row = 2;
    int max_rows = LINES - 4;
    
    attron(A_BOLD);
    mvprintw(row++, 0, " PROCESS TREE VIEW (PID Map)");
    attroff(A_BOLD);
    
    for (int root : roots) {
        draw_tree_recursive(root, children, processes, 0, row, max_rows);
    }
}

void TUI::draw_header(size_t visible_count, double total_cpu, long total_mem) {
    attron(COLOR_PAIR(2));
    std::string mem_str = std::to_string(total_mem / 1024) + " MB";
    std::string sort_str = (current_sort == SortMode::CPU ? "CPU" : (current_sort == SortMode::MEM ? "MEM" : "PID"));
    if (!filter_text.empty()) sort_str += " [F: " + filter_text + "]";

    mvprintw(0, 0, " Linux Process Manager (AI Enabled) | Procs: %lu | CPU: %.1f%% | MEM: %s | Sort: %s   ", 
             visible_count, total_cpu, mem_str.c_str(), sort_str.c_str());
    
    for (int i = 0; i < COLS; ++i) { /* clean rest if needed */ }
    attroff(COLOR_PAIR(2));
}

void TUI::show_process_details(const ProcessInfo& p) {
    erase();
    int row = 2;
    
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(0, 0, " PROCESS DETAILS: %s (PID: %d) ", p.name.c_str(), p.pid);
    attroff(COLOR_PAIR(2) | A_BOLD);

    mvprintw(row++, 2, "State: %s", p.state.c_str());
    mvprintw(row++, 2, "CPU Usage: %.1f%%", p.cpu_percent);
    mvprintw(row++, 2, "Memory RSS: %ld KB", p.rss);
    mvprintw(row++, 2, "User Time: %llu ticks", p.utime);
    mvprintw(row++, 2, "System Time: %llu ticks", p.stime);
    
    row++;
    attron(A_BOLD);
    mvprintw(row++, 2, "CPU History (Last 60s):");
    attroff(A_BOLD);
    
    // Simple bar chart for CPU
    // 100% = 50 chars width
    for (double val : p.cpu_history) {
        if (row >= LINES - 3) break;
        int width = (int)(val / 2.0);
        if (width > 50) width = 50;
        
        mvprintw(row, 4, "%.1f%% ", val);
        for(int i=0; i<width; i++) addch('|');
        row++;
    }

    mvprintw(LINES-1, 0, "Press ANY KEY to return...");
    refresh();
    
    nodelay(stdscr, FALSE); // Blocking input
    getch();
    nodelay(stdscr, TRUE);  // Restore non-blocking
}

void TUI::draw_footer() {
    int row = LINES - 1;
    
    // Warnings area (line above command)
    if (!warnings.empty()) {
        attron(COLOR_PAIR(4) | A_BOLD); 
        
        std::string warn_msg = " [!] WARNINGS: ";
        for (const auto& w : warnings) {
            warn_msg += w.type + " (PID " + std::to_string(w.pid) + ") ";
        }
        mvprintw(row - 2, 0, "%s", warn_msg.c_str());
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    attron(COLOR_PAIR(2));
    // Pad with spaces to clear line
    std::string bar = " Command > " + input_buffer;
    mvprintw(row, 0, "%s", bar.c_str());
    for(int i = bar.length(); i < COLS; i++) addch(' ');

    if (!last_message.empty()) {
        mvprintw(row - 1, 0, "Last Action: %s", last_message.c_str());
    }
    attroff(COLOR_PAIR(2));
}

std::string TUI::get_input() {
    int ch = getch();
    if (ch == ERR) return "";

    if (ch == '\n' || ch == '\r') {
        std::string cmd = input_buffer;
        input_buffer.clear();
        return cmd;
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
        if (!input_buffer.empty()) {
            input_buffer.pop_back();
        }
    } else if (ch >= 32 && ch <= 126) {
        input_buffer += (char)ch;
    }
    
    return "";
}
