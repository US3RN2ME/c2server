module;
#include <nlohmann/json.hpp>

module c2server.config;

import c2server.error;
import std;

namespace c2server {

   namespace {

      template <class T>
      void setIfPresent(const nlohmann::json& json, std::string_view key, T& out) {
         if (const auto it = json.find(std::string{key}); it != json.end()) {
            out = it->get<T>();
         }
      }

      std::uint16_t readPort(const nlohmann::json& json, std::string_view key, std::uint16_t fallback) {
         if (const auto it = json.find(std::string{key}); it != json.end()) {
            const auto value = it->get<unsigned>();
            if (value == 0 || value > std::numeric_limits<std::uint16_t>::max()) {
               throw ConfigError{"server.port must be between 1 and 65535"};
            }
            return static_cast<std::uint16_t>(value);
         }
         return fallback;
      }

      ServerSettings parseSettings(const nlohmann::json& root, ServerSettings settings) {
         if (const auto server = root.find("server"); server != root.end()) {
            if (!server->is_object()) {
               throw ConfigError{"server must be a JSON object"};
            }

            setIfPresent(*server, "host", settings.host);
            settings.port = readPort(*server, "port", settings.port);
            setIfPresent(*server, "worker_threads", settings.workerThreads);
            setIfPresent(*server, "request_body_limit_bytes", settings.requestBodyLimitBytes);
            setIfPresent(*server, "request_timeout_seconds", settings.requestTimeoutSeconds);

            if (const auto ssl = server->find("ssl"); ssl != server->end()) {
               if (!ssl->is_object()) {
                  throw ConfigError{"server.ssl must be a JSON object"};
               }

               setIfPresent(*ssl, "enabled", settings.ssl.enabled);
               setIfPresent(*ssl, "allow_plain_http", settings.ssl.allowPlainHttp);
               setIfPresent(*ssl, "certificate_chain_file", settings.ssl.certificateChainFile);
               setIfPresent(*ssl, "private_key_file", settings.ssl.privateKeyFile);
               setIfPresent(*ssl, "private_key_password", settings.ssl.privateKeyPassword);
               setIfPresent(*ssl, "dh_params_file", settings.ssl.dhParamsFile);
            }
         }

         validate(settings);
         return settings;
      }

   } // namespace

   AppConfig defaultAppConfig() {
      return {
          .server = {},
          .logFile = "c2server.log",
      };
   }

   AppConfig loadAppConfig(std::string_view path) {
      auto in = std::ifstream{std::string{path}};
      if (!in) {
         throw ConfigError{"failed to open config file '{}'", path};
      }

      nlohmann::json root;
      try {
         in >> root;
      } catch (const nlohmann::json::exception& e) {
         throw ConfigError{"failed to parse config file '{}': {}", path, e.what()};
      }

      if (!root.is_object()) {
         throw ConfigError{"config root must be a JSON object"};
      }

      try {
         auto config = defaultAppConfig();
         config.server = parseSettings(root, config.server);
         setIfPresent(root, "log_file", config.logFile);

         return config;
      } catch (const ConfigError&) {
         throw;
      } catch (const nlohmann::json::exception& e) {
         throw ConfigError{"invalid config value in '{}': {}", path, e.what()};
      }
   }

   ServerSettings loadServerSettings(std::string_view path) {
      return loadAppConfig(path).server;
   }

   ServerSettings loadRestServerSettings(std::string_view path) {
      return loadServerSettings(path);
   }

   void validate(const ServerSettings& settings) {
      if (settings.host.empty()) {
         throw ConfigError{"server.host must not be empty"};
      }
      if (settings.port == 0) {
         throw ConfigError{"server.port must be between 1 and 65535"};
      }
      if (settings.requestBodyLimitBytes == 0) {
         throw ConfigError{"server.request_body_limit_bytes must be greater than 0"};
      }
      if (settings.requestTimeoutSeconds == 0) {
         throw ConfigError{"server.request_timeout_seconds must be greater than 0"};
      }
      if (settings.ssl.enabled && settings.ssl.certificateChainFile.empty()) {
         throw ConfigError{"server.ssl.certificate_chain_file must not be empty when SSL is enabled"};
      }
      if (settings.ssl.enabled && settings.ssl.privateKeyFile.empty()) {
         throw ConfigError{"server.ssl.private_key_file must not be empty when SSL is enabled"};
      }
   }

} // namespace c2server
