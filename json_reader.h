// Вставьте сюда решение из предыдущего спринта#pragma once

#include <istream>
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"

namespace reader
{
	class JSONReader
	{
	public:
		JSONReader(std::istream& input) : doc_(json::Load(input)) {}

		void FillCatalogue(catalogue::TransportCatalogue& catalogue);
		
		render::RenderSettings GetSettings();
        graph::RoutingSettings GetRouterSettings();
        
		const json::Node& GetBaseRequests() const;
		const json::Node& GetStatRequests() const;
		const json::Node JSONGetBusInfo(const json::Dict& request_map, catalogue::TransportCatalogue& db_);
		const json::Node JSONGetStopInfo(const json::Dict& request_map, catalogue::TransportCatalogue& db_);
		const json::Node JSONGetMapInfo(const json::Dict& request_map, catalogue::TransportCatalogue& db_, render::MapRenderer& render);
        const json::Node JSONGetRoteInfo(const json::Dict& request_map, graph::TransportRouter& transport_router);
        
	private:
		json::Document doc_;
		json::Node nothing_ = nullptr;
		static void AddDistancesToStop(catalogue::TransportCatalogue&, std::vector<json::Dict>&);
		static std::pair<std::vector<json::Dict>, std::vector<json::Dict>> FillStops(catalogue::TransportCatalogue&, json::Array&);
		static void FillBuses(catalogue::TransportCatalogue&, std::vector<json::Dict>&);
        
		const json::Node& GetRenderSettings() const;
        const json::Node& GetRoutingSettings() const;
	};
}