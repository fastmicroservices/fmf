#pragma once
#include <string>
#include <list>
#include <memory>
#include <functional>

namespace FMF {
    class Version {
        static char const *__fmf_commit_slug;
    public:
        static std::string slug() {
            return __fmf_commit_slug;
        }
    };
    class Configuration {
    public:
        virtual ~Configuration() = default;

        std::string get(std::string const &key, std::string const &default_value = std::string()) {
            auto value = do_get(key);
            return value.empty() ? default_value : value;
        }
        std::string get(std::string const &key, std::function<std::string(void)> fn_default_getter) {
            auto ret = get(key);
            return ret.empty() ? fn_default_getter() : ret;
        }
        void set(std::string const &key, std::string const &value) {
            return do_set(key,value);
        }
        void set_if_not_present(std::string const &key, std::function<std::string(void)> fn_getter) {
            if (get(key).empty()) {
                set(key, fn_getter());
            }
        }
        void set_if_not_present(std::string const &key, std::string const & new_value) {
            if (get(key).empty()) {
                set(key, new_value);
            }
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