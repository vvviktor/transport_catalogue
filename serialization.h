#pragma once

#include <transport_catalogue.pb.h>

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace serialization {

struct SerializationSettings {
    std::string file;
};

namespace detail {

struct ColorProtoGen {
    transport_catalogue_serialize::Color operator()(std::monostate) const;
    transport_catalogue_serialize::Color operator()(const svg::Rgb& rgb_color) const;
    transport_catalogue_serialize::Color operator()(const svg::Rgba& rgba_color) const;
    transport_catalogue_serialize::Color operator()(const std::string& string_color) const;
};

transport_catalogue_serialize::Edge SerializeEdge(const graph::Edge<double>& edge);
graph::Edge<double> DeserializeEdge(const transport_catalogue_serialize::Edge& edge);

transport_catalogue_serialize::IncidenceList SerializeIncidenceList(
    const graph::DirectedWeightedGraph<double>::IncidenceList& incidence_list);
graph::DirectedWeightedGraph<double>::IncidenceList DeserealizeIncidenceList(
    const transport_catalogue_serialize::IncidenceList& incidence_list);

transport_catalogue_serialize::Graph SerializeGraph(
    const std::vector<graph::Edge<double>>& edges,
    const std::vector<graph::DirectedWeightedGraph<double>::IncidenceList>& incidence_lists);
graph::DirectedWeightedGraph<double> DeserializeGraph(const transport_catalogue_serialize::Graph& graph);

transport_catalogue_serialize::Vertexes SerializeVertexes(const transport_router::Vertexes& vertexes);
transport_router::Vertexes DeserializeVertexes(const transport_catalogue_serialize::Vertexes& vertexes);

transport_catalogue_serialize::RouteItem SerializeRouteItem(const transport_router::RouteItem& route_item);
transport_router::RouteItem DeserializeRouteItem(const transport_catalogue_serialize::RouteItem& route_item);

transport_catalogue_serialize::RouterEssentials SerializeRouterEssentials(
    const std::vector<std::pair<std::string, transport_router::Vertexes>>& stop_to_vertex_index,
    const std::unordered_map<graph::EdgeId, transport_router::RouteItem>& edge_to_route_item_index);
transport_router::RouterEssentials DeserializeRouterEssentials(
    const transport_catalogue_serialize::RouterEssentials& router_essentials);

transport_catalogue_serialize::RoutingSettings SerializeRoutingSettings(
    const transport_router::RoutingSettings& settings);
transport_router::RoutingSettings DeserializeRoutingSettings(
    const transport_catalogue_serialize::RoutingSettings& settings);

transport_catalogue_serialize::RouteInternalData SerializeRouteInternalData(
    const graph::Router<double>::RouteInternalData& route_internal_data);
graph::Router<double>::RouteInternalData DeserializeRouteInternalData(
    const transport_catalogue_serialize::RouteInternalData& route_internal_data);

transport_catalogue_serialize::RouteInternalDataRepeated SerializeInnerDataContainer(
    const std::vector<std::optional<graph::Router<double>::RouteInternalData>>& route_internal_data_cont);
std::vector<std::optional<graph::Router<double>::RouteInternalData>> DeserializeInnerContainer(
    const transport_catalogue_serialize::RouteInternalDataRepeated& route_internal_data_cont);

transport_catalogue_serialize::RouterData SerializeRouterData(
    const graph::Router<double>::RoutesInternalData& routes_internal_data);
graph::Router<double>::RoutesInternalData DeserializeRouterData(
    const transport_catalogue_serialize::RouterData& router_data);

transport_catalogue_serialize::Color SerializeColor(const svg::Color& color);
svg::Color DeserializeColor(const transport_catalogue_serialize::Color& color);

transport_catalogue_serialize::Point SerializePoint(const svg::Point& point);

transport_catalogue_serialize::RenderSettings SerializeRenderSettings(const map_renderer::RenderSettings& settings);
map_renderer::RenderSettings DeserializeRenderSettings(const transport_catalogue_serialize::RenderSettings& settings);

transport_catalogue_serialize::Stop SerializeStop(
    const transport_routine::domain::Stop& stop,
    const std::unordered_map<const transport_routine::domain::Stop*, int>* stop_distances);

transport_catalogue_serialize::Bus SerializeBus(const transport_routine::domain::Bus& bus);
transport_routine::domain::Bus DeserializeBus(const transport_routine::catalogue::TransportCatalogue& catalogue,
                                              const transport_catalogue_serialize::Bus& bus);

transport_catalogue_serialize::TransportCatalogue SerializeBase(
    const transport_routine::catalogue::TransportCatalogue& catalogue,
    const transport_router::TransportRouter& transport_router, const map_renderer::RenderSettings& render_settings);

}  // namespace detail

void SerializeCatalogue(const transport_routine::request_handler::RequestHandler& handler,
                        const SerializationSettings& settings);

void DeserializeCatalogue(transport_routine::request_handler::RequestHandler& handler,
                          const SerializationSettings& settings);

}  // namespace serialization