#pragma once

#include "svg.h"
#include "transport_catalogue.h"
#include "geo.h"

#include <array>
#include <vector>
#include <algorithm>
#include <string>
#include <set>
#include <map>

struct RenderSettings {
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;

    int bus_label_font_size;
    svg::Point bus_label_offset;
    int stop_label_font_size;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};

using Color_Iterator = std::vector<svg::Color>::const_iterator;

class MapRenderer {
public:
    void SetSettings(const RenderSettings& settings);
    void RenderMap(const catalogue::TransportCatalogue& catalogue, std::ostream& out) const;
private:
    void RenderLines(const std::vector<const Bus*> buses, Color_Iterator color_iter, svg::Document& doc, geo::SphereProjector& proj) const;
    void RenderNames(const std::vector<const Bus*> buses, Color_Iterator color_iter, svg::Document& doc, geo::SphereProjector& proj) const;
    void RenderStopsCircles(const std::map<std::string_view, const Stop*>& stops, svg::Document& doc, geo::SphereProjector& proj) const;
    void RenderStopsNames(const std::map<std::string_view, const Stop*>& stops, svg::Document& doc, geo::SphereProjector& proj) const;

    RenderSettings settings_;
};