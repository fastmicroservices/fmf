#include <string>
#include <functional>
#include <list>

namespace FMF {
    class Endpoint {
    public:
        virtual ~Endpoint() = default;
        std::function<std::string(std::string const &)> bind(std::string const &registration) {
            return do_bind(registration);
        }
    private:
        virtual std::function<std::string(std::string const &)> do_bind(std::string const &registration) = 0;
    };
    class BindingEndpoint: public Endpoint {
    public:
        virtual ~BindingEndpoint() = default;
        std::string handle_topic(std::string const &topic, int version_major, int version_minor, std::function<std::string(std::string const &)> handler) {
            return do_handle_topic(topic, version_major, version_minor, handler);
        }
    private:
        virtual std::string do_handle_topic(std::string const &topic, int version_major, int version_minor, std::function<std::string(std::string const &)> handler) = 0;
    };

    class BindingEndpointFactory {
    public:
        static std::unique_ptr<BindingEndpoint> create(std::string const &name) {
            for(auto &factory: get_factories()) {
                auto endp = factory->do_create(name);
                if (endp) {
                    return endp;
                }
            }
            return std::unique_ptr<BindingEndpoint>();
        }
    protected:
        static void add(std::unique_ptr<BindingEndpointFactory> &&factory) {
            auto &factories = get_factories();
            factories.push_back(std::move(factory));
        }
    private:
        virtual std::unique_ptr<BindingEndpoint> do_create(std::string const &name) = 0;
        static std::list<std::unique_ptr<BindingEndpointFactory>> &get_factories() {
            static std::list<std::unique_ptr<BindingEndpointFactory>> factories;
            return factories;
        }
    };
}