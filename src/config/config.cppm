export module c2server.config;

import c2server.error;
import std;

export namespace c2server {

   /** @brief TLS configuration for the server listener. */
   struct SslSettings {
      /** @brief Enable TLS connections. */
      bool enabled = false;
      /** @brief Allow plain HTTP connections when TLS is enabled. */
      bool allowPlainHttp = true;
      /** @brief Path to the PEM certificate chain file. */
      std::string certificateChainFile;
      /** @brief Path to the PEM private key file. */
      std::string privateKeyFile;
      /** @brief Password used to decrypt the private key. */
      std::string privateKeyPassword;
      /** @brief Path to the PEM Diffie-Hellman parameters file. */
      std::string dhParamsFile;
   };

   /** @brief Runtime configuration for an HTTP server. */
   struct ServerSettings {
      /** @brief Network interface or address to bind. */
      std::string host = "0.0.0.0";
      /** @brief TCP port to bind. */
      std::uint16_t port = 8080;
      /** @brief Number of worker threads, or zero to select automatically. */
      std::size_t workerThreads = 0;
      /** @brief Maximum accepted request body size in bytes. */
      std::uint64_t requestBodyLimitBytes = 1024 * 1024;
      /** @brief Maximum request processing time in seconds. */
      std::uint64_t requestTimeoutSeconds = 30;
      /** @brief TLS configuration. */
      SslSettings ssl;
   };

   /** @brief Strong type for a server host value. */
   struct Host {
      /** @brief Host name or IP address. */
      std::string value;
   };

   /** @brief Strong type for a server port value. */
   struct Port {
      /** @brief TCP port number. */
      std::uint16_t value;
   };

   /** @brief Application-level configuration. */
   struct AppConfig {
      /** @brief HTTP server configuration. */
      ServerSettings server;
      /** @brief Path to the application log file. */
      std::string logFile = "c2server.log";
   };

   /**
    * @brief Create the default application configuration.
    *
    * @return Default application configuration.
    */
   [[nodiscard]] AppConfig defaultAppConfig();
   /**
    * @brief Load application configuration from a file.
    *
    * @param path Path to the configuration file.
    *
    * @return Parsed application configuration.
    */
   [[nodiscard]] AppConfig loadAppConfig(std::string_view path);
   /**
    * @brief Load server configuration from a file.
    *
    * @param path Path to the configuration file.
    *
    * @return Parsed server configuration.
    */
   [[nodiscard]] ServerSettings loadServerSettings(std::string_view path);
   /**
    * @brief Load REST server configuration from a file.
    *
    * @param path Path to the configuration file.
    *
    * @return Parsed server configuration.
    */
   [[nodiscard]] ServerSettings loadRestServerSettings(std::string_view path);
   /**
    * @brief Validate server configuration.
    *
    * @param settings Server configuration to validate.
    */
   void validate(const ServerSettings& settings);

} // namespace c2server
