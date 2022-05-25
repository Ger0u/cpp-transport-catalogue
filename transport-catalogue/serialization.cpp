#include "serialization.h"
#include <transport_catalogue.pb.h>
#include <utility>
#include <string>
#include <string_view>

using namespace std;

namespace serialization {

void Serialize(const transport::TransportCatalogue& transport_catalogue, const map_renderer::VectorDrawables& drawables,
               const graph::DirectedWeightedGraph<double>& transport_graph, const transport_router::TransportRoutes& transport_routes, ostream& output) {
    proto::Data proto_data;

    *proto_data.mutable_transport_catalogue() = transport_catalogue.OutProto();
    *proto_data.mutable_drawables() = drawables.OutProto();
    *proto_data.mutable_transport_graph() = transport_graph.OutProto();
    *proto_data.mutable_transport_routes() = transport_routes.OutProto();

    proto_data.SerializeToOstream(&output);
}

tuple<transport::TransportCatalogue, vector<unique_ptr<svg::Drawable>>,
    graph::DirectedWeightedGraph<double>, transport_router::TransportRoutes> Deserialize(istream& input)
{
    proto::Data proto_data;
    proto_data.ParseFromIstream(&input);

    transport::TransportCatalogue transport_catalogue;
    transport_catalogue.InProto(proto_data.transport_catalogue());

    map_renderer::VectorDrawables drawables;
    drawables.InProto(proto_data.drawables());

    graph::DirectedWeightedGraph<double> transport_graph;
    transport_graph.InProto(proto_data.transport_graph());

    transport_router::TransportRoutes transport_routes;
    transport_routes.InProto(proto_data.transport_routes());

    return { move(transport_catalogue), move(drawables.drawables), move(transport_graph), move(transport_routes) };
}

} // namespace serilization