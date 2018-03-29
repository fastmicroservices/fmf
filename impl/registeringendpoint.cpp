#include <endpoint.hpp>
#include <discovery.hpp>

namespace FMF {
    namespace impl {
        class RegisteringEndpoint: public BindingEndpoint {
        public:
            RegisteringEndpoint(std::string const &endpoint_name, std::string const &discovery_name, std::unique_ptr<Configuration> &config):
            BindingEndpoint(config) {
                _endpoint = BindingEndpointFactory::create(endpoint_name, config);
                _discovery = DiscoveryFactory::create(discovery_name, config);
            }
            virtual bool listen() 
            {
                return _endpoint->listen();
            }
            virtual void close() {
                _endpoint->close();
            }
        private:
            std::unique_ptr<BindingEndpoint> _endpoint;
            std::unique_ptr<Discovery> _discovery;
            virtual std::string do_handle_topic(std::string const &topic, int version_major, int version_minor, 
                std::function<std::string(std::string const &, Context &)> handler) 
            {
                auto addr = _endpoint->handle_topic(topic, version_major, version_minor, handler);
                _discovery->publish(topic, version_major, version_minor, addr, std::string());
                return addr;
            }
            virtual std::function<std::string(std::string const &, FMF::Context &)> do_bind(std::string const &url) 
            {
                throw "Not to be used as a binding endpoint";
            }

        };

        class RegisteringEndpointFactory : public BindingEndpointFactory {
        public:
            static void enable() {
                auto endp = std::make_unique<RegisteringEndpointFactory>();
                BindingEndpointFactory::add(std::move(endp));
            }
        private:
            virtual std::unique_ptr<BindingEndpoint> do_create(std::string const &name, std::unique_ptr<Configuration> &config) {
                auto colonpos = name.find(':');
                if (colonpos != std::string::npos) {
                    auto first_name = name.substr(0, colonpos);
                    auto second_name = name.substr(colonpos + 1);
                    return std::make_unique<RegisteringEndpoint>(first_name, second_name, config);
                }
                return std::unique_ptr<BindingEndpoint>();
            }
        };
    
        class EnableRegistering {
        public:
            EnableRegistering() 
            {
                RegisteringEndpointFactory::enable();
            }
        };
        static EnableRegistering enable_registering;
    }
}
