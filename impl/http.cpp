#include <string>
#include <functional>
#include <map>
#include "../include/endpoint.hpp"
extern "C" {
#include "../include/mongoose.h"
}

namespace FMF {
    namespace impl {
        class HttpEndpoint: public BindingEndpoint {
        public:
            HttpEndpoint(std::unique_ptr<Configuration> &config): BindingEndpoint(config) {
                _http_port = _config->get("PORT", "80");
            }
            virtual ~HttpEndpoint() 
            {
                if (_started) close();
            }
            virtual bool listen() 
            {
                if (!_started) start();
                mg_mgr_poll(&_mgr, 1000);
                return true;
            }
            virtual void close() {
                mg_mgr_free(&_mgr);
                _started = false;
            }
            static constexpr const char *_construction_id { "http" };
        private:
            virtual std::string do_handle_topic(std::string const &topic, int version_major, int version_minor, 
                std::function<std::string(std::string const &)> handler) 
            {
                _handlers[topic] = handler;
                return std::string("http://") + _config->get("HOSTNAME", "localhost") + ":" + _http_port + "/" + topic;
            }
            virtual std::function<std::string(std::string const &)> do_bind(std::string const &url) 
            {
                return [url,this](std::string const &payload) {
                    struct mg_mgr mgr;
                    
                      mg_mgr_init(&mgr, this);
                      _done_polling = false;
                      _polling_result = std::string();
                      mg_connect_http(&mgr, client_ev_handler, url.c_str(), NULL, payload.c_str());
                      while (!_done_polling) {
                        mg_mgr_poll(&mgr, 1000);
                      }
                      mg_mgr_free(&mgr);
                    
                      return _polling_result;
                };
            }

            std::map<std::string,std::function<std::string(std::string const &)>> _handlers;
            struct mg_serve_http_opts _http_server_opts = {};
            bool _started {false};
            struct mg_mgr _mgr;

            static void ev_handler(struct mg_connection *nc, int ev, void *p)
            {
                static_cast<HttpEndpoint*>(nc->mgr->user_data)->handler(nc, ev, p);
            }

            void handler(struct mg_connection *nc, int ev, void *p) 
            {
                if (ev == MG_EV_HTTP_REQUEST) {
                    auto httpm = (struct http_message *)p;
                    struct mg_str path;
                    mg_parse_uri(httpm->uri, NULL, NULL, NULL, NULL, &path, NULL, NULL);
                    std::string local_path(path.p, path.len);
                    std::string query_text(httpm->query_string.p, httpm->query_string.len);
                    if (local_path.size() > 1 && local_path[0] == '/') {
                        auto topic = local_path.substr(1);
                        auto pos = _handlers.find(topic);
                        if (pos == _handlers.end()) {
                            mg_http_send_error(nc, 404, NULL);
                        }
                        else {
                            auto result = pos->second(std::string(httpm->body.p, httpm->body.len));
                            mg_printf(nc,
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: %d\r\n\r\n%s",
                                (int)result.size(), result.c_str());
                        }
                    }
                }
            }

            static void client_ev_handler(struct mg_connection *nc, int ev, void *p)
            {
                static_cast<HttpEndpoint*>(nc->mgr->user_data)->client_handler(nc, ev, p);
            }

            bool _done_polling;
            std::string _polling_result;
            std::string _http_port;

            void client_handler(struct mg_connection *nc, int ev, void *ev_data) 
            {
                if (ev == MG_EV_HTTP_REPLY) {
                    auto hm = static_cast<struct http_message *>(ev_data);
                    nc->flags |= MG_F_CLOSE_IMMEDIATELY;
                    _polling_result += std::string(hm->body.p, hm->body.len);
                    _done_polling = true;
                  } 
                else if (ev == MG_EV_CLOSE) {
                    _done_polling = true;
                }
            }

            void start() 
            {
                struct mg_connection *nc;
                mg_mgr_init(&_mgr, this);
                nc = mg_bind(&_mgr, _http_port.c_str(), &ev_handler);
                if (nc == NULL)
                {
                    throw "Failed to create listener";
                }

                // Set up HTTP server parameters
                mg_set_protocol_http_websocket(nc);
                _http_server_opts.document_root = "www";
                _http_server_opts.enable_directory_listing = "yes";

                
                _started = true;
            }
        };

        class EnableHttp {
        public:
            EnableHttp() 
            {
                TEndpointFactory<HttpEndpoint>::enable();
            }
        };
        static EnableHttp enable_http;
    }
}
