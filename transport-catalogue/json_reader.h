#pragma once

#include <map>
#include <memory>

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "router.h"
#include "transport_router.h"

using namespace catalogue;





struct CommandBus {
    std::string name;
    std::vector<std::string> stop_names;
    bool is_roundtrip;
};

struct CommandStop {
    std::string name;
    geo::Coordinates coords;
    std::unordered_map<std::string, int> distances;

};

enum OutType
{
    BUS,
    STOP,
    MAP,
    ROUTE
};

struct RoutingSettings {
    int bus_wait_time;
    int bus_velocity;
};

struct RoutingRequest {
    int id;
    std::string from;
    std::string to;
};

struct CommandToOut {
    int id;
    OutType type;
    std::string name;
    std::string to;
};

class JsonReader {
public:
    void Read(std::istream& input);
    void ApplyCatalogueCommands(TransportCatalogue& catalogue) const;
    void ApplyRendererSetting(MapRenderer& renderer) const;
    void PrintRequests(TransportCatalogue& catalogue, std::ostream& output);
private:
    json::Node PrintStop(const CommandToOut& com, const TransportCatalogue& catalogue) const;
    json::Node PrintBus(const CommandToOut& com, const TransportCatalogue& catalogue) const;

    json::Node BuildRouteNode(const CommandToOut& com, const std::vector<router::RouteElem>& route_data) const;

    void ApplyStopCommands(TransportCatalogue& catalogue) const;
    void ApplyBusCommands(TransportCatalogue& catalogue) const;
    void ParseCommandsToCatalogue(const json::Array& elem);
    void ParseRenderSettings(const json::Dict& elem);
    void ParseCommandsToPrint(const json::Array& com_node);
    void ParseRoutingSettings(const json::Dict& elem);

    svg::Color GetColor(const json::Node& elem) const;
    void ApplyDistances(TransportCatalogue& catalogue) const;

    json::Node BuildErrorNode(const CommandToOut& com) const;

    std::vector<CommandStop> stop_commands_;
    std::vector<CommandBus> bus_commands_;

    RenderSettings commands_to_render_;
    std::vector<CommandToOut> commands_to_out_;

    std::vector<RoutingRequest> routing_requests_;
    RoutingSettings routing_settings_;
};

