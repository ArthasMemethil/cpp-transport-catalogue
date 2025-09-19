
#include "transport_router.h"


graph::Router<double> graph::TransportRouter::BuildRouter() {

    
    double speed = routing_settings_.bus_velocity * 1000 / 60;
    GraphAddStops();
    

    const auto& buses = db_.GetDeqBuses();
    for (const auto& bus : buses) {
        const auto& bus_route = bus.route;
        auto range = ranges::AsRange(bus_route);
        if (bus.is_roundtrip) { 
            AddCirculeBuses(range, speed, bus.name);
            } else {
            AddStraghtBuses(range, speed, bus.name);
            }
        }

    graph::Router<double> result(graph_);

    return result;
}

graph::Edge<double> graph::TransportRouter::GetEdge(graph::EdgeId edge_id) const {
    return graph_.GetEdge(edge_id);
}

   std::optional<std::pair<std::vector<graph::Edge<double>>, double>> graph::TransportRouter::GetRouteInfo(const std::string& from, const std::string& to) const{
       
            std::optional<std::pair<std::vector<graph::Edge<double>>, double>> result;


            const auto& buses_from = db_.GetBusesToStop(from);
            const auto& buses_to = db_.GetBusesToStop(to);
            if (buses_from.empty() || buses_to.empty()) {
                return std::nullopt;
            }        

            const auto& stops = db_.GetSortedStops();
            auto it_from = stops.find(from);
            auto it_to = stops.find(to);

            if (it_from == stops.end()) {
                return std::nullopt;
            }
            if (it_to == stops.end()) {
                return std::nullopt;
            }

            int id_from = stop_name_id_.at(from);
            int id_to = stop_name_id_.at(to);

            auto info = router_.BuildRoute(id_from, id_to);

            if (info == std::nullopt) {
                return std::nullopt;
            }


            std::vector <graph::Edge<double>> res;

            for (const auto edge_id : info.value().edges) {
                const auto edge = this->GetEdge(edge_id);
                res.push_back(edge);
            }
            
            result = {res, info.value().weight};

            return result;
        }
        
        graph::RoutingSettings graph::TransportRouter::GetRoutingSettings(){
            return routing_settings_;
        }

        void graph::TransportRouter::GraphAddStops(){
            const auto& stops = db_.GetDeqStops();
            auto v_size = stops.size();
            graph_ = graph::DirectedWeightedGraph<double>(v_size * 2);
            graph::VertexId id = 0;
            for (auto stop : stops) {
                graph::Edge<double> edge_wait;
                edge_wait.bus_name = "Wait";
                edge_wait.stop_name = stop.name;
                edge_wait.from = id;
                edge_wait.to = id + 1;
                edge_wait.weight = routing_settings_.bus_wait_time;
                stop_name_id_[stop.name] = id;
                graph_.AddEdge(edge_wait);
                id = id + 2;
            }
        }

        

        /*void graph::TransportRouter::AddStraghtBuses(const ranges::Range<std::vector<domain::Stop*>::iterator> range, double speed, const std::string name) {
            for (int i = 0; i < bus_route.size() / 2; ++i) {
                graph::Edge<double> edge;

                int s = 0;
                int distance = 0;

                edge.bus_name = name;

                auto temp_from = std::string(bus_route[i]->name);

                edge.from = stop_name_id_.at(temp_from) + 1;


                for (int j = i; j < bus_route.size() / 2; ++j) {

                    ++s;

                    auto temp_to = std::string(bus_route[j + 1]->name);

                    edge.to = stop_name_id_.at(temp_to);

                    if (db_.GetDistanceStops(bus_route[j], bus_route[j + 1]) == 0) {
                        distance = distance + db_.GetDistanceStops(bus_route[j + 1], bus_route[j]);
                    }
                    else {
                        distance = distance + db_.GetDistanceStops(bus_route[j], bus_route[j + 1]);
                    }

                    edge.weight = distance / speed;
                    edge.stop_count = s;
                    graph_.AddEdge(edge);
                }
            }

            int route_size = bus_route.size() / 2;

            for (int i = route_size; i < bus_route.size(); ++i) {

                int s = 0;
                int distance = 0;
                graph::Edge<double> edge;

                edge.bus_name = name;

                auto temp_from = std::string(bus_route[i]->name);

                edge.from = stop_name_id_.at(temp_from) + 1;


                for (int j = i; j < bus_route.size() - 1; ++j) {
                    ++s;

                    auto temp_to = std::string(bus_route[j + 1]->name);

                    edge.to = stop_name_id_.at(temp_to);


                    if (db_.GetDistanceStops(bus_route[j], bus_route[j + 1]) == 0) {
                        distance = distance + db_.GetDistanceStops(bus_route[j + 1], bus_route[j]);
                    }
                    else {
                        distance = distance + db_.GetDistanceStops(bus_route[j], bus_route[j + 1]);
                    }

                    edge.weight = distance / speed;
                    edge.stop_count = s;

                    graph_.AddEdge(edge);

                }
            }
        }*/

     