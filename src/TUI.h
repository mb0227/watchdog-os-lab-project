#pragma once
#include <map>
#include <ncurses.h>
#include "ProcessInfo.h"
#include "RuleEngine.h" // For Warning struct

class TUI {
public:
    void init();
    void close();
    void draw(const std::map<int, ProcessInfo>& processes);
    
    // Returns the current command string if user pressed Enter, else empty
    std::string get_input();
    
    void set_warnings(const std::vector<Warning>& w) { warnings = w; }

private:
    std::string input_buffer;
    std::string last_message;
    std::vector<Warning> warnings;

    void draw_header(const std::map<int, ProcessInfo>& processes);
    void draw_table(const std::map<int, ProcessInfo>& processes);
    void draw_footer();
    
public:
    void set_message(const std::string& msg) { last_message = msg; }
};
