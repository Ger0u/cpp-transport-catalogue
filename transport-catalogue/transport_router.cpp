#include "transport_router.h"

using namespace std;

namespace transport_router {

TransportRoutes::TransportRoutes(
    RoutingSettings routing_settings,
    vector<BusData> bus_data_by_edge_id,
    vector<size_t> stop_index_by_vertex_id,
    unordered_map<size_t, graph::VertexId> vertex_id_by_stop_index)
: routing_settings_(move(routing_settings))
, bus_data_by_edge_id_(move(bus_data_by_edge_id))
, stop_index_by_vertex_id_(move(stop_index_by_vertex_id))
, vertex_id_by_stop_index_(move(vertex_id_by_stop_index)) {
}

const RoutingSettings& TransportRoutes::GetRoutingSettings() const {
    return routing_settings_;
}

const TransportRoutes::BusData& TransportRoutes::GetBusData(graph::EdgeId edge_id) const {
    return bus_data_by_edge_id_[edge_id];
}

size_t TransportRoutes::GetStopIndex(graph::VertexId vetex_id) const {
    return stop_index_by_vertex_id_[vetex_id];
}

graph::VertexId TransportRoutes::GetVertexId(size_t stop_index) const {
    return vertex_id_by_stop_index_.at(stop_index);
}

proto::TransportRoutes TransportRoutes::OutProto() const {
    proto::TransportRoutes proto_transport_routes;

    {
        proto::RoutingSettings proto_routing_settings;
        proto_routing_settings.set_bus_wait_time(routing_settings_.bus_wait_time);
        proto_routing_settings.set_bus_velocity(routing_settings_.bus_velocity);

        *proto_transport_routes.mutable_routing_settings() = move(proto_routing_settings);
    }
    for (int i = 0; i < bus_data_by_edge_id_.size(); ++i) {
        proto::BusData proto_bus_data;
        proto_bus_data.set_index(bus_data_by_edge_id_[i].index);
        proto_bus_data.set_span_count(bus_data_by_edge_id_[i].span_count);

        proto_transport_routes.add_bus_data_by_edge_id();
        *proto_transport_routes.mutable_bus_data_by_edge_id(i) = move(proto_bus_data);
    }
    for (int i = 0; i < stop_index_by_vertex_id_.size(); ++i) {
        proto_transport_routes.add_stop_index_by_vertex_id(stop_index_by_vertex_id_[i]);
    }
    for (int i = 0; i < vertex_id_by_stop_index_.size(); ++i) {
        proto::VertexIdByStopIndex proto_vertex_id_by_stop_index;
        proto_vertex_id_by_stop_index.set_index(i);
        proto_vertex_id_by_stop_index.set_vertex_id(vertex_id_by_stop_index_.at(i));

        proto_transport_routes.add_vertex_id_by_stop_index();
        *proto_transport_routes.mutable_vertex_id_by_stop_index(i) = move(proto_vertex_id_by_stop_index);
    }

    return proto_transport_routes;
}

void TransportRoutes::InProto(const proto::TransportRoutes& proto_transport_routes) {
    routing_settings_ = { proto_transport_routes.routing_settings().bus_wait_time(),
        proto_transport_routes.routing_settings().bus_velocity() };

    bus_data_by_edge_id_.resize(proto_transport_routes.bus_data_by_edge_id_size());
    for (int i = 0; i < proto_transport_routes.bus_data_by_edge_id_size(); ++i) {
        const proto::BusData& proto_bus_data = proto_transport_routes.bus_data_by_edge_id(i);

        bus_data_by_edge_id_[i] = { proto_bus_data.index(), proto_bus_data.span_count() };
    }

    stop_index_by_vertex_id_.resize(proto_transport_routes.stop_index_by_vertex_id_size());
    for (int i = 0; i < proto_transport_routes.stop_index_by_vertex_id_size(); ++i) {
        stop_index_by_vertex_id_[i] = proto_transport_routes.stop_index_by_vertex_id(i);
    }

    vertex_id_by_stop_index_.clear();
    for (int i = 0; i < proto_transport_routes.vertex_id_by_stop_index_size(); ++i) {
        const proto::VertexIdByStopIndex& proto_vertex_id_by_stop_index = proto_transport_routes.vertex_id_by_stop_index(i);

        vertex_id_by_stop_index_.insert({ proto_vertex_id_by_stop_index.index(), proto_vertex_id_by_stop_index.vertex_id() });
    }
}

} // namespace transport_router