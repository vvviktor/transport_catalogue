#include "serialization.h"

#include <fstream>
#include <string_view>
#include <unordered_map>

namespace serialization {

namespace detail {

transport_catalogue_serialize::Color ColorProtoGen::operator()(std::monostate) const { return {}; }

transport_catalogue_serialize::Color ColorProtoGen::operator()(const svg::Rgb& rgb_color) const {
    transport_catalogue_serialize::Color ret;
    ret.mutable_rgb_color()->set_r(rgb_color.red);
    ret.mutable_rgb_color()->set_g(rgb_color.green);
    ret.mutable_rgb_color()->set_b(rgb_color.blue);
    return ret;
}

transport_catalogue_serialize::Color ColorProtoGen::operator()(const svg::Rgba& rgba_color) const {
    transport_catalogue_serialize::Color ret;
    ret.mutable_rgba_color()->set_r(rgba_color.red);
    ret.mutable_rgba_color()->set_g(rgba_color.green);
    ret.mutable_rgba_color()->set_b(rgba_color.blue);
    ret.mutable_rgba_color()->set_a(rgba_color.opacity);
    return ret;
}

transport_catalogue_serialize::Color ColorProtoGen::operator()(const std::string& string_color) const {
    transport_catalogue_serialize::Color ret;
    ret.set_str_color(string_color);
    return ret;
}

transport_catalogue_serialize::Edge SerializeEdge(const graph::Edge<double>& edge) {
    transport_catalogue_serialize::Edge ret;
    ret.set_from(edge.from);
    ret.set_to(edge.to);
    ret.set_weight(edge.weight);
    return ret;
}

graph::Edge<double> DeserializeEdge(const transport_catalogue_serialize::Edge& edge) {
    graph::Edge<double> ret;
    ret.from = edge.from();
    ret.to = edge.to();
    ret.weight = edge.weight();
    return ret;
}

transport_catalogue_serialize::IncidenceList SerializeIncidenceList(
    const graph::DirectedWeightedGraph<double>::IncidenceList& incidence_list) {
    transport_catalogue_serialize::IncidenceList ret;
    if (!incidence_list.empty()) {
        ret.mutable_edge_id_list()->Add(incidence_list.begin(), incidence_list.end());
    }
    return ret;
}

graph::DirectedWeightedGraph<double>::IncidenceList DeserealizeIncidenceList(
    const transport_catalogue_serialize::IncidenceList& incidence_list) {
    graph::DirectedWeightedGraph<double>::IncidenceList ret;
    if (!incidence_list.edge_id_list_size() != 0) {
        ret = {incidence_list.edge_id_list().begin(), incidence_list.edge_id_list().end()};
    }
    return ret;
}

transport_catalogue_serialize::Graph SerializeGraph(
    const std::vector<graph::Edge<double>>& edges,
    const std::vector<graph::DirectedWeightedGraph<double>::IncidenceList>& incidence_lists) {
    transport_catalogue_serialize::Graph ret;
    if (!edges.empty()) {
        for (const graph::Edge<double>& edge : edges) {
            *ret.add_edges() = SerializeEdge(edge);
        }
    }
    if (!incidence_lists.empty()) {
        for (const graph::DirectedWeightedGraph<double>::IncidenceList& incidence_list : incidence_lists) {
            *ret.add_incidence_lists() = SerializeIncidenceList(incidence_list);
        }
    }
    return ret;
}

graph::DirectedWeightedGraph<double> DeserializeGraph(const transport_catalogue_serialize::Graph& graph) {
    std::vector<graph::Edge<double>> edges;
    std::vector<graph::DirectedWeightedGraph<double>::IncidenceList> incidence_lists;
    if (graph.edges_size() != 0) {
        for (const transport_catalogue_serialize::Edge& edge : graph.edges()) {
            edges.push_back(DeserializeEdge(edge));
        }
    }
    if (graph.incidence_lists_size() != 0) {
        for (const transport_catalogue_serialize::IncidenceList& incidence_list : graph.incidence_lists()) {
            incidence_lists.push_back(DeserealizeIncidenceList(incidence_list));
        }
    }
    return graph::DirectedWeightedGraph<double>(std::move(edges), std::move(incidence_lists));
}

transport_catalogue_serialize::Vertexes SerializeVertexes(const transport_router::Vertexes& vertexes) {
    transport_catalogue_serialize::Vertexes ret;
    ret.set_terminal(vertexes.terminal);
    ret.set_on_route(vertexes.on_route);
    return ret;
}

transport_router::Vertexes DeserializeVertexes(const transport_catalogue_serialize::Vertexes& vertexes) {
    transport_router::Vertexes ret;
    ret.terminal = vertexes.terminal();
    ret.on_route = vertexes.on_route();
    return ret;
}

transport_catalogue_serialize::RouteItem SerializeRouteItem(const transport_router::RouteItem& route_item) {
    transport_catalogue_serialize::RouteItem ret;
    ret.set_type(route_item.type == transport_router::RouteItemType::WAIT ? transport_catalogue_serialize::RIT_WAIT
                                                                          : transport_catalogue_serialize::RIT_BUS);
    ret.set_name(route_item.name);
    ret.set_time(route_item.time);
    ret.set_span_count(route_item.span_count);
    return ret;
}

transport_router::RouteItem DeserializeRouteItem(const transport_catalogue_serialize::RouteItem& route_item) {
    transport_router::RouteItem ret;
    ret.type = route_item.type() == transport_catalogue_serialize::RIT_WAIT ? transport_router::RouteItemType::WAIT
                                                                            : transport_router::RouteItemType::BUS;
    ret.name = route_item.name();
    ret.time = route_item.time();
    ret.span_count = route_item.span_count();
    return ret;
}

transport_catalogue_serialize::RouterEssentials SerializeRouterEssentials(
    const std::vector<std::pair<std::string, transport_router::Vertexes>>& stop_to_vertex_index,
    const std::unordered_map<graph::EdgeId, transport_router::RouteItem>& edge_to_route_item_index) {
    transport_catalogue_serialize::RouterEssentials ret;
    if (!stop_to_vertex_index.empty()) {
        for (const auto& [stop_name, vertexes] : stop_to_vertex_index) {
            auto add_ptr = ret.add_stop_to_vertex_index();
            add_ptr->set_stop_name(stop_name);
            *add_ptr->mutable_vertexes() = SerializeVertexes(vertexes);
        }
    }
    if (!edge_to_route_item_index.empty()) {
        for (const auto& [edge_id, route_item] : edge_to_route_item_index) {
            auto add_ptr = ret.add_edge_to_route_item_index();
            add_ptr->set_edge_id(edge_id);
            *add_ptr->mutable_route_item() = SerializeRouteItem(route_item);
        }
    }
    return ret;
}

transport_router::RouterEssentials DeserializeRouterEssentials(
    const transport_catalogue_serialize::RouterEssentials& router_essentials) {
    transport_router::RouterEssentials ret;
    std::vector<std::pair<std::string, transport_router::Vertexes>> stop_to_vertex_index;
    std::unordered_map<graph::EdgeId, transport_router::RouteItem> edge_to_route_item_index;
    if (router_essentials.stop_to_vertex_index_size() != 0) {
        for (const transport_catalogue_serialize::StrVertexesPair& str_vertex_pair :
             router_essentials.stop_to_vertex_index()) {
            stop_to_vertex_index.push_back(
                std::make_pair(str_vertex_pair.stop_name(), DeserializeVertexes(str_vertex_pair.vertexes())));
        }
    }
    if (router_essentials.edge_to_route_item_index_size() != 0) {
        for (const transport_catalogue_serialize::EdgIDRouteItemPair& edg_route_item_pair :
             router_essentials.edge_to_route_item_index()) {
            edge_to_route_item_index.insert(
                std::make_pair(edg_route_item_pair.edge_id(), DeserializeRouteItem(edg_route_item_pair.route_item())));
        }
    }
    return {std::move(stop_to_vertex_index), std::move(edge_to_route_item_index)};
}

transport_catalogue_serialize::RoutingSettings SerializeRoutingSettings(
    const transport_router::RoutingSettings& settings) {
    transport_catalogue_serialize::RoutingSettings ret;
    ret.set_bus_wait_time(settings.bus_wait_time);
    ret.set_bus_velocity(settings.bus_velocity);
    return ret;
}

transport_router::RoutingSettings DeserializeRoutingSettings(
    const transport_catalogue_serialize::RoutingSettings& settings) {
    transport_router::RoutingSettings ret;
    ret.bus_wait_time = settings.bus_wait_time();
    ret.bus_velocity = settings.bus_velocity();
    return ret;
}

transport_catalogue_serialize::RouteInternalData SerializeRouteInternalData(
    const graph::Router<double>::RouteInternalData& route_internal_data) {
    transport_catalogue_serialize::RouteInternalData ret;
    ret.set_weight(route_internal_data.weight);
    if (route_internal_data.prev_edge) {
        ret.set_prev_edge(*route_internal_data.prev_edge);
    }
    return ret;
}

graph::Router<double>::RouteInternalData DeserializeRouteInternalData(
    const transport_catalogue_serialize::RouteInternalData& route_internal_data) {
    graph::Router<double>::RouteInternalData ret;
    ret.weight = route_internal_data.weight();
    if (route_internal_data.opt_prev_edge_case() !=
        transport_catalogue_serialize::RouteInternalData::OptPrevEdgeCase::OPT_PREV_EDGE_NOT_SET) {
        ret.prev_edge = route_internal_data.prev_edge();
    }
    return ret;
}

transport_catalogue_serialize::RouteInternalDataRepeated SerializeInnerDataContainer(
    const std::vector<std::optional<graph::Router<double>::RouteInternalData>>& route_internal_data_cont) {
    transport_catalogue_serialize::RouteInternalDataRepeated ret;
    if (!route_internal_data_cont.empty()) {
        for (const std::optional<graph::Router<double>::RouteInternalData>& route_internal_data :
             route_internal_data_cont) {
            transport_catalogue_serialize::RoutesInternalDataElem elem;
            if (route_internal_data) {
                *elem.mutable_route_internal_data() = SerializeRouteInternalData(*route_internal_data);
            }
            *ret.add_routes_internal_data_elem() = elem;
        }
    }
    return ret;
}

std::vector<std::optional<graph::Router<double>::RouteInternalData>> DeserializeInnerContainer(
    const transport_catalogue_serialize::RouteInternalDataRepeated& route_internal_data_cont) {
    std::vector<std::optional<graph::Router<double>::RouteInternalData>> ret;
    if (route_internal_data_cont.routes_internal_data_elem_size() != 0) {
        for (const transport_catalogue_serialize::RoutesInternalDataElem& elem :
             route_internal_data_cont.routes_internal_data_elem()) {
            std::optional<graph::Router<double>::RouteInternalData> data;
            if (elem.opt_route_internal_data_case() != transport_catalogue_serialize::RoutesInternalDataElem::
                                                           OptRouteInternalDataCase::OPT_ROUTE_INTERNAL_DATA_NOT_SET) {
                data = DeserializeRouteInternalData(elem.route_internal_data());
            }
            ret.push_back(data);
        }
    }
    return ret;
}

transport_catalogue_serialize::RouterData SerializeRouterData(
    const graph::Router<double>::RoutesInternalData& routes_internal_data) {
    transport_catalogue_serialize::RouterData ret;
    if (!routes_internal_data.empty()) {
        for (const std::vector<std::optional<graph::Router<double>::RouteInternalData>>& route_internal_data_cont :
             routes_internal_data) {
            *ret.add_routes_internal_data() = SerializeInnerDataContainer(route_internal_data_cont);
        }
    }
    return ret;
}

graph::Router<double>::RoutesInternalData DeserializeRouterData(
    const transport_catalogue_serialize::RouterData& router_data) {
    graph::Router<double>::RoutesInternalData ret;
    if (router_data.routes_internal_data_size() != 0) {
        for (const transport_catalogue_serialize::RouteInternalDataRepeated& inner_cont :
             router_data.routes_internal_data()) {
            ret.push_back(DeserializeInnerContainer(inner_cont));
        }
    }
    return ret;
}

transport_catalogue_serialize::Color SerializeColor(const svg::Color& color) {
    return std::visit(ColorProtoGen{}, color);
}

svg::Color DeserializeColor(const transport_catalogue_serialize::Color& color) {
    svg::Color ret;
    const transport_catalogue_serialize::Color::OptColorCase field = color.opt_color_case();
    switch (field) {
        case transport_catalogue_serialize::Color::OptColorCase::kRgbColor:
            ret = svg::Rgb(color.rgb_color().r(), color.rgb_color().g(), color.rgb_color().b());
            break;
        case transport_catalogue_serialize::Color::OptColorCase::kRgbaColor:
            ret = svg::Rgba(color.rgba_color().r(), color.rgba_color().g(), color.rgba_color().b(),
                            color.rgba_color().a());
            break;
        case transport_catalogue_serialize::Color::OptColorCase::kStrColor:
            ret = color.str_color();
            break;
        default:
            break;
    }
    return ret;
}

transport_catalogue_serialize::Point SerializePoint(const svg::Point& point) {
    transport_catalogue_serialize::Point ret;
    ret.set_x(point.x);
    ret.set_y(point.y);
    return ret;
}

transport_catalogue_serialize::RenderSettings SerializeRenderSettings(const map_renderer::RenderSettings& settings) {
    transport_catalogue_serialize::RenderSettings ret;
    ret.set_width(settings.width);
    ret.set_height(settings.height);
    ret.set_padding(settings.padding);
    ret.set_line_width(settings.line_width);
    ret.set_stop_radius(settings.stop_radius);
    ret.set_bus_label_font_size(settings.bus_label_font_size);
    *ret.mutable_bus_label_offset() = SerializePoint(settings.bus_label_offset);
    ret.set_stop_label_font_size(settings.stop_label_font_size);
    *ret.mutable_stop_label_offset() = SerializePoint(settings.stop_label_offset);
    *ret.mutable_underlayer_color() = SerializeColor(settings.underlayer_color);
    ret.set_underlayer_width(settings.underlayer_width);
    if (!settings.color_palette.empty()) {
        for (const svg::Color& color : settings.color_palette) {
            *ret.add_color_palette() = SerializeColor(color);
        }
    }
    return ret;
}

map_renderer::RenderSettings DeserializeRenderSettings(const transport_catalogue_serialize::RenderSettings& settings) {
    map_renderer::RenderSettings ret;
    ret.width = settings.width();
    ret.height = settings.height();
    ret.padding = settings.padding();
    ret.line_width = settings.line_width();
    ret.stop_radius = settings.stop_radius();
    ret.bus_label_font_size = settings.bus_label_font_size();
    ret.bus_label_offset = svg::Point(settings.bus_label_offset().x(), settings.bus_label_offset().y());
    ret.stop_label_font_size = settings.stop_label_font_size();
    ret.stop_label_offset = svg::Point(settings.stop_label_offset().x(), settings.stop_label_offset().y());
    ret.underlayer_color = DeserializeColor(settings.underlayer_color());
    ret.underlayer_width = settings.underlayer_width();
    if (settings.color_palette_size() != 0) {
        for (const transport_catalogue_serialize::Color& color : settings.color_palette()) {
            ret.color_palette.push_back(DeserializeColor(color));
        }
    }
    return ret;
}

transport_catalogue_serialize::Stop SerializeStop(
    const transport_routine::domain::Stop& stop,
    const std::unordered_map<const transport_routine::domain::Stop*, int>* stop_distances) {
    transport_catalogue_serialize::Stop ret;
    ret.set_name(stop.name);
    ret.mutable_point()->set_lat(stop.point.lat);
    ret.mutable_point()->set_lng(stop.point.lng);
    if (stop_distances) {
        for (const auto [stop_ptr, distance] : *stop_distances) {
            auto add_ptr = ret.add_stop_distances();
            add_ptr->set_name(stop_ptr->name);
            add_ptr->set_distance(distance);
        }
    }
    return ret;
}

transport_catalogue_serialize::Bus SerializeBus(const transport_routine::domain::Bus& bus) {
    transport_catalogue_serialize::Bus ret;
    ret.set_name(bus.name);
    if (!bus.route.empty()) {
        for (const transport_routine::domain::Stop* stop_ptr : bus.route) {
            ret.add_route(stop_ptr->name);
        }
    }
    ret.set_is_roundtrip(bus.is_roundtrip);
    return ret;
}

transport_routine::domain::Bus DeserializeBus(const transport_routine::catalogue::TransportCatalogue& catalogue,
                                              const transport_catalogue_serialize::Bus& bus) {
    transport_routine::domain::Bus ret;
    ret.name = bus.name();
    if (bus.route_size() != 0) {
        for (const std::string& stop : bus.route()) {
            const transport_routine::domain::Stop* stop_ptr = catalogue.FindStop(stop);
            ret.unique_stops.insert(stop_ptr);
            ret.route.push_back(stop_ptr);
        }
    }
    ret.is_roundtrip = bus.is_roundtrip();
    return ret;
}

transport_catalogue_serialize::TransportCatalogue SerializeBase(
    const transport_routine::catalogue::TransportCatalogue& catalogue,
    const transport_router::TransportRouter& transport_router, const map_renderer::RenderSettings& render_settings) {
    transport_catalogue_serialize::TransportCatalogue ret;
    const auto* all_stops_ptr = catalogue.GetAllStops();
    if (all_stops_ptr) {
        for (const transport_routine::domain::Stop& stop : *all_stops_ptr) {
            const std::unordered_map<const transport_routine::domain::Stop*, int>* stop_distances =
                catalogue.GetStopDistances(&stop);
            *ret.mutable_base()->add_stops() = SerializeStop(stop, stop_distances);
        }
    }
    const auto* all_routes_ptr = catalogue.GetAllRoutes();
    if (all_routes_ptr) {
        for (const transport_routine::domain::Bus& bus : *all_routes_ptr) {
            *ret.mutable_base()->add_routes() = SerializeBus(bus);
        }
    }
    *ret.mutable_map_renderer()->mutable_render_settings() = SerializeRenderSettings(render_settings);
    *ret.mutable_transport_router()->mutable_routing_settings() =
        SerializeRoutingSettings(transport_router.GetRoutingSettings());
    *ret.mutable_transport_router()->mutable_router_data() =
        SerializeRouterData(transport_router.GetRouter().GetRoutesInternalData());
    *ret.mutable_transport_router()->mutable_graph() =
        SerializeGraph(transport_router.GetGraph().GetEdges(), transport_router.GetGraph().GetIncidenceLists());
    *ret.mutable_transport_router()->mutable_router_essentials() =
        SerializeRouterEssentials(transport_router.GetRouterEssentials().stop_to_vertex_index,
                                  transport_router.GetRouterEssentials().edge_to_route_item_index);
    return ret;
}

}  // namespace detail

void SerializeCatalogue(const transport_routine::request_handler::RequestHandler& handler,
                        const SerializationSettings& settings) {
    std::ofstream file(settings.file, std::ios::binary);
    detail::SerializeBase(handler.GetCatalogue(), handler.GetTransportRouter(), handler.GetRenderSettings())
        .SerializeToOstream(&file);
}

void DeserializeCatalogue(transport_routine::request_handler::RequestHandler& handler,
                          const SerializationSettings& settings) {
    std::ifstream file(settings.file, std::ios::binary);
    transport_catalogue_serialize::TransportCatalogue catalogue;
    catalogue.ParseFromIstream(&file);

    if (catalogue.mutable_base()->stops_size() != 0) {
        for (const transport_catalogue_serialize::Stop& stop : catalogue.mutable_base()->stops()) {
            handler.AddStop({stop.name(), {stop.point().lat(), stop.point().lng()}});
        }
        for (const transport_catalogue_serialize::Stop& stop : catalogue.mutable_base()->stops()) {
            for (const transport_catalogue_serialize::StopDistance stop_distance : stop.stop_distances()) {
                handler.SetDistance(stop.name(), stop_distance.name(), stop_distance.distance());
            }
        }
    }
    if (catalogue.mutable_base()->routes_size() != 0) {
        for (const transport_catalogue_serialize::Bus& bus : catalogue.mutable_base()->routes()) {
            // handler.AddBus(detail::MakeBusRequest(bus));
            handler.AddBus(detail::DeserializeBus(handler.GetCatalogue(), bus));
        }
    }

    handler.SetRenderSettings(detail::DeserializeRenderSettings(catalogue.mutable_map_renderer()->render_settings()));

    std::unique_ptr<transport_router::TransportRouter> t_router = std::make_unique<transport_router::TransportRouter>(
        handler.GetCatalogue(), detail::DeserializeGraph(catalogue.mutable_transport_router()->graph()),
        detail::DeserializeRouterEssentials(catalogue.mutable_transport_router()->router_essentials()),
        detail::DeserializeRouterData(catalogue.mutable_transport_router()->router_data()),
        detail::DeserializeRoutingSettings(catalogue.mutable_transport_router()->routing_settings()));
    handler.SetUpTransportRouter(std::move(t_router));
}

}  // namespace serialization