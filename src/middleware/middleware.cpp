module c2server.middleware;

import c2server.error;
import c2server.logger;
import std;

namespace c2server::middleware {

   namespace {

      std::string join(const std::vector<std::string>& values) {
         std::string result;
         for (const auto& value : values) {
            if (!result.empty()) {
               result += ", ";
            }
            result += value;
         }
         return result;
      }

      bool contains(const std::vector<std::string>& values, std::string_view candidate) {
         return std::ranges::any_of(values, [candidate](const auto& value) {
            return value == candidate;
         });
      }

      std::string allowedOrigin(const CorsOptions& options, std::string_view origin) {
         if (origin.empty()) {
            return {};
         }
         if (contains(options.allowedOrigins, "*")) {
            return options.allowCredentials ? std::string{origin} : "*";
         }
         return contains(options.allowedOrigins, origin) ? std::string{origin} : std::string{};
      }

      void applyCorsHeaders(HttpResponse& response, const CorsOptions& options, std::string_view origin) {
         const auto allowed = allowedOrigin(options, origin);
         if (allowed.empty()) {
            return;
         }
         setHeader(response, "Access-Control-Allow-Origin", allowed);
         setHeader(response, "Vary", "Origin");
         if (options.allowCredentials) {
            setHeader(response, "Access-Control-Allow-Credentials", "true");
         }
         if (!options.exposedHeaders.empty()) {
            setHeader(response, "Access-Control-Expose-Headers", join(options.exposedHeaders));
         }
      }

      std::string makeRequestId() {
         static std::atomic_uint64_t sequence{0};
         const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
         return std::format("{:x}-{:x}", now, sequence.fetch_add(1, std::memory_order_relaxed));
      }

      void setHeaderIfAbsent(HttpResponse& response, std::string name, std::string value) {
         if (header(response.headers, name).empty()) {
            setHeader(response, std::move(name), std::move(value));
         }
      }

      struct RateLimitEntry {
         std::size_t requests = 0;
         std::chrono::steady_clock::time_point resetAt;
      };

      struct RateLimitState {
         std::mutex mutex;
         std::unordered_map<std::string, RateLimitEntry> clients;
         std::size_t requestsUntilCleanup = 1024;
      };

   } // namespace

   Middleware cors(CorsOptions options) {
      return [options = std::move(options)](const HttpRequest& req, const Next& next) {
         const auto origin = header(req.headers, "Origin");
         const auto preflightMethod = header(req.headers, "Access-Control-Request-Method");
         if (req.method == HttpMethod::Options && !origin.empty() && !preflightMethod.empty()) {
            HttpResponse response{.status = 204};
            applyCorsHeaders(response, options, origin);
            setHeader(response, "Access-Control-Allow-Methods", join(options.allowedMethods));
            setHeader(response, "Access-Control-Allow-Headers", join(options.allowedHeaders));
            setHeader(response, "Access-Control-Max-Age", std::to_string(options.maxAge.count()));
            return response;
         }

         auto response = next(req);
         applyCorsHeaders(response, options, origin);
         return response;
      };
   }

   Middleware securityHeaders() {
      return [](const HttpRequest& req, const Next& next) {
         auto response = next(req);
         setHeaderIfAbsent(response, "X-Content-Type-Options", "nosniff");
         setHeaderIfAbsent(response, "X-Frame-Options", "DENY");
         setHeaderIfAbsent(response, "Referrer-Policy", "no-referrer");
         setHeaderIfAbsent(response, "Cross-Origin-Resource-Policy", "same-origin");
         setHeaderIfAbsent(response, "Content-Security-Policy", "default-src 'none'; frame-ancestors 'none'");
         return response;
      };
   }

   Middleware requestId() {
      return [](const HttpRequest& req, const Next& next) {
         auto nextReq = req;
         auto id = std::string{header(req.headers, "X-Request-ID")};
         if (id.empty() || id.size() > 128) {
            id = makeRequestId();
         }
         setHeader(nextReq.headers, "X-Request-ID", id);
         auto response = next(nextReq);
         setHeader(response, "X-Request-ID", std::move(id));
         return response;
      };
   }

   Middleware accessLog() {
      return [](const HttpRequest& req, const Next& next) {
         const auto started = std::chrono::steady_clock::now();
         auto response = next(req);
         const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started);
         log::info("{} {} -> {} in {} ms from {} request_id={}", req.methodText, req.target, response.status, elapsed.count(),
                   req.remoteIp, header(req.headers, "X-Request-ID"));
         return response;
      };
   }

   Middleware rateLimit(RateLimitOptions options) {
      if (options.maxRequests == 0 || options.window <= std::chrono::seconds::zero()) {
         throw MiddlewareError{"rate limit maxRequests and window must be greater than zero"};
      }

      return [options, state = std::make_shared<RateLimitState>()](const HttpRequest& req, const Next& next) {
         const auto now = std::chrono::steady_clock::now();
         std::chrono::seconds retryAfter{};
         {
            std::lock_guard lock{state->mutex};
            if (--state->requestsUntilCleanup == 0) {
               std::erase_if(state->clients, [now](const auto& item) {
                  return item.second.resetAt <= now;
               });
               state->requestsUntilCleanup = 1024;
            }
            auto& entry = state->clients[req.remoteIp];
            if (entry.resetAt <= now) {
               entry = {.requests = 0, .resetAt = now + options.window};
            }
            if (entry.requests >= options.maxRequests) {
               retryAfter =
                   std::max(std::chrono::seconds{1}, std::chrono::duration_cast<std::chrono::seconds>(entry.resetAt - now));
            } else {
               ++entry.requests;
            }
         }

         if (retryAfter != std::chrono::seconds::zero()) {
            HttpResponse response{.status = 429, .body = "too many requests"};
            setHeader(response, "Retry-After", std::to_string(retryAfter.count()));
            return response;
         }
         return next(req);
      };
   }

} // namespace c2server::middleware
