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
        private:
            virtual std::string do_handle_topic(std::string const &topic, int version_major, int version_minor, std::function<std::string(std::string const &)> handler) {
                // create the slug
                auto slug = topic + "," + std::to_string(version_major) + "." + std::to_string(version_minor);
                _handlers[slug] = handler;
                return slug;
            }
            virtual std::function<std::string(std::string const &)> do_bind(std::string const &registration) {
                return _handlers[registration];
            }
            std::map<std::string,std::function<std::string(std::string const &)>> _handlers;
        };
        
        class InmemEndpointFactory : public BindingEndpointFactory {
        public:
            static void enable() {
                auto endp = std::make_unique<InmemEndpointFactory>();
                BindingEndpointFactory::add(std::move(endp));
            }
            class EnableInmemEndpointFactory {
            public:
                EnableInmemEndpointFactory() {
                    InmemEndpointFactory::enable();
                }
            };
        private:
            virtual std::unique_ptr<BindingEndpoint> do_create(std::string const &name) {
                if (name == "inmem") {
                    return std::make_unique<InmemEndpoint>();
                }
                return std::unique_ptr<BindingEndpoint>();
            }
        };
        
        static InmemConfigurationFactory::EnableInmemConfigurationFactory enable_config;
        static InmemEndpointFactory::EnableInmemEndpointFactory enable_endpoint;
    }
}
