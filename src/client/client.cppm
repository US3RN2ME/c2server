export module c2server.client;

import c2server.http;
import std;

export namespace c2server {

   struct ClientSslSettings {
      bool enabled = false;
      bool verifyPeer = true;
      std::string caFile;
   };

   struct ClientSettings {
      std::string host = "127.0.0.1";
      std::uint16_t port = 80;
      std::chrono::seconds timeout{30};
      ClientSslSettings ssl;
   };

   struct Client {
      explicit Client(ClientSettings settings);

      [[nodiscard]] HttpResponse request(HttpMethod method, std::string target, std::string body = {},
                                         HttpHeaders headers = {}) const;
      [[nodiscard]] HttpResponse get(std::string target, HttpHeaders headers = {}) const;
      [[nodiscard]] HttpResponse post(std::string target, std::string body, HttpHeaders headers = {}) const;
      [[nodiscard]] HttpResponse put(std::string target, std::string body, HttpHeaders headers = {}) const;
      [[nodiscard]] HttpResponse delete_(std::string target, HttpHeaders headers = {}) const;
      [[nodiscard]] HttpResponse patch(std::string target, std::string body, HttpHeaders headers = {}) const;

   private:
      ClientSettings settings_;
   };

} // namespace c2server
