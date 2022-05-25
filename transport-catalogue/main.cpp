#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>
#include <memory>
#include <utility>
#include "serialization.h"
#include "json_reader.h"
#include "svg.h"
#include "map_renderer.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        const auto document = json::Load(std::cin);
        const auto& requests = document.GetRoot().AsDict();
        map_renderer::RenderSettings render_settings = transport::json_reader::CreateRenderSettings(requests.at("render_settings"s).AsDict());
        transport_router::RoutingSettings routing_settings = transport::json_reader::CreateRoutingSettings(requests.at("routing_settings"s).AsDict());
        auto [transport_catalogue, picture, transport_graph, transport_routes] = transport::json_reader::CreateTransportCatalogue(
            requests.at("base_requests"s).AsArray(), render_settings, routing_settings);
        std::ofstream ofs(requests.at("serialization_settings"s).AsDict().at("file"s).AsString(), std::ios::binary);
        serialization::Serialize(transport_catalogue, { std::move(picture) }, transport_graph, transport_routes, ofs);
    }
    else if (mode == "process_requests"sv) {
        const auto document = json::Load(std::cin);
        const auto& requests = document.GetRoot().AsDict();

        std::ifstream ifs(requests.at("serialization_settings"s).AsDict().at("file"s).AsString(), std::ios::binary);
        auto [transport_catalogue, picture, transport_graph, transport_routes] = serialization::Deserialize(ifs);
        transport::json_reader::HandleRequests(
            transport_catalogue,
            std::cout,
            requests.at("stat_requests"s).AsArray(),
            picture,
            transport_graph,
            transport_routes
        );
    }
    else {
        PrintUsage();
        return 1;
    }
}