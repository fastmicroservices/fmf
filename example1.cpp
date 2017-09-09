#include <iostream>
#include "include/config.hpp"
#include "include/endpoint.hpp"

int main() {
    std::cout << "SAMPLING CONFIGURATION" << std::endl;
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
    
    std::cout << "SAMPLING ENDPOINTS" << std::endl;
    auto endp = FMF::BindingEndpointFactory::create("inmem", multiconf);
    std::cout << "We have endp: " << static_cast<bool>(endp) << std::endl;
    auto testFn = [](std::string const &src) {
        std::cout << "TEST has been called with " << src << std::endl;
        return "PONG";
    };
    auto slug = endp->handle_topic("test", 1, 0, testFn);
    std::cout << "Registered as: " << slug << std::endl;
    auto bound = endp->bind(slug);
    std::cout << "Now calling TEST" << std::endl << bound("test payload") << std::endl;

    multiconf->set("PORT", "8080");
    std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    auto http = FMF::BindingEndpointFactory::create("http", multiconf);
    std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    auto registration = http->handle_topic("test", 1, 0, testFn);
    std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    while (http->listen()) { ; }
    std::cout << __FILE__ << ":" << __LINE__ << std::endl;
}