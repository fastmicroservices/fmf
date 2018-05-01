#include <memory>
#include <sstream>
#include <config.hpp>
#include <context.hpp>
#include <endpoint.hpp>
#include <curl/curl.h>

namespace FMF {
    namespace impl {
        class CurlEndpoint: public FMF::Endpoint {
        public:
            CurlEndpoint(std::unique_ptr<Configuration> &config)
            : FMF::Endpoint(config) {
                curl_global_init(CURL_GLOBAL_DEFAULT);
            }
            virtual ~CurlEndpoint() {
                curl_global_cleanup();
            }
            virtual std::function<std::string(std::string const &, Context &)> do_bind(std::string const &registration) {
                return [registration](std::string const &data, Context &ctx){
                    std::string result;
                    CURL *curl;
                    CURLcode res;
                    curl = curl_easy_init();
                    if(curl) {
                        curl_easy_setopt(curl, CURLOPT_URL, registration.c_str());
                        // don't verify cert
                        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
                        // don't verify name
                        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

                        if (ctx.get("FollowRedirects", "yes") == "yes") {
                            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                        }

                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);

                        if (!data.empty()) {
                            curl_easy_setopt(curl, CURLOPT_POST, 1);
                            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
                            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
                        }

                        // additional headers?
                        auto extra_headers = ctx.get("ExtraHeaders");
                        if (!extra_headers.empty()) {
                            struct curl_slist *list = NULL;
                            std::stringstream hdrs_stream(extra_headers);
                            std::string buff;
                            while (hdrs_stream.good()) {
                                std::getline(hdrs_stream, buff);
                                if (!buff.empty()) {
                                    if (buff[buff.size()-1] == '\r') {
                                        buff.resize(buff.size() - 1);
                                    }
                                    if (!buff.empty()) {
                                        list = curl_slist_append(list, buff.c_str());
                                    }
                                }
                            }
                            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
                        }

                        res = curl_easy_perform(curl);
                        /* Check for errors */
                        if(res != CURLE_OK) {
                            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                                curl_easy_strerror(res));
                        }
                        curl_easy_cleanup(curl);
                    }
                    return result;
                };
            }
            static constexpr const char *_construction_id { "curl" };
        private:
            static size_t write_to_string(void *contents, size_t size, size_t nmemb, void *userp) {
                auto pstr = (std::string*)userp;
                auto total_size = size * nmemb;
                auto pcontents = (char const *)contents;
                *pstr += std::string(pcontents, pcontents + total_size);
                return total_size;
            }
        };

        class EnableCurl {
        public:
            EnableCurl() 
            {
                TRegisteringFactory<Endpoint,CurlEndpoint>::enable();
            }
        };
        static EnableCurl enable_curl;
    }
}
