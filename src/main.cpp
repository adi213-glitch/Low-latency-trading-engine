#include "../include/engine.hpp"
#include <iostream>
#include <vector>

int main() {
    Engine engine;
    
    std::cout << "Loading ticks...\n";
    std::vector<csot::Tick> ticks = engine.load_ticks("data/synthetic_small.csv");

    if (ticks.empty()) {
        std::cerr << "No ticks loaded!\n";
        return 1;
    }

    std::cout << "Successfully loaded " << ticks.size() << " ticks.\n\n";

    // 1. Print the first tick
    std::cout << "--- FIRST TICK ---\n";
    std::cout << "Time:   " << ticks.front().timestamp_ns << "\n";
    std::cout << "Symbol: " << ticks.front().symbol << "\n";
    std::cout << "Bid:    " << ticks.front().bid_px << " x " << ticks.front().bid_qty << "\n";
    std::cout << "Ask:    " << ticks.front().ask_px << " x " << ticks.front().ask_qty << "\n\n";

    // 2. Print the last tick
    std::cout << "--- LAST TICK ---\n";
    std::cout << "Time:   " << ticks.back().timestamp_ns << "\n";
    std::cout << "Symbol: " << ticks.back().symbol << "\n";
    std::cout << "Bid:    " << ticks.back().bid_px << " x " << ticks.back().bid_qty << "\n";
    std::cout << "Ask:    " << ticks.back().ask_px << " x " << ticks.back().ask_qty << "\n\n";

    // 3. The crucial String Interning Test
    std::cout << "--- INTERNING TEST ---\n";
    std::cout << "First tick symbol is still: " << ticks[0].symbol << "\n";

    return 0;
}