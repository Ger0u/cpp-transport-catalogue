#include "transport_catalogue.h"
#include <algorithm>
#include <unordered_set>

using namespace std;
using Bus = domain::Bus;
using Stop = domain::Stop;

namespace transport {
    
const Bus& TransportCatalogue::AddBus(string name, vector<string_view> names_stops, bool ring) {
    const size_t bus_index = buses_.size();
    
    Bus& bus = buses_.emplace_back(Bus{move(name), {}, ring});
    bus_index_by_name_.insert({bus.name, bus_index});

    vector<size_t>& stop_indexs = bus.stop_indexs;
    stop_indexs.reserve(names_stops.size());
    for (const string_view name_stop : names_stops) {
        const size_t stop_index = stop_index_by_name_.at(name_stop);
        vector<size_t>& bus_indexs = stops_[stop_index].bus_indexs;

        if (bus_indexs.empty()) {
            bus_indexs.push_back(bus_index);
        } else {
            auto it = lower_bound(bus_indexs.begin(), bus_indexs.end(), bus_index,
                [this](size_t lhs, size_t rhs) { return buses_[lhs].name < buses_[rhs].name; });
            if (it == bus_indexs.end() || *it != bus_index) {
                bus_indexs.insert(it, bus_index);
            }
        }

        stop_indexs.push_back(stop_index);
    }

    if (ring) {
        for (size_t i = 0; i < stop_indexs.size() - 1; ++i) {
            auto it = stops_[stop_indexs[i]].distance_to_stops.find(stop_indexs[i + 1]);
            it = it != stops_[stop_indexs[i]].distance_to_stops.end() ?
                 it : stops_[stop_indexs[i + 1]].distance_to_stops.find(stop_indexs[i]);
            bus.length += it->second;
            bus.ideal_length += geo::ComputeDistance(stops_[stop_indexs[i]].coordinates,
                                                 stops_[stop_indexs[i + 1]].coordinates);
        }
    } else {
        for (size_t i = 0; i < stop_indexs.size() - 1; ++i) {
            auto it1 = stops_[stop_indexs[i]].distance_to_stops.find(stop_indexs[i + 1]);
            auto it2 = stops_[stop_indexs[i + 1]].distance_to_stops.find(stop_indexs[i]);
            bus.length += it1 != stops_[stop_indexs[i]].distance_to_stops.end() &&
                      it2 != stops_[stop_indexs[i + 1]].distance_to_stops.end() ?
                          it1->second + it2->second :
                            it1 != stops_[stop_indexs[i]].distance_to_stops.end() ?
                                2 * it1->second : 2 * it2->second;
            bus.ideal_length += 2 * geo::ComputeDistance(stops_[stop_indexs[i]].coordinates,
                                                     stops_[stop_indexs[i + 1]].coordinates);
        }
    }

    bus.count_stops = ring ? stop_indexs.size() : 2 * stop_indexs.size() - 1;
    bus.count_unique_stops = [stop_indexs]() {
        unordered_set<size_t> set(stop_indexs.begin(), stop_indexs.end());
        return set.size();
    }();

    return bus;
}
    
const Stop& TransportCatalogue::AddStop(string name, geo::Coordinates coordinates) {
    Stop& stop = stops_.emplace_back(Stop{move(name), move(coordinates)});
    stop_index_by_name_.insert({stop.name, stops_.size() - 1});
    return stop;
}
    
const Bus& TransportCatalogue::FindBus(size_t index) const {
    return buses_[index];
}

size_t TransportCatalogue::IndexBus(std::string_view name) const {
    return bus_index_by_name_.at(name);
}
    
const Bus* TransportCatalogue::FindBus(string_view name) const {
    auto bus = bus_index_by_name_.find(name);
    return bus != bus_index_by_name_.end() ? &buses_[bus->second] : nullptr;
}
    
const Stop& TransportCatalogue::FindStop(size_t index) const {
    return stops_[index];
}

size_t TransportCatalogue::IndexStop(std::string_view name) const {
    return stop_index_by_name_.at(name);
}
    
const Stop* TransportCatalogue::FindStop(string_view name) const {
    auto stop = stop_index_by_name_.find(name);
    return stop != stop_index_by_name_.end() ? &stops_[stop->second] : nullptr;
}
    
void TransportCatalogue::SetDistanceBetweenStops(
    string_view stop1, string_view stop2, int distance)
{
    stops_[stop_index_by_name_.at(stop1)].distance_to_stops.emplace(
        stop_index_by_name_.at(stop2), distance);
}
    
proto::TransportCatalogue TransportCatalogue::OutProto() const {
    proto::TransportCatalogue proto_transport_catalogue;

    for (int i = 0; i < buses_.size(); ++i) {
        const Bus& bus = buses_[i];

        proto::Bus proto_bus;
        proto_bus.set_name(bus.name);
        for (const size_t stop_index : bus.stop_indexs) {
            proto_bus.add_stop_index(stop_index);
        }
        proto_bus.set_ring(bus.ring);
        proto_bus.set_length(bus.length);
        proto_bus.set_ideal_length(bus.ideal_length);
        proto_bus.set_count_stops(bus.count_stops);
        proto_bus.set_count_unique_stops(bus.count_unique_stops);

        proto_transport_catalogue.add_bus();
        *proto_transport_catalogue.mutable_bus(i) = move(proto_bus);
    }

    for (int i = 0; i < stops_.size(); ++i) {
        const Stop& stop = stops_[i];

        proto::Stop proto_stop;
        proto_stop.set_name(stop.name);
        {
            proto::Coordinates proto_coordinates;
            proto_coordinates.set_lat(stop.coordinates.lat);
            proto_coordinates.set_lng(stop.coordinates.lng);

            *proto_stop.mutable_coordinates() = move(proto_coordinates);
        }
        for (const auto& [stop_index, distance] : stop.distance_to_stops) {
            proto::DistanceToStop proto_distance_to_stop;
            proto_distance_to_stop.set_stop_index(stop_index);
            proto_distance_to_stop.set_distance(distance);

            proto_stop.add_distance_to_stop();
            *proto_stop.mutable_distance_to_stop(proto_stop.distance_to_stop_size() - 1) = move(proto_distance_to_stop);
        }
        for (const size_t bus_index : stop.bus_indexs) {
            proto_stop.add_bus_index(bus_index);
        }

        proto_transport_catalogue.add_stop();
        *proto_transport_catalogue.mutable_stop(i) = move(proto_stop);
    }

    return proto_transport_catalogue;
}
   
void TransportCatalogue::InProto(const proto::TransportCatalogue& proto_transport_catalogue) {
    bus_index_by_name_.clear();
    buses_.resize(proto_transport_catalogue.bus_size());
    for (int i = 0; i < proto_transport_catalogue.bus_size(); ++i) {
        const proto::Bus& proto_bus = proto_transport_catalogue.bus(i);

        vector<size_t> stop_indexs;
        stop_indexs.reserve(proto_bus.stop_index_size());
        for (int j = 0; j < proto_bus.stop_index_size(); ++j) {
            stop_indexs.push_back(proto_bus.stop_index(j));
        }
        
        Bus bus{proto_bus.name(), move(stop_indexs), proto_bus.ring(),
                proto_bus.length(), proto_bus.ideal_length(),
                proto_bus.count_stops(), proto_bus.count_unique_stops()};
        buses_[i] = move(bus);
        bus_index_by_name_.insert({buses_[i].name, i});
    }
    
    stop_index_by_name_.clear();
    stops_.resize(proto_transport_catalogue.stop_size());
    for (int i = 0; i < proto_transport_catalogue.stop_size(); ++i) {
        const proto::Stop& proto_stop = proto_transport_catalogue.stop(i);

        geo::Coordinates coordinates{proto_stop.coordinates().lat(),
                                     proto_stop.coordinates().lng()};
        
        unordered_map<size_t, int> distance_to_stops;
        for (int j = 0; j < proto_stop.distance_to_stop_size(); ++j) {
            const proto::DistanceToStop& proto_distance_to_stop =
                proto_stop.distance_to_stop(j);
            
            distance_to_stops.insert({proto_distance_to_stop.stop_index(),
                                      proto_distance_to_stop.distance()});
        }
        
        vector<size_t> bus_indexs;
        bus_indexs.reserve(proto_stop.bus_index_size());
        for (int j = 0; j < proto_stop.bus_index_size(); ++j) {
            bus_indexs.push_back(proto_stop.bus_index(j));
        }
        
        Stop stop{proto_stop.name(), move(coordinates),
                  move(distance_to_stops), move(bus_indexs)};
        stops_[i] = move(stop);
        stop_index_by_name_.insert({stops_[i].name, i});
    }
}
    
} //namespace transport