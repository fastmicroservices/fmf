#include "../include/discovery.hpp"
#include "../include/config.hpp"
extern "C" {
#include "../include/mongoose.h"
}
#include <regex>
#include <iostream>

namespace FMF {
    namespace impl {
        class EurekaDiscovery: public Discovery {
        public:
            EurekaDiscovery(std::unique_ptr<Configuration> &config) : Discovery(config) 
            {
            }
            static constexpr char const *_construction_id = "eureka";
        private:
            virtual std::string do_publish(std::string const &topic, int version_major, int version_minor, std::string const &uri, std::string const &test) 
            {
                // parse the URI
                std::regex rx_parse_uri("https?://([^:/]+)(:(\\d+))?(/.*)");
                std::smatch rx_result;
                if (!std::regex_search(uri, rx_result, rx_parse_uri)) {
                    std::cerr << "The URI " << uri << " is not supported" << std::endl;
                    throw "Unsupported URI";
                }

                auto host = rx_result[1].str();
                auto port = rx_result[3].str();
                if (port.empty()) {
                    port = "80";
                }
                auto resource = rx_result[3].str();
                // register over netflix eureka

                // POST /eureka/v2/apps/appID
                // Input: JSON/XML payload HTTP Code: 204 on success

                struct mg_mgr mgr;
                struct Result {
                    bool done_polling = false;
                    std::string body;
                } result;
                mg_mgr_init(&mgr, &result);
                auto slug = topic + "-" + std::to_string(version_major) + "." + std::to_string(version_minor);
                auto url = std::string("http://") + 
                    _config->get("EUREKA_HOST", "localhost") + ":" + _config->get("EUREKA_PORT", "32768") + 
                    "/eureka/v2/apps/" + slug;
                auto payload = std::string("{\
                    \"instance\": {\
                        \"hostName\": \"") + host + "\",\
                        \"app\": \"" + topic + "\",\
                        \"vipAddress\": \"com.automationrhapsody.eureka.app\",\
                        \"secureVipAddress\": \"com.automationrhapsody.eureka.app\",\
                        \"ipAddr\": \"10.0.0.10\",\
                        \"status\": \"STARTING\",\
                        \"port\": {\"$\": \"" + port + "\", \"@enabled\": \"true\"},\
                        \"securePort\": {\"$\": \"8443\", \"@enabled\": \"true\"},\
                        \"healthCheckUrl\": \"http://WKS-SOF-L011:8080/healthcheck\",\
                        \"statusPageUrl\": \"\",\
                        \"homePageUrl\": \"\",\
                        \"dataCenterInfo\": {\
                            \"@class\": \"com.netflix.appinfo.InstanceInfo$DefaultDataCenterInfo\", \
                            \"name\": \"MyOwn\"\
                        }\
                    }\
                }";
                
                std::cout << "====================================" << std::endl;
                std::cout << payload << std::endl;
                std::cout << "====================================" << std::endl;
                mg_connect_http(&mgr, [](struct mg_connection *nc, int ev, void *ev_data) {
                        auto resultp = static_cast<Result*>(nc->mgr->user_data);
                        if (ev == MG_EV_HTTP_REPLY) {
                            auto hm = static_cast<struct http_message *>(ev_data);
                            nc->flags |= MG_F_CLOSE_IMMEDIATELY;
                            resultp->body += std::string(hm->message.p, hm->message.len);
                            resultp->done_polling = true;
                        }
                        else if (ev == MG_EV_CLOSE) {
                            resultp->done_polling = true;
                        }
                    }, url.c_str(), 
                    "Content-Type: application/json\r\n", payload.c_str());
                while (!result.done_polling) {
                    mg_mgr_poll(&mgr, 1000);
                }
                mg_mgr_free(&mgr);

                return result.body;
            }
            virtual std::string do_find(std::string const &topic, int version_major, int version_minor)
            {
                // find over netflix
                return "";
            }
        };

        class EurekaEnabler {
        public:
            EurekaEnabler() 
            {
                TDiscoveryFactory<EurekaDiscovery>::enable();
            }
        };

        static EurekaEnabler _enable_eureka;
    }
}