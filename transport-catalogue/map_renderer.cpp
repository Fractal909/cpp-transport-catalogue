#include "map_renderer.h"

void MapRenderer::SetSettings(const RenderSettings& settings) {
    settings_ = settings;
}

void MapRenderer::RenderMap(const catalogue::TransportCatalogue& catalogue, std::ostream& out) const {

    std::vector<const Bus*> buses;
    for (const auto& bus : catalogue.GetBuses()) {
        if (!bus->stops.empty()) {
            buses.push_back(bus);
        }
    }
    std::sort(buses.begin(), buses.end(), [](const Bus* lhs, const Bus* rhs) {
        return lhs->name < rhs->name;
        });


    auto color_iter = settings_.color_palette.begin();
    svg::Document document;

    std::vector<geo::Coordinates> points_to_proj;
    for (const auto& bus_ptr : buses) {
        for (const auto& stop_ptr : bus_ptr->stops) {
            points_to_proj.push_back(stop_ptr->coords);
        }
    }
    geo::SphereProjector projector(points_to_proj.begin(), points_to_proj.end(), settings_.width, settings_.height, settings_.padding);


    RenderLines(buses, color_iter, document, projector);

    RenderNames(buses, color_iter, document, projector);


    std::map<std::string_view, const Stop*> stops;
    for (const auto& bus : buses) {
        for (const auto& stop : bus->stops) {
            stops.insert({ stop->name, stop });
        }
    }

    RenderStopsCircles(stops, document, projector);

    RenderStopsNames(stops, document, projector);

    document.Render(out);
}

void MapRenderer::RenderLines(const std::vector<const Bus*> buses, Color_Iterator color_iter, svg::Document& doc, geo::SphereProjector& proj) const {

    for (const auto& bus : buses) {
        svg::Polyline line;
        line.SetStrokeColor(*color_iter);
        line.SetFillColor("none");
        line.SetStrokeWidth(settings_.line_width);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (const auto stop : bus->stops) {
            line.AddPoint(proj(stop->coords));
        }
        doc.Add(line);

        ++color_iter;
        if (color_iter == settings_.color_palette.end()) {
            color_iter = settings_.color_palette.begin();
        }
    }
    color_iter = settings_.color_palette.begin();
}

void MapRenderer::RenderNames(const std::vector<const Bus*> buses, Color_Iterator color_iter, svg::Document& doc, geo::SphereProjector& proj) const {

    for (const auto& bus : buses) {

        svg::Text text_f;
        text_f.SetPosition(proj(bus->stops.at(0)->coords));
        text_f.SetOffset(settings_.bus_label_offset);
        text_f.SetFontSize(settings_.bus_label_font_size);
        text_f.SetFontFamily("Verdana");
        text_f.SetFontWeight("bold");
        text_f.SetData(bus->name);
        text_f.SetFillColor(*color_iter);

        svg::Text text_f_h = text_f;
        text_f_h.SetFillColor(settings_.underlayer_color);
        text_f_h.SetStrokeColor(settings_.underlayer_color);
        text_f_h.SetStrokeWidth(settings_.underlayer_width);
        text_f_h.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        text_f_h.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        doc.Add(text_f_h);
        doc.Add(text_f);

        if (!bus->is_roundtrip && (bus->stops.front()) != (bus->stops.at(bus->stops.size() / 2))) {
            svg::Text text_l = text_f;
            svg::Text text_l_h = text_f_h;
            svg::Point coords = proj(bus->stops.at(bus->stops.size() / 2)->coords);
            text_l.SetPosition(coords);
            text_l_h.SetPosition(coords);

            doc.Add(text_l_h);
            doc.Add(text_l);
        }

        ++color_iter;
        if (color_iter == settings_.color_palette.end()) {
            color_iter = settings_.color_palette.begin();
        }
    }
    color_iter = settings_.color_palette.begin();
}

void MapRenderer::RenderStopsCircles(const std::map<std::string_view, const Stop*>& stops, svg::Document& doc, geo::SphereProjector& proj) const {
    for (const auto& stop : stops) {
        doc.Add(svg::Circle().SetCenter(proj(stop.second->coords)).SetRadius(settings_.stop_radius).SetFillColor("white"));
    }
}

void MapRenderer::RenderStopsNames(const std::map<std::string_view, const Stop*>& stops, svg::Document& doc, geo::SphereProjector& proj) const {
    for (const auto& stop : stops) {
        svg::Text name;
        name.SetPosition(proj(stop.second->coords));
        name.SetOffset(settings_.stop_label_offset);
        name.SetFontSize(settings_.stop_label_font_size);
        name.SetFontFamily("Verdana");
        name.SetData(stop.second->name);
        name.SetFillColor("black");

        svg::Text hedge = name;
        hedge.SetFillColor(settings_.underlayer_color);
        hedge.SetStrokeColor(settings_.underlayer_color);
        hedge.SetStrokeWidth(settings_.underlayer_width);
        hedge.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        hedge.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        doc.Add(hedge);
        doc.Add(name);
    }
}