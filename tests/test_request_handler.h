#pragma once

#include "transport-catalogue/request_handler.h"
#include "transport-catalogue/transport_catalogue.h"
#include "transport-catalogue/json.h"
#include <string>
#include <sstream>

namespace test_request_handler {

void TestRequestCreation();
void TestStopRequest();
void TestBusRequest();
void TestMapRequest();
void TestRequestRegistry();
void TestRequestFactory();
void TestRequestHandler();
void TestJsonProcessing();
void TestErrorHandling();
void TestComplexRequests();
void RunAllTests();

} // namespace test_request_handler 