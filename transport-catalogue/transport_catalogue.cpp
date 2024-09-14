#include "transport_catalogue.h"


namespace catalogue {
    Bus* TransportCatalogue::FindBusByName(const std::string& name) {
        if (buses_ptrs_.contains(name)) {
            return buses_ptrs_.at(name);
        }
        else {
            return nullptr;
        }
    }

    const Bus* TransportCatalogue::FindBusByName(const std::string& name) const {
        if (buses_ptrs_.contains(name)) {
            return buses_ptrs_.at(name);
        }
        else {
            return nullptr;
        }
    }

    Stop* TransportCatalogue::FindStopByName(const std::string& name) {
        if (stops_ptrs_.contains(name)) {
            return stops_ptrs_.at(name);
        }
        else {
            return nullptr;
        }
    }

    const Stop* TransportCatalogue::FindStopByName(const std::string& name) const {
        if (stops_ptrs_.contains(name)) {
            return stops_ptrs_.at(name);
        }
        else {
            return nullptr;
        }
    }

    std::unordered_set<Bus*>* TransportCatalogue::FindBusesByStop(const std::string& name) {
        auto stop = FindStopByName(name);
        if (stop == nullptr) {
            return nullptr;
        }
        return &(buses_by_stop_.at(stop));
    }

    const std::unordered_set<Bus*>* TransportCatalogue::FindBusesByStop(const std::string& name) const {
        auto stop = FindStopByName(name);
        if (stop == nullptr) {
            return nullptr;
        }
        return &(buses_by_stop_.at(stop));
    }

    BusData TransportCatalogue::GetBusData(const std::string& name) const {
        BusData result;
        auto bus = FindBusByName(name);
        if (bus == nullptr) {
            return result;
        }

        result.name = bus->name;
        result.number_of_stops = bus->stops.size();

        std::unordered_set<const Stop*> x;
        for (const auto& stop : bus->stops) {
            x.insert(stop);
        }
        result.number_of_unique_stops = x.size();

        double length = 0;
        for (auto iter = bus->stops.begin(); iter != std::prev(bus->stops.end()); ++iter) {
            length += ComputeDistance((*iter)->coords, (*std::next(iter))->coords);
        }
        result.route_length = length;

        return result;
    }

    void TransportCatalogue::AddStop(std::string name, Coordinates coords) {
        stops_.push_back({ std::move(name), std::move(coords) });
        stops_ptrs_.insert({ stops_.back().name, &stops_.back() });

        buses_by_stop_.insert({ &stops_.back(), std::unordered_set<Bus*>() });
    }

    void TransportCatalogue::AddBus(std::string name, std::deque<const Stop*> stops) {
        buses_.push_back({ std::move(name), std::move(stops) });
        buses_ptrs_.insert({ buses_.back().name, &buses_.back() });

        for (auto stop : buses_.back().stops) {
            buses_by_stop_.at(stop).insert(&buses_.back());
        }
    }

    std::deque<Stop>& TransportCatalogue::GetStops() {
        return stops_;
    }

    std::deque<Bus>& TransportCatalogue::GetBuses() {
        return buses_;
    }
}