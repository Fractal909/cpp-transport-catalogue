#pragma once

#include <memory>

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

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
    MAP
};

struct CommandToOut {
    int id;
    OutType type;
    std::string name;
};

class JsonReader {
public:
    void Read(std::istream& input);
    void ApplyCatalogueCommands(TransportCatalogue& catalogue) const;
    void ApplyRendererSetting(MapRenderer& renderer) const;
    void Print(const TransportCatalogue& catalogue, std::ostream& output) const;
private:
    void PrintStop(const CommandToOut& com, const TransportCatalogue& catalogue, std::ostream& out) const;
    void PrintBus(const CommandToOut& com, const TransportCatalogue& catalogue, std::ostream& out) const;
    void ApplyStopCommands(TransportCatalogue& catalogue) const;
    void ApplyBusCommands(TransportCatalogue& catalogue) const;
    void ParseCommandsToCatalogue(const json::Array& elem);
    void ParseRenderSettings(const json::Dict& elem);
    void ParseCommandsToPrint(const json::Array& com_node);
    svg::Color GetColor(const json::Node& elem) const;
    void ApplyDistances(TransportCatalogue& catalogue) const;

    std::vector<CommandStop> stop_commands_;
    std::vector<CommandBus> bus_commands_;

    RenderSettings commands_to_render_;
    std::vector<CommandToOut> commands_to_out_;
};

