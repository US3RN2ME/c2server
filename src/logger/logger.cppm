export module c2server.logger;

import c2server.error;
import std;

export namespace c2server::log {

   /** @brief RAII guard that shuts logging down when released. */
   class ShutdownGuard {
   public:
      ShutdownGuard(const ShutdownGuard&) = delete;
      ShutdownGuard& operator=(const ShutdownGuard&) = delete;
      /**
       * @brief Move a logging shutdown guard.
       *
       * @param other Guard to move from.
       */
      ShutdownGuard(ShutdownGuard&& other) noexcept;
      /**
       * @brief Move-assign a logging shutdown guard.
       *
       * @param other Guard to move from.
       *
       * @return This guard.
       */
      ShutdownGuard& operator=(ShutdownGuard&& other) noexcept;
      /** @brief Shut logging down when this guard is active. */
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
    * @brief Initialize logging.
    *
    * @param logFile Optional path to a log file.
    *
    * @return Guard that shuts logging down on destruction.
    */
   [[nodiscard]] ShutdownGuard init(std::string_view logFile = "");
   /** @brief Flush log output and shut logging down. */
   void shutdown();

} // namespace c2server::log
