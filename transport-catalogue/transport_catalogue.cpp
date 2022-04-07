#include "transport_catalogue.h"

using namespace std;
using Bus = domain::Bus;
using Stop = domain::Stop;

namespace transport {
    
const Bus& TransportCatalogue::AddBus(string name, vector<string_view> names_stops, bool ring) {
    vector<Stop*> stops;
    for (const string_view name_stop : names_stops) {
        stops.push_back(data_stop_by_name_.at(name_stop));
    }
    Bus& bus = data_buses_.emplace_back(move(name), move(stops), ring);
    data_bus_by_name_.insert({bus.GetName(), &bus});
    return bus;
}
    
const Stop& TransportCatalogue::AddStop(string name, geo::Coordinates coordinates) {
    Stop& stop = data_stops_.emplace_back(move(name), move(coordinates));
    data_stop_by_name_.insert({stop.GetName(), &stop});
    return stop;
}
    
const Bus* TransportCatalogue::FindBus(string_view name) const {
    auto bus = data_bus_by_name_.find(name);
    return bus != data_bus_by_name_.end() ? bus->second : nullptr;
}
    
const Stop* TransportCatalogue::FindStop(string_view name) const {
    auto stop = data_stop_by_name_.find(name);
    return stop != data_stop_by_name_.end() ? stop->second : nullptr;
}
    
void TransportCatalogue::SetDistanceBetweenStops(
    string_view stop1, string_view stop2, int distance)
{
    data_stop_by_name_.at(stop1)->AddDistance(
        data_stop_by_name_.find(stop2)->first, distance);
}
    
} //namespace transport