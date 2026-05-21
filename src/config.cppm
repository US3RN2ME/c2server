export module c2server.config;

import c2server.error;
import std;

export namespace c2server {

   struct SslSettings {
      bool enabled = false;
      bool allowPlainHttp = true;
      std::string certificateChainFile;
      std::string privateKeyFile;
      std::string privateKeyPassword;
      std::string dhParamsFile;
   };

   struct ServerSettings {
      std::string host = "0.0.0.0";
      std::uint16_t port = 8080;
      std::size_t workerThreads = 0;
      std::uint64_t requestBodyLimitBytes = 1024 * 1024;
      std::uint64_t requestTimeoutSeconds = 30;
      SslSettings ssl;
   };

   struct Host {
      std::string value;
   };

   struct Port {
      std::uint16_t value;
   };

   struct AppConfig {
      ServerSettings server;
      std::string logFile = "c2server.log";
   };

   [[nodiscard]] AppConfig defaultAppConfig();
   [[nodiscard]] AppConfig loadAppConfig(std::string_view path);
   [[nodiscard]] ServerSettings loadServerSettings(std::string_view path);
   [[nodiscard]] ServerSettings loadRestServerSettings(std::string_view path);
   void validate(const ServerSettings& settings);

} // namespace c2server
