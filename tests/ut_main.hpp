#pragma once

#include <boost/asio.hpp>
#include <boost/ut.hpp>

import c2server.client;
import c2server.config;
import c2server.error;
import c2server.http;
import c2server.router;
import c2server.server;
import std;

using namespace boost::ut;

namespace c2server::test {

   inline constexpr std::string_view kLoopbackAddress = "127.0.0.1";
   inline constexpr std::string_view kReadyPath = "/ready";
   inline constexpr std::string_view kTestSourceDir = C2SERVER_TEST_SOURCE_DIR;
   inline constexpr auto kTimeout = std::chrono::seconds{2};

   inline std::string assetPath(std::string_view name) {
      return std::format("{}/assets/{}", kTestSourceDir, name);
   }

   inline std::uint16_t findAvailablePort() {
      boost::asio::io_context ioc;
      boost::asio::ip::tcp::acceptor acceptor{ioc, {boost::asio::ip::tcp::v4(), 0}};
      return acceptor.local_endpoint().port();
   }

   inline std::shared_ptr<Router> makeRouter() {
      auto router = std::make_shared<Router>();
      router->get(std::string{kReadyPath}, [](const HttpRequest&) {
         return ok("ready");
      });
      return router;
   }

   inline Client makePlainClient(std::uint16_t port) {
      return Client{{.host = std::string{kLoopbackAddress}, .port = port, .timeout = kTimeout}};
   }

   inline Client makeTlsClient(std::uint16_t port, bool trustTestCertificate = true) {
      return Client{{
          .host = std::string{kLoopbackAddress},
          .port = port,
          .timeout = kTimeout,
          .ssl =
              {
                  .enabled = true,
                  .verifyPeer = true,
                  .caFile = trustTestCertificate ? assetPath("localhost.crt") : std::string{},
              },
      }};
   }

   inline ServerSettings makeServerSettings(std::uint16_t port) {
      auto settings = ServerSettings{};
      settings.host = kLoopbackAddress;
      settings.port = port;
      settings.workerThreads = 1;
      settings.requestTimeoutSeconds = kTimeout.count();
      return settings;
   }

   inline ServerSettings makeSslServerSettings(std::uint16_t port, bool allowPlainHttp) {
      auto settings = makeServerSettings(port);
      settings.ssl = {
          .enabled = true,
          .allowPlainHttp = allowPlainHttp,
          .certificateChainFile = assetPath("localhost.crt"),
          .privateKeyFile = assetPath("localhost.key"),
      };
      return settings;
   }

   struct RunningServer {
      RunningServer(ServerSettings settings, std::shared_ptr<Router> router, bool useTlsForReadiness = false)
          : port{settings.port}
          , completion{completionPromise.get_future()}
          , thread{[this, settings = std::move(settings), router = std::move(router)](std::stop_token stopToken) mutable {
             try {
                Server server{std::move(settings), std::move(router)};
                server.run(stopToken);
                completionPromise.set_value();
             } catch (...) {
                completionPromise.set_exception(std::current_exception());
             }
          }}
          , plainClient{makePlainClient(port)}
          , tlsClient{useTlsForReadiness ? makeTlsClient(port) : makePlainClient(port)}
          , readinessClient{useTlsForReadiness ? &tlsClient : &plainClient} {
         waitUntilReady();
      }

      RunningServer(std::shared_ptr<Router> router, bool useSsl = false, bool allowPlainHttp = true)
          : RunningServer{useSsl ? makeSslServerSettings(findAvailablePort(), allowPlainHttp)
                                 : makeServerSettings(findAvailablePort()),
                          std::move(router), useSsl} {}

      ~RunningServer() {
         thread.request_stop();
      }

      std::uint16_t port;
      std::promise<void> completionPromise;
      std::future<void> completion;
      std::jthread thread;
      Client plainClient;
      Client tlsClient;
      Client* readinessClient;

   private:
      void waitUntilReady() {
         for (auto attempt = 0; attempt < 100; ++attempt) {
            if (completion.wait_for(std::chrono::milliseconds{0}) == std::future_status::ready) {
               completion.get();
               throw ServerError{"server stopped before accepting requests"};
            }
            try {
               static_cast<void>(readinessClient->get(std::string{kReadyPath}));
               return;
            } catch (const ClientError&) {
               std::this_thread::sleep_for(std::chrono::milliseconds{10});
            }
         }
         throw ServerError{"server did not start accepting requests"};
      }
   };

} // namespace c2server::test

int main(int argc, const char* argv[]) {
   return cfg<override>.run({.argc = argc, .argv = argv});
}
