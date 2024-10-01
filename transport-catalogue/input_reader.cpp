#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <optional>

#include <deque>

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return { nan, nan };
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return { lat, lng };
}

std::unordered_map<std::string_view, int> ParseDistances(std::string_view str) {

    std::unordered_map<std::string_view, int> result;

    auto start_pos = str.find_first_of(',');


    start_pos = str.find_first_of(',', ++start_pos);

    while (start_pos != str.npos) {
        start_pos = str.find_first_not_of(' ', ++start_pos);
        auto end_pos = str.find_first_of('m', start_pos);

        int distance = std::stoi(std::string(str.substr(start_pos, end_pos - start_pos)));

        start_pos = end_pos = str.find_first_of("to", end_pos) + 2;
        start_pos = str.find_first_not_of(' ', end_pos);
        end_pos = str.find_first_of(',', start_pos);
        if (end_pos == str.npos) {
            end_pos = str.size();
        }

        std::string_view stop_name = str.substr(start_pos, end_pos - start_pos);

        result.insert({ stop_name, distance });

        start_pos = str.find_first_of(',', ++start_pos);
    }

    return result;
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return { std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1)) };
}



void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] catalogue::TransportCatalogue& catalogue) const {
    // Реализуйте метод самостоятельно
    std::vector<const CommandDescription*> buses_commands;
    std::vector<const CommandDescription*> stops_commands;

    for (const auto& command : commands_) {
        if (command.command == "Stop") {
            ApplyCommand(command, catalogue);
            stops_commands.push_back(&command);
        }
        else {
            buses_commands.push_back(&command);
        }
    }

    for (const auto& command_ptr : buses_commands) {
        ApplyCommand(*command_ptr, catalogue);
    }

    for (const auto& command_ptr : stops_commands) {
        ApplyStopDistances(*command_ptr, catalogue);
    }
}

void InputReader::ApplyCommand(const CommandDescription& command, catalogue::TransportCatalogue& catalogue) const {
    using namespace catalogue;
    if (command.command == "Stop") {
        auto coords = ParseCoordinates(command.description);
        catalogue.AddStop(command.id, coords);
    }

    else if (command.command == "Bus") {
        std::vector<const Stop*> route_stops;
        auto route = ParseRoute(command.description);

        for (const auto& stop : route) {
            auto finded_stop = catalogue.FindStopByName(std::string(stop));
            route_stops.push_back(finded_stop);
        }
        catalogue.AddBus(command.id, route_stops);
    }
}

void InputReader::ApplyStopDistances(const CommandDescription& command, catalogue::TransportCatalogue& catalogue) const {
    std::unordered_map<std::string_view, int> distances = ParseDistances(command.description);
    if (!distances.empty()) {
        for (const auto& distance : distances) {
            catalogue.AddDistance(command.id, distance.first, distance.second);
        }
    }
}
