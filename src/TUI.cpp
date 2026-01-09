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
    
    // Draw Table from sorted vector
    int row = 2;
    int max_rows = LINES - 4;
    
    attron(A_BOLD);
    mvprintw(row++, 0, "%-8s %-25s %-10s %-10s %-10s", "PID", "NAME", "STATE", "CPU%", "RSS(KB)");
    attroff(A_BOLD);

    for (const auto& p : visible_procs) {
        if (row >= max_rows + 2) break;
        
        if (p.state == "R") attron(COLOR_PAIR(3));
        
        mvprintw(row++, 0, "%-8d %-25s %-10s %-10.1f %-10ld", 
            p.pid, 
            p.name.substr(0, 25).c_str(), 
            p.state.c_str(), 
            p.cpu_percent, 
            p.rss);
            
        if (p.state == "R") attroff(COLOR_PAIR(3));
    }

    draw_footer();
    refresh();
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
