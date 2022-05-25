#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"
#include "graph.h"
#include "transport_router.h"
#include <tuple>
#include <iostream>
#include <vector>
#include <memory>

namespace serialization {

void Serialize(const transport::TransportCatalogue& transport_catalogue, const map_renderer::VectorDrawables& drawables,
			const graph::DirectedWeightedGraph<double>& transport_graph, const transport_router::TransportRoutes& transport_routes, std::ostream& output);

std::tuple<transport::TransportCatalogue, std::vector<std::unique_ptr<svg::Drawable>>,
	graph::DirectedWeightedGraph<double>, transport_router::TransportRoutes> Deserialize(std::istream& input);

} // namespace serilization