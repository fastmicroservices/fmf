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
            auto target = do_bind(registration);
            return [target](std::string const &payload) {
                Context ctx;
                return target(payload, ctx);
            };
        }
        std::function<std::string(std::string const &, Context &)> bind_with_context(std::string const &registration) {
            return do_bind(registration);
        }
    protected:
        std::unique_ptr<Configuration> &_config;
    private:
        virtual std::function<std::string(std::string const &, Context &)> do_bind(std::string const &registration) = 0;
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

    template<typename T>
    class RegisteringFactory {
    public:
        static std::unique_ptr<T> create(std::string const &name, std::unique_ptr<Configuration> &config) {
            for(auto &factory: get_factories()) {
                auto endp = factory->do_create(name, config);
                if (endp) {
                    return endp;
                }
            }
            return std::unique_ptr<T>();
        }
        virtual ~RegisteringFactory() = default;
    protected:
        static void add(std::unique_ptr<RegisteringFactory<T>> &&factory) {
            auto &factories = get_factories();
            factories.push_back(std::move(factory));
        }
    private:
        virtual std::unique_ptr<T> do_create(std::string const &name, std::unique_ptr<Configuration> &config) = 0;
        static std::list<std::unique_ptr<RegisteringFactory<T>>> &get_factories() {
            static std::list<std::unique_ptr<RegisteringFactory<T>>> factories;
            return factories;
        }
    };

    // typedef RegisteringFactory<BindingEndpoint> BindingEndpointFactory;
    // typedef RegisteringFactory<Endpoint> EndpointFactory;

    template<typename B, typename C>
    class TRegisteringFactory : public RegisteringFactory<B> {
    public:
        static void enable() {
            std::cerr << "enabling " << C::_construction_id << std::endl;
            auto endp = std::make_unique<TRegisteringFactory<B,C> >();
            RegisteringFactory<B>::add(std::move(endp));
        }
        virtual ~TRegisteringFactory<B,C>() = default;
    private:
        virtual std::unique_ptr<B> do_create(std::string const &name, std::unique_ptr<Configuration> &config) {
            if (name == C::_construction_id) {
                return std::make_unique<C>(config);
            }
            return std::unique_ptr<B>();
        }
    };

    template<typename T>
    class TEndpointFactory : public TRegisteringFactory<BindingEndpoint,T> {};

}