#pragma once
#include "graph.h"
#include <vector>
#include <unordered_map>
#include <string_view>

namespace transport_router {

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};

class TransportRoutes {
public:
    struct BusData {
        std::string_view name;
        size_t span_count;
    };

    TransportRoutes(RoutingSettings routing_settings,
                    std::vector<BusData> bus_data_by_edge_id,
                    std::vector<std::string_view> stop_name_by_vertex_id,
                    std::unordered_map<std::string_view, graph::VertexId> vertex_id_by_stop_name);

    const RoutingSettings& GetRoutingSettings() const;
    
    const BusData& GetBusData(graph::EdgeId edge_id) const;
    
    std::string_view GetStopName(graph::VertexId vetex_id) const;
    
    graph::VertexId GetVertexId(std::string_view stop_name) const;
    
private:
    RoutingSettings routing_settings_;
    std::vector<BusData> bus_data_by_edge_id_;
    std::vector<std::string_view> stop_name_by_vertex_id_;
    std::unordered_map<std::string_view, graph::VertexId> vertex_id_by_stop_name_;
};
    
} //namespace transport_router