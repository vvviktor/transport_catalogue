#pragma once

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace transport_routine::request_handler {

struct StopBaseRequest {
    std::string name;
    geo::Coordinates coordinates;
    std::map<std::string, int> distances_to;
};

struct BusBaseRequest {
    std::string name;
    std::vector<std::string> route;
    bool is_roundtrip = false;
};

class StatRequest {
   protected:
    StatRequest(int id, std::string type) : id_(id), type_(std::move(type)) {}

   public:
    virtual ~StatRequest() = default;

    virtual int GetId() const { return id_; }

    virtual const std::string& GetType() const { return type_; }

   protected:
    int id_ = 0;
    std::string type_;
};

class BusStopStatRequest final : public StatRequest {
   public:
    BusStopStatRequest(int id, std::string type, std::string name)
        : StatRequest(id, std::move(type)), name_(std::move(name)) {}

    const std::string& GetName() const { return name_; }

   private:
    std::string name_;
};

class MapRequest final : public StatRequest {
   public:
    MapRequest(int id, std::string type) : StatRequest(id, std::move(type)) {}
};

class RouteRequest final : public StatRequest {
   public:
    RouteRequest(int id, std::string type, std::string from, std::string to)
        : StatRequest(id, std::move(type)), from_(std::move(from)), to_(std::move(to)) {}

    const std::string& GetFrom() const { return from_; }

    const std::string& GetTo() const { return to_; }

   private:
    std::string from_;
    std::string to_;
};

class RequestHandler {
   public:
    explicit RequestHandler(catalogue::TransportCatalogue& db, map_renderer::MapRenderer& renderer);

    void AddStop(const StopBaseRequest& stop_base_request);

    void AddBus(const BusBaseRequest& bus_base_request);

    void AddBus(const transport_routine::domain::Bus& bus);

    void SetDistance(std::string_view from, std::string_view dest, int distance);

    const domain::RouteStats* GetBusStat(std::string_view bus_name) const;

    const std::set<std::string_view>* GetBusesByStop(std::string_view stop_name) const;

    map_renderer::SortedRoutes GetAllBuses() const;

    void SetRenderSettings(const map_renderer::RenderSettings& render_settings);

    void SetUpTransportRouter(std::unique_ptr<transport_router::TransportRouter> router);

    svg::Document RenderRouteMap() const;

    transport_router::Route GetRoute(std::string_view from, std::string_view to) const;

    const transport_router::TransportRouter& GetTransportRouter() const;

    const catalogue::TransportCatalogue& GetCatalogue() const;

    const map_renderer::RenderSettings& GetRenderSettings() const;

   private:
    catalogue::TransportCatalogue& db_;
    map_renderer::MapRenderer& renderer_;
    std::unique_ptr<transport_router::TransportRouter> router_;
};

}  // namespace transport_routine::request_handler