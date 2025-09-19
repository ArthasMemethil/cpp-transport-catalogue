// Вставьте сюда решение из предыдущего спринта
#pragma once

#include <string>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <iomanip>
#include <vector>
#include <functional>
#include <optional>

#include "domain.h"

namespace catalogue
{
	class TransportCatalogue {
	public:
		void AddStop(std::string_view, domain::Geo::Coordinates); 
		void AddBus(std::string_view, const std::vector<std::string_view>&, bool round); 
		void SetDistanceStops(const domain::Stop*, const domain::Stop*, int);
		const domain::Stop* GetStop(std::string);
		const domain::Bus* GetBus(std::string_view); 
		int GetDistanceStops(const domain::Stop*, const domain::Stop*); 
		const std::map<std::string_view, const domain::Bus*> GetSortedBuses() const;
        const std::map<std::string_view, const domain::Stop*> GetSortedStops() const;
		std::optional<domain::BusInfo> GetBusInfo(std::string_view);
		std::vector <std::string_view> GetBusesToStop(std::string_view);
		std::deque<domain::Stop> GetDeqStops() const;
		std::deque<domain::Bus> GetDeqBuses() const;
        
	private:
		std::deque<domain::Stop> stops_; 
		std::deque<domain::Bus> buses_;	
		
		std::unordered_map<std::string_view, domain::Stop*, domain::hasher::StrView> stopname_to_stop_;
		std::unordered_map<std::string_view, domain::Bus*, domain::hasher::StrView> busname_to_bus_;
		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, domain::hasher::StopsDistancePair> distance_stops_; 
	};
} //namespace catalogue