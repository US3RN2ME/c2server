module;
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

module c2server.session;

import c2server.logger;

namespace c2server {

   namespace beast = boost::beast;
   namespace http = beast::http;
   using net::use_awaitable;

   namespace {

      HttpMethod toDomainMethod(http::verb method) {
         switch (method) {
            case http::verb::get:
               return HttpMethod::Get;
            case http::verb::post:
               return HttpMethod::Post;
            case http::verb::put:
               return HttpMethod::Put;
            case http::verb::delete_:
               return HttpMethod::Delete;
            case http::verb::patch:
               return HttpMethod::Patch;
            case http::verb::head:
               return HttpMethod::Head;
            case http::verb::options:
               return HttpMethod::Options;
            default:
               return HttpMethod::Unknown;
         }
      }

      HttpRequest toDomain(const http::request<http::string_body>& req, std::string_view ip) {
         return {
             .method = toDomainMethod(req.method()),
             .methodText = std::string(req.method_string()),
             .target = std::string(req.target()),
             .body = req.body(),
             .remoteIp = std::string{ip},
         };
      }

      http::response<http::string_body> toBeast(const HttpResponse& res, unsigned version) {
         http::response<http::string_body> out{http::status(res.status), version};
         out.set(http::field::server, "c2server");
         out.set(http::field::content_type, res.contentType);
         out.keep_alive(false);
         out.body() = res.body;
         out.prepare_payload();
         return out;
      }

   } // namespace

   net::awaitable<void> runSession(tcp::socket socket, HttpHandler handler) {
      const auto ip = socket.remote_endpoint().address().to_string();
      log::debug("Connection from {}", ip);
      try {
         beast::flat_buffer buf;
         http::request<http::string_body> req;
         co_await http::async_read(socket, buf, req, use_awaitable);

         auto res = handler(toDomain(req, ip));
         co_await http::async_write(socket, toBeast(res, req.version()), use_awaitable);

         socket.shutdown(tcp::socket::shutdown_send);
      } catch (const std::exception& e) {
         log::error("Session [{}]: {}", ip, e.what());
      }
   }

} // namespace c2server
