#include "ut_main.hpp"

namespace {

   std::shared_ptr<c2server::Router> makeSslRouter() {
      auto router = c2server::test::makeRouter();
      router->get("/secure", [](const c2server::HttpRequest&) {
         return c2server::ok("secure");
      });
      return router;
   }

   suite<"[SslServer]"> sslServerTests = [] {
      "[TlsOnlyServesVerifiedTls]"_test = [] {
         c2server::test::RunningServer server{makeSslRouter(), true, false};
         const auto response = server.tlsClient.get("/secure");
         expect(200_u == response.status);
         expect("secure" == response.body);
      };

      "[TlsOnlyRejectsPlainHttp]"_test = [] {
         c2server::test::RunningServer server{makeSslRouter(), true, false};
         expect(throws<c2server::ClientError>([&] {
            static_cast<void>(server.plainClient.get("/secure"));
         }));
      };

      "[MixedModeServesPlainHttpAndTls]"_test = [] {
         c2server::test::RunningServer server{makeSslRouter(), true};
         expect(200_u == server.plainClient.get("/secure").status);
         expect(200_u == server.tlsClient.get("/secure").status);
      };

      "[ClientRejectsUntrustedCertificate]"_test = [] {
         c2server::test::RunningServer server{makeSslRouter(), true, false};
         auto untrustedClient = c2server::test::makeTlsClient(server.port, false);
         expect(throws<c2server::ClientError>([&] {
            static_cast<void>(untrustedClient.get("/secure"));
         }));
      };
   };

} // namespace
