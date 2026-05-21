export module c2server.logger;

import std;

export namespace c2server {
   namespace log {
      class ShutdownGuard {
      public:
         ShutdownGuard(const ShutdownGuard&) = delete;
         ShutdownGuard& operator=(const ShutdownGuard&) = delete;
         ShutdownGuard(ShutdownGuard&& other) noexcept;
         ShutdownGuard& operator=(ShutdownGuard&& other) noexcept;
         ~ShutdownGuard();

      private:
         friend ShutdownGuard init(std::string_view logFile);

         ShutdownGuard() = default;

         bool active_{true};
      };

      /**
       * @brief Log a debug message.
       *
       * @param msg Message text.
       */
      void debug(std::string_view msg);

      /**
       * @brief Log an informational message.
       *
       * @param msg Message text.
       */
      void info(std::string_view msg);

      /**
       * @brief Log a warning message.
       *
       * @param msg Message text.
       */
      void warn(std::string_view msg);

      /**
       * @brief Log an error message.
       *
       * @param msg Message text.
       */
      void error(std::string_view msg);

      /**
       * @brief Format and log a debug message.
       *
       * @tparam Args Format argument types.
       *
       * @param fmt Format string.
       * @param args Format arguments.
       */
      template <class... Args>
      void debug(std::format_string<Args...> fmt, Args&&... args) {
         debug(std::format(fmt, std::forward<Args>(args)...));
      }

      /**
       * @brief Format and log an informational message.
       *
       * @tparam Args Format argument types.
       *
       * @param fmt Format string.
       * @param args Format arguments.
       */
      template <class... Args>
      void info(std::format_string<Args...> fmt, Args&&... args) {
         info(std::format(fmt, std::forward<Args>(args)...));
      }

      /**
       * @brief Format and log a warning message.
       *
       * @tparam Args Format argument types.
       *
       * @param fmt Format string.
       * @param args Format arguments.
       */
      template <class... Args>
      void warn(std::format_string<Args...> fmt, Args&&... args) {
         warn(std::format(fmt, std::forward<Args>(args)...));
      }

      /**
       * @brief Format and log an error message.
       *
       * @tparam Args Format argument types.
       *
       * @param fmt Format string.
       * @param args Format arguments.
       */
      template <class... Args>
      void error(std::format_string<Args...> fmt, Args&&... args) {
         error(std::format(fmt, std::forward<Args>(args)...));
      }

      /**
       * @brief Initialize logging. When logFile is empty, logs are written to stderr only.
       *
       * @param logFile Optional file path for persistent log output.
       */
      [[nodiscard]] ShutdownGuard init(std::string_view logFile = "");

      /**
       * @brief Flush and release logging resources.
       */
      void shutdown();
   } // namespace log

} // namespace c2server
