module;
#include <boost/asio.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/system/system_error.hpp>

module c2server.server;

import :detail;
import c2server.error;
import c2server.http;
import c2server.logger;
import std;

namespace c2server::detail {

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

      HttpResponse safeHandle(const HttpHandler& handler, const HttpRequest& req) {
         try {
            return handler(req);
         } catch (const Error& e) {
            log::error("HTTP handler error for {} {}: {}", req.methodText, req.target, e.what());
            return {500, e.what(), "text/plain"};
         } catch (const std::exception& e) {
            log::error("HTTP handler error for {} {}: {}", req.methodText, req.target, e.what());
            return {500, e.what(), "text/plain"};
         } catch (...) {
            log::error("HTTP handler error for {} {}", req.methodText, req.target);
            return {500, "internal server error", "text/plain"};
         }
      }

      bool isExpectedSessionClose(const boost::system::error_code& ec) {
         return ec == beast::error::timeout || ec == http::error::end_of_stream || ec == net::error::operation_aborted ||
                ec == net::error::eof || ec == net::error::connection_reset || ec == ssl::error::stream_truncated;
      }

      void logSessionSystemError(std::string_view ip, const boost::system::error_code& ec) {
         if (ec == beast::error::timeout) {
            log::debug("Session [{}] timed out", ip);
            return;
         }

         if (isExpectedSessionClose(ec)) {
            log::debug("Session [{}] closed: {}", ip, ec.message());
            return;
         }

         log::error("Session [{}] failed: {} [{}:{}]", ip, ec.message(), ec.category().name(), ec.value());
      }

      template <class Stream>
      net::awaitable<void> runHttpSession(Stream& stream, beast::flat_buffer& buf, std::string ip, HttpHandler handler,
                                          std::uint64_t requestBodyLimitBytes, std::uint64_t requestTimeoutSeconds) {
         try {
            http::request_parser<http::string_body> parser;
            parser.body_limit(requestBodyLimitBytes);
            beast::get_lowest_layer(stream).expires_after(std::chrono::seconds{requestTimeoutSeconds});
            co_await http::async_read(stream, buf, parser, net::use_awaitable);

            auto req = parser.release();
            auto domainReq = toDomain(req, ip);
            auto res = safeHandle(handler, domainReq);
            beast::get_lowest_layer(stream).expires_after(std::chrono::seconds{requestTimeoutSeconds});
            co_await http::async_write(stream, toBeast(res, req.version()), net::use_awaitable);
         } catch (const boost::system::system_error& e) {
            logSessionSystemError(ip, e.code());
         } catch (const std::exception& e) {
            log::error("Session [{}]: {}", ip, e.what());
         }
      }

      void shutdownPlain(beast::tcp_stream& stream) {
         beast::error_code ec;
         stream.socket().shutdown(tcp::socket::shutdown_send, ec);
      }

      net::awaitable<void> shutdownSsl(ssl::stream<beast::tcp_stream>& stream, std::uint64_t requestTimeoutSeconds) {
         beast::get_lowest_layer(stream).expires_after(std::chrono::seconds{requestTimeoutSeconds});
         auto [ec] = co_await stream.async_shutdown(net::as_tuple(net::use_awaitable));
         if (ec && ec != ssl::error::stream_truncated) {
            log::debug("TLS shutdown failed: {}", ec.message());
         }
      }

   } // namespace

   net::awaitable<void> runPlainSession(tcp::socket socket, HttpHandler handler, std::uint64_t requestBodyLimitBytes,
                                        std::uint64_t requestTimeoutSeconds) {
      beast::tcp_stream stream{std::move(socket)};
      const auto ip = stream.socket().remote_endpoint().address().to_string();
      log::debug("Plain HTTP connection from {}", ip);

      beast::flat_buffer buf;
      co_await runHttpSession(stream, buf, ip, handler, requestBodyLimitBytes, requestTimeoutSeconds);
      shutdownPlain(stream);
   }

   net::awaitable<void> runFlexSession(tcp::socket socket, std::shared_ptr<ssl::context> sslContext, bool allowPlainHttp,
                                       HttpHandler handler, std::uint64_t requestBodyLimitBytes,
                                       std::uint64_t requestTimeoutSeconds) {
      beast::tcp_stream stream{std::move(socket)};
      const auto ip = stream.socket().remote_endpoint().address().to_string();
      beast::flat_buffer buf;

      try {
         stream.expires_after(std::chrono::seconds{requestTimeoutSeconds});
         if (co_await beast::async_detect_ssl(stream, buf, net::use_awaitable)) {
            log::debug("TLS connection from {}", ip);
            ssl::stream<beast::tcp_stream> sslStream{std::move(stream), *sslContext};
            beast::get_lowest_layer(sslStream).expires_after(std::chrono::seconds{requestTimeoutSeconds});
            auto bytesTransferred =
                co_await sslStream.async_handshake(ssl::stream_base::server, buf.data(), net::use_awaitable);
            buf.consume(bytesTransferred);

            co_await runHttpSession(sslStream, buf, ip, handler, requestBodyLimitBytes, requestTimeoutSeconds);
            co_await shutdownSsl(sslStream, requestTimeoutSeconds);
            co_return;
         }

         if (!allowPlainHttp) {
            log::warn("Rejected non-TLS connection from {}", ip);
            co_return;
         }

         log::debug("Plain HTTP connection from {}", ip);
         co_await runHttpSession(stream, buf, ip, handler, requestBodyLimitBytes, requestTimeoutSeconds);
         shutdownPlain(stream);
      } catch (const boost::system::system_error& e) {
         logSessionSystemError(ip, e.code());
      } catch (const std::exception& e) {
         log::error("Session [{}]: {}", ip, e.what());
      }
   }

} // namespace c2server::detail
