module;
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/system/error_code.hpp>
#include <csignal>

module c2server.server;

import c2server.error;
import c2server.http;
import c2server.logger;
import std;

#include "server_detail.hpp"

namespace c2server::detail {

   ShutdownSignalHandler::ShutdownSignalHandler(net::io_context& ioc,
                                                c2server::ShutdownCallback callback,
                                                c2server::ShutdownCallback repeatedCallback)
       : signals_{ioc}
       , callback_{std::move(callback)}
       , repeatedCallback_{std::move(repeatedCallback)} {
      if (!callback_ || !repeatedCallback_) {
         throw SignalHandlerError{"shutdown callback must not be empty"};
      }

      addShutdownSignals();
   }

   ShutdownSignalHandler::~ShutdownSignalHandler() {
      cancel();
   }

   void ShutdownSignalHandler::start() {
      signals_.async_wait([this](const boost::system::error_code& ec, int signalNumber) {
         handleSignal(ec, signalNumber);
      });
   }

   void ShutdownSignalHandler::cancel() {
      boost::system::error_code ec;
      signals_.cancel(ec);
      if (ec) {
         log::debug("Failed to cancel shutdown signal handler: {}", ec.message());
      }
   }

   void ShutdownSignalHandler::handleSignal(const boost::system::error_code& ec, int signalNumber) {
      if (ec == net::error::operation_aborted) {
         return;
      }

      if (ec) {
         log::error("Shutdown signal wait failed: {}", ec.message());
         return;
      }

      try {
         if (handled_.exchange(true)) {
            repeatedCallback_(signalNumber);
            return;
         }

         callback_(signalNumber);
         start();
      } catch (const std::exception& e) {
         log::error("Shutdown callback failed: {}", e.what());
      } catch (...) {
         log::error("Shutdown callback failed with an unknown exception");
      }
   }

   void ShutdownSignalHandler::addShutdownSignals() {
      boost::system::error_code ec;

      signals_.add(SIGINT, ec);
      if (ec) {
         throw SignalHandlerError{"failed to register SIGINT handler: {}", ec.message()};
      }

      signals_.add(SIGTERM, ec);
      if (ec) {
         throw SignalHandlerError{"failed to register SIGTERM handler: {}", ec.message()};
      }

#ifdef SIGBREAK
      signals_.add(SIGBREAK, ec);
      if (ec) {
         throw SignalHandlerError{"failed to register SIGBREAK handler: {}", ec.message()};
      }
#endif
   }

} // namespace c2server::detail
