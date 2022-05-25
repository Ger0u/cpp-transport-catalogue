#pragma once

#include "transport_catalogue.h"
#include "transport_router.h"
#include "graph.h"
#include "router.h"
#include "map_renderer.h"
#include "json.h"
#include "svg.h"
#include <iostream>
#include <string_view>
#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>
#include <memory>

namespace transport {
    
namespace json_reader {
    
map_renderer::RenderSettings CreateRenderSettings(const json::Dict& render_settings);
    
transport_router::RoutingSettings CreateRoutingSettings(const json::Dict& routing_settings);
    
svg::Point ArrayToPoint(const json::Array& arr);
    
svg::Color NodeToColor(const json::Node& node);
    
std::vector<svg::Color> ArrayToVectorColor(const json::Array& arr);
    
void CreateTransportCatalogueAndHandleRequests(std::istream& input, std::ostream& output);
    
std::tuple<TransportCatalogue,
           std::unordered_map<std::string_view, const domain::Stop*>,
           std::map<std::string_view, const domain::Stop*>,
           std::map<std::string_view, const domain::Bus*>>
CreateStopsAndBuses(const json::Array& base_requests);
    
std::tuple<TransportCatalogue, std::vector<std::unique_ptr<svg::Drawable>>,
           graph::DirectedWeightedGraph<double>,
           transport_router::TransportRoutes>
CreateTransportCatalogue(const json::Array& base_requests,
                         const map_renderer::RenderSettings& render_settings,
                         const transport_router::RoutingSettings& routing_settings);
    
void HandleRequests(
    const TransportCatalogue& transport_catalogue,
    std::ostream& output,
    const json::Array& stat_requests,
    const std::vector<std::unique_ptr<svg::Drawable>>& picture,
    const graph::DirectedWeightedGraph<double>& transport_graph,
    const transport_router::TransportRoutes& transport_routes);
    
std::unordered_map<std::string_view, const domain::Stop*> CreateStops(
    TransportCatalogue& transport_catalogue,
    const std::vector<const json::Dict*>& stops);
   
void SetDistanceBetweenStops(
    TransportCatalogue& transport_catalogue,
    const std::vector<const json::Dict*>& stops);
    
std::tuple<std::map<std::string_view, const domain::Stop*>,
           std::map<std::string_view, const domain::Bus*>>
CreateBuses(
    TransportCatalogue& transport_catalogue,
    const std::vector<const json::Dict*>& buses,
    const std::unordered_map<std::string_view, const domain::Stop*>& stops);
    
std::tuple<double, double, double, double> FindExtremeCoordinates(
    const std::map<std::string_view, const domain::Stop*> stops);
    
std::map<std::string_view, svg::Point> ScaleStopPoints(
    const std::map<std::string_view, const domain::Stop*> stops,
    const map_renderer::ScalingPoints& scaling_points);
    
std::vector<std::unique_ptr<svg::Drawable>> CreateMapObjects(
    TransportCatalogue& transport_catalogue,
    const std::map<std::string_view, svg::Point> stops_points,
    const std::map<std::string_view, const domain::Bus*> buses,
    const map_renderer::RenderSettings& render_settings);
    
void CreatePolylinesOfRoutes(
    TransportCatalogue& transport_catalogue,
    std::vector<std::unique_ptr<svg::Drawable>>& picture,
    const std::map<std::string_view, svg::Point> stops_points,
    const std::map<std::string_view, const domain::Bus*> buses,
    const map_renderer::RenderSettings& render_settings);
   
void CreateNamesOfRoutes(
    TransportCatalogue& transport_catalogue,
    std::vector<std::unique_ptr<svg::Drawable>>& picture,
    const std::map<std::string_view, svg::Point> stops_points,
    const std::map<std::string_view, const domain::Bus*> buses,
    const map_renderer::RenderSettings& render_settings);
    
void CreateCirclesOfStops(
    std::vector<std::unique_ptr<svg::Drawable>>& picture,
    const std::map<std::string_view, svg::Point> stops_points,
    const map_renderer::RenderSettings& render_settings);
    
void CreateNamesOfStops(
    std::vector<std::unique_ptr<svg::Drawable>>& picture,
    const std::map<std::string_view, svg::Point> stops_points,
    const map_renderer::RenderSettings& render_settings);
    
template<typename InputIt>
void FillingInObjectsForTransportRoutes(
    InputIt first, InputIt last,
    size_t bus_index,
    TransportCatalogue& transport_catalogue,
    const transport_router::RoutingSettings& routing_settings,
    const std::unordered_map<size_t, graph::VertexId>& vertex_id_by_stop_name,
    graph::DirectedWeightedGraph<double>& transport_graph,
    std::vector<transport_router::TransportRoutes::BusData>& bus_data_by_edge_id)
{
    for (auto stop1 = first; stop1 != last - 1; ++stop1) {
        double time = routing_settings.bus_wait_time;
        for (auto stop2 = stop1 + 1; stop2 != last && *stop1 != *stop2; ++stop2) {
            auto it = transport_catalogue.FindStop(*(stop2 - 1)).distance_to_stops.find(*stop2);
            if (it == transport_catalogue.FindStop(*(stop2 - 1)).distance_to_stops.end()) {
                it = transport_catalogue.FindStop(*stop2).distance_to_stops.find(*(stop2 - 1));
            }
            time += it->second / (routing_settings.bus_velocity * 1000.0 / 60);
            transport_graph.AddEdge({
                vertex_id_by_stop_name.at(*stop1),
                vertex_id_by_stop_name.at(*stop2),
                time
            });
            bus_data_by_edge_id.push_back({bus_index, (size_t)(stop2 - stop1)});
        }
    }
}
    
} //namespace json_reader
    
} //namespace transport