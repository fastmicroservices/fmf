#include <string>
#include <functional>
#include <map>
#include <iostream>
#include "../include/endpoint.hpp"
extern "C" {
#include "../include/mongoose.h"
}

namespace FMF {
    namespace impl {
        class HttpEndpoint: public BindingEndpoint {
        public:
            HttpEndpoint(std::unique_ptr<Configuration> &config): BindingEndpoint(config) {
            }
            virtual ~HttpEndpoint() {
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                if (_started) close();
            }
            virtual bool listen() {
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                if (!_started) start();
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                mg_mgr_poll(&_mgr, 1000);
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                return true;
            }
            virtual void close() {
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                mg_mgr_free(&_mgr);
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                _started = false;
            }
            static constexpr const char *_construction_id { "http" };
        private:
            virtual std::string do_handle_topic(std::string const &topic, int version_major, int version_minor, std::function<std::string(std::string const &)> handler) {
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                _handlers[topic] = handler;
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                return std::string("http://") + _config->get("HOSTNAME") + "/" + topic;
            }
            virtual std::function<std::string(std::string const &)> do_bind(std::string const &url) {
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                return [](std::string const &payload) { return std::string(); };
            }
            std::map<std::string,std::function<std::string(std::string const &)>> _handlers;
            struct mg_serve_http_opts _http_server_opts = {};
            bool _started {false};
            struct mg_mgr _mgr;

            static void ev_handler(struct mg_connection *nc, int ev, void *p) {
                static_cast<HttpEndpoint*>(nc->mgr->user_data)->handler(nc, ev, p);
            }

            void handler(struct mg_connection *nc, int ev, void *p) {
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                if (ev == MG_EV_HTTP_REQUEST)
                {
                    std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                    auto httpm = (struct http_message *)p;
                    struct mg_str path;
                    mg_parse_uri(httpm->uri, NULL, NULL, NULL, NULL, &path, NULL, NULL);
                    std::string local_path(path.p, path.len);
                    std::string query_text(httpm->query_string.p, httpm->query_string.len);
                    std::cout << "req: " << local_path << std::endl;
                    std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                    if (local_path.size() > 1 && local_path[0] == '/') {
                        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                        auto topic = local_path.substr(1);
                        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                        auto pos = _handlers.find(topic);
                        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                        if (pos == _handlers.end()) {
                            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                            mg_http_send_error(nc, 404, NULL);
                        }
                        else {
                            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                            auto result = pos->second("");
                            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                            mg_printf(nc,
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: %d\r\n\r\n%s",
                                (int)result.size(), result.c_str());                
                        }
                        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                    }
                }
            }                
            void start() {
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                struct mg_connection *nc;
                mg_mgr_init(&_mgr, this);
                auto http_port = _config->get("PORT");
                if (http_port.empty()) http_port = "80";
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                std::cout << "Starting web server on port " << http_port << std::endl;
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                nc = mg_bind(&_mgr, http_port.c_str(), &ev_handler);
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                if (nc == NULL)
                {
                    std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                    throw "Failed to create listener";
                }

                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                // Set up HTTP server parameters
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                mg_set_protocol_http_websocket(nc);
                _http_server_opts.document_root = "www";
                _http_server_opts.enable_directory_listing = "yes";

                std::cout << "The server is running" << std::endl;
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                
                _started = true;
            }
        };

        class EnableHttp {
        public:
            EnableHttp() {
                TEndpointFactory<HttpEndpoint>::enable();
            }
        };
        static EnableHttp enable_http;
    }
}
