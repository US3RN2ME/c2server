export module c2server.http;

import std;

export namespace c2server {

   /** @brief Collection of HTTP header names and values. */
   using HttpHeaders = std::unordered_map<std::string, std::string>;

   /** @brief Supported HTTP request methods. */
   enum class HttpMethod {
      Get,
      Post,
      Put,
      Delete,
      Patch,
      Head,
      Options,
      Unknown,
   };

   /** @brief Parsed HTTP request passed to handlers. */
   struct HttpRequest {
      /** @brief Parsed request method. */
      HttpMethod method = HttpMethod::Unknown;
      /** @brief Original request method text. */
      std::string methodText;
      /** @brief Original request target including any query string. */
      std::string target;
      /** @brief Request path without a query string. */
      std::string path;
      /** @brief Request query string. */
      std::string query;
      /** @brief Request body. */
      std::string body;
      /** @brief Remote client IP address. */
      std::string remoteIp;
      /** @brief Request headers. */
      HttpHeaders headers;
   };

   /** @brief HTTP response returned by handlers. */
   struct HttpResponse {
      /** @brief HTTP status code. */
      unsigned status = 200;
      /** @brief Response body. */
      std::string body;
      /** @brief Response content type. */
      std::string contentType = "text/plain";
      /** @brief Response headers. */
      HttpHeaders headers;
   };

   /** @brief Flat string key-value object used for JSON responses. */
   using JsonObject = std::unordered_map<std::string, std::string>;

   /**
    * @brief Create a successful HTTP response.
    *
    * @param body Response body.
    * @param contentType Response content type.
    *
    * @return HTTP response with status code 200.
    */
   [[nodiscard]] HttpResponse ok(std::string body, std::string contentType = "text/plain");
   /**
    * @brief Create a JSON HTTP response.
    *
    * @param body JSON object values.
    * @param status HTTP status code.
    *
    * @return JSON HTTP response.
    */
   [[nodiscard]] HttpResponse json(JsonObject body, unsigned status = 200);
   /**
    * @brief Create a successful JSON HTTP response.
    *
    * @param body JSON object values.
    *
    * @return JSON HTTP response with status code 200.
    */
   [[nodiscard]] HttpResponse jsonOk(JsonObject body);
   /**
    * @brief Normalize an HTTP header name for lookup.
    *
    * @param name Header name.
    *
    * @return Normalized header name.
    */
   [[nodiscard]] std::string normalizeHeaderName(std::string_view name);
   /**
    * @brief Find an HTTP header value.
    *
    * @param headers Header collection.
    * @param name Header name.
    *
    * @return Header value, or an empty view when absent.
    */
   [[nodiscard]] std::string_view header(const HttpHeaders& headers, std::string_view name);
   /**
    * @brief Set an HTTP header value.
    *
    * @param headers Header collection.
    * @param name Header name.
    * @param value Header value.
    */
   void setHeader(HttpHeaders& headers, std::string name, std::string value);
   /**
    * @brief Set an HTTP response header value.
    *
    * @param response HTTP response.
    * @param name Header name.
    * @param value Header value.
    */
   void setHeader(HttpResponse& response, std::string name, std::string value);

   /** @brief Function that handles an HTTP request. */
   using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;
   /** @brief Next handler in a middleware chain. */
   using Next = HttpHandler;
   /** @brief Function that wraps an HTTP handler. */
   using Middleware = std::function<HttpResponse(const HttpRequest&, const Next&)>;

} // namespace c2server
