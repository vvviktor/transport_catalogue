#include "request_handler.h"

namespace transport_routine::request_handler {

RequestHandler::RequestHandler(catalogue::TransportCatalogue& db, map_renderer::MapRenderer& renderer)
    : db_(db), renderer_(renderer) {}

void RequestHandler::AddStop(const StopBaseRequest& stop_base_request) {
    db_.AddStop({stop_base_request.name, stop_base_request.coordinates});
}

void RequestHandler::AddBus(const BusBaseRequest& bus_base_request) {
    domain::Bus new_bus;
    new_bus.name = bus_base_request.name;
    new_bus.is_roundtrip = bus_base_request.is_roundtrip;
    for (std::string_view stop_name : bus_base_request.route) {
        const domain::Stop* stop = db_.FindStop(stop_name);
        new_bus.unique_stops.insert(stop);
        new_bus.route.push_back(stop);
    }
    db_.AddRoute(new_bus);
}

void RequestHandler::AddBus(const transport_routine::domain::Bus& bus) { db_.AddRoute(bus); }

void RequestHandler::SetDistance(std::string_view from, std::string_view dest, int distance) {
    db_.SetDistance(db_.FindStop(from), db_.FindStop(dest), distance);
}

const domain::RouteStats* RequestHandler::GetBusStat(std::string_view bus_name) const {
    return db_.FindRouteStats(bus_name);
}

const std::set<std::string_view>* RequestHandler::GetBusesByStop(std::string_view stop_name) const {
    return db_.FindStopUniqueBuses(stop_name);
}

map_renderer::SortedRoutes RequestHandler::GetAllBuses() const {
    map_renderer::SortedRoutes ret;

    if (const auto buses = db_.GetAllRoutes()) {
        for (const domain::Bus& bus : *buses) {
            ret.insert(&bus);
        }
    }

    return ret;
}

void RequestHandler::SetRenderSettings(const map_renderer::RenderSettings& render_settings) {
    renderer_.SetRenderSettings(render_settings);
}

void RequestHandler::SetUpTransportRouter(std::unique_ptr<transport_router::TransportRouter> router) {
    router_ = std::move(router);
}

transport_router::Route RequestHandler::GetRoute(std::string_view from, std::string_view to) const {
    using namespace std::literals;
    if (!router_) {
        throw std::logic_error("No router found"s);
    }
    return router_->GetRoute(from, to);
}

const transport_router::TransportRouter& RequestHandler::GetTransportRouter() const {
    using namespace std::literals;
    if (!router_) {
        throw std::logic_error("No router found"s);
    }
    return *router_;
}

svg::Document RequestHandler::RenderRouteMap() const { return renderer_.RenderRouteMap(GetAllBuses()); }

const catalogue::TransportCatalogue& RequestHandler::GetCatalogue() const { return db_; }

const map_renderer::RenderSettings& RequestHandler::GetRenderSettings() const { return renderer_.GetRenderSettings(); }

}  // namespace transport_routine::request_handler