#include <memory>
#include <cstdlib>
#include <map>
#include <string>
#include "../include/config.hpp"
#include "../include/endpoint.hpp"

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
        private:
            virtual std::unique_ptr<Configuration> do_create(std::string const &name) {
                if (name == "inmem") {
                    return std::make_unique<InmemConfiguration>();
                }
                return std::unique_ptr<Configuration>();
            }
        };

        
        class InmemEndpoint : public BindingEndpoint {
        public:
            InmemEndpoint(std::unique_ptr<Configuration> &config): BindingEndpoint(config) {}
            static const char constexpr *_construction_id  { "inmem" };
        private:
            virtual std::string do_handle_topic(std::string const &topic, int version_major, int version_minor, std::function<std::string(std::string const &, Context &)> handler) {
                // create the slug
                auto slug = topic + "," + std::to_string(version_major) + "." + std::to_string(version_minor);
                _handlers[slug] = handler;
                return slug;
            }
            virtual std::function<std::string(std::string const &)> do_bind(std::string const &registration) {
                return [this, registration](std::string const &payload) {
                    Context tmp;
                    return _handlers[registration](payload, tmp);
                };
            }
            std::map<std::string,std::function<std::string(std::string const &, Context &)>> _handlers;
        };

        class EnableInmemEndpointFactory {
        public:
            EnableInmemEndpointFactory() {
                TEndpointFactory<InmemEndpoint>::enable();
            }
        };
        
        static InmemConfigurationFactory::EnableInmemConfigurationFactory enable_config;
        static EnableInmemEndpointFactory enable_endpoint;
    }
}
