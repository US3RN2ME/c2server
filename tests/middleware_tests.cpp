#include "ut_main.hpp"

import c2server.middleware;

namespace {

   std::shared_ptr<c2server::Router> makeMiddlewareRouter() {
      auto router = c2server::test::makeRouter();
      router->get("/ok", [](const c2server::HttpRequest& req) {
         return c2server::ok(std::string{c2server::header(req.headers, "X-Request-ID")});
      });
      return router;
   }

   suite<"[Middleware]"> middlewareTests = [] {
      "[RateLimitRejectsAfterThreshold]"_test = [] {
         auto router = makeMiddlewareRouter();
         router->use(c2server::middleware::rateLimit({.maxRequests = 2, .window = std::chrono::seconds{60}}));
         c2server::test::RunningServer server{router};

         expect(200_u == server.plainClient.get("/ok").status);
         expect(429_u == server.plainClient.get("/ok").status);
         const auto rejected = server.plainClient.get("/ok");
         expect(429_u == rejected.status);
         expect("too many requests" == rejected.body);
         const auto retryAfter = std::stoul(std::string{c2server::header(rejected.headers, "Retry-After")});
         expect(retryAfter > 0_u);
         expect(retryAfter <= 60_u);
      };

      "[RateLimitResetsAfterWindow]"_test = [] {
         auto router = makeMiddlewareRouter();
         router->use(c2server::middleware::rateLimit({.maxRequests = 2, .window = std::chrono::seconds{1}}));
         c2server::test::RunningServer server{router};

         expect(200_u == server.plainClient.get("/ok").status);
         expect(429_u == server.plainClient.get("/ok").status);
         std::this_thread::sleep_for(std::chrono::milliseconds{1100});
         expect(200_u == server.plainClient.get("/ok").status);
      };

      "[RequestIdGeneratesAndPreservesIds]"_test = [] {
         auto router = makeMiddlewareRouter();
         router->use(c2server::middleware::requestId());
         c2server::test::RunningServer server{router};

         const auto generated = server.plainClient.get("/ok");
         expect(!c2server::header(generated.headers, "X-Request-ID").empty());
         expect(c2server::header(generated.headers, "X-Request-ID") == generated.body);

         const auto preserved = server.plainClient.get("/ok", {{"X-Request-ID", "known-id"}});
         expect("known-id" == c2server::header(preserved.headers, "X-Request-ID"));
         expect("known-id" == preserved.body);
      };

      "[RequestIdReplacesOversizedIds]"_test = [] {
         auto router = makeMiddlewareRouter();
         router->use(c2server::middleware::requestId());
         c2server::test::RunningServer server{router};

         const auto response = server.plainClient.get("/ok", {{"X-Request-ID", std::string(129, 'x')}});
         expect(200_u == response.status);
         expect(129_u != response.body.size());
         expect(c2server::header(response.headers, "X-Request-ID") == response.body);
      };

      "[CorsOmitsHeadersForDisallowedOrigin]"_test = [] {
         auto router = makeMiddlewareRouter();
         router->use(c2server::middleware::cors({.allowedOrigins = {"https://allowed.example"}}));
         c2server::test::RunningServer server{router};

         const auto response = server.plainClient.get("/ok", {{"Origin", "https://denied.example"}});
         expect(200_u == response.status);
         expect(c2server::header(response.headers, "Access-Control-Allow-Origin").empty());
      };

      "[CorsCredentialsEchoAllowedOrigin]"_test = [] {
         auto router = makeMiddlewareRouter();
         router->use(c2server::middleware::cors({.allowedOrigins = {"*"}, .allowCredentials = true}));
         c2server::test::RunningServer server{router};

         const auto response = server.plainClient.get("/ok", {{"Origin", "https://app.example"}});
         expect("https://app.example" == c2server::header(response.headers, "Access-Control-Allow-Origin"));
         expect("true" == c2server::header(response.headers, "Access-Control-Allow-Credentials"));
      };

      "[SecurityHeadersAreApplied]"_test = [] {
         auto router = makeMiddlewareRouter();
         router->use(c2server::middleware::securityHeaders());
         c2server::test::RunningServer server{router};

         const auto response = server.plainClient.get("/ok");
         expect("nosniff" == c2server::header(response.headers, "X-Content-Type-Options"));
         expect("DENY" == c2server::header(response.headers, "X-Frame-Options"));
         expect("no-referrer" == c2server::header(response.headers, "Referrer-Policy"));
         expect("same-origin" == c2server::header(response.headers, "Cross-Origin-Resource-Policy"));
      };

      "[SecurityHeadersPreserveExplicitCsp]"_test = [] {
         auto router = makeMiddlewareRouter();
         router->get("/custom-csp", [](const c2server::HttpRequest&) {
            auto response = c2server::ok("custom");
            c2server::setHeader(response, "Content-Security-Policy", "default-src 'self'");
            return response;
         });
         router->use(c2server::middleware::securityHeaders());
         c2server::test::RunningServer server{router};

         const auto response = server.plainClient.get("/custom-csp");
         expect("default-src 'self'" == c2server::header(response.headers, "Content-Security-Policy"));
         expect("nosniff" == c2server::header(response.headers, "X-Content-Type-Options"));
      };

      "[CustomMiddlewareCanShortCircuit]"_test = [] {
         auto router = makeMiddlewareRouter();
         router->use([](const c2server::HttpRequest&, const c2server::Next&) {
            return c2server::HttpResponse{.status = 401, .body = "blocked"};
         });
         c2server::test::RunningServer server{router};

         const auto response = server.plainClient.get("/ok");
         expect(401_u == response.status);
         expect("blocked" == response.body);
      };
   };

} // namespace
