module;
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/system/error_code.hpp>

module c2server.server;

import :detail;
import c2server.error;
import c2server.http;
import c2server.logger;
import std;

namespace c2server::detail {

   void runServer(const ServerSettings& settings, std::shared_ptr<Router> router, c2server::ShutdownCallback shutdownCallback) {
      const auto workerThreads =
          settings.workerThreads == 0 ? std::max(1u, std::thread::hardware_concurrency()) : settings.workerThreads;

      net::io_context ioc{static_cast<int>(workerThreads)};
      tcp::endpoint endpoint;
      auto acceptor = std::make_shared<tcp::acceptor>(ioc);
      try {
         endpoint = tcp::endpoint{net::ip::make_address(settings.host), settings.port};
         acceptor->open(endpoint.protocol());
         acceptor->set_option(net::socket_base::reuse_address(true));
         acceptor->bind(endpoint);
         acceptor->listen(net::socket_base::max_listen_connections);
      } catch (const std::exception& e) {
         throw ServerError{"failed to bind server on {}:{}: {}", settings.host, settings.port, e.what()};
      }

      auto sslContext =
          settings.ssl.enabled ? std::make_shared<ssl::context>(makeSslContext(settings.ssl)) : std::shared_ptr<ssl::context>{};

      ShutdownSignalHandler shutdownSignals{ioc,
                                            [acceptor, shutdownCallback = std::move(shutdownCallback)](int signalNumber) {
                                               log::info("Graceful shutdown requested by signal {}", signalNumber);

                                               boost::system::error_code ec;
                                               acceptor->close(ec);
                                               if (ec) {
                                                  log::warn("Failed to close listener during shutdown: {}", ec.message());
                                               }

                                               if (shutdownCallback) {
                                                  try {
                                                     shutdownCallback(signalNumber);
                                                  } catch (const std::exception& e) {
                                                     log::error("Shutdown callback failed: {}", e.what());
                                                  } catch (...) {
                                                     log::error("Shutdown callback failed with an unknown exception");
                                                  }
                                               }
                                            },
                                            [&ioc](int signalNumber) {
                                               log::warn("Forced shutdown requested by signal {}", signalNumber);
                                               ioc.stop();
                                            }};
      shutdownSignals.start();

      net::co_spawn(net::make_strand(ioc),
                    listen(
                        acceptor, sslContext, settings.ssl.allowPlainHttp,
                        [router = std::move(router)](const HttpRequest& req) {
                           return router->handle(req);
                        },
                        settings.requestBodyLimitBytes, settings.requestTimeoutSeconds),
                    net::detached);

      log::info("Server running on {}:{} with {} worker thread(s), ssl {}", settings.host, settings.port, workerThreads,
                settings.ssl.enabled ? "enabled" : "disabled");

      std::vector<std::jthread> threads;
      threads.reserve(workerThreads);
      for (std::size_t i = 0; i < workerThreads; ++i) {
         threads.emplace_back([&ioc] {
            ioc.run();
         });
      }
   }

} // namespace c2server::detail
