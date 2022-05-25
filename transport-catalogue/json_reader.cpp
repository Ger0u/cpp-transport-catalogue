#include "json_reader.h"
#include "json_builder.h"
#include "graph.h"
#include <cassert>
#include <algorithm>
#include <sstream>
using namespace std;

namespace transport {
    
namespace json_reader {
    
map_renderer::RenderSettings CreateRenderSettings(
    const json::Dict& render_settings)
{
    return {
        render_settings.at("width"s).AsDouble(),
        render_settings.at("height"s).AsDouble(),
        
        render_settings.at("padding"s).AsDouble(),
        
        render_settings.at("line_width"s).AsDouble(),
        render_settings.at("stop_radius"s).AsDouble(),
        
        render_settings.at("bus_label_font_size"s).AsInt(),
        ArrayToPoint(render_settings.at("bus_label_offset"s).AsArray()),
        
        render_settings.at("stop_label_font_size"s).AsInt(),
        ArrayToPoint(render_settings.at("stop_label_offset"s).AsArray()),
        
        NodeToColor(render_settings.at("underlayer_color"s)),
        render_settings.at("underlayer_width"s).AsDouble(),
        
        ArrayToVectorColor(render_settings.at("color_palette"s).AsArray())
    };
}
    
transport_router::RoutingSettings CreateRoutingSettings(
    const json::Dict& routing_settings)
{
    return {
        routing_settings.at("bus_wait_time"s).AsInt(),
        routing_settings.at("bus_velocity"s).AsDouble()
    };
}
    
svg::Point ArrayToPoint(const json::Array& arr) {
    assert(arr.size() == 2);
    return {arr[0].AsDouble(), arr[1].AsDouble()};
}
    
svg::Color NodeToColor(const json::Node& node) {
    if (node.IsString()) {
        return node.AsString();
    } else {
        const json::Array& arr = node.AsArray();
        if (arr.size() == 3) {
            return svg::Rgb{(uint8_t)arr[0].AsInt(),
                            (uint8_t)arr[1].AsInt(),
                            (uint8_t)arr[2].AsInt()};
        } else if (arr.size() == 4) {
            return svg::Rgba{(uint8_t)arr[0].AsInt(),
                             (uint8_t)arr[1].AsInt(),
                             (uint8_t)arr[2].AsInt(),
                              arr[3].AsDouble()};
        } else {
            assert(false);
        }
    }
}
    
vector<svg::Color> ArrayToVectorColor(const json::Array& arr) {
    vector<svg::Color> result(arr.size());
    transform(
        arr.begin(), arr.end(),
        result.begin(),
        NodeToColor
    );
    return result;
}
    
void CreateTransportCatalogueAndHandleRequests(istream& input, ostream& output) {
    const auto document = json::Load(input);
    const auto& requests = document.GetRoot().AsDict();
    const auto render_settings =
        CreateRenderSettings(requests.at("render_settings"s).AsDict());
    const auto routing_settings =
        CreateRoutingSettings(requests.at("routing_settings"s).AsDict());
    auto [transport_catalogue, picture, transport_graph, transport_routes] = CreateTransportCatalogue(
        requests.at("base_requests"s).AsArray(), render_settings, routing_settings);
    HandleRequests(
        transport_catalogue,
        output,
        requests.at("stat_requests"s).AsArray(),
        picture,
        transport_graph,
        transport_routes
    );
}
 
tuple<TransportCatalogue, unordered_map<string_view, const domain::Stop*>,
      map<string_view, const domain::Stop*>, map<string_view, const domain::Bus*>>
CreateStopsAndBuses(const json::Array& base_requests) {
    vector<const json::Dict*> node_stops;
    vector<const json::Dict*> node_buses;
    for (const json::Node& node_request : base_requests) {
        const json::Dict& request = node_request.AsDict();
        string_view type = request.at("type"s).AsString();
        if (type == "Stop"sv) {
            node_stops.push_back(&request);
        } else if (type == "Bus"sv) {
            node_buses.push_back(&request);
        }
    }
    TransportCatalogue transport_catalogue;
    auto all_stops = CreateStops(transport_catalogue, node_stops);
    SetDistanceBetweenStops(transport_catalogue, node_stops);
    auto [stops, buses] = CreateBuses(transport_catalogue, node_buses, all_stops);
    return {move(transport_catalogue), move(all_stops),
            move(stops), move(buses)};
}

tuple<TransportCatalogue, vector<unique_ptr<svg::Drawable>>,
      graph::DirectedWeightedGraph<double>,
      transport_router::TransportRoutes>
CreateTransportCatalogue(const json::Array& base_requests,
                         const map_renderer::RenderSettings& render_settings,
                         const transport_router::RoutingSettings& routing_settings)
{
    auto [transport_catalogue, all_stops, stops, buses] = CreateStopsAndBuses(base_requests);
    auto [min_lon, max_lon, min_lat, max_lat] = FindExtremeCoordinates(stops);
    map_renderer::ScalingPoints scaling_points(
        render_settings.width,
        render_settings.height,
        render_settings.padding,
        min_lon,
        max_lon,
        min_lat,
        max_lat
    );
    auto stops_points = ScaleStopPoints(stops, scaling_points);
    auto picture = CreateMapObjects(transport_catalogue, stops_points, buses, render_settings);
    graph::DirectedWeightedGraph<double> transport_graph(all_stops.size());
    vector<size_t> stop_index_by_vertex_id(all_stops.size());
    unordered_map<size_t, graph::VertexId> vertex_id_by_stop_index;
    {
        graph::VertexId i = 0;
        for (const auto& [name, _] : all_stops) {
            size_t index = transport_catalogue.IndexStop(name);
            vertex_id_by_stop_index.emplace(index, i);
            stop_index_by_vertex_id[i] = index;
            ++i;
        }
    }
    vector<transport_router::TransportRoutes::BusData> bus_data_by_edge_id;
    for (const auto [name, bus] : buses) {
        size_t index_bus = transport_catalogue.IndexBus(name);
        FillingInObjectsForTransportRoutes(
            bus->stop_indexs.begin(), bus->stop_indexs.end(),
            index_bus,
            transport_catalogue,
            routing_settings,
            vertex_id_by_stop_index,
            transport_graph,
            bus_data_by_edge_id
        );
        if (!bus->ring) {
            FillingInObjectsForTransportRoutes(
                bus->stop_indexs.rbegin(), bus->stop_indexs.rend(),
                index_bus,
                transport_catalogue,
                routing_settings,
                vertex_id_by_stop_index,
                transport_graph,
                bus_data_by_edge_id
            );
        }
    }
    return {move(transport_catalogue), move(picture), move(transport_graph),
            transport_router::TransportRoutes(
                routing_settings,
                move(bus_data_by_edge_id),
                move(stop_index_by_vertex_id),
                move(vertex_id_by_stop_index)
            )};
}
    
void HandleStopRequest(const TransportCatalogue& transport_catalogue, const json::Dict& stat_request, json::Builder::ArrayItemContext& response) {
    int id = stat_request.at("id"s).AsInt();
    string_view name = stat_request.at("name"s).AsString();
    const domain::Stop* stop = transport_catalogue.FindStop(name);

    if (!stop) {
        response = response.Value(
            json::Builder{}
            .StartDict()
            .Key("request_id"s).Value(id)
            .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build()
            .AsDict()
        );
    }
    else {
        json::Builder buses_names;
        auto buse_name = buses_names.StartArray();
        for (const size_t bus_index : stop->bus_indexs) {
            buse_name = buse_name.Value(transport_catalogue.FindBus(bus_index).name);
        }
        response = response.Value(
            json::Builder{}
            .StartDict()
            .Key("request_id"s).Value(id)
            .Key("buses"s).Value(buse_name.EndArray().Build().AsArray())
            .EndDict()
            .Build()
            .AsDict()
        );
    }
}

void HandleBusRequest(const TransportCatalogue& transport_catalogue, const json::Dict& stat_request, json::Builder::ArrayItemContext& response) {
    int id = stat_request.at("id"s).AsInt();
    string_view name = stat_request.at("name"s).AsString();
    const domain::Bus* bus = transport_catalogue.FindBus(name);

    if (!bus) {
        response = response.Value(
            json::Builder{}
            .StartDict()
            .Key("request_id"s).Value(id)
            .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build()
            .AsDict()
        );
    }
    else {
        response = response.Value(
            json::Builder{}
            .StartDict()
            .Key("request_id"s)
            .Value(id)
            .Key("curvature"s)
            .Value(bus->length / bus->ideal_length)
            .Key("route_length"s)
            .Value((double)bus->length)
            .Key("stop_count"s)
            .Value((int)bus->count_stops)
            .Key("unique_stop_count"s)
            .Value((int)bus->count_unique_stops)
            .EndDict()
            .Build()
            .AsDict()
        );
    }
}

void HandleMapRequest(const vector<unique_ptr<svg::Drawable>>& picture, const json::Dict& stat_request, json::Builder::ArrayItemContext& response) {
    int id = stat_request.at("id"s).AsInt();
    svg::Document doc;
    for (const auto& item : picture) {
        item->Draw(doc);
    }
    stringstream buf;
    doc.Render(buf);
    response = response.Value(
        json::Builder{}
        .StartDict()
        .Key("request_id"s).Value(id)
        .Key("map"s).Value(buf.str())
        .EndDict()
        .Build()
        .AsDict()
    );
}

void HandleRouteRequest(const TransportCatalogue& transport_catalogue, const graph::DirectedWeightedGraph<double>& transport_graph,
    const transport_router::TransportRoutes& transport_routes, const graph::Router<double>& router,
    const json::Dict& stat_request, json::Builder::ArrayItemContext& response)
{
    int id = stat_request.at("id"s).AsInt();
    string_view from = stat_request.at("from"s).AsString();
    string_view to = stat_request.at("to"s).AsString();
    const auto route = router.BuildRoute(
        transport_routes.GetVertexId(transport_catalogue.IndexStop(from)),
        transport_routes.GetVertexId(transport_catalogue.IndexStop(to)));

    if (!route) {
        response = response.Value(
            json::Builder{}
            .StartDict()
            .Key("request_id"s).Value(id)
            .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build()
            .AsDict()
        );
    }
    else {
        json::Builder items;
        auto item = items.StartArray();
        for (size_t i : route.value().edges) {
            const auto& edge = transport_graph.GetEdge(i);
            const auto bus_data = transport_routes.GetBusData(i);
            item = item
                .Value(
                    json::Builder{}
                    .StartDict()
                    .Key("type"s).Value("Wait"s)
                    .Key("stop_name"s).Value(transport_catalogue.FindStop(transport_routes.GetStopIndex(edge.from)).name)
                    .Key("time"s).Value(transport_routes.GetRoutingSettings().bus_wait_time)
                    .EndDict()
                    .Build()
                    .AsDict())
                .Value(
                    json::Builder{}
                    .StartDict()
                    .Key("type"s).Value("Bus"s)
                    .Key("bus"s).Value(transport_catalogue.FindBus(bus_data.index).name)
                    .Key("span_count"s).Value((int)bus_data.span_count)
                    .Key("time"s).Value(
                        edge.weight -
                        transport_routes.GetRoutingSettings().bus_wait_time)
                    .EndDict()
                    .Build()
                    .AsDict());
        }
        items = item.EndArray();
        response = response.Value(
            json::Builder{}
            .StartDict()
            .Key("request_id"s).Value(id)
            .Key("total_time"s).Value(route.value().weight)
            .Key("items"s).Value(items.Build().AsArray())
            .EndDict()
            .Build()
            .AsDict()
        );
    }
}

void HandleRequests(
    const TransportCatalogue& transport_catalogue,
    std::ostream& output,
    const json::Array& stat_requests,
    const vector<unique_ptr<svg::Drawable>>& picture,
    const graph::DirectedWeightedGraph<double>& transport_graph,
    const transport_router::TransportRoutes& transport_routes)
{
    const graph::Router router(transport_graph);
    json::Builder responses;
    auto response = responses.StartArray();
    for (const json::Node& node_stat_request : stat_requests) {
        const json::Dict& stat_request = node_stat_request.AsDict();
        string_view type = stat_request.at("type"s).AsString();

        if (type == "Stop"sv) {
            HandleStopRequest(transport_catalogue, stat_request, response);
        } else if (type == "Bus"sv) {
            HandleBusRequest(transport_catalogue, stat_request, response);
        } else if (type == "Map"sv) {
            HandleMapRequest(picture, stat_request, response);
        } else if (type == "Route"sv) {
            HandleRouteRequest(transport_catalogue, transport_graph, transport_routes, router, stat_request, response);
        }
    }
    json::Print(json::Document(response.EndArray().Build()), output);
}
    
unordered_map<string_view, const domain::Stop*> CreateStops(
    TransportCatalogue& transport_catalogue,
    const vector<const json::Dict*>& node_stops)
{
    unordered_map<string_view, const domain::Stop*> result;
    for (const json::Dict* node_stop : node_stops) {
        const domain::Stop& stop = transport_catalogue.AddStop(
            node_stop->at("name"s).AsString(),
            {node_stop->at("latitude"s).AsDouble(), node_stop->at("longitude"s).AsDouble()}
        );
        result.emplace(stop.name, &stop);
    }
    return result;
}
    
void SetDistanceBetweenStops(
    TransportCatalogue& transport_catalogue,
    const vector<const json::Dict*>& node_stops)
{
    for (const json::Dict* node_stop : node_stops) {
        string_view name_first_stop = node_stop->at("name"s).AsString();
        for (const auto& [name_second_stop, node_distance] :
            node_stop->at("road_distances"s).AsDict())
        {
            transport_catalogue.SetDistanceBetweenStops(
                name_first_stop,
                name_second_stop,
                node_distance.AsInt()
            );
        }
    }
}
    
tuple<map<string_view, const domain::Stop*>, map<string_view, const domain::Bus*>>
CreateBuses(
    TransportCatalogue& transport_catalogue,
    const vector<const json::Dict*>& node_buses,
    const unordered_map<string_view, const domain::Stop*>& stops)
{
    map<string_view, const domain::Stop*> result_stops;
    map<string_view, const domain::Bus*> result_buses;
    for (const json::Dict* node_bus : node_buses) {
        const json::Array& node_stops_names = node_bus->at("stops"s).AsArray();
        vector<string_view> stops_names;
        stops_names.reserve(node_stops_names.size());
        for (const json::Node& node_stop_name : node_stops_names) {
            const domain::Stop* stop = stops.at(node_stop_name.AsString());
            stops_names.push_back(stop->name);
            result_stops.emplace(stop->name, stop);
        }
        bool bus_empty = stops_names.empty();
        const domain::Bus& bus = transport_catalogue.AddBus(
            node_bus->at("name"s).AsString(),
            move(stops_names),
            node_bus->at("is_roundtrip"s).AsBool()
        );
        if (!bus_empty) {
            result_buses.emplace(bus.name, &bus);
        }
    }
    return {move(result_stops), move(result_buses)};
}
    
tuple<double, double, double, double> FindExtremeCoordinates(
    const map<string_view, const domain::Stop*> stops)
{
    if (stops.empty()) {
        return {0, 0, 0, 0};
    }
    double min_lon = stops.begin()->second->coordinates.lng;
    double max_lon = stops.begin()->second->coordinates.lng;
    double min_lat = stops.begin()->second->coordinates.lat;
    double max_lat = stops.begin()->second->coordinates.lat;
    for (const auto [_, stop] : stops) {
        min_lon = min(min_lon, stop->coordinates.lng);
        max_lon = max(max_lon, stop->coordinates.lng);
        min_lat = min(min_lat, stop->coordinates.lat);
        max_lat = max(max_lat, stop->coordinates.lat);
    }
    return {min_lon, max_lon, min_lat, max_lat};
}
    
map<string_view, svg::Point> ScaleStopPoints(
    const map<string_view, const domain::Stop*> stops,
    const map_renderer::ScalingPoints& scaling_points)
{
    map<string_view, svg::Point> result;
    for (const auto [name, stop] : stops) {
        result.emplace(name, scaling_points.ScalePoint(stop->coordinates));
    }
    return result;
}
    
vector<unique_ptr<svg::Drawable>> CreateMapObjects(
    TransportCatalogue& transport_catalogue,
    const map<string_view, svg::Point> stops_points,
    const map<string_view, const domain::Bus*> buses,
    const map_renderer::RenderSettings& render_settings)
{
    vector<unique_ptr<svg::Drawable>> result;
    CreatePolylinesOfRoutes(transport_catalogue, result, stops_points, buses, render_settings);
    CreateNamesOfRoutes(transport_catalogue, result, stops_points, buses, render_settings);
    CreateCirclesOfStops(result, stops_points, render_settings);
    CreateNamesOfStops(result, stops_points, render_settings);
    return result;
}
    
void CreatePolylinesOfRoutes(
    TransportCatalogue& transport_catalogue,
    vector<unique_ptr<svg::Drawable>>& picture,
    const map<string_view, svg::Point> stops_points,
    const map<string_view, const domain::Bus*> buses,
    const map_renderer::RenderSettings& render_settings)
{
    size_t index = 0;
    for (const auto [_, bus] : buses) {
        vector<svg::Point> points;
        points.reserve(bus->count_stops);
        for (const size_t stop_index : bus->stop_indexs) {
            points.push_back(stops_points.at(transport_catalogue.FindStop(stop_index).name));
        }
        if (!bus->ring) {
            size_t size = points.size();
            points.resize(points.capacity());
            reverse_copy(
                points.begin(),
                points.begin() + size - 1, points.begin() + size
            );
        }
        picture.push_back(make_unique<map_renderer::PolylineOfRoute>(
            move(points),
            render_settings.line_width,
            render_settings.color_palette[index]
        ));
        ++index;
        if (index >= render_settings.color_palette.size()) {
            index = 0;
        }
    }
}
    
void CreateNamesOfRoutes(
    TransportCatalogue& transport_catalogue,
    vector<unique_ptr<svg::Drawable>>& picture,
    const map<string_view, svg::Point> stops_points,
    const map<string_view, const domain::Bus*> buses,
    const map_renderer::RenderSettings& render_settings) 
{
    size_t index = 0;
    for (const auto [_, bus] : buses) {
        picture.push_back(make_unique<map_renderer::NameOfRoute>(
            bus->name,
            stops_points.at(transport_catalogue.FindStop(bus->stop_indexs.front()).name),
            render_settings.bus_label_font_size,
            render_settings.bus_label_offset,
            render_settings.underlayer_color,
            render_settings.underlayer_width,
            render_settings.color_palette[index]
        ));
        if (bus->stop_indexs.front() != bus->stop_indexs.back()) {
            picture.push_back(make_unique<map_renderer::NameOfRoute>(
                string(bus->name),
                stops_points.at(transport_catalogue.FindStop(bus->stop_indexs.back()).name),
                render_settings.bus_label_font_size,
                render_settings.bus_label_offset,
                render_settings.underlayer_color,
                render_settings.underlayer_width,
                render_settings.color_palette[index]
            ));
        }
        ++index;
        if (index >= render_settings.color_palette.size()) {
            index = 0;
        }
    }
}
    
void CreateCirclesOfStops(
    vector<unique_ptr<svg::Drawable>>& picture,
    const map<string_view, svg::Point> stops_points,
    const map_renderer::RenderSettings& render_settings)
{
    for (const auto [_, point] : stops_points) {
        picture.push_back(make_unique<map_renderer::CircleOfStop>(
            point,
            render_settings.stop_radius
        ));
    }
}
    
void CreateNamesOfStops(
    vector<unique_ptr<svg::Drawable>>& picture,
    const map<string_view, svg::Point> stops_points,
    const map_renderer::RenderSettings& render_settings)
{
    for (const auto [name, point] : stops_points) {
        picture.push_back(make_unique<map_renderer::NameOfStop>(
            string(name),
            point,
            render_settings.stop_label_font_size,
            render_settings.stop_label_offset,
            render_settings.underlayer_color,
            render_settings.underlayer_width
        ));
    }
}
    
} //namespace json_reader
    
} //namespace transport