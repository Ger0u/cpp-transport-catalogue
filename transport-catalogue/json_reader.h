#pragma once

#include "transport_catalogue.h"
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
    
svg::Point ArrayToPoint(const json::Array& arr);
    
svg::Color NodeToColor(const json::Node& node);
    
std::vector<svg::Color> ArrayToVectorColor(const json::Array& arr);
    
void CreateAndRequestsTransportCatalogue(std::istream& input, std::ostream& output);
    
std::tuple<TransportCatalogue, std::vector<std::unique_ptr<svg::Drawable>>>
CreateTransportCatalogue(const json::Array& base_requests,
                         const map_renderer::RenderSettings& render_settings);
    
void RequestsTransportCatalogue(
    const TransportCatalogue& transport_catalogue,
    std::ostream& output,
    const json::Array& stat_requests,
    const std::vector<std::unique_ptr<svg::Drawable>>& picture);
    
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
    const std::map<std::string_view, svg::Point> stops_points,
    const std::map<std::string_view, const domain::Bus*> buses,
    const map_renderer::RenderSettings& render_settings);
    
void CreatePolylinesOfRoutes(
    std::vector<std::unique_ptr<svg::Drawable>>& picture,
    const std::map<std::string_view, svg::Point> stops_points,
    const std::map<std::string_view, const domain::Bus*> buses,
    const map_renderer::RenderSettings& render_settings);
   
void CreateNamesOfRoutes(
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
    
} //namespace json_reader
    
} //namespace transport