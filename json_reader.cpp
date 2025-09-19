// Вставьте сюда решение из предыдущего спринта
#include <variant>
#include "json_reader.h"

const json::Node& reader::JSONReader::GetBaseRequests() const {
	const auto it = doc_.GetRoot().AsDict().find("base_requests");
	if (it == doc_.GetRoot().AsDict().end())
		return nothing_;
	return (*it).second;
}

const json::Node& reader::JSONReader::GetStatRequests() const {
	const auto it = doc_.GetRoot().AsDict().find("stat_requests");
	if (it == doc_.GetRoot().AsDict().end())
		return nothing_;
	return (*it).second;
}

const json::Node& reader::JSONReader::GetRenderSettings() const
{
	const auto it = doc_.GetRoot().AsDict().find("render_settings");
	if (it == doc_.GetRoot().AsDict().end())
		return nothing_;
	return (*it).second;
}

const json::Node& reader::JSONReader::GetRoutingSettings() const
{
	const auto it = doc_.GetRoot().AsDict().find("routing_settings");
	if (it == doc_.GetRoot().AsDict().end())
		return nothing_;
	return (*it).second;
}

void reader::JSONReader::FillCatalogue(catalogue::TransportCatalogue& catalogue)
{
	auto base_requests_arr = GetBaseRequests().AsArray();
	std::pair<std::vector<json::Dict>, std::vector<json::Dict>> stops_and_buses_requests;
	stops_and_buses_requests = reader::JSONReader::FillStops(catalogue, base_requests_arr);
	reader::JSONReader::AddDistancesToStop(catalogue, stops_and_buses_requests.first);
	reader::JSONReader::FillBuses(catalogue, stops_and_buses_requests.second);

}

svg::Color GetUnderlayerColor(json::Dict render_settings) {
	if (render_settings.at("underlayer_color").IsString())
		return  render_settings.at("underlayer_color").AsString();
	else
	{
		if (render_settings.at("underlayer_color").AsArray().size() == 3)
		{
			auto underlayer_color = render_settings.at("underlayer_color").AsArray();
			return  svg::Rgb({
				static_cast<uint8_t>(underlayer_color[0].AsInt()),
				static_cast<uint8_t>(underlayer_color[1].AsInt()),
				static_cast<uint8_t>(underlayer_color[2].AsInt()) });
		}
		else if (render_settings.at("underlayer_color").AsArray().size() == 4)
		{
			auto underlayer_color = render_settings.at("underlayer_color").AsArray();
			return  svg::Rgba({
				static_cast<uint8_t>(underlayer_color[0].AsInt()),
				static_cast<uint8_t>(underlayer_color[1].AsInt()),
				static_cast<uint8_t>(underlayer_color[2].AsInt()),
				underlayer_color[3].AsDouble() });
		}
		else
		{
			throw std::logic_error("wrong color type");
		}
	}
}
std::vector<svg::Color> GetColorPalette(json::Dict render_settings) {
	std::vector<svg::Color> result;
	auto color_palette = render_settings.at("color_palette").AsArray();
	for (const auto& color : color_palette)
	{
		if (color.IsString())
			result.emplace_back(color.AsString());
		else
		{
			if (color.AsArray().size() == 3)
			{
				result.emplace_back(svg::Rgb({
					static_cast<uint8_t>(color.AsArray()[0].AsInt()),
					static_cast<uint8_t>(color.AsArray()[1].AsInt()),
					static_cast<uint8_t>(color.AsArray()[2].AsInt()) }));
			}
			else if (color.AsArray().size() == 4)
			{
				result.emplace_back(svg::Rgba({
					static_cast<uint8_t>(color.AsArray()[0].AsInt()),
					static_cast<uint8_t>(color.AsArray()[1].AsInt()),
					static_cast<uint8_t>(color.AsArray()[2].AsInt()),
					color.AsArray()[3].AsDouble() }));
			}
			else
				throw std::logic_error("wrong color type");
		}
	}
	return result;
}


