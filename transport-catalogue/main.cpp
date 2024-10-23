#include <iostream>
#include <string>

#include "json_reader.h"

using namespace std;

int main() {




    JsonReader reader;
    reader.Read(cin);

    TransportCatalogue catalogue;
    reader.ApplyCatalogueCommands(catalogue);

    reader.Print(catalogue, cout);
}