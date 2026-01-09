#pragma once
#include <map>
#include <ncurses.h>
#include <vector>
#include "ProcessInfo.h"
#include "RuleEngine.h"

enum class SortMode {
    PID,
    CPU,
    MEM
};

class TUI {
public:
    void init();
    void close();
    void draw(const std::map<int, ProcessInfo>& processes);
    
    std::string get_input();
    void set_warnings(const std::vector<Warning>& w) { warnings = w; }
    void set_message(const std::string& msg) { last_message = msg; }
    
    // UI Logic
    void set_sort_mode(SortMode mode) { current_sort = mode; }
    void set_filter(const std::string& f) { filter_text = f; }
    void show_process_details(const ProcessInfo& p);

private:
    std::string input_buffer;
    std::string last_message;
    std::vector<Warning> warnings;
    
    // UI State
    SortMode current_sort = SortMode::PID;
    std::string filter_text = "";

    void draw_header(size_t visible_count, double total_cpu, long total_mem);
    void draw_table(const std::map<int, ProcessInfo>& processes);
    void draw_footer();
};