render::RenderSettings reader::JSONReader::GetSettings()
{
	render::RenderSettings result;
	auto render_settings = GetRenderSettings().AsDict();
	result.width = render_settings.at("width").AsDouble();
	result.height = render_settings.at("height").AsDouble();
	result.padding = render_settings.at("padding").AsDouble();
	result.line_width = render_settings.at("line_width").AsDouble();
	result.stop_radius = render_settings.at("stop_radius").AsDouble();
	result.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
	auto bus_label_offset = render_settings.at("bus_label_offset").AsArray();
	result.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };
	result.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
	auto stop_label_offset = render_settings.at("stop_label_offset").AsArray();
	result.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };
	result.underlayer_width = render_settings.at("underlayer_width").AsDouble();
	result.underlayer_color = GetUnderlayerColor(render_settings);
	result.color_palette = GetColorPalette(render_settings);
	
	return result;
}

graph::RoutingSettings reader::JSONReader::GetRouterSettings()
{
	graph::RoutingSettings result;
	auto routing_settings = GetRoutingSettings().AsDict();
    result.bus_wait_time = routing_settings.at("bus_wait_time").AsInt();
	result.bus_velocity = routing_settings.at("bus_velocity").AsDouble();

	return result;
}

void reader::JSONReader::AddDistancesToStop(catalogue::TransportCatalogue& catalogue, std::vector<json::Dict>& data)
{
	for (size_t i = 0; i < data.size(); ++i)
	{
		bool is_empty = false;
		const domain::Stop* from = catalogue.GetStop(data[i].at("name").AsString());
		json::Dict distances;
		try
		{
			distances = data[i].at("road_distances").AsDict();
		}
		catch (...)
		{
			is_empty = true;
		}

		for (const auto& [stopname, distance] : distances)
		{
			if (is_empty)
				break;
			const domain::Stop* to = catalogue.GetStop(stopname);
			int metres = distance.AsInt();
			catalogue.SetDistanceStops(from, to, metres);
		}
	}
}

void reader::JSONReader::FillBuses(catalogue::TransportCatalogue& catalogue, std::vector<json::Dict>& buses_requests)
{
	for (size_t i = 0; i < buses_requests.size(); ++i)
	{
		auto stops = buses_requests[i].at("stops").AsArray();
		std::vector<std::string_view> route;
		if (buses_requests[i].at("is_roundtrip").AsBool())
		{
			for (size_t j = 0; j < stops.size(); ++j)
			{
				route.push_back(stops[j].AsString());
			}
			catalogue.AddBus(buses_requests[i].at("name").AsString(), route, buses_requests[i].at("is_roundtrip").AsBool());
		}
		else
		{
			route.resize(stops.size() * 2 - 1);
			for (size_t j = 0; j < stops.size(); ++j)
			{
				route[j] = stops[j].AsString();
				if (j < route.size() / 2)
					route[route.size() - j - 1] = stops[j].AsString();
			}
			catalogue.AddBus(buses_requests[i].at("name").AsString(), route, buses_requests[i].at("is_roundtrip").AsBool());
		}
	}
}

std::pair<std::vector<json::Dict>, std::vector<json::Dict>> reader::JSONReader::FillStops(catalogue::TransportCatalogue& catalogue, json::Array& all_base_data)
{
	std::vector<json::Dict> stops_requests;
	std::vector<json::Dict> buses_requests;
	for (size_t i = 0; i < all_base_data.size(); ++i)
	{
		auto base_data = all_base_data[i].AsDict();
		if (base_data["type"].AsString() == "Stop")
		{
			catalogue.AddStop(base_data.at("name").AsString(), { base_data.at("latitude").AsDouble(), base_data.at("longitude").AsDouble() });
			stops_requests.push_back(std::move(base_data));
		}
		else
			buses_requests.push_back(std::move(base_data));
	}
	return { stops_requests, buses_requests };
}

