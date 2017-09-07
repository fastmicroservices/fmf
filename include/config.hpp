#include <string>
#include <list>
#include <memory>
\
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