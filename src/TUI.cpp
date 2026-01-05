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
}

void TUI::close() {
    endwin();
}

void TUI::draw(const std::map<int, ProcessInfo>& processes) {
    erase(); // Clear screen

    draw_header();
    draw_table(processes);
    draw_footer();

    refresh(); // Refresh screen
}

void TUI::draw_header() {
    attron(COLOR_PAIR(2));
    mvprintw(0, 0, " Linux Process Manager (Phase 3) | Processes: ??? | CPU: ???%% | MEM: ???");
    
    // Fill the rest of the line
    for (int i = 0; i < COLS; ++i) {
        // Just simple padding if needed, but mvprintw overwrites
    }
    attroff(COLOR_PAIR(2));
}

void TUI::draw_table(const std::map<int, ProcessInfo>& processes) {
    int row = 2;
    int max_rows = LINES - 4; // Reserve space for header and footer

    // Table Header
    attron(A_BOLD);
    mvprintw(row++, 0, "%-8s %-25s %-10s %-10s %-10s", "PID", "NAME", "STATE", "CPU%", "RSS(KB)");
    attroff(A_BOLD);

    // Simple sorting or just iterating (map is sorted by PID key)
    // To sort by CPU, we'd need a vector. For now, just PID order.
    
    for (const auto& pair : processes) {
        if (row >= max_rows + 2) break; // Screen full

        const auto& p = pair.second;
        
        // Color code for state?
        if (p.state == "R") attron(COLOR_PAIR(3));
        
        mvprintw(row++, 0, "%-8d %-25s %-10s %-10.1f %-10ld", 
            p.pid, 
            p.name.substr(0, 25).c_str(), 
            p.state.c_str(), 
            p.cpu_percent, 
            p.rss);
            
        if (p.state == "R") attroff(COLOR_PAIR(3));
    }
}

void TUI::draw_footer() {
    int row = LINES - 1;
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
