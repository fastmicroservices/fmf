#pragma once
#include <string>
#include <list>
#include <memory>
#include <iostream>

namespace FMF {
    class Configuration {
    public:
        virtual ~Configuration() = default;

        std::string get(std::string const &key, std::string const &default_value = std::string()) {
            auto value = do_get(key);
            return value.empty() ? default_value : value;
        }
        void set(std::string const &key, std::string const &value) {
            return do_set(key,value);
        }

    private:
        virtual std::string do_get(std::string const &key) = 0;
        virtual void do_set(std::string const &key, std::string const &value) {};
    };

    class ConfigurationFactory {
    public:
        static std::unique_ptr<Configuration> create(std::string const &name) {
            for(auto &factory: get_factories()) {
                auto conf = factory->do_create(name);
                if (conf) {
                    return conf;
                }
            }
            return std::unique_ptr<Configuration>();
        }
    protected:
        static void add(std::unique_ptr<ConfigurationFactory> &&factory) {
            std::cout << "adding factory" << std::endl;
            auto &factories = get_factories();
            factories.push_back(std::move(factory));
        }
    private:
        virtual std::unique_ptr<Configuration> do_create(std::string const &name) = 0;
        static std::list<std::unique_ptr<ConfigurationFactory>> &get_factories() {
            static std::list<std::unique_ptr<ConfigurationFactory>> factories;
            return factories;
        }
    };
}