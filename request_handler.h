// Вставьте сюда решение из предыдущего спринта
#pragma once
#include <fstream>
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

class RequestHandler
{
public:
    RequestHandler(render::MapRenderer& renderer, catalogue::TransportCatalogue& db, graph::TransportRouter& transport_router, reader::JSONReader& requests) : renderer_(renderer), db_(db), transport_router_(transport_router), requests_(requests){}

    
    void PrintJSON() const;
    

private:
    render::MapRenderer& renderer_;
    catalogue::TransportCatalogue& db_;
    graph::TransportRouter& transport_router_;
    reader::JSONReader& requests_;
    
};