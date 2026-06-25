#include "ut_main.hpp"

import c2server.middleware;

namespace {

   std::shared_ptr<c2server::Router> makeEndpointRouter() {
      auto router = c2server::test::makeRouter();
      router->use(c2server::middleware::requestId())
          .use(c2server::middleware::securityHeaders())
          .use(c2server::middleware::cors({.allowedOrigins = {"https://example.com"}}));
      auto helloDoc = c2server::RouteDoc{
          .summary = "Say hello",
          .tags = {"tests"},
          .responses = {{.status = 200, .description = "Query string", .contentType = "text/plain"}},
      };
      router->get(
          "/hello",
          [](const c2server::HttpRequest& req) {
             return c2server::ok(req.query);
          },
          std::move(helloDoc));
      router->post("/echo", [](const c2server::HttpRequest& req) {
         return c2server::ok(req.body, "application/octet-stream");
      });
      router->serveOpenApi({.info = {.title = "test api", .version = "1.2.3"}});
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

      "[OpenApiEndpointReturnsGeneratedSpec]"_test = [] {
         c2server::test::RunningServer server{makeEndpointRouter()};
         const auto response = server.plainClient.get("/openapi.json");
         expect(200_u == response.status);
         expect("application/json" == response.contentType);
         expect(response.body.contains("\"openapi\": \"3.0.3\""));
         expect(response.body.contains("\"title\": \"test api\""));
         expect(response.body.contains("\"/hello\""));
         expect(response.body.contains("\"/echo\""));
         expect(response.body.contains("\"summary\": \"Say hello\""));
      };

      "[SwaggerUiEndpointReturnsDocsPage]"_test = [] {
         c2server::test::RunningServer server{makeEndpointRouter()};
         const auto response = server.plainClient.get("/docs");
         expect(200_u == response.status);
         expect("text/html" == response.contentType);
         expect(response.body.contains("SwaggerUIBundle"));
         expect(response.body.contains("/openapi.json"));
         expect(c2server::header(response.headers, "Content-Security-Policy").contains("cdn.jsdelivr.net"));
      };
   };

} // namespace
