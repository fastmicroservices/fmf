#include <iostream>
#include "include/config.hpp"

int main() {
    auto envconf = FMF::ConfigurationFactory::create("env");
    std::cout << "The value of PATH on envconf is " << envconf->get("PATH") << std::endl << std::flush;
    auto memconf = FMF::ConfigurationFactory::create("inmem");
    std::cout << "now memory: " << (static_cast<bool>(memconf)) << std::endl;
    auto val = memconf->get("PATH");
    std::cout << "The value of PATH on memconf is " << val << std::endl << std::flush;
    auto multiconf = FMF::ConfigurationFactory::create("env,inmem");
    std::cout << "The value of PATH on multiconf is " << multiconf->get("PATH") << std::endl << std::flush;
    std::cout << "The value of TEST1 on multiconf is " << multiconf->get("TEST1") << std::endl << std::flush;
    multiconf->set("TEST1", "(I just set this value!)");
    std::cout << "The value of TEST1 on multiconf is " << multiconf->get("TEST1") << std::endl << std::flush;
    
}