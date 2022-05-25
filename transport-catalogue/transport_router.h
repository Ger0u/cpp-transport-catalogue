#pragma once
#include "graph.h"
#include <transport_router.pb.h>
#include <vector>
#include <unordered_map>

namespace transport_router {

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};

class TransportRoutes {
public:
    struct BusData {
        size_t index;
        size_t span_count;
    };

    TransportRoutes() = default;
    
    TransportRoutes(RoutingSettings routing_settings,
                    std::vector<BusData> bus_data_by_edge_id,
                    std::vector<size_t> stop_index_by_vertex_id,
                    std::unordered_map<size_t, graph::VertexId> vertex_id_by_stop_index);

    const RoutingSettings& GetRoutingSettings() const;
    
    const BusData& GetBusData(graph::EdgeId edge_id) const;
    
    size_t GetStopIndex(graph::VertexId vetex_id) const;
    
    graph::VertexId GetVertexId(size_t stop_index) const;

    proto::TransportRoutes OutProto() const;
    void InProto(const proto::TransportRoutes& proto_transport_routes);
    
private:
    RoutingSettings routing_settings_;
    std::vector<BusData> bus_data_by_edge_id_;
    std::vector<size_t> stop_index_by_vertex_id_;
    std::unordered_map<size_t, graph::VertexId> vertex_id_by_stop_index_;
};
    
} //namespace transport_router