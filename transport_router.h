#pragma once

#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport_router {

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = .0;
};

enum class RouteItemType { WAIT, BUS };

struct RouteItem {
    RouteItemType type;
    std::string name;
    double time = .0;
    size_t span_count = 0;
};

struct Route {
    std::vector<RouteItem> items;
    double total_time = .0;
    bool IsSuccess = false;

    operator bool() const { return IsSuccess; }
};

struct Vertexes {
    graph::VertexId terminal = 0;
    graph::VertexId on_route = 0;
};

struct RouterEssentials {
    std::vector<std::pair<std::string, Vertexes>> stop_to_vertex_index;
    std::unordered_map<graph::EdgeId, RouteItem> edge_to_route_item_index;
};

class TransportRouter {
   public:
    friend class TransportRouterBuilder;

    class TransportRouterBuilder {
       public:
        TransportRouterBuilder(const transport_routine::catalogue::TransportCatalogue& catalogue,
                               RoutingSettings settings);

        std::unique_ptr<TransportRouter> Build();

        const TransportRouterBuilder& CreateGraph();

       private:
        const transport_routine::catalogue::TransportCatalogue& catalogue_;
        graph::DirectedWeightedGraph<double> graph_;
        RoutingSettings settings_;
        std::unordered_map<const transport_routine::domain::Stop*, Vertexes> stop_to_vertex_index_;
        std::unordered_map<graph::EdgeId, RouteItem> edge_to_route_item_index_;

        void AddStopsToGraph();

        void AddBusesToGraph();

        template <typename InputIt>
        void AddOneWayRouteToGraph(InputIt first, InputIt last, const transport_routine::domain::Bus& bus);

        double ComputeTimeMinutes(double distance_m) const;
    };

    TransportRouter(const transport_routine::catalogue::TransportCatalogue& catalogue,
                    graph::DirectedWeightedGraph<double> graph, RouterEssentials essentials,
                    graph::Router<double>::RoutesInternalData routes_internal_data, RoutingSettings settings);

    TransportRouter(const transport_routine::catalogue::TransportCatalogue& catalogue,
                    graph::DirectedWeightedGraph<double> graph, RoutingSettings settings,
                    std::unordered_map<const transport_routine::domain::Stop*, Vertexes> stop_to_vertex_index,
                    std::unordered_map<graph::EdgeId, RouteItem> edge_to_route_item_index);

    Route GetRoute(std::string_view from, std::string_view to) const;

    const RoutingSettings& GetRoutingSettings() const;

    const graph::DirectedWeightedGraph<double>& GetGraph() const;

    RouterEssentials GetRouterEssentials() const;

    const graph::Router<double>& GetRouter() const;

   private:
    const transport_routine::catalogue::TransportCatalogue& catalogue_;
    graph::DirectedWeightedGraph<double> graph_;
    graph::Router<double> router_;
    RoutingSettings settings_;

    std::unordered_map<const transport_routine::domain::Stop*, Vertexes> stop_to_vertex_index_;
    std::unordered_map<graph::EdgeId, RouteItem> edge_to_route_item_index_;
};

template <typename InputIt>
void TransportRouter::TransportRouterBuilder::AddOneWayRouteToGraph(InputIt first, InputIt last,
                                                                    const transport_routine::domain::Bus& bus) {
    for (auto it_from = first; it_from != last - 1; ++it_from) {
        double total_distance = .0;
        const transport_routine::domain::Stop* previous_stop = *it_from;
        for (auto it_to = it_from + 1; it_to != last; ++it_to) {
            size_t span = std::abs(it_to - it_from);
            const transport_routine::domain::Distances distances = catalogue_.GetDistance(previous_stop, *it_to);
            total_distance += distances.path_distance;
            previous_stop = *it_to;
            double time = ComputeTimeMinutes(total_distance);
            graph::EdgeId edge = graph_.AddEdge(
                {stop_to_vertex_index_.at(*it_from).on_route, stop_to_vertex_index_.at(*it_to).terminal, time});
            RouteItem item = {RouteItemType::BUS, bus.name, time, span};
            edge_to_route_item_index_.insert({edge, item});
        }
    }
}

}  // namespace transport_router
