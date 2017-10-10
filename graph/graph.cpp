#include <iostream>
#include <string>
#include <endpoint.hpp>

int main() 
{
    std::cout << "OK working" << std::endl;
    auto config = FMF::ConfigurationFactory::create("env,inmem");
    auto endpoint = FMF::BindingEndpointFactory::create("http", config);
    endpoint->handle_topic("graph", 0, 1, [](std::string const &payload, FMF::Context &ctx) {
        ctx.set("Content-Type", "image/svg+xml");
        return std::string("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" width=\"400\" height=\"110\">"
        "<rect width=\"300\" height=\"100\" style=\"fill:rgb(0,0,255);stroke-width:3;stroke:rgb(0,0,0)\" />") +
        "<text>"+ payload + "</text>"
      "</svg>";
    });
    endpoint->handle_topic("graph.test", 0, 1, [](std::string const &payload, FMF::Context &ctx) {
        ctx.set("Content-Type", "text/html");
        ctx.set("Serve-File", "graph/web/test.html");
        return std::string();
    });
    while (endpoint->listen()) {;}
    return 0;
}