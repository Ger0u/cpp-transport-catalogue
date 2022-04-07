#include "json_reader.h"
#include <iostream>

int main() {
    transport::json_reader::CreateTransportCatalogueAndHandleRequests(std::cin, std::cout);
}