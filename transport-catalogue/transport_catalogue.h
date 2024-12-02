#pragma once

#include "geo.h"
#include "domain.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <optional>
#include <functional>

namespace catalogue {

	struct BusData {
		std::string_view name;
		int number_of_stops;
		int number_of_unique_stops;
		int route_length;
		double curvature;
	};

	class StopsPairHasher {
	public:
		size_t operator()(std::pair<const Stop*, const Stop*> stops_pair) const {
			return (std::hash<const void*>{}(stops_pair.first) ^ (std::hash<const void*>{}(stops_pair.second)) >> 1);
		}
	};

	class TransportCatalogue {
		// Реализуйте класс самостоятельно
	public:
		const Bus* FindBusByName(const std::string_view name) const;
		const Stop* FindStopByName(const std::string_view name) const;
		std::optional<std::reference_wrapper<const std::unordered_set<Bus*>>> FindBusesByStop(const std::string_view name) const;

		BusData GetBusData(const std::string& name) const;
		std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopsPairHasher> GetAllDistances() const;
		int GetDistanceBetweenStops(const std::string_view from, const std::string_view to) const;
		int GetDistanceBetweenStops(const Stop* from, const Stop* to) const;

		void AddStop(const std::string& name, geo::Coordinates coords);
		void AddBus(const std::string& name, const std::vector<const Stop*>& stops, bool is_roundtrip);
		void AddDistance(const std::string_view from_stop, const std::string_view to_stop, int distance);

		std::vector<const Bus*> GetBuses() const;
		std::vector<const Stop*> GetStops() const;

	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;

		std::unordered_map<std::string_view, Stop*> stops_ptrs_;
		std::unordered_map<std::string_view, Bus*> buses_ptrs_;

		std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopsPairHasher> distances_;

		std::unordered_map<const Stop*, std::unordered_set<Bus*>> buses_by_stop_;
	};

}