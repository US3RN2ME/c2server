export module c2server.http;

import std;

export namespace c2server {

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
      std::string body;
      std::string remoteIp;
   };

   struct HttpResponse {
      unsigned status = 200;
      std::string body;
      std::string contentType = "text/plain";
   };

   using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;

} // namespace c2server
