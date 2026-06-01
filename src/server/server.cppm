export module c2server.server;

import c2server.config;
import c2server.error;
import c2server.router;
import std;

export namespace c2server {

   /** @brief Callback invoked when server shutdown is requested. */
   using ShutdownCallback = std::function<void(int signalNumber)>;

   /** @brief Configurable synchronous HTTP and HTTPS server. */
   struct Server {
      /**
       * @brief Construct a plain HTTP server.
       *
       * @param h Network interface or address to bind.
       * @param p TCP port to bind.
       * @param router Request router.
       */
      Server(Host h, Port p, std::shared_ptr<Router> router);
      /**
       * @brief Construct a server with runtime settings.
       *
       * @param settings Runtime server settings.
       * @param router Request router.
       */
      Server(ServerSettings settings, std::shared_ptr<Router> router);
      /**
       * @brief Set the callback invoked during shutdown.
       *
       * @param callback Shutdown callback.
       *
       * @return This server.
       */
      Server& setShutdownCallback(ShutdownCallback callback);
      /** @brief Run the server until shutdown is requested. */
      void run();
      /**
       * @brief Run the server until shutdown or stop is requested.
       *
       * @param stopToken Cooperative stop token.
       */
      void run(std::stop_token stopToken);

   private:
      ServerSettings settings_;
      std::shared_ptr<Router> router_;
      ShutdownCallback shutdownCallback_;
   };

} // namespace c2server
