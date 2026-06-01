export module c2server.middleware;

import c2server.http;
import std;

export namespace c2server::middleware {

   /** @brief Configuration for cross-origin resource sharing middleware. */
   struct CorsOptions {
      /** @brief Origins allowed to access the server. */
      std::vector<std::string> allowedOrigins{"*"};
      /** @brief HTTP methods accepted for cross-origin requests. */
      std::vector<std::string> allowedMethods{"GET", "POST", "PUT", "PATCH", "DELETE", "HEAD", "OPTIONS"};
      /** @brief Request headers accepted for cross-origin requests. */
      std::vector<std::string> allowedHeaders{"Content-Type", "Authorization", "X-Request-ID"};
      /** @brief Response headers exposed to cross-origin callers. */
      std::vector<std::string> exposedHeaders{"X-Request-ID"};
      /** @brief Allow browsers to send credentials. */
      bool allowCredentials = false;
      /** @brief Browser preflight cache lifetime. */
      std::chrono::seconds maxAge{600};
   };

   /** @brief Configuration for per-client request rate limiting. */
   struct RateLimitOptions {
      /** @brief Maximum accepted requests in one window. */
      std::size_t maxRequests = 100;
      /** @brief Rate-limit accounting window. */
      std::chrono::seconds window{60};
   };

   /**
    * @brief Create cross-origin resource sharing middleware.
    *
    * @param options CORS configuration.
    *
    * @return CORS middleware.
    */
   [[nodiscard]] Middleware cors(CorsOptions options = {});
   /**
    * @brief Create middleware that adds common security headers.
    *
    * @return Security header middleware.
    */
   [[nodiscard]] Middleware securityHeaders();
   /**
    * @brief Create middleware that provides a request ID.
    *
    * @return Request ID middleware.
    */
   [[nodiscard]] Middleware requestId();
   /**
    * @brief Create HTTP access logging middleware.
    *
    * @return Access logging middleware.
    */
   [[nodiscard]] Middleware accessLog();
   /**
    * @brief Create per-client request rate limiting middleware.
    *
    * @param options Rate-limit configuration.
    *
    * @return Rate limiting middleware.
    */
   [[nodiscard]] Middleware rateLimit(RateLimitOptions options = {});

} // namespace c2server::middleware
