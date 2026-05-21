export module c2server.logger;

import c2server.error;
import std;

export namespace c2server::log {

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

   void debug(std::string_view msg);
   void info(std::string_view msg);
   void warn(std::string_view msg);
   void error(std::string_view msg);

   template <class... Args>
   void debug(std::format_string<Args...> fmt, Args&&... args) {
      debug(std::format(fmt, std::forward<Args>(args)...));
   }

   template <class... Args>
   void info(std::format_string<Args...> fmt, Args&&... args) {
      info(std::format(fmt, std::forward<Args>(args)...));
   }

   template <class... Args>
   void warn(std::format_string<Args...> fmt, Args&&... args) {
      warn(std::format(fmt, std::forward<Args>(args)...));
   }

   template <class... Args>
   void error(std::format_string<Args...> fmt, Args&&... args) {
      error(std::format(fmt, std::forward<Args>(args)...));
   }

   [[nodiscard]] ShutdownGuard init(std::string_view logFile = "");
   void shutdown();

} // namespace c2server::log
