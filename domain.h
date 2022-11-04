#pragma once

#include <string>
#include <unordered_set>
#include <deque>

#include "geo.h"

namespace transport_routine::domain {

struct Stop {
    std::string name;
    geo::Coordinates point;
};

struct StopPtrComparatorLess {
    bool operator()(const Stop* lhs, const Stop* rhs) const;
};

struct Bus {
    std::string name;
    std::unordered_set<const Stop*> unique_stops;
    std::deque<const Stop*> route;
    bool is_roundtrip = false;
};

struct BusPtrComparatorLess {
    bool operator()(const Bus* lhs, const Bus* rhs) const;
};

struct RouteStats {
    double total_distance = .0;
    int total_stops = 0;
    int unique_stops = 0;
    double curvature = .0;
};

struct Distances {
    int path_distance = 0;
    double geo_distance = .0;
};

} // namespace domain