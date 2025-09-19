#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>
#include<iostream>
#include <ctype.h>
#include <unordered_map>

namespace transport_catalogue {
    namespace input_reader {
        /**
         * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
         */
        detail::Coordinates ParseCoordinates(std::string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return { nan, nan };
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);
            auto comma2 = str.find_first_of(',', not_space2);

            double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
            double lng = std::stod(std::string(str.substr(not_space2, comma2 - not_space2)));

            return { lat, lng };
        }



        /**
         * Удаляет пробелы в начале и конце строки
         */
        std::string_view Trim(std::string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        /**
         * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
         */
        std::vector<std::string_view> Split(std::string_view string, char delim) {
            std::vector<std::string_view> result;

            size_t pos = 0;
            while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                auto delim_pos = string.find(delim, pos);
                if (delim_pos == string.npos) {
                    delim_pos = string.size();
                }
                if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                    result.push_back(substr);
                }
                pos = delim_pos + 1;
            }

            return result;
        }
        std::unordered_map<std::string, detail::Distance> ParseDistanse(std::string_view str) {
            std::unordered_map<std::string, detail::Distance> stop_distance;
            
            
            std::vector<std::string_view> distance = Split(str, ',');
            for (int i = 2; i < distance.size(); ++i) {
                detail::Distance d;
                std::string sstop;
                auto m = distance[i].find_first_of('m');
                d.meters = std::stoi(std::string(distance[i].substr(0, m)));
                auto to = distance[i].find_first_of('o');
                auto not_space = distance[i].find_first_not_of(' ', to +1);
                sstop = std::string(distance[i].substr(not_space));
                stop_distance[sstop] = d;
            }
            return  stop_distance;
        }


        /**
         * Парсит маршрут.
         * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
         * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
         */
        std::vector<std::string_view> ParseRoute(std::string_view route) {
            if (route.find('>') != route.npos) {
                return Split(route, '>');
            }

            auto stops = Split(route, '-');
            std::vector<std::string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), std::next(stops.rbegin()), stops.rend());
            return results;
        }

        CommandDescription ParseCommandDescription(std::string_view line) {
            auto colon_pos = line.find(':');
            if (colon_pos == line.npos) {
                return {};
            }

            auto space_pos = line.find(' ');
            if (space_pos >= colon_pos) {
                return {};
            }

            auto not_space = line.find_first_not_of(' ', space_pos);
            if (not_space >= colon_pos) {
                return {};
            }

            return { std::string(line.substr(0, space_pos)),
                    std::string(line.substr(not_space, colon_pos - not_space)),
                    std::string(line.substr(colon_pos + 1)) };
        }

        void InputReader::ParseLine(std::string_view line) {
            auto command_description = ParseCommandDescription(line);
            if (command_description) {
                commands_.push_back(std::move(command_description));
            }
        }

        void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
            for (const auto& comdes : commands_) {
                if (comdes.command == "Stop") {
                    Stop new_stop;
                    new_stop.stop_name = move(comdes.id);
                    new_stop.stop_coordinates = ParseCoordinates(move(comdes.description));
                    catalogue.AddStop(new_stop);
                }
                else {
                    continue;
                }
            }
        
            for(const auto& comdes : commands_){
                if (comdes.command == "Stop") {
                    Stop* s1 = catalogue.FindStop(comdes.id);
                    for (auto& stop_and_distace : ParseDistanse(move(comdes.description))){
                        catalogue.AddDistance(s1, stop_and_distace.first, stop_and_distace.second);
                    }
                }
            }
                for (const auto& comdes : commands_) {
                    if (comdes.command == "Bus") {
                        Bus new_bus;
                        new_bus.bus_name = std::move(comdes.id);
                        std::vector<std::string_view> stops = ParseRoute(move(comdes.description));
                        for (const auto stop : stops) {
                            Stop* p = catalogue.FindStop(stop);
                            new_bus.stops.push_back(p);
                        }
                        catalogue.AddBus(new_bus);
                    }
                    else {
                        continue;
                    }
                }
            }
        

        void InputReader::GetCommands(TransportCatalogue& catalogue, int count, std::istream& input) {
            for (int i = 0; i < count; ++i) {
                std::string line;
                getline(input, line);
                ParseLine(line);
            }
            ApplyCommands(catalogue);
        }
    }
}
