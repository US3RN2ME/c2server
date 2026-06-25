import c2server.config;
import c2server.http;
import c2server.logger;
import c2server.middleware;
import c2server.router;
import c2server.server;
import c2server.version;
import std;

int main(int argc, char* argv[]) {
   try {
      const auto configPath = argc > 1 ? std::string_view{argv[1]} : std::string_view{"config.json"};
      auto config = c2server::loadAppConfig(configPath);

      auto logShutdownGuard = c2server::log::init(config.logFile);
      auto router = std::make_shared<c2server::Router>();
      router->use(c2server::middleware::requestId())
          .use(c2server::middleware::accessLog())
          .use(c2server::middleware::securityHeaders())
          .use(c2server::middleware::cors());

      auto healthDoc = c2server::RouteDoc{
          .summary = "Health check",
          .tags = {"system"},
          .responses = {{.status = 200, .description = "Service status", .contentType = "application/json"}},
      };
      router->get(
          "/health",
          [](const c2server::HttpRequest&) {
             return c2server::jsonOk({{"status", "ok"}});
          },
          std::move(healthDoc));

      auto echoDoc = c2server::RouteDoc{
          .summary = "Echo request body",
          .tags = {"examples"},
          .requestBody =
              c2server::RequestBodyDoc{
                  .description = "Text to echo back.",
                  .contentType = "text/plain",
                  .required = true,
              },
          .responses = {{.status = 200, .description = "Echoed text", .contentType = "text/plain"}},
      };
      router->post(
          "/echo",
          [](const c2server::HttpRequest& req) {
             return c2server::ok(req.body, "text/plain");
          },
          std::move(echoDoc));

      router->serveOpenApi({
          .info =
              {
                  .title = "c2server Swagger example",
                  .version = std::string{c2server::kVersion},
                  .description = "Example API with generated OpenAPI JSON and Swagger UI.",
              },
      });

      c2server::Server{std::move(config.server), router}.run();
      return 0;
   } catch (const std::exception& e) {
      std::println(std::cerr, "swagger_server: {}", e.what());
      return 1;
   }
}
