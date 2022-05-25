#pragma once

#include "geo.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace domain {
    
struct Stop {
    std::string name;
    geo::Coordinates coordinates;
    std::unordered_map<size_t, int> distance_to_stops = {};

    std::vector<size_t> bus_indexs = {};
};

struct Bus {
    std::string name;
    std::vector<size_t> stop_indexs;
    bool ring;

    int length = 0;
    double ideal_length = 0;
    size_t count_stops = 0;
    size_t count_unique_stops = 0;
};
    
} // namespace domain