#pragma once

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/system/error_code.hpp>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>

namespace c2server::detail {

   namespace net = boost::asio;
   namespace beast = boost::beast;
   namespace http = beast::http;
   namespace ssl = net::ssl;
   using tcp = net::ip::tcp;
   class ShutdownSignalHandler {
   public:
      ShutdownSignalHandler(net::io_context& ioc,
                            c2server::ShutdownCallback callback,
                            c2server::ShutdownCallback repeatedCallback);

      ShutdownSignalHandler(const ShutdownSignalHandler&) = delete;
      ShutdownSignalHandler& operator=(const ShutdownSignalHandler&) = delete;

      ~ShutdownSignalHandler();

      void start();
      void cancel();

   private:
      void handleSignal(const boost::system::error_code& ec, int signalNumber);
      void addShutdownSignals();

      net::signal_set signals_;
      c2server::ShutdownCallback callback_;
      c2server::ShutdownCallback repeatedCallback_;
      std::atomic_bool handled_{false};
   };

   ssl::context makeSslContext(const SslSettings& settings);

   net::awaitable<void> runPlainSession(tcp::socket socket,
                                        HttpHandler handler,
                                        std::uint64_t requestBodyLimitBytes,
                                        std::uint64_t requestTimeoutSeconds);

   net::awaitable<void> runFlexSession(tcp::socket socket,
                                       std::shared_ptr<ssl::context> sslContext,
                                       bool allowPlainHttp,
                                       HttpHandler handler,
                                       std::uint64_t requestBodyLimitBytes,
                                       std::uint64_t requestTimeoutSeconds);

   net::awaitable<void> listen(std::shared_ptr<tcp::acceptor> acceptor,
                               std::shared_ptr<ssl::context> sslContext,
                               bool allowPlainHttp,
                               HttpHandler handler,
                               std::uint64_t requestBodyLimitBytes,
                               std::uint64_t requestTimeoutSeconds);

   void runServer(const ServerSettings& settings,
                  std::shared_ptr<Router> router,
                  c2server::ShutdownCallback shutdownCallback);

} // namespace c2server::detail
