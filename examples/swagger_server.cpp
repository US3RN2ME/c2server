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

      router->get(
          "/health",
          [](const c2server::HttpRequest&) {
             return c2server::jsonOk({{"status", "ok"}});
          },
          c2server::RouteDoc{
              .summary = "Health check",
              .tags = {"system"},
              .responses =
                  {
                      {.status = 200, .description = "Service is healthy", .contentType = "application/json"},
                  },
          });

      router->get(
          "/version",
          [](const c2server::HttpRequest&) {
             return c2server::jsonOk({
                 {"version", std::string{c2server::kVersion}},
                 {"major", std::to_string(c2server::kVersionMajor)},
                 {"minor", std::to_string(c2server::kVersionMinor)},
                 {"patch", std::to_string(c2server::kVersionPatch)},
             });
          },
          c2server::RouteDoc{
              .summary = "Server version",
              .description = "Returns the compiled version components.",
              .tags = {"system"},
              .responses =
                  {
                      {.status = 200, .description = "Version object", .contentType = "application/json"},
                  },
          });

      router->post(
          "/echo",
          [](const c2server::HttpRequest& req) {
             return c2server::ok(req.body, "text/plain");
          },
          c2server::RouteDoc{
              .summary = "Echo request body",
              .tags = {"examples"},
              .requestBody =
                  c2server::RequestBodyDoc{
                      .description = "Text to echo back.",
                      .contentType = "text/plain",
                      .required = true,
                  },
              .responses =
                  {
                      {.status = 200, .description = "Echoed text", .contentType = "text/plain"},
                  },
          });

      router->put(
          "/items",
          [](const c2server::HttpRequest& req) {
             return c2server::jsonOk({{"replaced", req.body}});
          },
          c2server::RouteDoc{
              .summary = "Replace item",
              .tags = {"examples"},
              .requestBody =
                  c2server::RequestBodyDoc{
                      .description = "New item content.",
                      .contentType = "text/plain",
                      .required = true,
                  },
              .responses =
                  {
                      {.status = 200, .description = "Item replaced", .contentType = "application/json"},
                  },
          });

      router->patch(
          "/items",
          [](const c2server::HttpRequest& req) {
             return c2server::jsonOk({{"patched", req.body}});
          },
          c2server::RouteDoc{
              .summary = "Patch item",
              .tags = {"examples"},
              .requestBody =
                  c2server::RequestBodyDoc{
                      .description = "Fields to update.",
                      .contentType = "text/plain",
                      .required = true,
                  },
              .responses =
                  {
                      {.status = 200, .description = "Item patched", .contentType = "application/json"},
                  },
          });

      router->delete_(
          "/items",
          [](const c2server::HttpRequest&) {
             return c2server::jsonOk({{"status", "deleted"}});
          },
          c2server::RouteDoc{
              .summary = "Delete item",
              .tags = {"examples"},
              .responses =
                  {
                      {.status = 200, .description = "Item deleted", .contentType = "application/json"},
                  },
          });

      router->head(
          "/items",
          [](const c2server::HttpRequest&) {
             c2server::HttpResponse resp;
             resp.status = 200;
             resp.headers["X-Count"] = "0";
             return resp;
          },
          c2server::RouteDoc{
              .summary = "Item count (HEAD)",
              .tags = {"examples"},
              .responses = {{.status = 200, .description = "X-Count header contains item count"}},
          });

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
