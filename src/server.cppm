export module c2server.server;

import c2server.config;
import c2server.router;
import std;

export namespace c2server {

   struct Server {
      Server(Host h, Port p, std::shared_ptr<Router> router);
      void run();

   private:
      Host host_;
      Port port_;
      std::shared_ptr<Router> router_;
   };

} // namespace c2server
