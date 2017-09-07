#include <iostream>
#include "include/config.hpp"

int main() {
    std::cout << "This is a test" << std::endl;
    auto config = FMF::ConfigurationFactory::create("env");
    std::cout << "This is a test" << std::endl;
    std::cout << "The value of PATH is " << config->get("PATH") << std::endl << std::flush;
    std::cout << "This is a test" << std::endl;
}