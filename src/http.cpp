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

} // namespace c2server
