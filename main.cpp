// Вставьте сюда решение из предыдущего спринта
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <chrono>
#include <sstream>
#include <string_view>
#include <fstream>
#include "map_renderer.h"
#include "request_handler.h"



int main(){
	std::fstream in("input3.txt");

    catalogue::TransportCatalogue catalogue;
	reader::JSONReader input(in);
	input.FillCatalogue(catalogue);
	render::MapRenderer map(input.GetSettings());
	graph::RoutingSettings r_settings = input.GetRouterSettings();
    graph::TransportRouter transpot_router(r_settings, catalogue);
	RequestHandler output(map, catalogue, transpot_router, input);
	output.PrintJSON();

	 
}