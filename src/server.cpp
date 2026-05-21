module;
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

module c2server.server;

import c2server.acceptor;
import c2server.logger;

namespace c2server {

   namespace net = boost::asio;
   using tcp = net::ip::tcp;

   Server::Server(Host h, Port p, std::shared_ptr<Router> router)
       : host_{std::move(h)}
       , port_{p}
       , router_{std::move(router)} {}

   void Server::run() {
      net::io_context ioc{1};

      tcp::acceptor acceptor{ioc, {net::ip::make_address(host_.value), port_.value}};

      net::co_spawn(ioc.get_executor(),
                    listen(std::move(acceptor),
                           [router = router_](const HttpRequest& req) {
                              return router->handle(req);
                           }),
                    net::detached);

      log::info("Server running on {}:{}", host_.value, port_.value);

      std::vector<std::jthread> threads;
      threads.reserve(std::thread::hardware_concurrency());
      for (auto i = 0u; i < threads.capacity(); ++i)
         threads.emplace_back([&ioc] {
            ioc.run();
         });
   }

} // namespace c2server
