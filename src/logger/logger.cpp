module;
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

module c2server.logger;

import c2server.error;
import std;

namespace c2server {
   namespace log {

      ShutdownGuard::ShutdownGuard(ShutdownGuard&& other) noexcept
          : active_{std::exchange(other.active_, false)} {}

      ShutdownGuard& ShutdownGuard::operator=(ShutdownGuard&& other) noexcept {
         if (this != &other) {
            if (active_) {
               shutdown();
            }
            active_ = std::exchange(other.active_, false);
         }
         return *this;
      }

      ShutdownGuard::~ShutdownGuard() {
         if (active_) {
            shutdown();
         }
      }

      ShutdownGuard init(std::string_view logFile) {
         try {
            std::vector<spdlog::sink_ptr> sinks;
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

            if (!logFile.empty())
               sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string{logFile}));

            auto logger = std::make_shared<spdlog::logger>("c2server", sinks.begin(), sinks.end());
            spdlog::set_default_logger(logger);
            spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
            spdlog::set_level(spdlog::level::debug);
            spdlog::flush_on(spdlog::level::info);
            return {};
         } catch (const std::exception& e) {
            throw LoggerError{"failed to initialize logger: {}", e.what()};
         }
      }

      void debug(std::string_view msg) {
         spdlog::debug("{}", msg);
      }

      void info(std::string_view msg) {
         spdlog::info("{}", msg);
      }

      void warn(std::string_view msg) {
         spdlog::warn("{}", msg);
      }

      void error(std::string_view msg) {
         spdlog::error("{}", msg);
      }

      void shutdown() {
         spdlog::shutdown();
      }
   } // namespace log

} // namespace c2server
