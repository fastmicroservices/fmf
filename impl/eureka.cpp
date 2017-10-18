#ifndef _MACH_PORT_T
#define _MACH_PORT_T
#include <sys/_types.h> /* __darwin_mach_port_t */
typedef __darwin_mach_port_t mach_port_t;
#include <pthread.h>
mach_port_t pthread_mach_thread_np(pthread_t);
#endif /* _MACH_PORT_T */


#include "../include/discovery.hpp"
#include "../include/config.hpp"
extern "C" {
#include "../include/mongoose.h"    
}
#include <regex>
#include <iostream>
#include "parsehttp.hpp"


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
                auto url = std::string("http://") + 
                    _config->get("EUREKA_HOST", "localhost") + ":" + _config->get("EUREKA_PORT", "32768") + 
                    "/eureka/v2/apps/" + topic;
                auto payload = std::string("{\
                    \"instance\": {")
                        + "\"hostName\": \"" + host + "\""
                        ", \"instanceId\": \"" + uri + "\""
                        ", \"app\": \"" + topic + "\""
                        ", \"vipAddress\": \"mainpool.app\""
                        // "\"secureVipAddress\": \"com.automationrhapsody.eureka.app\","
                        ", \"ipAddr\": \"10.0.0.10\""
                        ", \"status\": \"UP\""
                        ", \"port\": {\"$\": \"" + port + "\", \"@enabled\": \"true\"}"
                        // "\"securePort\": {\"$\": \"8443\", \"@enabled\": \"true\"},"
                        // "\"healthCheckUrl\": \"http://WKS-SOF-L011:8080/healthcheck\","
                        // "\"statusPageUrl\": \"\","
                        ", \"homePageUrl\": \"" + uri + "\""
                        ", \"dataCenterInfo\": {\
                            \"@class\": \"com.netflix.appinfo.InstanceInfo$DefaultDataCenterInfo\", \
                            \"name\": \"MyOwn\"\
                        }\n"
                        ", \"metadata\": { \"version\": " + std::to_string(version_major) + "." + std::to_string(version_minor) + " }"
                    "}\
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

                HttpResult hr(result.body);
                std::cout << "Result code: " << hr._response_code << std::endl;

                return result.body;
            }
            virtual std::string do_find(std::string const &topic, int version_major, int version_minor)
            {
                // get all apps
                if (topic == "*") {
                    struct mg_mgr mgr;
                    struct Result {
                        bool done_polling = false;
                        std::string body;
                    } result;
                    mg_mgr_init(&mgr, &result);
                    auto url = std::string("http://") + 
                        _config->get("EUREKA_HOST", "localhost") + ":" + _config->get("EUREKA_PORT", "32768") + 
                        "/eureka/v2/apps";
                    
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
                        }, 
                        url.c_str(), 
                        "Accept-Encoding: gzip\r\n"
                        "Accept: application/json\r\n", NULL);
                    while (!result.done_polling) {
                        mg_mgr_poll(&mgr, 1000);
                    }
                    mg_mgr_free(&mgr);
                    HttpResult hr(result.body);
                    if (hr._response_code >= 400) {
                        return hr._response_description;
                    }
                    return hr._contents;
                }
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