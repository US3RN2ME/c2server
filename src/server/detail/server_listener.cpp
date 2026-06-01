module;
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>

module c2server.server;

import :detail;
import c2server.error;
import c2server.http;
import c2server.logger;
import std;

namespace c2server::detail {

   net::awaitable<void> listen(std::shared_ptr<tcp::acceptor> acceptor, std::shared_ptr<ssl::context> sslContext,
                               bool allowPlainHttp, HttpHandler handler, std::uint64_t requestBodyLimitBytes,
                               std::uint64_t requestTimeoutSeconds) {
      log::info("Listening on {}:{}", acceptor->local_endpoint().address().to_string(), acceptor->local_endpoint().port());

      try {
         auto executor = co_await net::this_coro::executor;
         while (true) {
            auto socketExecutor = net::make_strand(executor);
            auto socket = co_await acceptor->async_accept(socketExecutor, net::use_awaitable);
            if (sslContext) {
               net::co_spawn(std::move(socketExecutor),
                             runFlexSession(std::move(socket), sslContext, allowPlainHttp, handler, requestBodyLimitBytes,
                                            requestTimeoutSeconds),
                             net::detached);
            } else {
               net::co_spawn(std::move(socketExecutor),
                             runPlainSession(std::move(socket), handler, requestBodyLimitBytes, requestTimeoutSeconds),
                             net::detached);
            }
         }
      } catch (const std::exception& e) {
         log::error("Accept loop stopped: {}", AcceptorError{"{}", e.what()}.what());
      }
   }

} // namespace c2server::detail
