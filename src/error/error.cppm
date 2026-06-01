export module c2server.error;

import std;

export namespace c2server {

   /** @brief Base exception for c2server failures. */
   struct Error : std::runtime_error {
      using std::runtime_error::runtime_error;

      /**
       * @brief Construct an exception from a formatted message.
       *
       * @tparam Args Format argument types.
       *
       * @param fmt Format string.
       * @param args Format arguments.
       */
      template <class... Args>
      explicit Error(std::format_string<Args...> fmt, Args&&... args)
          : std::runtime_error{std::format(fmt, std::forward<Args>(args)...)} {}
   };

   /** @brief Exception raised for invalid configuration. */
   struct ConfigError : Error {
      using Error::Error;
   };
   /** @brief Exception raised while processing HTTP data. */
   struct HttpError : Error {
      using Error::Error;
   };
   /** @brief Exception raised by the HTTP client. */
   struct ClientError : Error {
      using Error::Error;
   };
   /** @brief Exception raised while invoking an endpoint. */
   struct EndpointError : Error {
      using Error::Error;
   };
   /** @brief Exception raised by the router. */
   struct RouterError : Error {
      using Error::Error;
   };
   /** @brief Exception raised by the server runtime. */
   struct ServerError : Error {
      using Error::Error;
   };
   /** @brief Exception raised while configuring signal handling. */
   struct SignalHandlerError : Error {
      using Error::Error;
   };
   /** @brief Exception raised while accepting network connections. */
   struct AcceptorError : Error {
      using Error::Error;
   };
   /** @brief Exception raised by a client session. */
   struct SessionError : Error {
      using Error::Error;
   };
   /** @brief Exception raised by logging facilities. */
   struct LoggerError : Error {
      using Error::Error;
   };
   /** @brief Exception raised by middleware. */
   struct MiddlewareError : Error {
      using Error::Error;
   };
   /** @brief Exception raised while accessing payload data. */
   struct PayloadError : Error {
      using Error::Error;
   };
   /** @brief Exception raised while submitting asynchronous work. */
   struct SubmissionError : Error {
      using Error::Error;
   };

} // namespace c2server
