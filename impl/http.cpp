#include <string>
#include <functional>
#include <map>
#include <regex>
#include "../include/endpoint.hpp"
extern "C" {
#include "../include/mongoose.h"
}
#include <context.hpp>
#include <iostream>

namespace FMF {
    namespace impl {
        class HttpEndpoint: public BindingEndpoint {
        public:
            HttpEndpoint(std::unique_ptr<Configuration> &config): BindingEndpoint(config) {
                auto is_ssl = _config->get("IS_SSL", "no");
                _is_ssl = is_ssl != "no";
                _http_port = _config->get("PORT", _is_ssl ? "443" : "80");
                if (_is_ssl) {
                    _ssl_cert = _config->get("SSL_CERT", "server.pem");
                    _ssl_key = _config->get("SSL_KEY", "server.key");
                }
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
                std::function<std::string(std::string const &, Context &)> handler) 
            {
                _handlers[topic] = handler;
                return std::string("http://") + _config->get("HOSTNAME", "localhost") + ":" + _http_port + "/" + topic;
            }
            virtual std::function<std::string(std::string const &, FMF::Context &)> do_bind(std::string const &url) 
            {
                return [url,this](std::string const &payload, FMF::Context &ctx) {
                    struct mg_mgr mgr;
                    
                      mg_mgr_init(&mgr, this);
                      _done_polling = false;
                      _polling_result = std::string();
                      char const *post_data = payload.empty() ? NULL : payload.c_str();
                      auto hdrs = ctx["ExtraHeaders"];
                      char const *extra_headers = hdrs.empty() ? NULL : hdrs.c_str();
                      mg_connect_http(&mgr, client_ev_handler, url.c_str(), extra_headers, post_data);
                      while (!_done_polling) {
                        mg_mgr_poll(&mgr, 1000);
                      }
                      mg_mgr_free(&mgr);
                    
                      return _polling_result;
                };
            }

            std::map<std::string,std::function<std::string(std::string const &, Context &)>> _handlers;
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
                    if (local_path.size() > 0 && local_path[0] == '/') {
                        auto topic = local_path.substr(1);
                        auto pos = _handlers.find(topic);
                        if (pos == _handlers.end()) {
                            // second try: match using regex
                            pos = std::find_if(_handlers.begin(), _handlers.end(), [topic](auto const &tuple){
                                return std::regex_match(topic, std::regex(tuple.first));
                            });
                            if (pos == _handlers.end()) {
                                mg_http_send_error(nc, 404, NULL);
                            }
                        }
                        if (pos != _handlers.end()) {
                            Context ctx;
                            ctx.set("LocalPath", local_path);
                            ctx.set("QueryString", query_text);
                            ctx.set("Uri", std::string(httpm->uri.p, httpm->uri.len));
                            for (auto i = 0; httpm->header_names[i].len > 0; i++) {
                                std::string tag("HEADER_");
                                std::transform(
                                    httpm->header_names[i].p, 
                                    httpm->header_names[i].p + httpm->header_names[i].len,
                                    std::back_inserter(tag),
                                    [](unsigned char c){ return std::toupper(c); });
                                ctx.set(tag,
                                    std::string(httpm->header_values[i].p, httpm->header_values[i].len));
                            }
                            auto result = pos->second(std::string(httpm->body.p, httpm->body.len), ctx);
                            auto serve_file = ctx.get("Serve-File");
                            auto mime_type = ctx.get("Content-Type", "text/plain");
                            if (serve_file.empty()) {
                                std::string headers_fmt = std::string("HTTP/1.1 200 OK\r\n"
                                "Content-Type: ") + mime_type + "\r\n"
                                "Content-Length: %d\r\n\r\n%s";
                                mg_printf(nc,
                                    headers_fmt.c_str(),
                                    (int)result.size(), result.c_str());
                            }
                            else {
                                mg_http_serve_file(nc, httpm, serve_file.c_str(), mg_mk_str(mime_type.c_str()), mg_mk_str(""));
                            }
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
            std::string _ssl_cert;
            std::string _ssl_key;
            bool _is_ssl = {};

            void client_handler(struct mg_connection *nc, int ev, void *ev_data) 
            {
                if (ev == MG_EV_CONNECT){
                    if (*(int *) ev_data != 0) {
                        fprintf(stderr, "connect() failed: %s\n", strerror(*(int *) ev_data));
                    }
                }
                else if (ev == MG_EV_HTTP_REPLY) {
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
                if (_is_ssl) {
                    std::cout << "going SSL" << std::endl;
                    struct mg_bind_opts bind_opts;
                    memset(&bind_opts, 0, sizeof(bind_opts));
                    bind_opts.ssl_cert = _ssl_cert.c_str(); // "server.pem";
                    bind_opts.ssl_key = _ssl_key.c_str(); // "key.pem";

                    // Use bind_opts to specify SSL certificate & key file
                    nc = mg_bind_opt(&_mgr, _http_port.c_str(), &ev_handler, bind_opts);
                }
                else {
                    nc = mg_bind(&_mgr, _http_port.c_str(), &ev_handler);
                }
                if (nc == NULL)
                {
                    throw "Failed to create listener";
                }
                mg_set_protocol_http_websocket(nc);

                
                _started = true;
            }
        };

        class EnableHttp {
        public:
            EnableHttp() 
            {
                TRegisteringFactory<BindingEndpoint,HttpEndpoint>::enable();
            }
        };
        static EnableHttp enable_http;
    }
}
