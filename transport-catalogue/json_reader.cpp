#include "json_reader.h"
#include "json_builder.h"


#include <set>
#include <sstream>
#include <map>

using namespace std::literals;

void JsonReader::Read(std::istream& input) {
    const auto document = json::Load(input);
    const auto& commands = document.GetRoot().AsDict();


    const auto& coms_to_add = commands.find("base_requests"s);
    const auto& render_settings = commands.find("render_settings"s);
    const auto& coms_to_req = commands.find("stat_requests"s);
    const auto& routing_settings = commands.find("routing_settings"s);

    if (coms_to_add != commands.end()) {
        ParseCommandsToCatalogue(coms_to_add->second.AsArray());
    }

    if (render_settings != commands.end()) {
        ParseRenderSettings(render_settings->second.AsDict());
    }

    if (coms_to_req != commands.end()) {
        ParseCommandsToPrint(coms_to_req->second.AsArray());
    }

    if (routing_settings != commands.end()) {
        ParseRoutingSettings(routing_settings->second.AsDict());
    }

}

void JsonReader::ParseCommandsToCatalogue(const json::Array& commands) {

    for (const auto& com : commands) {
        const json::Dict& dict = com.AsDict();


        if (dict.at("type"s).AsString() == "Stop"s) {
            CommandStop result;
            result.name = dict.at("name"s).AsString();

            result.coords = geo::Coordinates{ dict.at("latitude"s).AsDouble(), dict.at("longitude"s).AsDouble() };

            for (const auto& dist : dict.at("road_distances"s).AsDict()) {
                result.distances.insert({ dist.first, dist.second.AsInt() });
            }
            stop_commands_.push_back(result);


        }
        else if (dict.at("type"s).AsString() == "Bus"s) {
            CommandBus result;
            result.name = dict.at("name"s).AsString();

            for (const auto& stop_name : dict.at("stops"s).AsArray()) {
                result.stop_names.push_back(stop_name.AsString());
            }

            result.is_roundtrip = dict.at("is_roundtrip"s).AsBool();
            bus_commands_.push_back(result);
        }
    }
}

void JsonReader::ParseRenderSettings(const json::Dict& elem) {
    commands_to_render_.height = elem.at("height"s).AsDouble();
    commands_to_render_.width = elem.at("width"s).AsDouble();
    commands_to_render_.padding = elem.at("padding"s).AsDouble();
    commands_to_render_.line_width = elem.at("line_width"s).AsDouble();
    commands_to_render_.stop_radius = elem.at("stop_radius"s).AsDouble();

    commands_to_render_.bus_label_font_size = elem.at("bus_label_font_size"s).AsInt();
    commands_to_render_.bus_label_offset = { elem.at("bus_label_offset"s).AsArray()[0].AsDouble(), elem.at("bus_label_offset"s).AsArray()[1].AsDouble() };

    commands_to_render_.stop_label_font_size = elem.at("stop_label_font_size"s).AsInt();
    commands_to_render_.stop_label_offset = { elem.at("stop_label_offset"s).AsArray()[0].AsDouble(), elem.at("stop_label_offset"s).AsArray()[1].AsDouble() };

    commands_to_render_.underlayer_color = GetColor(elem.at("underlayer_color"s));
    commands_to_render_.underlayer_width = elem.at("underlayer_width").AsDouble();

    for (const auto& color_node : elem.at("color_palette").AsArray()) {
        commands_to_render_.color_palette.push_back(GetColor(color_node));
    }
}

void JsonReader::ParseCommandsToPrint(const json::Array& com_node) {

    for (const auto& com : com_node) {
        const json::Dict& dict = com.AsDict();

        CommandToOut result;

        result.id = dict.at("id"s).AsInt();

        if (dict.at("type"s).AsString() == "Bus"s) {
            result.name = dict.at("name"s).AsString();
            result.type = OutType::BUS;
        }
        else if (dict.at("type"s).AsString() == "Stop"s) {
            result.name = dict.at("name"s).AsString();
            result.type = OutType::STOP;
        }
        else if (dict.at("type"s).AsString() == "Map") {
            result.type = OutType::MAP;
        }
        else if (dict.at("type"s).AsString() == "Route") {
            result.type = OutType::ROUTE;
            result.name = dict.at("from").AsString();
            result.to = dict.at("to").AsString();
        }

        commands_to_out_.push_back(result);
    }
}

void JsonReader::ParseRoutingSettings(const json::Dict& elem) {
    routing_settings_.bus_velocity = elem.at("bus_velocity"s).AsInt();
    routing_settings_.bus_wait_time = elem.at("bus_wait_time"s).AsInt();
}

void JsonReader::ApplyCatalogueCommands(TransportCatalogue& catalogue) const {

    ApplyStopCommands(catalogue);

    ApplyDistances(catalogue);

    ApplyBusCommands(catalogue);

}

void JsonReader::ApplyStopCommands(TransportCatalogue& catalogue) const {
    for (const auto& stop_com : stop_commands_) {
        catalogue.AddStop(stop_com.name, stop_com.coords);
    }
}

void JsonReader::ApplyDistances(TransportCatalogue& catalogue) const {
    for (const auto& stop_com : stop_commands_) {
        for (const auto& [name, dist] : stop_com.distances) {
            catalogue.AddDistance(stop_com.name, name, dist + 0);
        }
    }
}

void JsonReader::ApplyBusCommands(TransportCatalogue& catalogue) const {

    for (const auto& bus_com : bus_commands_) {
        std::vector<const Stop*> stops;

        if (!bus_com.stop_names.empty()) {
            for (const auto& stop : bus_com.stop_names) {
                stops.push_back(catalogue.FindStopByName(stop));
            }

            if (!bus_com.is_roundtrip) {
                for (auto iter = std::next(bus_com.stop_names.rbegin()); iter != bus_com.stop_names.rend(); ++iter) {
                    stops.push_back(catalogue.FindStopByName(*iter));
                }
            }
        }
        catalogue.AddBus(bus_com.name, stops, bus_com.is_roundtrip);
    }
}

