export module c2server.middleware;

import c2server.http;
import std;

export namespace c2server::middleware {

   struct CorsOptions {
      std::vector<std::string> allowedOrigins{"*"};
      std::vector<std::string> allowedMethods{"GET", "POST", "PUT", "PATCH", "DELETE", "HEAD", "OPTIONS"};
      std::vector<std::string> allowedHeaders{"Content-Type", "Authorization", "X-Request-ID"};
      std::vector<std::string> exposedHeaders{"X-Request-ID"};
      bool allowCredentials = false;
      std::chrono::seconds maxAge{600};
   };

   struct RateLimitOptions {
      std::size_t maxRequests = 100;
      std::chrono::seconds window{60};
   };

   [[nodiscard]] Middleware cors(CorsOptions options = {});
   [[nodiscard]] Middleware securityHeaders();
   [[nodiscard]] Middleware requestId();
   [[nodiscard]] Middleware accessLog();
   [[nodiscard]] Middleware rateLimit(RateLimitOptions options = {});

} // namespace c2server::middleware
