#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <variant>
#include <vector>

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "serialization.h"
#include "svg.h"
#include "transport_router.h"

namespace json_reader {

class JsonReader {
   public:
    explicit JsonReader(json::Document document);
    const std::vector<transport_routine::request_handler::StopBaseRequest>& GetStopRequests() const;
    const std::vector<transport_routine::request_handler::BusBaseRequest>& GetBusRequests() const;
    const std::vector<std::unique_ptr<transport_routine::request_handler::StatRequest>>& GetStatRequests() const;
    const map_renderer::RenderSettings& GetRenderSettings() const;
    const transport_router::RoutingSettings& GetRoutingSettings() const;
    const serialization::SerializationSettings& GetSerializationSettings() const;

    void ProcessDocumentBaseLoad();
    void ProcessDocumentRequestLoad();

   private:
    json::Document raw_document_;
    std::vector<transport_routine::request_handler::StopBaseRequest> stop_base_requests_;
    std::vector<transport_routine::request_handler::BusBaseRequest> bus_base_requests_;
    std::vector<std::unique_ptr<transport_routine::request_handler::StatRequest>> stat_requests_;
    map_renderer::RenderSettings render_settings_;
    transport_router::RoutingSettings routing_settings_;
    serialization::SerializationSettings serialization_settings_;

    void ProcessBaseRequests();
    void ProcessStatRequests();
    void ProcessStopRequests(const json::Array& stop_raw_requests);
    void ProcessBusRequests(const json::Array& bus_raw_requests);
    void ProcessRenderSettings();
    void ProcessRoutingSettings();
    void ProcessSerializationSettings();
};

namespace detail {

struct ColorReader {
    svg::Color operator()(const json::Array& arr) const;
    svg::Color operator()(const std::string& str) const;
    svg::Color operator()(...) const;
};

void AddBase(const std::vector<transport_routine::request_handler::StopBaseRequest>& stop_base_requests,
             const std::vector<transport_routine::request_handler::BusBaseRequest>& bus_base_requests,
             transport_routine::request_handler::RequestHandler& handler);

void AddBaseFromReader(const json_reader::JsonReader& reader,
                       transport_routine::request_handler::RequestHandler& handler);

}  // namespace detail

json::Document ProcessStatRequestToJSON(const JsonReader& document,
                                        transport_routine::request_handler::RequestHandler& handler);

void ProcessJsonRequest(transport_routine::request_handler::RequestHandler& handler, std::istream& input,
                        std::ostream& output);

void MakeBaseSerialize(transport_routine::request_handler::RequestHandler& handler, std::istream& input);

void PrintFromDeserializedBase(transport_routine::request_handler::RequestHandler& handler, std::istream& input,
                               std::ostream& output);

void PrintMapFromJSON(transport_routine::request_handler::RequestHandler& handler, std::istream& input,
                      std::ostream& output);

}  // namespace json_reader