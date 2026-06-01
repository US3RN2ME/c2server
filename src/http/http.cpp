module;
#include <nlohmann/json.hpp>

module c2server.http;

import std;

namespace c2server {

   HttpResponse ok(std::string body, std::string contentType) {
      return {200, std::move(body), std::move(contentType)};
   }

   HttpResponse json(JsonObject body, unsigned status) {
      const auto payload = nlohmann::json{std::move(body)}.dump();
      return {status, payload, "application/json"};
   }

   HttpResponse jsonOk(JsonObject body) {
      return json(std::move(body), 200);
   }

   std::string normalizeHeaderName(std::string_view name) {
      auto normalized = std::string{name};
      std::ranges::transform(normalized, normalized.begin(), [](unsigned char ch) {
         return static_cast<char>(std::tolower(ch));
      });
      return normalized;
   }

   std::string_view header(const HttpHeaders& headers, std::string_view name) {
      if (const auto it = headers.find(normalizeHeaderName(name)); it != headers.end()) {
         return it->second;
      }
      return {};
   }

   void setHeader(HttpHeaders& headers, std::string name, std::string value) {
      headers.insert_or_assign(normalizeHeaderName(name), std::move(value));
   }

   void setHeader(HttpResponse& response, std::string name, std::string value) {
      setHeader(response.headers, std::move(name), std::move(value));
   }

} // namespace c2server
