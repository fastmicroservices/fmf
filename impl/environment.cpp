#include <memory>
#include <cstdlib>
#include "../include/config.hpp"

// Environment configuration
namespace FMF {
    namespace impl {
        class EnvironmentConfiguration: public Configuration {
            virtual std::string do_get(std::string const &key) {
                auto val = getenv(key.c_str());
                return val ? val : std::string();
            }
        };

        class EnvironmentConfigurationFactory: public ConfigurationFactory {
        public:
            static void enable() {
                auto config = std::make_unique<EnvironmentConfigurationFactory>();
                ConfigurationFactory::add(std::move(config));
            }
            class EnableEnvironmentConfigurationFactory {
            public:
                EnableEnvironmentConfigurationFactory() {
                    EnvironmentConfigurationFactory::enable();
                }
            };
            static EnableEnvironmentConfigurationFactory _enable;
        private:
            virtual std::unique_ptr<Configuration> do_create(std::string const &name) {
                if (name == "env") {
                    return std::make_unique<EnvironmentConfiguration>();
                }
                return std::unique_ptr<Configuration>();
            }
        };

        EnvironmentConfigurationFactory::EnableEnvironmentConfigurationFactory EnvironmentConfigurationFactory::_enable;
    }
}
