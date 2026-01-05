#pragma once
#include <map>
#include <ncurses.h>
#include "ProcessInfo.h"

class TUI {
public:
    void init();
    void close();
    void draw(const std::map<int, ProcessInfo>& processes);
    
    // Returns the current command string if user pressed Enter, else empty
    std::string get_input();

private:
    std::string input_buffer;
    std::string last_message;

    void draw_header();
    void draw_table(const std::map<int, ProcessInfo>& processes);
    void draw_footer();
    
public:
    void set_message(const std::string& msg) { last_message = msg; }
};
