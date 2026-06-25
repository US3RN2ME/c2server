import c2server.config;
import c2server.http;
import c2server.middleware;
import c2server.router;
import c2server.server;
import std;

int main(int argc, char* argv[]) {
   try {
      const auto configPath = argc > 1 ? std::string_view{argv[1]} : std::string_view{"config.json"};
      auto settings = c2server::loadServerSettings(configPath);

      auto router = std::make_shared<c2server::Router>();
      router->use(c2server::middleware::requestId())
          .use(c2server::middleware::securityHeaders())
          .use(c2server::middleware::cors())
          .use(c2server::middleware::rateLimit({.maxRequests = 10, .window = std::chrono::seconds{60}}));

      router->get("/headers", [](const c2server::HttpRequest& req) {
         return c2server::jsonOk({
             {"request_id", std::string{c2server::header(req.headers, "X-Request-ID")}},
             {"remote_ip", req.remoteIp},
         });
      });

      c2server::Server{std::move(settings), router}.run();
      return 0;
   } catch (const std::exception& e) {
      std::println(std::cerr, "middleware_server: {}", e.what());
      return 1;
   }
}
