#pragma once
#include <string>
#include <functional>
#include <list>
#include <memory>
#include "config.hpp"
#include "context.hpp"

namespace FMF {
    class Endpoint {
    public:
        Endpoint(std::unique_ptr<Configuration> &config): _config(config) {}
        virtual ~Endpoint() = default;
        std::function<std::string(std::string const &)> bind(std::string const &registration) {
            return do_bind(registration);
        }
    protected:
        std::unique_ptr<Configuration> &_config;
    private:
        virtual std::function<std::string(std::string const &)> do_bind(std::string const &registration) = 0;
    };

    class BindingEndpoint: public Endpoint {
    public:
        BindingEndpoint(std::unique_ptr<Configuration> &config): Endpoint(config) {}
        virtual ~BindingEndpoint() = default;
        std::string handle_topic(std::string const &topic, int version_major, int version_minor, std::function<std::string(std::string const &, Context &)> handler) {
            auto safe_handler = [handler](std::string const &payload, Context &ctx){
                try {
                    return handler(payload, ctx);
                }
                catch(...) {
                    ctx.set("Result-Code", "500");
                    return std::string("Error occurred.");
                }
            };
            return do_handle_topic(topic, version_major, version_minor, safe_handler);
        }
        virtual bool listen() { return false; }
        virtual void close() {};
    private:
        virtual std::string do_handle_topic(std::string const &topic, int version_major, int version_minor, std::function<std::string(std::string const &, Context &)> handler) = 0;
    };

    class BindingEndpointFactory {
    public:
        static std::unique_ptr<BindingEndpoint> create(std::string const &name, std::unique_ptr<Configuration> &config) {
            for(auto &factory: get_factories()) {
                auto endp = factory->do_create(name, config);
                if (endp) {
                    return endp;
                }
            }
            return std::unique_ptr<BindingEndpoint>();
        }
    protected:
        static void add(std::unique_ptr<BindingEndpointFactory> &&factory) {
            auto &factories = get_factories();
            factories.push_back(std::move(factory));
        }
    private:
        virtual std::unique_ptr<BindingEndpoint> do_create(std::string const &name, std::unique_ptr<Configuration> &config) = 0;
        static std::list<std::unique_ptr<BindingEndpointFactory>> &get_factories() {
            static std::list<std::unique_ptr<BindingEndpointFactory>> factories;
            return factories;
        }
    };

    template<typename T>
    class TEndpointFactory : public BindingEndpointFactory {
    public:
        static void enable() {
            auto endp = std::make_unique<TEndpointFactory<T>>();
            BindingEndpointFactory::add(std::move(endp));
        }
    private:
        virtual std::unique_ptr<BindingEndpoint> do_create(std::string const &name, std::unique_ptr<Configuration> &config) {
            if (name == T::_construction_id) {
                return std::make_unique<T>(config);
            }
            return std::unique_ptr<BindingEndpoint>();
        }
    };

}