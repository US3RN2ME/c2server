#include "ut_main.hpp"

namespace {

   c2server::HttpRequest makeRequest(c2server::HttpMethod method, std::string path) {
      return {
          .method = method,
          .methodText = "TEST",
          .target = path,
          .path = std::move(path),
      };
   }

   suite<"[Router]"> routerTests = [] {
      "[MiddlewareRunsInDeclarationOrder]"_test = [] {
         auto router = c2server::test::makeRouter();
         auto calls = std::vector<std::string>{};
         router->use([&](const c2server::HttpRequest& req, const c2server::Next& next) {
            calls.push_back("before-1");
            auto response = next(req);
            calls.push_back("after-1");
            return response;
         });
         router->use([&](const c2server::HttpRequest& req, const c2server::Next& next) {
            calls.push_back("before-2");
            auto response = next(req);
            calls.push_back("after-2");
            return response;
         });

         expect(200_u == router->handle(makeRequest(c2server::HttpMethod::Get, "/ready")).status);
         expect(std::vector<std::string>{"before-1", "before-2", "after-2", "after-1"} == calls);
      };

      "[HandlerFailureReturnsInternalServerError]"_test = [] {
         auto router = c2server::test::makeRouter();
         router->get("/fail", [](const c2server::HttpRequest&) -> c2server::HttpResponse {
            throw c2server::EndpointError{"expected failure"};
         });

         const auto response = router->handle(makeRequest(c2server::HttpMethod::Get, "/fail"));
         expect(500_u == response.status);
         expect("internal server error" == response.body);
      };

      "[PipelineFailureReturnsInternalServerError]"_test = [] {
         auto router = c2server::test::makeRouter();
         router->use([](const c2server::HttpRequest&, const c2server::Next&) -> c2server::HttpResponse {
            throw c2server::MiddlewareError{"expected failure"};
         });

         const auto response = router->handle(makeRequest(c2server::HttpMethod::Get, "/ready"));
         expect(500_u == response.status);
         expect("internal server error" == response.body);
      };

      "[FrozenRouterRejectsMutation]"_test = [] {
         auto router = c2server::test::makeRouter();
         router->freeze();
         expect(router->frozen());
         expect(throws<c2server::RouterError>([&] {
            router->get("/late", [](const c2server::HttpRequest&) {
               return c2server::ok("late");
            });
         }));
         expect(throws<c2server::RouterError>([&] {
            router->use([](const c2server::HttpRequest& req, const c2server::Next& next) {
               return next(req);
            });
         }));
      };

      "[RejectsEmptyHandlersAndMiddleware]"_test = [] {
         auto router = c2server::test::makeRouter();
         expect(throws<c2server::EndpointError>([&] {
            router->get("/empty", {});
         }));
         expect(throws<c2server::RouterError>([&] {
            router->use({});
         }));
      };
   };

   suite<"[Server]"> serverTests = [] {
      "[ConstructorFreezesRouter]"_test = [] {
         auto router = c2server::test::makeRouter();
         c2server::Server server{c2server::test::makeServerSettings(c2server::test::findAvailablePort()), router};
         expect(router->frozen());
      };

      "[RejectsNullRouter]"_test = [] {
         expect(throws<c2server::ServerError>([] {
            c2server::Server server{c2server::test::makeServerSettings(c2server::test::findAvailablePort()), nullptr};
         }));
      };

      "[StopTokenStopsRunningServer]"_test = [] {
         const auto port = c2server::test::findAvailablePort();
         auto router = c2server::test::makeRouter();
         c2server::test::RunningServer server{c2server::test::makeServerSettings(port), router};
         expect(200_u == server.plainClient.get("/ready").status);
         server.thread.request_stop();
         expect(server.completion.wait_for(std::chrono::seconds{1}) == std::future_status::ready);
      };
   };

   suite<"[Config]"> configTests = [] {
      "[RejectsInvalidServerSettings]"_test = [] {
         auto settings = c2server::ServerSettings{};
         settings.host.clear();
         expect(throws<c2server::ConfigError>([&] {
            c2server::validate(settings);
         }));

         settings = {};
         settings.requestBodyLimitBytes = 0;
         expect(throws<c2server::ConfigError>([&] {
            c2server::validate(settings);
         }));

         settings = {};
         settings.ssl.enabled = true;
         expect(throws<c2server::ConfigError>([&] {
            c2server::validate(settings);
         }));
      };
   };

} // namespace
