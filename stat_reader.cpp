#include "stat_reader.h"
#include "input_reader.h"
#include <iomanip>
#include<iostream>
#include <string>

namespace transport_catalogue {
    namespace stat_reader {

        void PrintCommandStop( const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output) {
            std::string query(request);
            query = query.substr(5);
            query = query.substr(query.find_first_not_of(' '), query.find_last_not_of(' ') + 1);

            const  StopInfo info = tansport_catalogue.GetStopInfo(query);
            if (info.stopcount == false) {
                output << "Stop " << query << ": not found" << std::endl;

            }
            else if (info.buses.empty()) {
                output << "Stop " << query << ": no buses" << std::endl;

            }
            else {
                output << "Stop " << query << ": buses";
                for (const auto bus : info.buses) {
                    output << " " << bus;
                }
                output << std::endl;
            }
        }

        void PrintCommandBus(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output) {
            std::string query(request);
            query = query.substr(4);
            query = query.substr(query.find_first_not_of(' '), query.find_last_not_of(' ') + 1);
            const BusInfo info = tansport_catalogue.GetBusInfo(query);
            if (info.stops_count == 0) {
                output << "Bus " << query << ": not found" << std::endl;
            }
            else {
                output << "Bus " << query << ": " << info.stops_count << " stops on route, "
                    << info.uniq_stops << " unique stops, " << std::setprecision(6) << info.route_length.meters << " route length, " << info.curvature << " curvature" << std::endl;
            }
        }

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output) {

            std::string command(request);
            command = command.substr(0, command.find(' '));

            if (command == "Bus") {
                PrintCommandBus(tansport_catalogue, request, output);
            }

            if (command == "Stop") {
                PrintCommandStop(tansport_catalogue, request, output);

            }
        }

        void GetRequestToPrint(const TransportCatalogue& tansport_catalogue, int count, std::ostream& output, std::istream& input) {
            for (int i = 0; i < count; ++i) {
                std::string line;
                getline(input, line);
                ParseAndPrintStat(tansport_catalogue, line, output);
            }
        }

    }
}
