export module c2server.server;

import c2server.config;
import c2server.error;
import c2server.router;
import std;

export namespace c2server {

   using ShutdownCallback = std::function<void(int signalNumber)>;

   struct Server {
      Server(Host h, Port p, std::shared_ptr<Router> router);
      Server(ServerSettings settings, std::shared_ptr<Router> router);
      Server& setShutdownCallback(ShutdownCallback callback);
      void run();

   private:
      ServerSettings settings_;
      std::shared_ptr<Router> router_;
      ShutdownCallback shutdownCallback_;
   };

} // namespace c2server
