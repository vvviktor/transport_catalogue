#pragma once

#include <deque>
#include <set>
#include <algorithm>
#include <optional>
#include <unordered_set>
#include <iostream>
#include <string>

#include "svg.h"
#include "domain.h"
#include "geo.h"

namespace map_renderer {

using SortedRoutes = std::set<const transport_routine::domain::Bus*, transport_routine::domain::BusPtrComparatorLess>;
using SortedStops = std::set<const transport_routine::domain::Stop*, transport_routine::domain::StopPtrComparatorLess>;

namespace detail {

inline const double EPSILON = 1e-6;

bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template<typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding);

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

template<typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
                                 double max_height, double padding) : padding_(padding) //
{
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
        return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!detail::IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!detail::IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}

} // namespace detail

struct RenderSettings {
    double width = .0;
    double height = .0;
    double padding = .0;
    double line_width = .0;
    double stop_radius = .0;
    int bus_label_font_size = 0;
    svg::Point bus_label_offset;
    int stop_label_font_size = 0;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width = .0;
    std::deque<svg::Color> color_palette;
};

class MapRenderer {
public:
    MapRenderer() = default;

    explicit MapRenderer(RenderSettings settings);

    svg::Document RenderRouteMap(const SortedRoutes& routes) const;

    void SetRenderSettings(RenderSettings render_settings);

    const RenderSettings& GetRenderSettings() const;

private:
    RenderSettings settings_;

    struct BusLabel {
        svg::Text route_name_underlayer;
        svg::Text route_name;
    };

    void DrawLines(svg::Document& ret, const detail::SphereProjector& projector, const SortedRoutes& routes) const;

    void DrawBusLabels(svg::Document& ret, const detail::SphereProjector& projector, const SortedRoutes& routes) const;

    BusLabel UniformBusLabel(const std::string& name, const svg::Point&, size_t color_index) const;

    void DrawStops(svg::Document& ret, const detail::SphereProjector& projector, const SortedStops& all_stops) const;

    void DrawStopLabels(svg::Document& ret, const detail::SphereProjector& projector,
                        const SortedStops& all_stops) const;
};

} // namespace map_renderer

