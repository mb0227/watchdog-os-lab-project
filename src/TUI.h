#pragma once
#include <map>
#include <ncurses.h>
#include "ProcessInfo.h"

class TUI {
public:
    void init();
    void close();
    void draw(const std::map<int, ProcessInfo>& processes);

private:
    void draw_header();
    void draw_table(const std::map<int, ProcessInfo>& processes);
    void draw_footer();
};
