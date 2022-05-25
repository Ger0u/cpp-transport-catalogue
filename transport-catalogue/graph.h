#pragma once

#include "ranges.h"
#include <graph.pb.h>

#include <cstdlib>
#include <vector>
#include <utility>

namespace graph {

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight>
struct Edge {
    VertexId from;
    VertexId to;
    Weight weight;
};

template <typename Weight>
class DirectedWeightedGraph {
private:
    using IncidenceList = std::vector<EdgeId>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

public:
    DirectedWeightedGraph() = default;
    explicit DirectedWeightedGraph(size_t vertex_count);
    EdgeId AddEdge(const Edge<Weight>& edge);

    size_t GetVertexCount() const;
    size_t GetEdgeCount() const;
    const Edge<Weight>& GetEdge(EdgeId edge_id) const;
    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

    proto::DirectedWeightedGraph OutProto() const;
    void InProto(const proto::DirectedWeightedGraph& proto_directed_weighted_graph);

private:
    std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidence_lists_;
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {
}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    incidence_lists_.at(edge.from).push_back(id);
    return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
    return incidence_lists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
    return edges_.size();
}

template <typename Weight>
const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
    return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
    return ranges::AsRange(incidence_lists_.at(vertex));
}

template <typename Weight>
proto::DirectedWeightedGraph DirectedWeightedGraph<Weight>::OutProto() const {
    proto::DirectedWeightedGraph proto_directed_weighted_graph;

    for (int i = 0; i < edges_.size(); ++i) {
        proto::Edge proto_edge;
        proto_edge.set_from(edges_[i].from);
        proto_edge.set_to(edges_[i].to);
        proto_edge.set_weight(edges_[i].weight);

        proto_directed_weighted_graph.add_edge();
        *proto_directed_weighted_graph.mutable_edge(i) = std::move(proto_edge);
    }

    for (int i = 0; i < incidence_lists_.size(); ++i) {
        proto::IncidenceList proto_incidence_list;
        for (int j = 0; j < incidence_lists_[i].size(); ++j) {
            proto_incidence_list.add_incidence(incidence_lists_[i][j]);
        }

        proto_directed_weighted_graph.add_incidence_list();
        *proto_directed_weighted_graph.mutable_incidence_list(i) = std::move(proto_incidence_list);
    }

    return proto_directed_weighted_graph;
}

template <typename Weight>
void DirectedWeightedGraph<Weight>::InProto(const proto::DirectedWeightedGraph& proto_directed_weighted_graph) {
    edges_.resize(proto_directed_weighted_graph.edge_size());
    for (int i = 0; i < proto_directed_weighted_graph.edge_size(); ++i) {
        const proto::Edge& proto_edge = proto_directed_weighted_graph.edge(i);

        edges_[i] = { proto_edge.from(), proto_edge.to(), proto_edge.weight() };
    }

    incidence_lists_.resize(proto_directed_weighted_graph.incidence_list_size());
    for (int i = 0; i < proto_directed_weighted_graph.incidence_list_size(); ++i) {
        const proto::IncidenceList& proto_incidence_list = proto_directed_weighted_graph.incidence_list(i);

        IncidenceList incidence_list;
        incidence_list.reserve(proto_incidence_list.incidence_size());

        for (int j = 0; j < proto_incidence_list.incidence_size(); ++j) {
            incidence_list.push_back(proto_incidence_list.incidence(j));
        }

        incidence_lists_[i] = std::move(incidence_list);
    }
}

}  // namespace graph