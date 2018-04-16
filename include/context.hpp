#pragma once
#include <string>
#include <map>

namespace FMF {
    class Context {
    public:
        virtual ~Context() = default;

        std::string get(std::string const &key, std::string const &default_value = std::string()) {
            auto value = do_get(key);
            return value.empty() ? default_value : value;
        }
        std::string operator[](std::string const &key) {
            return get(key);
        }
        void set(std::string const &key, std::string const &value) {
            return do_set(key,value);
        }
        operator const std::map<std::string,std::string>&() {
            return _values;
        }

    private:
        std::map<std::string,std::string> _values;
        virtual std::string do_get(std::string const &key) {
            return _values[key];
        }
        virtual void do_set(std::string const &key, std::string const &value) {
            _values[key] = value;
        }
};
}