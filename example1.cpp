#include <iostream>
#include "include/config.hpp"

int main() {
    auto config = FMF::ConfigurationFactory::create("env");
    std::cout << "The value of PATH is " << config->get("PATH") << std::endl << std::flush;
}