#include "json_reader.h"

namespace json_reader {

JsonReader::JsonReader(json::Document document) : raw_document_(std::move(document)) {}

const std::vector<transport_routine::request_handler::StopBaseRequest>& JsonReader::GetStopRequests() const {
    return stop_base_requests_;
}

const std::vector<transport_routine::request_handler::BusBaseRequest>& JsonReader::GetBusRequests() const {
    return bus_base_requests_;
}

const std::vector<std::unique_ptr<transport_routine::request_handler::StatRequest>>& JsonReader::GetStatRequests()
    const {
    return stat_requests_;
}

const map_renderer::RenderSettings& JsonReader::GetRenderSettings() const { return render_settings_; }

const transport_router::RoutingSettings& JsonReader::GetRoutingSettings() const { return routing_settings_; }

const serialization::SerializationSettings& JsonReader::GetSerializationSettings() const {
    return serialization_settings_;
}

void JsonReader::ProcessDocumentBaseLoad() {
    ProcessBaseRequests();
    ProcessRenderSettings();
    ProcessRoutingSettings();
    ProcessSerializationSettings();
}

void JsonReader::ProcessDocumentRequestLoad() {
    ProcessStatRequests();
    ProcessSerializationSettings();
}

void JsonReader::ProcessBaseRequests() {
    using namespace std::literals;
    const json::Array raw_requests = raw_document_.GetRoot().AsDict().at("base_requests"s).AsArray();
    json::Array stop_raw_requests;
    json::Array bus_raw_requests;

    if (raw_requests.empty()) {
        return;
    }

    for (const json::Node& raw_request : raw_requests) {
        if (raw_request.AsDict().at("type"s) == "Stop"s) {
            stop_raw_requests.push_back(raw_request);
        } else if (raw_request.AsDict().at("type"s) == "Bus"s) {
            bus_raw_requests.push_back(raw_request);
        }
    }

    if (!stop_raw_requests.empty()) {
        ProcessStopRequests(stop_raw_requests);
    }
    if (!bus_raw_requests.empty()) {
        ProcessBusRequests(bus_raw_requests);
    }
}

void JsonReader::ProcessStatRequests() {
    using namespace std::literals;

    const json::Array raw_requests = raw_document_.GetRoot().AsDict().at("stat_requests"s).AsArray();

    if (raw_requests.empty()) {
        return;
    }

    for (const json::Node& stat_request : raw_requests) {
        const std::string type = stat_request.AsDict().at("type"s).AsString();
        const int id = stat_request.AsDict().at("id"s).AsInt();
        if (type == "Map"s) {
            stat_requests_.push_back(std::make_unique<transport_routine::request_handler::MapRequest>(id, type));
            continue;
        } else if (type == "Route"s) {
            const std::string from = stat_request.AsDict().at("from"s).AsString();
            const std::string to = stat_request.AsDict().at("to"s).AsString();
            stat_requests_.push_back(
                std::make_unique<transport_routine::request_handler::RouteRequest>(id, type, from, to));
            continue;
        }
        const std::string name = stat_request.AsDict().at("name"s).AsString();
        if (type == "Stop"s || type == "Bus"s) {
            stat_requests_.push_back(
                std::make_unique<transport_routine::request_handler::BusStopStatRequest>(id, type, name));
        }
    }
}

void JsonReader::ProcessStopRequests(const json::Array& stop_raw_requests) {
    using namespace std::literals;
    std::vector<transport_routine::request_handler::StopBaseRequest> ret;

    for (const json::Node& stop_raw_request : stop_raw_requests) {
        transport_routine::request_handler::StopBaseRequest request;
        request.name = stop_raw_request.AsDict().at("name"s).AsString();
        request.coordinates = {stop_raw_request.AsDict().at("latitude"s).AsDouble(),
                               stop_raw_request.AsDict().at("longitude"s).AsDouble()};
        const json::Dict stops_to_distances = stop_raw_request.AsDict().at("road_distances"s).AsDict();
        for (const std::pair<std::string, json::Node> value : stops_to_distances) {
            request.distances_to.emplace(make_pair(value.first, value.second.AsInt()));
        }
        ret.emplace_back(request);
    }

    stop_base_requests_ = std::move(ret);
}

void JsonReader::ProcessBusRequests(const json::Array& bus_raw_requests) {
    using namespace std::literals;
    std::vector<transport_routine::request_handler::BusBaseRequest> ret;

    for (const json::Node& bus_raw_request : bus_raw_requests) {
        transport_routine::request_handler::BusBaseRequest request;
        request.name = bus_raw_request.AsDict().at("name"s).AsString();
        const json::Array stops = bus_raw_request.AsDict().at("stops"s).AsArray();
        for (const json::Node& stop : stops) {
            request.route.emplace_back(stop.AsString());
        }
        request.is_roundtrip = bus_raw_request.AsDict().at("is_roundtrip"s).AsBool();
        ret.emplace_back(request);
    }

    bus_base_requests_ = std::move(ret);
}

void JsonReader::ProcessRenderSettings() {
    using namespace std::literals;
    if (raw_document_.GetRoot().AsDict().count("render_settings"s) == 0) {
        return;
    }
    json::Dict raw_render_settings = raw_document_.GetRoot().AsDict().at("render_settings"s).AsDict();
    if (raw_render_settings.empty()) {
        return;
    }
    render_settings_.width = raw_render_settings.at("width"s).AsDouble();
    render_settings_.height = raw_render_settings.at("height"s).AsDouble();
    render_settings_.padding = raw_render_settings.at("padding"s).AsDouble();
    render_settings_.stop_radius = raw_render_settings.at("stop_radius"s).AsDouble();
    render_settings_.line_width = raw_render_settings.at("line_width"s).AsDouble();
    render_settings_.bus_label_font_size = raw_render_settings.at("bus_label_font_size"s).AsInt();
    render_settings_.bus_label_offset = {raw_render_settings.at("bus_label_offset"s).AsArray()[0].AsDouble(),
                                         raw_render_settings.at("bus_label_offset"s).AsArray()[1].AsDouble()};
    render_settings_.stop_label_font_size = raw_render_settings.at("stop_label_font_size"s).AsInt();
    render_settings_.stop_label_offset = {raw_render_settings.at("stop_label_offset"s).AsArray()[0].AsDouble(),
                                          raw_render_settings.at("stop_label_offset"s).AsArray()[1].AsDouble()};
    auto raw_color = raw_render_settings.at("underlayer_color"s).GetValue();
    render_settings_.underlayer_color = std::visit(detail::ColorReader{}, raw_color);
    render_settings_.underlayer_width = raw_render_settings.at("underlayer_width"s).AsDouble();
    json::Array raw_palette = raw_render_settings.at("color_palette"s).AsArray();
    for (const json::Node& color_node : raw_palette) {
        auto color = color_node.GetValue();
        render_settings_.color_palette.push_back(std::visit(detail::ColorReader{}, color));
    }
}

void JsonReader::ProcessRoutingSettings() {
    using namespace std::literals;
    if (raw_document_.GetRoot().AsDict().count("routing_settings"s) == 0) {
        return;
    }
    json::Dict raw_routing_settings = raw_document_.GetRoot().AsDict().at("routing_settings"s).AsDict();
    if (raw_routing_settings.empty()) {
        return;
    }
    routing_settings_.bus_velocity = raw_routing_settings.at("bus_velocity"s).AsDouble();
    routing_settings_.bus_wait_time = raw_routing_settings.at("bus_wait_time"s).AsInt();
}

void JsonReader::ProcessSerializationSettings() {
    using namespace std::literals;
    json::Dict raw_serialization_settings = raw_document_.GetRoot().AsDict().at("serialization_settings"s).AsDict();
    if (raw_serialization_settings.empty()) {
        return;
    }
    serialization_settings_.file = raw_serialization_settings.at("file"s).AsString();
}

namespace detail {

svg::Color ColorReader::operator()(const json::Array& arr) const {
    if (arr.size() == 3) {
        return svg::Rgb(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
    } else if (arr.size() == 4) {
        return svg::Rgba(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
    }
    return {};
}

svg::Color ColorReader::operator()(const std::string& str) const { return str; }

svg::Color ColorReader::operator()(...) const { return {}; }

void AddBase(const std::vector<transport_routine::request_handler::StopBaseRequest>& stop_base_requests,
             const std::vector<transport_routine::request_handler::BusBaseRequest>& bus_base_requests,
             transport_routine::request_handler::RequestHandler& handler) {
    if (!stop_base_requests.empty()) {
        for (const auto& stop_base_request : stop_base_requests) {
            handler.AddStop(stop_base_request);
        }
        for (const auto& stop_base_request : stop_base_requests) {
            if (stop_base_request.distances_to.empty()) {
                continue;
            }
            for (const auto& [dest, distance] : stop_base_request.distances_to) {
                handler.SetDistance(stop_base_request.name, dest, distance);
            }
        }
    }
    if (!bus_base_requests.empty()) {
        for (const auto& bus_base_request : bus_base_requests) {
            handler.AddBus(bus_base_request);
        }
    }
}

void AddBaseFromReader(const json_reader::JsonReader& reader,
                       transport_routine::request_handler::RequestHandler& handler) {
    AddBase(reader.GetStopRequests(), reader.GetBusRequests(), handler);
}

}  // namespace detail

json::Document ProcessStatRequestToJSON(const JsonReader& document,
                                        transport_routine::request_handler::RequestHandler& handler) {
    using namespace std::literals;
    json::Builder builder{};
    builder.StartArray();

    for (const auto& request : document.GetStatRequests()) {
        json::Builder stat_dict{};
        stat_dict.StartDict();
        stat_dict.Key("request_id"s).Value(request->GetId());
        if (request->GetType() == "Stop"s) {
            auto stat_request = dynamic_cast<transport_routine::request_handler::BusStopStatRequest*>(request.get());
            if (const std::set<std::string_view>* stops = handler.GetBusesByStop(stat_request->GetName())) {
                json::Array buses;
                if (!stops->empty()) {
                    for (std::string_view bus : *stops) {
                        buses.emplace_back(std::string(bus));
                    }
                }
                stat_dict.Key("buses"s).Value(buses);
            } else {
                stat_dict.Key("error_message"s).Value("not found"s);
            }
        } else if (request->GetType() == "Bus"s) {
            auto stat_request = dynamic_cast<transport_routine::request_handler::BusStopStatRequest*>(request.get());
            if (const transport_routine::domain::RouteStats* bus_stats = handler.GetBusStat(stat_request->GetName())) {
                stat_dict.Key("route_length"s)
                    .Value(bus_stats->total_distance)
                    .Key("stop_count"s)
                    .Value(bus_stats->total_stops)
                    .Key("unique_stop_count"s)
                    .Value(bus_stats->unique_stops)
                    .Key("curvature"s)
                    .Value(bus_stats->curvature);
            } else {
                stat_dict.Key("error_message"s).Value("not found"s);
            }
        } else if (request->GetType() == "Map"s) {
            std::ostringstream ostr;
            handler.RenderRouteMap().Render(ostr);
            stat_dict.Key("map"s).Value(ostr.str());
        } else if (request->GetType() == "Route"s) {
            auto stat_request = dynamic_cast<transport_routine::request_handler::RouteRequest*>(request.get());
            transport_router::Route route = handler.GetRoute(stat_request->GetFrom(), stat_request->GetTo());
            if (!route) {
                stat_dict.Key("error_message"s).Value("not found"s);
            } else {
                stat_dict.Key("total_time"s).Value(route.total_time);
                stat_dict.Key("items"s).StartArray();
                for (const transport_router::RouteItem& item : route.items) {
                    stat_dict.StartDict();
                    if (item.type == transport_router::RouteItemType::WAIT) {
                        stat_dict.Key("type"s)
                            .Value("Wait"s)
                            .Key("stop_name"s)
                            .Value(item.name)
                            .Key("time"s)
                            .Value(item.time);
                    } else if (item.type == transport_router::RouteItemType::BUS) {
                        stat_dict.Key("type"s)
                            .Value("Bus"s)
                            .Key("bus"s)
                            .Value(item.name)
                            .Key("span_count"s)
                            .Value(static_cast<int>(item.span_count))
                            .Key("time"s)
                            .Value(item.time);
                    }
                    stat_dict.EndDict();
                }
                stat_dict.EndArray();
            }
        }
        stat_dict.EndDict();
        builder.Value(stat_dict.Build().AsDict());
    }

    builder.EndArray();
    return json::Document{builder.Build()};
}

void ProcessJsonRequest(transport_routine::request_handler::RequestHandler& handler, std::istream& input,
                        std::ostream& output) {
    using namespace std::literals;
    JsonReader document(json::Load(input));
    document.ProcessDocumentBaseLoad();
    document.ProcessDocumentRequestLoad();
    handler.SetRenderSettings(document.GetRenderSettings());
    detail::AddBaseFromReader(document, handler);
    std::unique_ptr<transport_router::TransportRouter> t_router =
        transport_router::TransportRouter::TransportRouterBuilder(handler.GetCatalogue(), document.GetRoutingSettings())
            .Build();
    handler.SetUpTransportRouter(std::move(t_router));

    if (document.GetStatRequests().empty()) {
        return;
    }

    json::Document result = ProcessStatRequestToJSON(document, handler);
    json::Print(result, output);
}

void MakeBaseSerialize(transport_routine::request_handler::RequestHandler& handler, std::istream& input) {
    JsonReader document(json::Load(input));
    document.ProcessDocumentBaseLoad();
    handler.SetRenderSettings(document.GetRenderSettings());
    detail::AddBaseFromReader(document, handler);
    std::unique_ptr<transport_router::TransportRouter> t_router =
        transport_router::TransportRouter::TransportRouterBuilder(handler.GetCatalogue(), document.GetRoutingSettings())
            .Build();
    handler.SetUpTransportRouter(std::move(t_router));
    serialization::SerializeCatalogue(handler, document.GetSerializationSettings());
}

void PrintFromDeserializedBase(transport_routine::request_handler::RequestHandler& handler, std::istream& input,
                               std::ostream& output) {
    JsonReader document(json::Load(input));
    document.ProcessDocumentRequestLoad();
    serialization::DeserializeCatalogue(handler, document.GetSerializationSettings());

    if (document.GetStatRequests().empty()) {
        return;
    }

    json::Document result = ProcessStatRequestToJSON(document, handler);
    json::Print(result, output);
}

void PrintMapFromJSON(transport_routine::request_handler::RequestHandler& handler, std::istream& input,
                      std::ostream& output) {
    JsonReader reader(json::Load(input));
    handler.SetRenderSettings(reader.GetRenderSettings());
    detail::AddBaseFromReader(reader, handler);
    handler.RenderRouteMap().Render(output);
}

}  // namespace json_reader