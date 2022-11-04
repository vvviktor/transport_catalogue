#include "transport_catalogue.h"

using namespace std;

namespace transport_routine::catalogue {

void TransportCatalogue::AddStop(const domain::Stop& stop) {
    const domain::Stop* new_stop_ptr = &stops_.emplace_back(stop);
    name_to_stop_unique_buses_.insert({new_stop_ptr->name, {}});
    name_to_stop_index_.insert({new_stop_ptr->name, new_stop_ptr});
}

void TransportCatalogue::AddRoute(const domain::Bus& route) {
    const domain::Bus* new_bus_ptr = &routes_.emplace_back(route);
    name_to_route_stats_.insert({new_bus_ptr->name, ComputeRouteStats(new_bus_ptr)});
    for (const domain::Stop* stop_ptr: new_bus_ptr->unique_stops) {
        name_to_stop_unique_buses_.at(stop_ptr->name).insert(new_bus_ptr->name);
    }
    name_to_route_index_.insert({new_bus_ptr->name, new_bus_ptr});
}

void TransportCatalogue::SetDistance(const domain::Stop* from, const domain::Stop* dest, int distance) {
    if (!from || !dest) {
        throw invalid_argument("Cannot set distance. Starting stop or destination stop not found"s);
    }
    stop_to_stop_real_distances_[from][dest] = distance;
}

domain::Distances TransportCatalogue::GetDistance(const domain::Stop* from, const domain::Stop* dest) const {
    int ret = 0;

    if ((stop_to_stop_real_distances_.count(from) != 0) && (stop_to_stop_real_distances_.at(from).count(dest) != 0)) {
        ret = stop_to_stop_real_distances_.at(from).at(dest);
    } else if ((stop_to_stop_real_distances_.count(dest) != 0) &&
               (stop_to_stop_real_distances_.at(dest).count(from) != 0)) {
        ret = stop_to_stop_real_distances_.at(dest).at(from);
    }

    return {ret, geo::ComputeDistance({from->point}, {dest->point})};
}

const domain::Stop* TransportCatalogue::FindStop(string_view stop_name) const {
    return (name_to_stop_index_.count(stop_name) != 0) ? name_to_stop_index_.at(stop_name) : nullptr;
}

const domain::Bus* TransportCatalogue::FindRoute(string_view route_name) const {
    return (name_to_route_index_.count(route_name) != 0) ? name_to_route_index_.at(route_name) : nullptr;
}

const domain::RouteStats* TransportCatalogue::FindRouteStats(std::string_view route_name) const {
    return (name_to_route_stats_.count(route_name) != 0) ? &name_to_route_stats_.at(route_name) : nullptr;
}

const std::set<std::string_view>* TransportCatalogue::FindStopUniqueBuses(std::string_view stop_name) const {
    return (name_to_stop_unique_buses_.count(stop_name) != 0) ? &name_to_stop_unique_buses_.at(stop_name) : nullptr;
}

const std::deque<domain::Bus>* TransportCatalogue::GetAllRoutes() const {
    return !routes_.empty() ? &routes_ : nullptr;
}
const std::deque<domain::Stop>* TransportCatalogue::GetAllStops() const {
    return !stops_.empty() ? &stops_ : nullptr;
}

const std::unordered_map<const domain::Stop*, int>* TransportCatalogue::GetStopDistances(const domain::Stop* stop) const {
    return stop_to_stop_real_distances_.count(stop) != 0 ? &stop_to_stop_real_distances_.at(stop) : nullptr;
}

TransportCatalogue::TotalDistanceCuravature
TransportCatalogue::ComputeTotalDistanceCurvature(const domain::Bus* route) const {
    double total_distance = .0;
    double total_computed_distance = .0;
    double curvature = .0;
    deque<const domain::Stop*> route_for_processing(route->route.begin(), route->route.end());

    if (!route->is_roundtrip) {
        route_for_processing.insert(route_for_processing.end(), route->route.rbegin(), route->route.rend());
    }

    if (!route_for_processing.empty()) {
        bool is_first = true;
        const domain::Stop* from = route_for_processing[0];
        for (const domain::Stop* stop: route_for_processing) {
            if (is_first) {
                is_first = false;
                continue;
            }
            const domain::Distances distances = GetDistance(from, stop);
            total_distance += distances.path_distance;
            total_computed_distance += distances.geo_distance;
            from = stop;
        }
        curvature = (total_computed_distance != 0) ? total_distance / total_computed_distance : 0;
    }

    return {total_distance, curvature};
}

domain::RouteStats TransportCatalogue::ComputeRouteStats(const domain::Bus* route) const {
    domain::RouteStats ret;
    int total_stops = static_cast<int>(route->route.size());
    if (!route->is_roundtrip) {
        total_stops = total_stops * 2 - 1;
    }
    const TransportCatalogue::TotalDistanceCuravature dist_curv = ComputeTotalDistanceCurvature(route);
    return {dist_curv.total_distance, total_stops, static_cast<int>(route->unique_stops.size()), dist_curv.curvature};
}

} // namespace transport_routine::catalogue
