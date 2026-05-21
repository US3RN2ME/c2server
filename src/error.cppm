export module c2server.error;

import std;

export namespace c2server {

   struct Error : std::runtime_error {
      using std::runtime_error::runtime_error;

      template <class... Args>
      explicit Error(std::format_string<Args...> fmt, Args&&... args)
          : std::runtime_error{std::format(fmt, std::forward<Args>(args)...)} {}
   };

   struct ConfigError : Error { using Error::Error; };
   struct HttpError : Error { using Error::Error; };
   struct EndpointError : Error { using Error::Error; };
   struct RouterError : Error { using Error::Error; };
   struct ServerError : Error { using Error::Error; };
   struct SignalHandlerError : Error { using Error::Error; };
   struct AcceptorError : Error { using Error::Error; };
   struct SessionError : Error { using Error::Error; };
   struct LoggerError : Error { using Error::Error; };
   struct PayloadError : Error { using Error::Error; };

} // namespace c2server
