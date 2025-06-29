#pragma once

#include "transport-catalogue/transport_catalogue.h"
#include "transport-catalogue/domain.h"
#include <string>
#include <vector>

namespace test_transport_catalogue {

void TestStopOperations();
void TestBusOperations();
void TestRouteInfo();
void TestDistanceCalculations();
void TestStopInfo();
void TestRouteExists();
void TestGetStopByName();
void TestGetRouteInfo();
void TestGetStopInfo();
void TestComplexScenarios();
void RunAllTests();

} // namespace test_transport_catalogue 