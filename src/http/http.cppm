export module c2server.http;

import std;

export namespace c2server {

   using HttpHeaders = std::unordered_map<std::string, std::string>;

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

   struct HttpRequest {
      HttpMethod method = HttpMethod::Unknown;
      std::string methodText;
      std::string target;
      std::string path;
      std::string query;
      std::string body;
      std::string remoteIp;
      HttpHeaders headers;
   };

   struct HttpResponse {
      unsigned status = 200;
      std::string body;
      std::string contentType = "text/plain";
      HttpHeaders headers;
   };

   using JsonObject = std::unordered_map<std::string, std::string>;

   [[nodiscard]] HttpResponse ok(std::string body, std::string contentType = "text/plain");
   [[nodiscard]] HttpResponse json(JsonObject body, unsigned status = 200);
   [[nodiscard]] HttpResponse jsonOk(JsonObject body);
   [[nodiscard]] std::string normalizeHeaderName(std::string_view name);
   [[nodiscard]] std::string_view header(const HttpHeaders& headers, std::string_view name);
   void setHeader(HttpHeaders& headers, std::string name, std::string value);
   void setHeader(HttpResponse& response, std::string name, std::string value);

   using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;
   using Next = HttpHandler;
   using Middleware = std::function<HttpResponse(const HttpRequest&, const Next&)>;

} // namespace c2server
