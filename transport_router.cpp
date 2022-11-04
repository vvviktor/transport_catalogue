#include "transport_router.h"

namespace transport_router {

TransportRouter::TransportRouterBuilder::TransportRouterBuilder(
    const transport_routine::catalogue::TransportCatalogue& catalogue, RoutingSettings settings)
    : catalogue_(catalogue), settings_(std::move(settings)) {
    if (catalogue_.GetAllStops()) {
        graph_ = graph::DirectedWeightedGraph<double>(catalogue_.GetAllStops()->size() * 2);
    }
    CreateGraph();
}

std::unique_ptr<TransportRouter> TransportRouter::TransportRouterBuilder::Build() {
    return std::make_unique<TransportRouter>(catalogue_, std::move(graph_), std::move(settings_),
                                             std::move(stop_to_vertex_index_), std::move(edge_to_route_item_index_));
}

const TransportRouter::TransportRouterBuilder& TransportRouter::TransportRouterBuilder::CreateGraph() {
    AddStopsToGraph();
    AddBusesToGraph();
    return *this;
}

void TransportRouter::TransportRouterBuilder::AddStopsToGraph() {
    using namespace std::literals;

    if (!catalogue_.GetAllStops()) {
        return;
    }

    int vertex_id = 0;

    for (const transport_routine::domain::Stop& stop : *catalogue_.GetAllStops()) {
        graph::VertexId terminal = vertex_id++;
        graph::VertexId on_route = vertex_id++;
        graph::EdgeId edge = graph_.AddEdge({terminal, on_route, static_cast<double>(settings_.bus_wait_time)});
        const transport_routine::domain::Stop* stop_ptr = &stop;
        const Vertexes vertexes = {terminal, on_route};
        if (!stop_ptr) {
            throw std::invalid_argument("Stop* is nullptr"s);
        }
        stop_to_vertex_index_[stop_ptr] = vertexes;
        RouteItem item = {RouteItemType::WAIT, stop.name, static_cast<double>(settings_.bus_wait_time), 0};
        edge_to_route_item_index_.insert({edge, item});
    }
}

void TransportRouter::TransportRouterBuilder::AddBusesToGraph() {
    if (catalogue_.GetAllRoutes()) {
        for (const transport_routine::domain::Bus& bus : *catalogue_.GetAllRoutes()) {
            std::deque<const transport_routine::domain::Stop*> route_for_processing(bus.route.begin(), bus.route.end());

            if (!route_for_processing.empty()) {
                AddOneWayRouteToGraph(route_for_processing.begin(), route_for_processing.end(), bus);
                if (!bus.is_roundtrip) {
                    AddOneWayRouteToGraph(route_for_processing.rbegin(), route_for_processing.rend(), bus);
                }
            }
        }
    }
}

double TransportRouter::TransportRouterBuilder::ComputeTimeMinutes(double distance_m) const {
    return distance_m / ((settings_.bus_velocity * 1000) / 60);
}

TransportRouter::TransportRouter(const transport_routine::catalogue::TransportCatalogue& catalogue,
                                 graph::DirectedWeightedGraph<double> graph, RouterEssentials essentials,
                                 graph::Router<double>::RoutesInternalData routes_internal_data,
                                 RoutingSettings settings)
    : catalogue_(catalogue),
      graph_(std::move(graph)),
      settings_(std::move(settings)),
      router_(graph_, std::move(routes_internal_data)),
      edge_to_route_item_index_(std::move(essentials.edge_to_route_item_index)) {
    if (!essentials.stop_to_vertex_index.empty()) {
        for (const auto& [stop_name, vertexes] : essentials.stop_to_vertex_index) {
            stop_to_vertex_index_.insert(std::make_pair(catalogue_.FindStop(stop_name), vertexes));
        }
    }
}

TransportRouter::TransportRouter(
    const transport_routine::catalogue::TransportCatalogue& catalogue, graph::DirectedWeightedGraph<double> graph,
    RoutingSettings settings, std::unordered_map<const transport_routine::domain::Stop*, Vertexes> stop_to_vertex_index,
    std::unordered_map<graph::EdgeId, RouteItem> edge_to_route_item_index)
    : catalogue_(catalogue),
      graph_(std::move(graph)),
      settings_(std::move(settings)),
      router_(graph_),
      stop_to_vertex_index_(std::move(stop_to_vertex_index)),
      edge_to_route_item_index_(std::move(edge_to_route_item_index)) {}

Route TransportRouter::GetRoute(std::string_view from, std::string_view to) const {
    std::vector<RouteItem> ret;
    const transport_routine::domain::Stop* from_ptr = catalogue_.FindStop(from);
    const transport_routine::domain::Stop* to_ptr = catalogue_.FindStop(to);
    if (!from_ptr || !to_ptr) {
        return {};
    }

    if (from_ptr == to_ptr) {
        return {{}, 0, true};
    }

    std::optional<graph::Router<double>::RouteInfo> route =
        router_.BuildRoute(stop_to_vertex_index_.at(from_ptr).terminal, stop_to_vertex_index_.at(to_ptr).terminal);

    if (!route) {
        return {};
    }

    for (graph::EdgeId edge : route->edges) {
        ret.push_back(edge_to_route_item_index_.at(edge));
    }

    return {std::move(ret), route->weight, true};
}

const RoutingSettings& TransportRouter::GetRoutingSettings() const { return settings_; }

const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const { return graph_; }

RouterEssentials TransportRouter::GetRouterEssentials() const {
    std::vector<std::pair<std::string, Vertexes>> stop_to_vertex_index;
    for (const auto& [stop_ptr, vertexes] : stop_to_vertex_index_) {
        stop_to_vertex_index.push_back(std::make_pair(stop_ptr->name, vertexes));
    }
    return {std::move(stop_to_vertex_index), edge_to_route_item_index_};
}

const graph::Router<double>& TransportRouter::GetRouter() const { return router_; }

}  // namespace transport_router
