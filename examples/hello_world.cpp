import c2server.config;
import c2server.http;
import c2server.router;
import c2server.server;
import std;

int main(int argc, char* argv[]) {
   try {
      const auto configPath = argc > 1 ? std::string_view{argv[1]} : std::string_view{"config.json"};
      auto settings = c2server::loadServerSettings(configPath);

      auto router = std::make_shared<c2server::Router>();
      router->get("/", [](const c2server::HttpRequest&) {
         return c2server::ok("hello from c2server\n");
      });

      c2server::Server{std::move(settings), router}.run();
      return 0;
   } catch (const std::exception& e) {
      std::println(std::cerr, "hello_world: {}", e.what());
      return 1;
   }
}
