// Вставьте сюда решение из предыдущего спринта
#pragma once
#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <map>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

#include "transport_catalogue.h"
#include <sstream>

inline const double EPSILON = 1e-6;
bool IsZero(double value);

namespace render
{
    class SphereProjector {
    public:
        // points_begin Рё points_end Р·Р°РґР°СЋС‚ РЅР°С‡Р°Р»Рѕ Рё РєРѕРЅРµС† РёРЅС‚РµСЂРІР°Р»Р° СЌР»РµРјРµРЅС‚РѕРІ geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Р•СЃР»Рё С‚РѕС‡РєРё РїРѕРІРµСЂС…РЅРѕСЃС‚Рё СЃС„РµСЂС‹ РЅРµ Р·Р°РґР°РЅС‹, РІС‹С‡РёСЃР»СЏС‚СЊ РЅРµС‡РµРіРѕ
            if (points_begin == points_end) {
                return;
            }

            // РќР°С…РѕРґРёРј С‚РѕС‡РєРё СЃ РјРёРЅРёРјР°Р»СЊРЅРѕР№ Рё РјР°РєСЃРёРјР°Р»СЊРЅРѕР№ РґРѕР»РіРѕС‚РѕР№
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // РќР°С…РѕРґРёРј С‚РѕС‡РєРё СЃ РјРёРЅРёРјР°Р»СЊРЅРѕР№ Рё РјР°РєСЃРёРјР°Р»СЊРЅРѕР№ С€РёСЂРѕС‚РѕР№
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Р’С‹С‡РёСЃР»СЏРµРј РєРѕСЌС„С„РёС†РёРµРЅС‚ РјР°СЃС€С‚Р°Р±РёСЂРѕРІР°РЅРёСЏ РІРґРѕР»СЊ РєРѕРѕСЂРґРёРЅР°С‚С‹ x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Р’С‹С‡РёСЃР»СЏРµРј РєРѕСЌС„С„РёС†РёРµРЅС‚ РјР°СЃС€С‚Р°Р±РёСЂРѕРІР°РЅРёСЏ РІРґРѕР»СЊ РєРѕРѕСЂРґРёРЅР°С‚С‹ y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // РљРѕСЌС„С„РёС†РёРµРЅС‚С‹ РјР°СЃС€С‚Р°Р±РёСЂРѕРІР°РЅРёСЏ РїРѕ С€РёСЂРёРЅРµ Рё РІС‹СЃРѕС‚Рµ РЅРµРЅСѓР»РµРІС‹Рµ,
                // Р±РµСЂС‘Рј РјРёРЅРёРјР°Р»СЊРЅС‹Р№ РёР· РЅРёС…
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // РљРѕСЌС„С„РёС†РёРµРЅС‚ РјР°СЃС€С‚Р°Р±РёСЂРѕРІР°РЅРёСЏ РїРѕ С€РёСЂРёРЅРµ РЅРµРЅСѓР»РµРІРѕР№, РёСЃРїРѕР»СЊР·СѓРµРј РµРіРѕ
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // РљРѕСЌС„С„РёС†РёРµРЅС‚ РјР°СЃС€С‚Р°Р±РёСЂРѕРІР°РЅРёСЏ РїРѕ РІС‹СЃРѕС‚Рµ РЅРµРЅСѓР»РµРІРѕР№, РёСЃРїРѕР»СЊР·СѓРµРј РµРіРѕ
                zoom_coeff_ = *height_zoom;
            }
        }

        // РџСЂРѕРµС†РёСЂСѓРµС‚ С€РёСЂРѕС‚Сѓ Рё РґРѕР»РіРѕС‚Сѓ РІ РєРѕРѕСЂРґРёРЅР°С‚С‹ РІРЅСѓС‚СЂРё SVG-РёР·РѕР±СЂР°Р¶РµРЅРёСЏ
        svg::Point operator()(Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    struct RenderSettings {
        double width = 0.0;
        double height = 0.0;
        double padding = 0.0;
        double line_width = 0.0;
        double stop_radius = 0.0; 
        int bus_label_font_size = 0; 
        svg::Point bus_label_offset = { 0.0, 0.0 }; 
        int stop_label_font_size = 0; 
        svg::Point stop_label_offset = { 0.0, 0.0 }; 
        svg::Color underlayer_color = { svg::NoneColor };
        double underlayer_width = 0.0;
        std::vector<svg::Color> color_palette = {};
    };

    class MapRenderer {

    public:
        MapRenderer(const RenderSettings& render_settings) : render_settings_(std::move(render_settings)) {}

        std::vector<svg::Polyline> GetLines(const std::map<std::string_view, const domain::Bus*>&, const SphereProjector&) const;
        
        std::vector<svg::Text> GetBusNames(const std::map<std::string_view, const domain::Bus*>& buses, const SphereProjector& points) const;
        
        std::vector<svg::Circle> GetCircles(const std::map<std::string_view, const domain::Stop*>& stops, const SphereProjector& points) const;
        
        std::vector<svg::Text> GetStopNames(const std::map<std::string_view, const domain::Stop*>& stops, const SphereProjector& points) const;
        
        svg::Document Render(const std::map<std::string_view, const domain::Bus*>& buses, const std::map<std::string_view, const domain::Stop*>& stops) const;
        
        std::ostringstream PrintMap(catalogue::TransportCatalogue& db_);
    private:
        const RenderSettings render_settings_;
    };
}