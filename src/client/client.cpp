module;
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <openssl/ssl.h>

module c2server.client;

import c2server.error;
import std;

namespace c2server {

   namespace {

      namespace beast = boost::beast;
      namespace http = beast::http;
      namespace net = boost::asio;
      namespace ssl = net::ssl;
      using tcp = net::ip::tcp;

      http::verb toBeastMethod(HttpMethod method) {
         switch (method) {
            case HttpMethod::Get:
               return http::verb::get;
            case HttpMethod::Post:
               return http::verb::post;
            case HttpMethod::Put:
               return http::verb::put;
            case HttpMethod::Delete:
               return http::verb::delete_;
            case HttpMethod::Patch:
               return http::verb::patch;
            case HttpMethod::Head:
               return http::verb::head;
            case HttpMethod::Options:
               return http::verb::options;
            default:
               throw ClientError{"unsupported HTTP method"};
         }
      }

      template <class Stream>
      HttpResponse exchange(Stream& stream, const ClientSettings& settings, HttpMethod method, std::string target,
                            std::string body, const HttpHeaders& headers) {
         http::request<http::string_body> request{toBeastMethod(method), std::move(target), 11};
         request.set(http::field::host, settings.host);
         request.set(http::field::user_agent, "c2server-client");
         request.keep_alive(false);
         for (const auto& [name, value] : headers) {
            request.set(name, value);
         }
         request.body() = std::move(body);
         request.prepare_payload();

         http::write(stream, request);

         beast::flat_buffer buffer;
         http::response<http::string_body> response;
         http::read(stream, buffer, response);

         HttpHeaders responseHeaders;
         for (const auto& field : response) {
            setHeader(responseHeaders, std::string{field.name_string()}, std::string{field.value()});
         }
         return {
             .status = response.result_int(),
             .body = std::move(response.body()),
             .contentType = std::string{response[http::field::content_type]},
             .headers = std::move(responseHeaders),
         };
      }

   } // namespace

   Client::Client(ClientSettings settings)
       : settings_{std::move(settings)} {
      if (settings_.host.empty()) {
         throw ClientError{"client host must not be empty"};
      }
      if (settings_.port == 0) {
         throw ClientError{"client port must be between 1 and 65535"};
      }
      if (settings_.timeout <= std::chrono::seconds::zero()) {
         throw ClientError{"client timeout must be greater than zero"};
      }
   }

   HttpResponse Client::request(HttpMethod method, std::string target, std::string body, HttpHeaders headers) const {
      if (target.empty() || target.front() != '/') {
         throw ClientError{"request target must start with '/'"};
      }

      try {
         net::io_context ioc;
         tcp::resolver resolver{ioc};
         beast::tcp_stream stream{ioc};
         stream.expires_after(settings_.timeout);
         stream.connect(resolver.resolve(settings_.host, std::to_string(settings_.port)));

         if (settings_.ssl.enabled) {
            ssl::context context{ssl::context::tls_client};
            if (settings_.ssl.verifyPeer) {
               context.set_verify_mode(ssl::verify_peer);
               if (settings_.ssl.caFile.empty()) {
                  context.set_default_verify_paths();
               } else {
                  context.load_verify_file(settings_.ssl.caFile);
               }
            } else {
               context.set_verify_mode(ssl::verify_none);
            }

            ssl::stream<beast::tcp_stream> sslStream{std::move(stream), context};
            if (!SSL_set_tlsext_host_name(sslStream.native_handle(), settings_.host.c_str())) {
               throw ClientError{"failed to configure TLS server name '{}'", settings_.host};
            }
            if (settings_.ssl.verifyPeer) {
               sslStream.set_verify_callback(ssl::host_name_verification{settings_.host});
            }
            beast::get_lowest_layer(sslStream).expires_after(settings_.timeout);
            sslStream.handshake(ssl::stream_base::client);
            auto response = exchange(sslStream, settings_, method, std::move(target), std::move(body), headers);
            beast::error_code ec;
            beast::get_lowest_layer(sslStream).socket().shutdown(tcp::socket::shutdown_both, ec);
            return response;
         }

         auto response = exchange(stream, settings_, method, std::move(target), std::move(body), headers);
         beast::error_code ec;
         stream.socket().shutdown(tcp::socket::shutdown_both, ec);
         return response;
      } catch (const ClientError&) {
         throw;
      } catch (const std::exception& e) {
         throw ClientError{"HTTP request to {}:{} failed: {}", settings_.host, settings_.port, e.what()};
      }
   }

   HttpResponse Client::get(std::string target, HttpHeaders headers) const {
      return request(HttpMethod::Get, std::move(target), {}, std::move(headers));
   }

   HttpResponse Client::post(std::string target, std::string body, HttpHeaders headers) const {
      return request(HttpMethod::Post, std::move(target), std::move(body), std::move(headers));
   }

   HttpResponse Client::put(std::string target, std::string body, HttpHeaders headers) const {
      return request(HttpMethod::Put, std::move(target), std::move(body), std::move(headers));
   }

   HttpResponse Client::delete_(std::string target, HttpHeaders headers) const {
      return request(HttpMethod::Delete, std::move(target), {}, std::move(headers));
   }

   HttpResponse Client::patch(std::string target, std::string body, HttpHeaders headers) const {
      return request(HttpMethod::Patch, std::move(target), std::move(body), std::move(headers));
   }

} // namespace c2server
