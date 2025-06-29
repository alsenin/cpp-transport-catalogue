#pragma once

#include "transport-catalogue/json.h"
#include <string>
#include <vector>

namespace test_json {

void TestNodeCreation();
void TestNodeTypes();
void TestNodeAccess();
void TestArrayOperations();
void TestDictOperations();
void TestJsonParsing();
void TestJsonPrinting();
void TestJsonSerialization();
void TestErrorHandling();
void RunAllTests();

} // namespace test_json 