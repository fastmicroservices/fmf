#include <memory>
#include <cstdlib>
#include <map>
#include <string>
#include "../include/config.hpp"

// Multi configuration
namespace FMF {
    namespace impl {
        class MultiConfiguration: public Configuration {
        public:
            MultiConfiguration(std::string const &first_name, std::string const &second_name) {
                first = ConfigurationFactory::create(first_name);
                second = ConfigurationFactory::create(second_name);
            }
        private:
            virtual std::string do_get(std::string const &key) {
                auto val = first->get(key);
                if (val.empty()) {
                    val = second->get(key);
                }
                return val;
            }
            virtual void do_set(std::string const &key, std::string const &value) {
                first->set(key, value);
                second->set(key, value);
            }
            std::unique_ptr<Configuration> first, second;
        };

        class MultiConfigurationFactory: public ConfigurationFactory {
        public:
            static void enable() {
                auto config = std::make_unique<MultiConfigurationFactory>();
                ConfigurationFactory::add(std::move(config));
            }
            class EnableMultiConfigurationFactory {
            public:
                EnableMultiConfigurationFactory() {
                    MultiConfigurationFactory::enable();
                }
            };
            static EnableMultiConfigurationFactory _enable;
        private:
            virtual std::unique_ptr<Configuration> do_create(std::string const &name) {
                auto commapos = name.find(',');
                if (commapos != std::string::npos) {
                    auto first_name = name.substr(0, commapos);
                    auto second_name = name.substr(commapos + 1);
                    return std::make_unique<MultiConfiguration>(first_name, second_name);
                }
                return std::unique_ptr<Configuration>();
            }
        };

        MultiConfigurationFactory::EnableMultiConfigurationFactory MultiConfigurationFactory::_enable;
    }
}
