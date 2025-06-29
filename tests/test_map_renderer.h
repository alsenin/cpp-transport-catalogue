#pragma once

#include "transport-catalogue/map_renderer.h"
#include "transport-catalogue/transport_catalogue.h"
#include "transport-catalogue/svg.h"
#include <string>
#include <sstream>

namespace test_map_renderer {

void TestRenderSettings();
void TestSphereProjector();
void TestSvgElements();
void TestMapRendering();
void TestColorPalette();
void TestRouteRendering();
void TestStopRendering();
void TestLabelRendering();
void TestComplexMap();
void RunAllTests();

} // namespace test_map_renderer 