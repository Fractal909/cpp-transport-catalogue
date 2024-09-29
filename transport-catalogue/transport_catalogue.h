#pragma once

#include "geo.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace catalogue {

	struct Stop {
		std::string name;
		Coordinates coords;
	};

	struct Bus {
		std::string name;
		std::vector<const Stop*> stops;
	};

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
		const std::unordered_set<Bus*>* FindBusesByStop(const std::string_view name) const;

		BusData GetBusData(const std::string& name) const;
		int GetDistanceBetweenStops(const std::string_view stop1, const std::string_view stop2) const;

		void AddStop(const std::string& name, Coordinates coords);
		void AddBus(const std::string& name, const std::vector<const Stop*>& stops);
		void AddDistance(const std::string_view stop_name, const std::unordered_map<std::string_view, int>& stops_distances);

	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;

		std::unordered_map<std::string_view, Stop*> stops_ptrs_;
		std::unordered_map<std::string_view, Bus*> buses_ptrs_;

		std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopsPairHasher> distances_;

		std::unordered_map<const Stop*, std::unordered_set<Bus*>> buses_by_stop_;
	};

}