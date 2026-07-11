#pragma once
#include "strategy.hpp"
#include <string>
#include <iostream>
#include <unordered_map>
#include <deque>
#include <vector>
#include <string_view>
class Engine{
private:
    std::unordered_map<std::string, std::string_view> phonebook;
    std::deque<std::string> symbols;
public:
    std::vector<csot::Tick> load_ticks(std::string_view path);

};