#include <memory>
#include <cstdlib>
#include <map>
#include <string>
#include "../include/config.hpp"

// Inmem configuration
namespace FMF {
    namespace impl {
        class InmemConfiguration: public Configuration {
        private:
            virtual std::string do_get(std::string const &key) {
                return _values[key];
            }
            virtual void do_set(std::string const &key, std::string const &value) {
                _values[key] = value;
            }
            std::map<std::string,std::string> _values;
        };

        class InmemConfigurationFactory: public ConfigurationFactory {
        public:
            static void enable() {
                auto config = std::make_unique<InmemConfigurationFactory>();
                ConfigurationFactory::add(std::move(config));
            }
            class EnableInmemConfigurationFactory {
            public:
                EnableInmemConfigurationFactory() {
                    InmemConfigurationFactory::enable();
                }
            };
            static EnableInmemConfigurationFactory _enable;
        private:
            virtual std::unique_ptr<Configuration> do_create(std::string const &name) {
                if (name == "inmem") {
                    return std::make_unique<InmemConfiguration>();
                }
                return std::unique_ptr<Configuration>();
            }
        };

        InmemConfigurationFactory::EnableInmemConfigurationFactory InmemConfigurationFactory::_enable;
    }
}
