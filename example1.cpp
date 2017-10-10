#include <iostream>
#include <string>
#include <list>
#include <algorithm>
#include "include/config.hpp"
#include "include/endpoint.hpp"
#include "include/discovery.hpp"

class options {
    std::list<std::string> _options;
public:
    options(int argc, char **argv) {
        while (argc--) {
            _options.push_front(argv[argc]);
        }
    }
    bool is_present(std::string const &key) const {
        return std::any_of(_options.cbegin(), _options.cend(), [key](std::string const &src) { return src == key; });
    }
};

int main(int argc, char **argv) {
    try {
        options opts(argc,argv);
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
        auto testFn = [](std::string const &src, FMF::Context &) {
            std::cout << "TEST has been called with " << src << std::endl;
            return "PONG";
        };
        auto slug = endp->handle_topic("test", 1, 0, testFn);
        std::cout << "Registered as: " << slug << std::endl;
        auto bound = endp->bind(slug);
        std::cout << "Now calling TEST" << std::endl << bound("test payload") << std::endl;

        if (opts.is_present("client")) {
            std::cout << "Binding to http://localhost:8080/test" << std::endl;
            auto http = FMF::BindingEndpointFactory::create("http", multiconf);
            auto fn = http->bind("http://localhost:8080/test");
            std::cout << "Calling now" << std::endl;
            auto result = fn("PING");
            std::cout << "The result is: " << result << std::endl;
        }
        else {
            multiconf->set("PORT", "8081");
            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
            auto http = FMF::BindingEndpointFactory::create("http", multiconf);
            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
            auto registration = http->handle_topic("test", 1, 0, testFn);

            multiconf->set_if_not_present("EUREKA_HOST", "192.168.56.101");
            multiconf->set_if_not_present("EUREKA_PORT", "8080");
            auto discovery_svc = FMF::DiscoveryFactory::create("eureka", multiconf);
            auto discovery_registration = discovery_svc->publish("test", 1, 0, registration, std::string());
            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
            std::cout << "Discovery_registration value " << discovery_registration << std::endl;
            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
            while (http->listen()) { ; }
            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        }
        
        return 0;
    }
    catch(const char *ex_descr) {
        std::cerr << "ERROR: " << ex_descr << std::endl;
        return -1;
    }
}