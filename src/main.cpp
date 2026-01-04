#include <iostream>
#include <iomanip>
#include "ProcessScanner.h"

#include <iostream>
#include <thread>
#include <chrono>
#include "ProcessScanner.h"
#include "TUI.h"

int main() {
    ProcessScanner scanner;
    TUI tui;
    
    try {
        tui.init();
    } catch (...) {
        std::cerr << "Failed to initialize ncurses." << std::endl;
        return 1;
    }

    bool running = true;
    while (running) {
        // 1. Scan
        auto processes = scanner.scan();

        // 2. Render
        tui.draw(processes);

        // 3. Input Handling (Basic quit for now)
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            running = false;
        }

        // 4. Sleep (refresh rate)
        // using napms from ncurses is better than thread sleep in TUI
        napms(1000); 
    }

    tui.close();
    return 0;
}