const json::Node reader::JSONReader::JSONGetBusInfo(const json::Dict& request_map, catalogue::TransportCatalogue& db) {
	json::Node result;
	const std::string& route_number = request_map.at("name").AsString();
	const int id = request_map.at("id").AsInt();

	if (db.GetBus(route_number) == nullptr) {
		result = json::Builder{}.
			StartDict().
			Key("error_message").Value("not found").
			Key("request_id").Value(id).
			EndDict().
			Build();
	}
	else {
		result = json::Builder{}.StartDict().
			Key("request_id").Value(id).
			Key("curvature").Value(db.GetBusInfo(route_number)->curvature).
			Key("route_length").Value(db.GetBusInfo(route_number)->total_metres_distance).
			Key("stop_count").Value(static_cast<int>(db.GetBusInfo(route_number)->stops_count)).
			Key("unique_stop_count").Value(static_cast<int>(db.GetBusInfo(route_number)->unique_stops_count)).
			EndDict().
			Build();
	}
	return result;
	return json::Node{ result };

}


const json::Node reader::JSONReader::JSONGetStopInfo(const json::Dict& request_map, catalogue::TransportCatalogue& db) {
	json::Node result;
	const std::string& stopname = request_map.at("name").AsString();
	const int id = request_map.at("id").AsInt();

	if (db.GetStop(stopname) == nullptr) {
		result = json::Builder{}.StartDict().
			Key("request_id").Value(id).
			Key("error_message").Value("not found").
			EndDict().
			Build();
	}
	else {
		json::Array buses;
		for (auto& bus : db.GetBusesToStop(stopname)) {
			buses.push_back(std::string(bus));
		}
		result = json::Builder{}.StartDict().
			Key("request_id").Value(id).
			Key("buses").Value(buses).
			EndDict().
			Build();
	}
	return result;
}
const json::Node reader::JSONReader::JSONGetMapInfo(const json::Dict& request_map, catalogue::TransportCatalogue& db, render::MapRenderer& render) {
	json::Node result;

	result = json::Builder{}.StartDict().
		Key("request_id").Value(request_map.at("id").AsInt()).
		Key("map").Value(render.PrintMap(db).str()).
		EndDict().
		Build();
	return result;
}

const json::Node reader::JSONReader::JSONGetRoteInfo(const json::Dict& request_map, graph::TransportRouter& transport_router){
    json::Node result;
	
    const int id = request_map.at("id").AsInt();

    const std::string& stopname_from = request_map.at("from").AsString();
    const std::string& stopname_to = request_map.at("to").AsString();

	if (stopname_from == stopname_to) {
		json::Array ara;
		auto js = json::Builder{}.StartDict().
			Key("items").Value(ara).
			Key("request_id").Value(id).
			Key("total_time").Value(0).EndDict().Build();
		result = js;
		return js;

	}
	
	std::optional<std::pair<std::vector<graph::Edge<double>>, double>> info = transport_router.GetRouteInfo(stopname_from, stopname_to);

    if(info == std::nullopt ){
		result = json::Builder{}.StartDict().
			Key("request_id").Value(id).
			Key("error_message").Value("not found").
			EndDict().
			Build();
        
	} else {
		json::Array ara;

		for (auto edge: info.value().first) {

			if (edge.bus_name != "Wait") {
				ara.emplace_back(json::Builder{}.StartDict().
					Key("bus").Value(edge.bus_name).
					Key("span_count").Value(edge.stop_count).
					Key("time").Value(edge.weight).
					Key("type").Value("Bus").
					EndDict().Build());

			}
			else {

					ara.emplace_back(json::Builder{}.StartDict().
						Key("type").Value("Wait").
						Key("stop_name").Value(edge.stop_name).
						Key("time").Value(edge.weight).
						EndDict().Build());
				
			}


			auto js = json::Builder{}.StartDict().
				Key("items").Value(ara).
				Key("request_id").Value(id).
				Key("total_time").Value(info.value().second).EndDict().Build();
			result = js;

		}

	}
    return result;
}

