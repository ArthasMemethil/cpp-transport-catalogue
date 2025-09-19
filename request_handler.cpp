// Вставьте сюда решение из предыдущего спринта
#include "request_handler.h"

void RequestHandler::PrintJSON() const
{
	json::Array result;
	const json::Array& arr = requests_.GetStatRequests().AsArray();
	for (auto& request : arr) {
		const auto& request_map = request.AsDict();
		const auto& type = request_map.at("type").AsString();
		if (type == "Stop") {
			result.push_back(requests_.JSONGetStopInfo(request_map, db_).AsDict());
		}
		if (type == "Bus") {
			result.push_back(requests_.JSONGetBusInfo(request_map, db_).AsDict());
		}
        if (type == "Map") {
            result.push_back(requests_.JSONGetMapInfo(request_map, db_, renderer_).AsDict());
        } 
        if (type == "Route"){
             result.push_back(requests_.JSONGetRoteInfo(request_map, transport_router_).AsDict());
        }   
	}
	std::fstream out("out.txt");
	json::Print(json::Document{ result }, out);
}