module;

module c2server.router;

namespace c2server {

   namespace {

      HttpResponse notFound() {
         return {404, "not found"};
      }

   } // namespace

   RouteEndpoint::RouteEndpoint(HttpMethod method, std::string target, HttpHandler handler)
       : method_{std::move(method)}
       , target_{std::move(target)}
       , handler_{std::move(handler)} {
      if (!handler_) {
         throw std::invalid_argument{"route handler must not be empty"};
      }
   }

   bool RouteEndpoint::matches(const HttpRequest& req) const {
      return req.method == method_ && req.target == target_;
   }

   HttpResponse RouteEndpoint::handle(const HttpRequest& req) const {
      return handler_(req);
   }

   Router::Router() = default;

   Router::Router(std::vector<EndpointPtr> endpoints)
       : Router() {
      for (auto& endpoint : endpoints) {
         addEndpoint(std::move(endpoint));
      }
   }

   Router& Router::addEndpoint(EndpointPtr endpoint) {
      if (!endpoint) {
         throw std::invalid_argument{"endpoint must not be null"};
      }
      endpoints_.push_back(std::move(endpoint));
      return *this;
   }

   Router& Router::addRoute(HttpMethod method, std::string target, HttpHandler handler) {
      return addEndpoint(std::make_shared<RouteEndpoint>(method, std::move(target), std::move(handler)));
   }

   Router& Router::get(std::string target, HttpHandler handler) {
      return addRoute(HttpMethod::Get, std::move(target), std::move(handler));
   }

   Router& Router::post(std::string target, HttpHandler handler) {
      return addRoute(HttpMethod::Post, std::move(target), std::move(handler));
   }

   Router& Router::put(std::string target, HttpHandler handler) {
      return addRoute(HttpMethod::Put, std::move(target), std::move(handler));
   }

   Router& Router::delete_(std::string target, HttpHandler handler) {
      return addRoute(HttpMethod::Delete, std::move(target), std::move(handler));
   }

   Router& Router::patch(std::string target, HttpHandler handler) {
      return addRoute(HttpMethod::Patch, std::move(target), std::move(handler));
   }

   Router& Router::head(std::string target, HttpHandler handler) {
      return addRoute(HttpMethod::Head, std::move(target), std::move(handler));
   }

   Router& Router::options(std::string target, HttpHandler handler) {
      return addRoute(HttpMethod::Options, std::move(target), std::move(handler));
   }

   HttpResponse Router::handle(const HttpRequest& req) const {
      for (const auto& endpoint : endpoints_) {
         if (endpoint->matches(req)) {
            return endpoint->handle(req);
         }
      }

      return notFound();
   }

} // namespace c2server
