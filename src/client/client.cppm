export module c2server.client;

import c2server.http;
import std;

export namespace c2server {

   /**
    * @brief TLS configuration for an HTTP client.
    */
   struct ClientSslSettings {
      /** @brief Enable TLS for requests. */
      bool enabled = false;
      /** @brief Verify the server certificate and host name. */
      bool verifyPeer = true;
      /** @brief Optional CA certificate file used for peer verification. */
      std::string caFile;
   };

   /**
    * @brief Connection settings for an HTTP client.
    */
   struct ClientSettings {
      /** @brief Remote host name or IP address. */
      std::string host = "127.0.0.1";
      /** @brief Remote TCP port. */
      std::uint16_t port = 80;
      /** @brief Connection and I/O timeout. */
      std::chrono::seconds timeout{30};
      /** @brief Optional TLS configuration. */
      ClientSslSettings ssl;
   };

   /**
    * @brief Synchronous HTTP and HTTPS client.
    */
   struct Client {
      /**
       * @brief Construct a client with connection settings.
       *
       * @param settings Connection settings.
       */
      explicit Client(ClientSettings settings);

      /**
       * @brief Send an HTTP request.
       *
       * @param method HTTP method.
       * @param target Request target beginning with '/'.
       * @param body Request body.
       * @param headers Request headers.
       *
       * @return HTTP response.
       */
      [[nodiscard]] HttpResponse request(HttpMethod method, std::string target, std::string body = {},
                                         HttpHeaders headers = {}) const;
      /**
       * @brief Send a GET request.
       *
       * @param target Request target.
       * @param headers Request headers.
       *
       * @return HTTP response.
       */
      [[nodiscard]] HttpResponse get(std::string target, HttpHeaders headers = {}) const;
      /**
       * @brief Send a POST request.
       *
       * @param target Request target.
       * @param body Request body.
       * @param headers Request headers.
       *
       * @return HTTP response.
       */
      [[nodiscard]] HttpResponse post(std::string target, std::string body, HttpHeaders headers = {}) const;
      /**
       * @brief Send a PUT request.
       *
       * @param target Request target.
       * @param body Request body.
       * @param headers Request headers.
       *
       * @return HTTP response.
       */
      [[nodiscard]] HttpResponse put(std::string target, std::string body, HttpHeaders headers = {}) const;
      /**
       * @brief Send a DELETE request.
       *
       * @param target Request target.
       * @param headers Request headers.
       *
       * @return HTTP response.
       */
      [[nodiscard]] HttpResponse delete_(std::string target, HttpHeaders headers = {}) const;
      /**
       * @brief Send a PATCH request.
       *
       * @param target Request target.
       * @param body Request body.
       * @param headers Request headers.
       *
       * @return HTTP response.
       */
      [[nodiscard]] HttpResponse patch(std::string target, std::string body, HttpHeaders headers = {}) const;

   private:
      ClientSettings settings_;
   };

} // namespace c2server
