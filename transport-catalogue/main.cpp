#include "json_reader.h"
#include <iostream>

int main() {
    transport::json_reader::CreateAndRequestsTransportCatalogue(std::cin, std::cout);
}