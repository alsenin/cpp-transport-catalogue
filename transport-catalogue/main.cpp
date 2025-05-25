#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    TransportCatalogue catalogue;
    InputReader reader;
    reader.ReadDataBase(catalogue, std::cin);
    DoStatRequests(catalogue, std::cin, std::cout);
    return 0;
}