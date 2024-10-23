#pragma once

#include "geo.h"

#include <string>
#include <vector>

struct Stop {
	std::string name;
	geo::Coordinates coords;
};

struct Bus {
	std::string name;
	std::vector<const Stop*> stops;
	bool is_roundtrip;
};