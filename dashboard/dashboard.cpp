#include <iostream>
#include <config.hpp>
#include <endpoint.hpp>
#include <discovery.hpp>

int main()
{
    std::cout << "Starting the DASHBOARD" << std::endl;
    auto multiconf = FMF::ConfigurationFactory::create("env,inmem");
    multiconf->set_if_not_present("EUREKA_HOST", "localhost");
    multiconf->set_if_not_present("EUREKA_PORT", "32768");
    multiconf->set_if_not_present("DISCOVERY_CLASS", "eureka");
    multiconf->set("PORT", "8080");
    auto discovery_svc = FMF::DiscoveryFactory::create(multiconf->get("DISCOVERY_CLASS"), multiconf);

    auto dashboard = [](std::string const &src, FMF::Context &ctx) {
        ctx.set("Content-Type", "text/html");
        ctx.set("Serve-File", "web/index.html");
        return std::string();
    };

    auto test_discovery = [&discovery_svc](std::string const &src, FMF::Context &ctx) {
        ctx.set("Content-Type", "application/json");
        return discovery_svc->find("*",0,0);
    };

    auto http = FMF::BindingEndpointFactory::create("http", multiconf);
    http->handle_topic("dashboard", 1, 0, dashboard);
    http->handle_topic("test_discovery", 1, 0, test_discovery);
    while (http->listen()) { ; }
    return 0;
}