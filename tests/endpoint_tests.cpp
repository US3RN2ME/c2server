#include "ut_main.hpp"

import c2server.middleware;

namespace {

   std::shared_ptr<c2server::Router> makeEndpointRouter() {
      auto router = c2server::test::makeRouter();
      router->use(c2server::middleware::requestId())
          .use(c2server::middleware::securityHeaders())
          .use(c2server::middleware::cors({.allowedOrigins = {"https://example.com"}}));
      router->get("/hello", [](const c2server::HttpRequest& req) {
         return c2server::ok(req.query);
      });
      router->post("/echo", [](const c2server::HttpRequest& req) {
         return c2server::ok(req.body, "application/octet-stream");
      });
      return router;
   }

   suite<"[Endpoint]"> endpointTests = [] {
      "[GetMatchesPathWithoutQueryString]"_test = [] {
         c2server::test::RunningServer server{makeEndpointRouter()};
         const auto response = server.plainClient.get("/hello?name=cpp23", {
                                                                               {"Origin", "https://example.com"},
                                                                               {"X-Request-ID", "endpoint-test"},
                                                                           });
         expect(200_u == response.status);
         expect("name=cpp23" == response.body);
         expect("endpoint-test" == c2server::header(response.headers, "X-Request-ID"));
         expect("https://example.com" == c2server::header(response.headers, "Access-Control-Allow-Origin"));
         expect("nosniff" == c2server::header(response.headers, "X-Content-Type-Options"));
      };

      "[PostReceivesBody]"_test = [] {
         c2server::test::RunningServer server{makeEndpointRouter()};
         const auto response = server.plainClient.post("/echo", "payload");
         expect(200_u == response.status);
         expect("payload" == response.body);
         expect("application/octet-stream" == response.contentType);
      };

      "[MissingReturns404]"_test = [] {
         c2server::test::RunningServer server{makeEndpointRouter()};
         const auto response = server.plainClient.get("/missing");
         expect(404_u == response.status);
         expect("not found" == response.body);
      };

      "[CorsHandlesPreflight]"_test = [] {
         c2server::test::RunningServer server{makeEndpointRouter()};
         const auto response = server.plainClient.request(c2server::HttpMethod::Options, "/echo", {},
                                                          {
                                                              {"Origin", "https://example.com"},
                                                              {"Access-Control-Request-Method", "POST"},
                                                          });
         expect(204_u == response.status);
         expect("https://example.com" == c2server::header(response.headers, "Access-Control-Allow-Origin"));
         expect("GET, POST, PUT, PATCH, DELETE, HEAD, OPTIONS" ==
                c2server::header(response.headers, "Access-Control-Allow-Methods"));
      };
   };

} // namespace
