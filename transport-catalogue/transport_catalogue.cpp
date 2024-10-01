#include "transport_catalogue.h"


namespace catalogue {

    const Bus* TransportCatalogue::FindBusByName(const std::string_view name) const {
        auto res = buses_ptrs_.find(name);
        if (res != buses_ptrs_.end()) {
            return (*res).second;
        }
        else {
            return nullptr;
        }
    }

    const Stop* TransportCatalogue::FindStopByName(const std::string_view name) const {
        auto res = stops_ptrs_.find(name);
        if (res != stops_ptrs_.end()) {
            return (*res).second;
        }
        else {
            return nullptr;
        }
    }

    std::optional<std::reference_wrapper<const std::unordered_set<Bus*>>> TransportCatalogue::FindBusesByStop(const std::string_view name) const {
        auto stop = FindStopByName(name);
        if (stop == nullptr) {
            return std::nullopt;
        }
        return std::ref(buses_by_stop_.at(stop));
    }

    BusData TransportCatalogue::GetBusData(const std::string& name) const {
        BusData result;
        auto bus = FindBusByName(name);
        if (bus == nullptr) {
            return result;
        }

        result.name = bus->name;
        result.number_of_stops = bus->stops.size();

        std::unordered_set<const Stop*> unique_stops_set;
        for (const auto& stop : bus->stops) {
            unique_stops_set.insert(stop);
        }
        result.number_of_unique_stops = unique_stops_set.size();

        int length = 0;
        for (auto iter = bus->stops.begin(); iter != std::prev(bus->stops.end()); ++iter) {
            auto it = distances_.find({ *iter, *(std::next(iter)) });
            if (it != distances_.end()) {
                length += (*it).second;
            }
            else {
                it = distances_.find({ *(std::next(iter)), *iter });
                if (it != distances_.end()) {
                    length += (*it).second;
                }
            }
        }
        result.route_length = length;

        double geo_length = 0;
        for (auto iter = bus->stops.begin(); iter != std::prev(bus->stops.end()); ++iter) {
            geo_length += ComputeDistance((*iter)->coords, (*std::next(iter))->coords);
        }
        result.curvature = length / geo_length;

        return result;
    }

    void TransportCatalogue::AddStop(const std::string& name, Coordinates coords) {
        stops_.push_back({ std::move(name), std::move(coords) });
        stops_ptrs_.insert({ stops_.back().name, &stops_.back() });

        buses_by_stop_.insert({ &stops_.back(), std::unordered_set<Bus*>() });
    }

    void TransportCatalogue::AddBus(const std::string& name, const std::vector<const Stop*>& stops) {
        buses_.push_back({ std::move(name), std::move(stops) });
        buses_ptrs_.insert({ buses_.back().name, &buses_.back() });

        for (auto stop : buses_.back().stops) {
            buses_by_stop_.at(stop).insert(&buses_.back());
        }
    }

    void TransportCatalogue::AddDistance(const std::string_view from_stop, const std::string_view to_stop, int distance) {

        const Stop* from_stop_ptr = FindStopByName(from_stop);
        const Stop* to_stop_ptr = FindStopByName(to_stop);

        distances_.insert({ {from_stop_ptr, to_stop_ptr}, distance });
    }

    int TransportCatalogue::GetDistanceBetweenStops(const std::string_view from, const std::string_view to) const {

        auto iter = distances_.find({ FindStopByName(from), FindStopByName(to) });
        if (iter != distances_.end()) {
            return (*iter).second;
        }
        else {
            return -1;
        }
    }
}