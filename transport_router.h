#pragma once

#include "router.h"
#include "transport_catalogue.h"
#include <iostream>

namespace graph{
    
struct RoutingSettings {
        int bus_wait_time;
        double bus_velocity;
    };
    
    
    class TransportRouter{
    public:
        TransportRouter(RoutingSettings& routing_settings, catalogue::TransportCatalogue& db) :routing_settings_(routing_settings), db_(db), router_(BuildRouter()) {}

        std::optional<std::pair<std::vector<graph::Edge<double>>, double>> GetRouteInfo(const std::string& from, const std::string& to) const;

    private:
        graph::Router<double> BuildRouter();

        graph::Edge<double> GetEdge(graph::EdgeId edge_id) const;

        RoutingSettings GetRoutingSettings();

        void GraphAddStops();

        template <typename It>
        void GraphAddEdges(ranges::Range<It> range,const std::string name, double speed) {

            for (auto i = range.begin(); i < range.end(); ++i) {

                int s = 0;
                int distance = 0;

                graph::Edge<double> edge;

                edge.bus_name = name;

                auto temp_from = std::string((*i)->name);

                edge.from = stop_name_id_.at(temp_from) + 1;

                for (auto j = i; j <  (range.end() - 1); ++j) {
                    ++s;

                    auto temp_to = std::string((*(j + 1))->name);

                    edge.to = stop_name_id_.at(temp_to);


                    if (db_.GetDistanceStops(*j, *(j + 1)) == 0) {

                        distance = distance + db_.GetDistanceStops(*(j + 1), *j);
                    }
                    else {
                        distance = distance + db_.GetDistanceStops(*j, *(j + 1));
                    }

                    edge.weight = distance / speed;
                    edge.stop_count = s;

                    graph_.AddEdge(edge);
                }
            }
        }

        template <typename It>
        void AddCirculeBuses(ranges::Range<It> range, double speed, const std::string name) {
            GraphAddEdges(range, name, speed);
        }

        template <typename It>
        void AddStraghtBuses(ranges::Range<It> range, double speed, const std::string name) {
            size_t dist = std::distance(range.begin(), range.end());

            ranges::Range<It> Foward(range.begin(), range.begin() + (dist / 2)+1);
            GraphAddEdges(Foward, name, speed);
            ranges::Range<It> Back(range.begin() + (dist / 2), range.end());
            GraphAddEdges(Back, name, speed);        
        }

        

    private:
        std::unordered_map<std::string, graph::VertexId> stop_name_id_;
        RoutingSettings& routing_settings_;
        catalogue::TransportCatalogue& db_;
        graph::DirectedWeightedGraph<double> graph_;
        graph::Router<double> router_;
    };
}


