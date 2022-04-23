#include "transport_router.h"

using namespace std;

namespace transport_router {

TransportRoutes::TransportRoutes(
    RoutingSettings routing_settings,
    vector<BusData> bus_data_by_edge_id,
    vector<string_view> stop_name_by_vertex_id,
    unordered_map<string_view, graph::VertexId> vertex_id_by_stop_name)
: routing_settings_(move(routing_settings))
, bus_data_by_edge_id_(move(bus_data_by_edge_id))
, stop_name_by_vertex_id_(move(stop_name_by_vertex_id))
, vertex_id_by_stop_name_(move(vertex_id_by_stop_name)) {
}

const RoutingSettings& TransportRoutes::GetRoutingSettings() const {
    return routing_settings_;
}

const TransportRoutes::BusData& TransportRoutes::GetBusData(graph::EdgeId edge_id) const {
    return bus_data_by_edge_id_[edge_id];
}

string_view TransportRoutes::GetStopName(graph::VertexId vetex_id) const {
    return stop_name_by_vertex_id_[vetex_id];
}

graph::VertexId TransportRoutes::GetVertexId(string_view stop_name) const {
    return vertex_id_by_stop_name_.at(stop_name);
}

} // namespace transport_router