void JsonReader::PrintRequests(TransportCatalogue& catalogue, std::ostream& output) {

    if (!commands_to_out_.empty()) {

        router::TransportRouter router(catalogue, routing_settings_.bus_wait_time, routing_settings_.bus_velocity);

        json::Builder builder;
        builder.StartArray();

        for (const auto& command : commands_to_out_) {

            if (command.type == OutType::STOP) {
                builder.Value(std::move(PrintStop(command, catalogue).GetValue()));
            }
            else if (command.type == OutType::BUS) {
                builder.Value(std::move(PrintBus(command, catalogue).GetValue()));
            }
            else if (command.type == OutType::MAP) {
                json::Node out_node;

                MapRenderer renderer;
                this->ApplyRendererSetting(renderer);
                std::stringstream map_out;
                renderer.RenderMap(catalogue, map_out);

                out_node = json::Builder{}
                    .StartDict()
                    .Key("map"s).Value(map_out.str())
                    .Key("request_id").Value(command.id)
                    .EndDict()
                    .Build();

                builder.Value(std::move(out_node.GetValue()));
            }
            else if (command.type == OutType::ROUTE) {

                const auto result = router.ComputeRoute(command.name, command.to);
                if (result.has_value()) {
                    builder.Value(std::move(BuildRouteNode(command, result.value()).GetValue()));
                }
                else {
                    builder.Value(std::move(BuildErrorNode(command).GetValue()));
                }
            }
        }


        builder.EndArray();
        json::Print(json::Document{ builder.Build() }, output);
    }
}

json::Node JsonReader::PrintStop(const CommandToOut& com, const TransportCatalogue& catalogue) const {
    json::Node out_node;

    json::Array buses;
    auto buses_res = catalogue.FindBusesByStop(com.name);
    if (buses_res.has_value()) {

        std::set<std::string> buses_names;
        for (const auto& bus : buses_res.value().get()) {
            buses_names.insert(bus->name);
        }
        for (const auto& bus_name : buses_names) {
            buses.push_back(bus_name);
        }

        out_node = json::Builder{}
            .StartDict()
            .Key("buses"s).Value(buses)
            .Key("request_id"s).Value(com.id)
            .EndDict()
            .Build();
        return out_node;
    }
    else {
        out_node = json::Builder{}
            .StartDict()
            .Key("error_message"s).Value("not found"s)
            .Key("request_id"s).Value(com.id)
            .EndDict()
            .Build();
        return out_node;
    }
}

json::Node JsonReader::PrintBus(const CommandToOut& com, const TransportCatalogue& catalogue) const {
    json::Node out_node;
    auto bus_data = catalogue.GetBusData(com.name);

    if (bus_data.name.empty()) {

        out_node = json::Builder{}
            .StartDict()
            .Key("request_id"s).Value(com.id)
            .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build();

        return out_node;
    }
    else {

        out_node = json::Builder{}
            .StartDict()
            .Key("curvature"s).Value(bus_data.curvature)
            .Key("request_id"s).Value(com.id)
            .Key("route_length"s).Value(bus_data.route_length)
            .Key("stop_count"s).Value(bus_data.number_of_stops)
            .Key("unique_stop_count"s).Value(bus_data.number_of_unique_stops)
            .EndDict()
            .Build();

        return out_node;
    }
}

svg::Color JsonReader::GetColor(const json::Node& elem) const {
    if (elem.IsString()) {
        return svg::Color(elem.AsString());
    }
    else {
        if (elem.AsArray().size() == 3) {
            return svg::Color(svg::Rgb(elem.AsArray()[0].AsInt(), (uint8_t)elem.AsArray()[1].AsInt(), (uint8_t)elem.AsArray()[2].AsInt()));
        }
        else {
            return svg::Color(svg::Rgba(elem.AsArray()[0].AsInt(), (uint8_t)elem.AsArray()[1].AsInt(), (uint8_t)elem.AsArray()[2].AsInt(), elem.AsArray()[3].AsDouble()));
        }
    }
}

void JsonReader::ApplyRendererSetting(MapRenderer& renderer) const {
    renderer.SetSettings(commands_to_render_);
}

json::Node JsonReader::BuildRouteNode(const CommandToOut& com, const std::vector<router::RouteElem>& route_data) const {

    json::Builder j_builder;

    double total_time = 0;

    j_builder.StartDict()
        .Key("items")
        .StartArray();

    for (const auto& elem : route_data) {

        j_builder.StartDict().Key("type");
        total_time += elem.time;

        if (elem.type == router::RouteElemType::GO) {
            j_builder.Value("Bus");
            j_builder.Key("bus").Value(std::string(elem.bus_name));
            j_builder.Key("span_count").Value(elem.span_count);
            j_builder.Key("time").Value(elem.time);
        }
        else if (elem.type == router::RouteElemType::WAIT) {
            j_builder.Value("Wait");
            j_builder.Key("stop_name").Value(elem.to->name);
            j_builder.Key("time").Value(elem.time);
        }
        j_builder.EndDict();
    }

    j_builder.EndArray();
    j_builder.Key("request_id").Value(com.id);
    j_builder.Key("total_time").Value(total_time);
    j_builder.EndDict();




    return j_builder.Build();
}

json::Node JsonReader::BuildErrorNode(const CommandToOut& com) const {
    return json::Builder().StartDict()
        .Key("request_id").Value(com.id)
        .Key("error_message").Value("not found")
        .EndDict().Build();
}