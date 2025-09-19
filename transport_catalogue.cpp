// Вставьте сюда решение из предыдущего спринта
#include "transport_catalogue.h"

void catalogue::TransportCatalogue::AddStop(std::string_view name, domain::Geo::Coordinates coords)
{
	stops_.push_back({ std::string(name), std::move(coords) });
	domain::Stop* cur_elem = &stops_[stops_.size() - 1];
	stopname_to_stop_[cur_elem->name] = std::move(cur_elem);
}

void catalogue::TransportCatalogue::AddBus(std::string_view name, const std::vector<std::string_view>& str_route, bool round)
{
	std::vector<domain::Stop*> route;

	for (size_t i = 0; i < str_route.size(); ++i)
	{
		route.push_back(stopname_to_stop_[str_route[i]]);
	}

	buses_.push_back({ std::string(name), std::move(route),  round});
	domain::Bus* cur_elem = &buses_[buses_.size() - 1];
	for (size_t i = 0; i < str_route.size(); ++i)
	{
		stopname_to_stop_[str_route[i]]->buses_to_stop_.insert(cur_elem->name);
	}
	busname_to_bus_[cur_elem->name] = std::move(cur_elem);
}

void catalogue::TransportCatalogue::SetDistanceStops(const domain::Stop* from, const domain::Stop* to, int distance)
{
	distance_stops_[{from, to}] = distance;
}

const std::map<std::string_view, const domain::Bus*> catalogue::TransportCatalogue::GetSortedBuses() const {
	std::map < std::string_view, const domain::Bus* > result;
	for (const auto& bus : busname_to_bus_) {
		result.emplace(bus);
	}
	return result;
}

const std::map<std::string_view, const domain::Stop*> catalogue::TransportCatalogue::GetSortedStops() const 
{
	std::map <std::string_view, const domain::Stop*> result;
	for (const auto& stop : stopname_to_stop_) {
		result.emplace(stop);
	}
	return result;
}

const domain::Stop* catalogue::TransportCatalogue::GetStop(std::string stopname) 
{
	return stopname_to_stop_[stopname];
}

const domain::Bus* catalogue::TransportCatalogue::GetBus(std::string_view busname) 
{
	return busname_to_bus_[busname];
}

int catalogue::TransportCatalogue::GetDistanceStops(const domain::Stop* from, const domain::Stop* to) 
{
	return distance_stops_[{from, to}];
}

std::optional<domain::BusInfo> catalogue::TransportCatalogue::GetBusInfo(std::string_view busname)
{
	std::optional<domain::BusInfo> result;
	if (GetBus(busname) != nullptr)
	{
		auto busname_to_bus = GetBus(busname);
		size_t stops_count = busname_to_bus->route.size();
		std::unordered_set<domain::Stop*> unique_stops;
		double curvature = 0.0;
		double total_geo_distance = 0.0;
		double total_metres_distance = 0;
		domain::Geo::Coordinates from = { 0.0, 0.0 };
		domain::Geo::Coordinates to = { 0.0, 0.0 };

		for (size_t i = 0; i < busname_to_bus->route.size(); ++i)
		{
			unique_stops.insert(busname_to_bus->route[i]);
			from = busname_to_bus->route[i]->coordinates;
			if (i + 1 < busname_to_bus->route.size())
			{
				to = busname_to_bus->route[i + 1]->coordinates;
				total_geo_distance += domain::Geo::ComputeDistance(from, to);
				if (GetDistanceStops(busname_to_bus->route[i], busname_to_bus->route[i + 1]))
					total_metres_distance += GetDistanceStops(busname_to_bus->route[i], busname_to_bus->route[i + 1]);
				else
					total_metres_distance += GetDistanceStops(busname_to_bus->route[i + 1], busname_to_bus->route[i]);
			}
		}

		curvature = total_metres_distance / total_geo_distance;
		size_t unique_stops_count = unique_stops.size();
		result = { std::move(stops_count), std::move(unique_stops_count), std::move(total_metres_distance), std::move(curvature) };
	}

	return result;
}

std::vector <std::string_view> catalogue::TransportCatalogue::GetBusesToStop(std::string_view stopname)
{
	std::vector <std::string_view> result;
	if (GetStop(std::string(stopname)) != nullptr)
	{
		const domain::Stop* stopname_to_stop = GetStop(std::string(stopname));
		if (stopname_to_stop->buses_to_stop_.empty())
			return result;
		else
		{
			for (auto& bus : stopname_to_stop->buses_to_stop_)
			{
				result.push_back(bus);
			}
		}
	}
	else
		result.push_back("not found");

	return result;
}

std::deque<domain::Stop> catalogue::TransportCatalogue::GetDeqStops() const{
	return stops_;
}

std::deque<domain::Bus> catalogue::TransportCatalogue::GetDeqBuses() const{
	return buses_;
}