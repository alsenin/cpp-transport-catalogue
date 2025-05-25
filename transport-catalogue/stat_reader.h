#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"
#include "string_utils.h"

void  DoStatRequests(TransportCatalogue& catalogue, std::istream& input, std::ostream& output);
