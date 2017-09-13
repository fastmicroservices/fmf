#pragma once

#include <string>
#include <memory>
#include "config.hpp"

namespace FMF {
    class Discovery {
    public:
        Discovery(std::unique_ptr<Configuration> &config): _config(config) {}
        virtual ~Discovery() = default;

        std::string publish(std::string const &topic, int version_major, int version_minor, std::string const &uri, std::string const &test) {
            return do_publish(topic, version_major, version_minor, uri, test);
        }
        std::string find(std::string const &topic, int version_major, int version_minor) {
            return do_find(topic, version_major, version_minor);
        }
    protected:
        std::unique_ptr<Configuration> &_config;
    private:
        virtual std::string do_publish(std::string const &topic, int version_major, int version_minor, std::string const &uri, std::string const &test) = 0;
        virtual std::string do_find(std::string const &topic, int version_major, int version_minor) = 0;
    };

    class DiscoveryFactory {
    public:
        static std::unique_ptr<Discovery> create(std::string const &name, std::unique_ptr<Configuration> &config) {
            for(auto &factory: get_factories()) {
                auto disco = factory->do_create(name, config);
                if (disco) {
                    return disco;
                }
            }
            return std::unique_ptr<Discovery>();
        }
    protected:
        static void add(std::unique_ptr<DiscoveryFactory> &&factory) {
            auto &factories = get_factories();
            factories.push_back(std::move(factory));
        }
    private:
        virtual std::unique_ptr<Discovery> do_create(std::string const &name, std::unique_ptr<Configuration> &config) = 0;
        static std::list<std::unique_ptr<DiscoveryFactory>> &get_factories() {
            static std::list<std::unique_ptr<DiscoveryFactory>> factories;
            return factories;
        }
    };

    template<typename T>
    class TDiscoveryFactory : public DiscoveryFactory {
    public:
        static void enable() {
            auto disco = std::make_unique<TDiscoveryFactory<T>>();
            DiscoveryFactory::add(std::move(disco));
        }
    private:
        virtual std::unique_ptr<Discovery> do_create(std::string const &name, std::unique_ptr<Configuration> &config) {
            if (name == T::_construction_id) {
                return std::make_unique<T>(config);
            }
            return std::unique_ptr<Discovery>();
        }
    };

}
