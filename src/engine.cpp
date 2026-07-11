#include "../include/engine.hpp"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include <deque>
#include <vector>
#include <string_view>

std::vector<csot::Tick> Engine::load_ticks(std::string_view path){
        std::ifstream file(static_cast<std::string>(path));
        if (!file.is_open()) {
            std::cerr << "Failed to open file\n";
            return {};
        }
        // read and ignore first line
        std::string line;  
        std::getline(file,line);
        std::vector<csot::Tick> storeTicks;
        while(std::getline(file,line)){
            //line holds data now
            std::stringstream ss(line);
            csot::Tick tick {};

            std::string token;

            std::getline(ss,token, ',');
            tick.timestamp_ns =std::stoull(token);
            token.clear();

            std::string symbol;
            std::getline(ss,symbol, ',');
            
            if (phonebook.find(symbol) == phonebook.end()) {
                symbols.push_back(symbol);
                phonebook[symbol] = symbols.back(); // Store the view in the map
            }
            tick.symbol = phonebook[symbol];
            

            std::getline(ss,token, ',');
            tick.bid_px = std::stod(token);
            token.clear();

            std::getline(ss,token, ',');
            tick.ask_px = std::stod(token);
            token.clear();

            std::getline(ss,token, ',');
            tick.bid_qty = std::stoul(token);
            token.clear();

            std::getline(ss,token, ',');
            tick.ask_qty = std::stoul(token);
            token.clear();

            storeTicks.push_back(tick);
        }
        return storeTicks;
}


void Engine::run(csot::Strategy* strategy, const std::vector<csot::Tick>& ticks){
    csot::LatencyHistogram hist;
    strategy->on_init();
    
    uint32_t counter_ticks = 0;
    for (const auto& tick : ticks) {
        auto t1 = std::chrono::steady_clock::now();

        std::vector<csot::Order> orders{strategy->on_tick(tick)};

        auto t2 = std::chrono::steady_clock::now();
        hist.record(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());

        for(const auto& o : orders){
            strategy->on_fill(o,o.price,o.qty);
        }
        counter_ticks++;
    }
    std::cout << counter_ticks << " ticks were processed.\n";
    hist.print(std::cout);
}