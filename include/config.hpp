#include <string>
#include <list>
#include <memory>
#include <iostream>

namespace FMF {
    class Configuration {
    public:
        virtual ~Configuration() = default;

        std::string get(std::string const &key) {
            return do_get(key);
        }

    private:
        virtual std::string do_get(std::string const &key) = 0;
    };

    class ConfigurationFactory {
    public:
        static std::unique_ptr<Configuration> create(std::string const &name) {
            std::cout << __FILE__ << ":" << __LINE__ << ", create" << std::endl;
            for(auto &factory: get_factories()) {
                std::cout << __FILE__ << ":" << __LINE__ << ", create" << std::endl;
                auto conf = factory->do_create(name);
                std::cout << __FILE__ << ":" << __LINE__ << ", create" << std::endl;
                if (conf) {
                    std::cout << __FILE__ << ":" << __LINE__ << ", create" << std::endl;
                    return conf;
                }
            }
            std::cout << __FILE__ << ":" << __LINE__ << ", create" << std::endl;
            return std::unique_ptr<Configuration>();
        }
    protected:
        static void add(std::unique_ptr<ConfigurationFactory> &&factory) {
            std::cout << __FILE__ << ":" << __LINE__ << ", adding factory test" << std::endl;
            auto &factories = get_factories();
            std::cout << __FILE__ << ":" << __LINE__ << ", adding factory" << std::endl;
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