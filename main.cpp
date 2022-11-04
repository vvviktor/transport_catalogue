#include <fstream>
#include <iostream>
#include <string_view>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        transport_routine::catalogue::TransportCatalogue cat;
        map_renderer::MapRenderer renderer;
        transport_routine::request_handler::RequestHandler handler(cat, renderer);
        json_reader::MakeBaseSerialize(handler, std::cin);

    } else if (mode == "process_requests"sv) {
        transport_routine::catalogue::TransportCatalogue cat;
        map_renderer::MapRenderer renderer;
        transport_routine::request_handler::RequestHandler handler(cat, renderer);
        json_reader::PrintFromDeserializedBase(handler, std::cin, std::cout);
    } else {
        PrintUsage();
        return 1;
    }
}