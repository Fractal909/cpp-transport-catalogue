#include <iostream>

#include "json_reader.h"


using namespace std;

int main() {

    JsonReader reader;
    reader.Read(cin);

    TransportCatalogue catalogue;
    reader.ApplyCatalogueCommands(catalogue);

    reader.PrintRequests(catalogue, cout);
}