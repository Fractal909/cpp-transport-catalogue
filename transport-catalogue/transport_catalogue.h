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
		double route_length;
	};

	class TransportCatalogue {
		// Реализуйте класс самостоятельно
	public:
		const Bus* FindBusByName(const std::string_view name) const;
		const Stop* FindStopByName(const std::string_view name) const;
		const std::unordered_set<Bus*>* FindBusesByStop(const std::string_view name) const;

		BusData GetBusData(const std::string& name) const;

		void AddStop(const std::string& name, Coordinates coords);
		void AddBus(const std::string& name, const std::vector<const Stop*>& stops);

	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;

		std::unordered_map<std::string_view, Stop*> stops_ptrs_;
		std::unordered_map<std::string_view, Bus*> buses_ptrs_;

		std::unordered_map<const Stop*, std::unordered_set<Bus*>> buses_by_stop_;
	};

}