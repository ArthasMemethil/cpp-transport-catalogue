// Вставьте сюда решение из предыдущего спринта
#include "map_renderer.h"

svg::Point render::SphereProjector::operator()(Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Document render::MapRenderer::Render(const std::map<std::string_view, const domain::Bus*>& buses, const std::map<std::string_view, const domain::Stop*>& stops) const
{
    svg::Document result;
    std::vector<domain::Geo::Coordinates> route_stops_coord;
    for (const auto& [bus_number, bus] : buses) {
        if (bus==nullptr) {
            continue;
        }

        for (const auto& stop : bus->route) {
            route_stops_coord.push_back(stop->coordinates);
        }
    }
    SphereProjector sp(route_stops_coord.begin(), route_stops_coord.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
    for (const auto& line : GetLines(buses, sp)) {
        result.Add(line);
    }
    for (const auto& text : GetBusNames(buses, sp)) {
        result.Add(text);
    }
    for (const auto& circle : GetCircles(stops, sp)) {
        result.Add(circle);
    }
    for (const auto& stop : GetStopNames(stops, sp)) {
        result.Add(stop);
    }
    
    return result;
}

std::vector<svg::Polyline> render::MapRenderer::GetLines(const std::map<std::string_view, const domain::Bus*>& buses, const SphereProjector& points) const
{
    std::vector<svg::Polyline> result;

    int color_variations = 0;
    for (const auto& bus : buses)
    {
        
        if (bus.second == nullptr) {
            continue;
        }

        if (bus.second->route.empty())
            continue;

        svg::Polyline line;
        for (size_t i = 0; i < bus.second->route.size(); ++i)
        {
            line.AddPoint(points(bus.second->route[i]->coordinates));
        }
        line.SetFillColor("none");
        line.SetStrokeWidth(render_settings_.line_width);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        line.SetStrokeColor(render_settings_.color_palette[color_variations++]);

        if (static_cast<uint8_t>(color_variations) == render_settings_.color_palette.size())
            color_variations = 0;

        result.push_back(line);
    }
    return result;
}

std::vector<svg::Text> render::MapRenderer::GetBusNames(const std::map<std::string_view, const domain::Bus*>& buses, const SphereProjector& points) const{
    std::vector<svg::Text> result;
    int color_variations = 0;
    for (const auto& bus : buses)
    {
        if (bus.second == nullptr) {
            continue;
        }

        if (bus.second->route.empty())
            continue;

        svg::Text underlayer;
        svg::Text route_name;

        if (bus.second->is_roundtrip || bus.second->route[0] == bus.second->route[bus.second->route.size() >> 1])
        {
            underlayer.SetPosition(points(bus.second->route[0]->coordinates));
            underlayer.SetOffset(render_settings_.bus_label_offset);
            underlayer.SetFontSize(render_settings_.bus_label_font_size);
            underlayer.SetFontFamily("Verdana");
            underlayer.SetFontWeight("bold");
            underlayer.SetData(bus.second->name);
            underlayer.SetFillColor(render_settings_.underlayer_color);
            underlayer.SetStrokeColor(render_settings_.underlayer_color);
            underlayer.SetStrokeWidth(render_settings_.underlayer_width);
            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);


            route_name.SetPosition(points(bus.second->route[0]->coordinates));
            route_name.SetOffset(render_settings_.bus_label_offset);
            route_name.SetFontSize(render_settings_.bus_label_font_size);
            route_name.SetFontFamily("Verdana");
            route_name.SetFontWeight("bold");
            route_name.SetData(bus.second->name);
            route_name.SetFillColor(render_settings_.color_palette[color_variations++]);


            result.push_back(std::move(underlayer));
            result.push_back(std::move(route_name));
        }
        else
        {
            int first_and_midlle_stop = 0;
            for (int i = 0; i < 2; ++i)
            {

                underlayer.SetPosition(points(bus.second->route[first_and_midlle_stop]->coordinates));
                underlayer.SetOffset(render_settings_.bus_label_offset);
                underlayer.SetFillColor(render_settings_.underlayer_color);
                underlayer.SetStrokeColor(render_settings_.underlayer_color);
                underlayer.SetStrokeWidth(render_settings_.underlayer_width);
                underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                underlayer.SetFontSize(render_settings_.bus_label_font_size);
                underlayer.SetFontFamily("Verdana");
                underlayer.SetFontWeight("bold");
                underlayer.SetData(bus.second->name);

                route_name.SetPosition(points(bus.second->route[first_and_midlle_stop]->coordinates));
                route_name.SetFillColor(render_settings_.color_palette[color_variations]);
                route_name.SetOffset(render_settings_.bus_label_offset);
                route_name.SetFontSize(render_settings_.bus_label_font_size);
                route_name.SetFontFamily("Verdana");
                route_name.SetFontWeight("bold");
                route_name.SetData(bus.second->name);

                first_and_midlle_stop = static_cast<int>(bus.second->route.size() >> 1);
                result.push_back(underlayer);
                result.push_back(route_name);
            }
            ++color_variations;
        }

        if (color_variations == static_cast<int>(render_settings_.color_palette.size()))
            color_variations = 0;

    }
    return result;
}

std::vector<svg::Circle> render::MapRenderer::GetCircles(const std::map<std::string_view, const domain::Stop*>& stops, const SphereProjector& points) const{
    std::vector<svg::Circle> result;       
    for(const auto& stop : stops){

        if (stop.second == nullptr)
            continue;

        if (stop.second->buses_to_stop_.empty())
            continue;
        svg::Circle circle;
        circle.SetCenter(points(stop.second->coordinates)).
        SetRadius(render_settings_.stop_radius).
        SetFillColor("white");
        result.push_back(circle);
        }
    return result;
}

std::vector<svg::Text> render::MapRenderer::GetStopNames(const std::map<std::string_view, const domain::Stop*>& stops, const SphereProjector& points) const{
    std::vector<svg::Text> result;       
    for(const auto& stop : stops){

        if (stop.second == nullptr)
            continue;

        if (stop.second->buses_to_stop_.empty())
                continue;

        //Общие характеристики
            svg::Text text_under;

            text_under.SetPosition(points(stop.second->coordinates));
            text_under.SetOffset(render_settings_.stop_label_offset);
            text_under.SetFontSize(render_settings_.stop_label_font_size);
            text_under.SetFontFamily("Verdana");
            text_under.SetData(stop.second->name);
            
            svg::Text text_above = text_under;
            //Особенности подложки
            text_under.SetFillColor(render_settings_.underlayer_color);
            text_under.SetStrokeColor(render_settings_.underlayer_color);
            text_under.SetStrokeWidth(render_settings_.underlayer_width);
            text_under.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            text_under.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            //Особенности надписи
            text_above.SetFillColor("black");
            
            result.push_back(text_under);
            result.push_back(text_above);
    }
    return result;
}

std::ostringstream render::MapRenderer::PrintMap(catalogue::TransportCatalogue& db)
{
    std::ostringstream stream;
    auto map = Render(db.GetSortedBuses(), db.GetSortedStops());
    map.Render(stream);
    return stream;
}