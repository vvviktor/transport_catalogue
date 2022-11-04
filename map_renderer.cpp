#include "map_renderer.h"

namespace map_renderer {

namespace detail {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

// Проецирует широту и долготу в координаты внутри SVG-изображения
svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

} // namespace detail

MapRenderer::MapRenderer(map_renderer::RenderSettings settings) : settings_(std::move(settings)) {
}

void MapRenderer::SetRenderSettings(map_renderer::RenderSettings render_settings) {
    settings_ = std::move(render_settings);
}

const RenderSettings& MapRenderer::GetRenderSettings() const {
    return settings_;
}

svg::Document MapRenderer::RenderRouteMap(const SortedRoutes& routes) const {
    using namespace std::literals;
    svg::Document ret;
    std::vector<geo::Coordinates> all_points;
    SortedStops all_stops;
    std::for_each(routes.begin(), routes.end(), [&all_points, &all_stops](const auto bus) {
        if (!bus->route.empty()) {
            for (const transport_routine::domain::Stop* stop: bus->route) {
                all_points.push_back(stop->point);
                all_stops.insert(stop);
            }
        }
    });
    detail::SphereProjector projector(all_points.begin(), all_points.end(), settings_.width, settings_.height,
                                      settings_.padding);
    DrawLines(ret, projector, routes);
    DrawBusLabels(ret, projector, routes);
    DrawStops(ret, projector, all_stops);
    DrawStopLabels(ret, projector, all_stops);

    return ret;
}

void
MapRenderer::DrawLines(svg::Document& ret, const detail::SphereProjector& projector, const SortedRoutes& routes) const {
    using namespace std::literals;
    const size_t MAX_COLOR_INDEX = settings_.color_palette.size() - 1;
    size_t color_index = 0;

    for (const auto bus: routes) {
        if (bus->route.empty()) {
            continue;
        }
        svg::Polyline route_line;
        route_line.
                SetFillColor("none"s).
                SetStrokeColor(settings_.color_palette[color_index]).
                SetStrokeWidth(settings_.line_width).
                SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        for (const auto stop: bus->route) {
            route_line.AddPoint(projector(stop->point));
        }
        if (!bus->is_roundtrip) {
            for (auto it = next(bus->route.rbegin()); it != bus->route.rend(); ++it) {
                route_line.AddPoint(projector((*it)->point));
            }
        }
        if (color_index == MAX_COLOR_INDEX) {
            color_index = 0;
        } else {
            ++color_index;
        }
        ret.Add(route_line);
    }
}

void MapRenderer::DrawBusLabels(svg::Document& ret, const detail::SphereProjector& projector,
                                const SortedRoutes& routes) const {
    using namespace std::literals;
    const size_t MAX_COLOR_INDEX = settings_.color_palette.size() - 1;
    size_t color_index = 0;

    for (const auto bus: routes) {
        if (bus->route.empty()) {
            continue;
        }
        {
            BusLabel label{UniformBusLabel(bus->name, projector(bus->route.front()->point), color_index)};
            ret.Add(label.route_name_underlayer);
            ret.Add(label.route_name);
        }
        if (!bus->is_roundtrip && (bus->route.front() != bus->route.back())) {
            BusLabel label{UniformBusLabel(bus->name, projector(bus->route.back()->point), color_index)};
            ret.Add(label.route_name_underlayer);
            ret.Add(label.route_name);
        }
        if (color_index == MAX_COLOR_INDEX) {
            color_index = 0;
        } else {
            ++color_index;
        }
    }
}

MapRenderer::BusLabel
MapRenderer::UniformBusLabel(const std::string& name, const svg::Point& point, size_t color_index) const {
    using namespace std::literals;
    svg::Text route_name_underlayer;
    svg::Text route_name;
    route_name_underlayer.
            SetPosition(point).
            SetOffset(settings_.bus_label_offset).
            SetFontSize(settings_.bus_label_font_size).
            SetFontFamily("Verdana"s).
            SetFontWeight("bold"s).
            SetFillColor(settings_.underlayer_color).
            SetStrokeColor(settings_.underlayer_color).
            SetStrokeWidth(settings_.underlayer_width).
            SetStrokeLineCap(svg::StrokeLineCap::ROUND).
            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
            SetData(name);
    route_name.
            SetPosition(point).
            SetOffset(settings_.bus_label_offset).
            SetFontSize(settings_.bus_label_font_size).
            SetFontFamily("Verdana"s).
            SetFontWeight("bold"s).
            SetFillColor(settings_.color_palette[color_index]).
            SetData(name);

    return {std::move(route_name_underlayer), std::move(route_name)};
}

void MapRenderer::DrawStops(svg::Document& ret, const detail::SphereProjector& projector,
                            const SortedStops& all_stops) const {
    using namespace std::literals;

    for (const transport_routine::domain::Stop* stop: all_stops) {
        svg::Circle stop_circle;
        stop_circle.
                SetCenter(projector(stop->point)).
                SetRadius(settings_.stop_radius).
                SetFillColor("white"s);
        ret.Add(stop_circle);
    }
}

void MapRenderer::DrawStopLabels(svg::Document& ret, const detail::SphereProjector& projector,
                                 const SortedStops& all_stops) const {
    using namespace std::literals;

    for (const transport_routine::domain::Stop* stop: all_stops) {
        svg::Text stop_label_underlayer;
        svg::Text stop_label;
        stop_label_underlayer.
                SetPosition(projector(stop->point)).
                SetOffset(settings_.stop_label_offset).
                SetFontSize(settings_.stop_label_font_size).
                SetFontFamily("Verdana"s).
                SetFillColor(settings_.underlayer_color).
                SetStrokeColor(settings_.underlayer_color).
                SetStrokeWidth(settings_.underlayer_width).
                SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
                SetData(stop->name);
        stop_label.
                SetPosition(projector(stop->point)).
                SetOffset(settings_.stop_label_offset).
                SetFontSize(settings_.stop_label_font_size).
                SetFontFamily("Verdana"s).
                SetFillColor("black"s).
                SetData(stop->name);
        ret.Add(stop_label_underlayer);
        ret.Add(stop_label);
    }
}

} // namespace map_renderer