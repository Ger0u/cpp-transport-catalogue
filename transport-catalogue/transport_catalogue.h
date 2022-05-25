#pragma once

#include <transport_catalogue.pb.h>
#include "geo.h"
#include "domain.h"
#include <unordered_map>
#include <deque>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>

namespace transport {

class TransportCatalogue final {
public:
    const domain::Bus& AddBus(std::string name, std::vector<std::string_view> names_stops, bool ring);
    const domain::Stop& AddStop(std::string name, geo::Coordinates coordinates);
    
    const domain::Bus& FindBus(size_t index) const;
    size_t IndexBus(std::string_view name) const;
    const domain::Bus* FindBus(std::string_view name) const;
    const domain::Stop& FindStop(size_t index) const;
    size_t IndexStop(std::string_view name) const;
    const domain::Stop* FindStop(std::string_view name) const;
    
    void SetDistanceBetweenStops(
        std::string_view stop1, std::string_view stop2, int distance);
    
    proto::TransportCatalogue OutProto() const;
    void InProto(const proto::TransportCatalogue& proto_transport_catalogue);
    
private:  
    std::deque<domain::Bus> buses_;
    std::deque<domain::Stop> stops_;
    std::unordered_map<std::string_view, size_t> bus_index_by_name_;
    std::unordered_map<std::string_view, size_t> stop_index_by_name_;
};
    
} //namespace transport