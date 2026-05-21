module c2server.server;

import c2server.error;
import std;

namespace c2server {

   Server::Server(Host h, Port p, std::shared_ptr<Router> router)
       : Server(ServerSettings{.host = std::move(h.value), .port = p.value}, std::move(router)) {}

   Server::Server(ServerSettings settings, std::shared_ptr<Router> router)
       : settings_{std::move(settings)}
       , router_{std::move(router)} {
      validate(settings_);
      if (!router_) {
         throw ServerError{"router must not be null"};
      }
      router_->freeze();
   }

   Server& Server::setShutdownCallback(ShutdownCallback callback) {
      shutdownCallback_ = std::move(callback);
      return *this;
   }

   void Server::run() {
      detail::runServer(settings_, router_, shutdownCallback_);
   }

} // namespace c2server
