#pragma once

#include "geo.h"
#include "domain.h"
#include <unordered_map>
#include <deque>
#include <vector>
#include <string>
#include <string_view>

namespace transport {

class TransportCatalogue final {
public:
    const domain::Bus& AddBus(std::string name, std::vector<std::string_view> names_stops, bool ring);
    const domain::Stop& AddStop(std::string name, geo::Coordinates coordinates);
    
    const domain::Bus* FindBus(std::string_view name) const;
    const domain::Stop* FindStop(std::string_view name) const;
    
    void SetDistanceBetweenStops(
        std::string_view stop1, std::string_view stop2, int distance);
    
private:  
    std::deque<domain::Bus> data_buses_;
    std::deque<domain::Stop> data_stops_;
    std::unordered_map<std::string_view, domain::Bus*> data_bus_by_name_;
    std::unordered_map<std::string_view, domain::Stop*> data_stop_by_name_;
};
    
} //namespace transport