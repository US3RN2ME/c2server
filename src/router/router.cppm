export module c2server.router;

import c2server.error;
import c2server.http;
import std;

export namespace c2server {

   struct EndpointBase {
      virtual ~EndpointBase() = default;
      virtual bool matches(const HttpRequest& req) const = 0;
      virtual HttpResponse handle(const HttpRequest& req) const = 0;
   };

   using EndpointPtr = std::shared_ptr<const EndpointBase>;

   struct RouteEndpoint final : EndpointBase {
      RouteEndpoint(HttpMethod method, std::string target, HttpHandler handler);

      bool matches(const HttpRequest& req) const override;
      HttpResponse handle(const HttpRequest& req) const override;

   private:
      HttpMethod method_;
      std::string target_;
      HttpHandler handler_;
   };

   struct Router {
      Router();
      explicit Router(std::vector<EndpointPtr> endpoints);

      Router& addEndpoint(EndpointPtr endpoint);
      Router& addRoute(HttpMethod method, std::string target, HttpHandler handler);
      Router& use(Middleware middleware);
      Router& get(std::string target, HttpHandler handler);
      Router& post(std::string target, HttpHandler handler);
      Router& put(std::string target, HttpHandler handler);
      Router& delete_(std::string target, HttpHandler handler);
      Router& patch(std::string target, HttpHandler handler);
      Router& head(std::string target, HttpHandler handler);
      Router& options(std::string target, HttpHandler handler);
      Router& freeze();
      [[nodiscard]] bool frozen() const;
      HttpResponse handle(const HttpRequest& req) const;

   private:
      void ensureMutable() const;
      HttpResponse dispatch(const HttpRequest& req) const;
      HttpResponse dispatchMiddleware(std::size_t index, const HttpRequest& req) const;

      std::vector<EndpointPtr> endpoints_;
      std::vector<Middleware> middleware_;
      bool frozen_ = false;
   };

} // namespace c2server
