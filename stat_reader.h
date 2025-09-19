#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace stat_reader {

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);

        void PrintCommandStop(const const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);

        void PrintCommandBus(const const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);

        void GetRequestToPrint(const TransportCatalogue& tansport_catalogue, int count, std::ostream& output, std::istream& input);
    }
}
