#include "stat_reader.h"

#include <iomanip>
#include <iostream>
#include <set>

using namespace std::literals;
using namespace catalogue;

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output) {
    // Реализуйте самостоятельно
    auto pos = request.find_first_of(' ');
    std::string_view command = request.substr(0, pos);
    std::string_view name = request.substr(pos + 1, request.find_last_not_of(' '));

    if (command == "Bus") {
        PrintBus(tansport_catalogue, name, output);
    }

    else if (command == "Stop") {
        PrintStop(tansport_catalogue, name, output);
    }
}

void PrintBus(const TransportCatalogue& tansport_catalogue, std::string_view name, std::ostream& output) {

    BusData data = tansport_catalogue.GetBusData(std::string(name));

    output << "Bus "s;

    if (data.name.empty()) {
        output << name << ": not found"s << '\n';
    }
    else {
        output << data.name << ": "s
            << data.number_of_stops << " stops on route, "s
            << data.number_of_unique_stops << " unique stops, "s
            << std::setprecision(6) << data.route_length << " route length"s << "\n"s;
    }
}

void PrintStop(const TransportCatalogue& tansport_catalogue, std::string_view name, std::ostream& output) {

    const std::unordered_set<Bus*>* stop_data = tansport_catalogue.FindBusesByStop(std::string(name));

    output << "Stop "s << name;

    if (stop_data == nullptr) {
        output << ": not found"s << '\n';
    }
    else if (stop_data->empty()) {
        output << ": no buses"s << '\n';
    }
    else {
        std::set<std::string_view> res;
        for (auto bus : *stop_data) {
            res.insert(bus->name);
        }
        output << ": buses ";
        for (auto iter = res.cbegin(); iter != res.cend(); ++iter) {
            output << *iter << ' ';
        }
        output << '\n';
    }
}