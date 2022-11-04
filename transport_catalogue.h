#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <string_view>
#include <deque>
#include <functional>
#include <set>
#include <stdexcept>

#include "geo.h"
#include "domain.h"

namespace transport_routine::catalogue {

class TransportCatalogue {
public:
    TransportCatalogue() = default;
    TransportCatalogue(TransportCatalogue&&) = default;

    TransportCatalogue& operator=(TransportCatalogue&&) = default;

    TransportCatalogue(const TransportCatalogue& other) = delete;
    TransportCatalogue& operator=(const TransportCatalogue& other) = delete;

    void AddStop(const domain::Stop& stop);
    void AddRoute(const domain::Bus& route);

    void SetDistance(const domain::Stop* from, const domain::Stop* dest, int distance);
    domain::Distances GetDistance(const domain::Stop* from, const domain::Stop* dest) const;

    const domain::Stop* FindStop(std::string_view stop_name) const;
    const domain::Bus* FindRoute(std::string_view route_name) const;
    const domain::RouteStats* FindRouteStats(std::string_view route_name) const;
    const std::set<std::string_view>* FindStopUniqueBuses(std::string_view stop_name) const;

    const std::deque<domain::Bus>* GetAllRoutes() const;
    const std::deque<domain::Stop>* GetAllStops() const;
    const std::unordered_map<const domain::Stop*, int>* GetStopDistances(const domain::Stop* stop) const;

private:
    std::deque<domain::Stop> stops_;
    std::deque<domain::Bus> routes_;
    std::unordered_map<std::string_view, const domain::Stop*> name_to_stop_index_;
    std::unordered_map<std::string_view, const domain::Bus*> name_to_route_index_;
    std::unordered_map<std::string_view, domain::RouteStats> name_to_route_stats_;
    std::unordered_map<std::string_view, std::set<std::string_view>> name_to_stop_unique_buses_;
    std::unordered_map<const domain::Stop*, std::unordered_map<const domain::Stop*, int>> stop_to_stop_real_distances_;

    struct TotalDistanceCuravature {
        double total_distance = .0;
        double curvature = .0;
    };

    TotalDistanceCuravature ComputeTotalDistanceCurvature(const domain::Bus* route) const;
    domain::RouteStats ComputeRouteStats(const domain::Bus* route) const;
};

} // namespace transport_routine::catalogue