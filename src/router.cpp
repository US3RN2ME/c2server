module;

module c2server.router;

import c2server.error;
import c2server.logger;

namespace c2server {

   namespace {

      HttpResponse notFound() {
         return {404, "not found"};
      }

      HttpResponse internalServerError(std::string_view message) {
         return {500, std::string{message}, "text/plain"};
      }

   } // namespace

   RouteEndpoint::RouteEndpoint(HttpMethod method, std::string target, HttpHandler handler)
       : method_{std::move(method)}
      , target_{std::move(target)}
      , handler_{std::move(handler)} {
      if (!handler_) {
         throw EndpointError{"route handler must not be empty"};
      }
   }

   bool RouteEndpoint::matches(const HttpRequest& req) const {
      return req.method == method_ && req.target == target_;
   }

   HttpResponse RouteEndpoint::handle(const HttpRequest& req) const {
      try {
         return handler_(req);
      } catch (const Error& e) {
         throw EndpointError{"endpoint '{}' failed: {}", target_, e.what()};
      } catch (const std::exception& e) {
         throw EndpointError{"endpoint '{}' failed: {}", target_, e.what()};
      } catch (...) {
         throw EndpointError{"endpoint '{}' failed with an unknown exception", target_};
      }
   }

   Router::Router() = default;

   Router::Router(std::vector<EndpointPtr> endpoints)
       : Router() {
      for (auto& endpoint : endpoints) {
         addEndpoint(std::move(endpoint));
      }
   }

   Router& Router::addEndpoint(EndpointPtr endpoint) {
      ensureMutable();
      if (!endpoint) {
         throw RouterError{"endpoint must not be null"};
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

   Router& Router::freeze() {
      frozen_ = true;
      return *this;
   }

   bool Router::frozen() const {
      return frozen_;
   }

   HttpResponse Router::handle(const HttpRequest& req) const {
      for (const auto& endpoint : endpoints_) {
         if (endpoint->matches(req)) {
            try {
               return endpoint->handle(req);
            } catch (const EndpointError& e) {
               log::error("Endpoint handler error for {} {}: {}", req.methodText, req.target, e.what());
               return internalServerError(e.what());
            } catch (const Error& e) {
               log::error("Router error for {} {}: {}", req.methodText, req.target, e.what());
               return internalServerError(e.what());
            } catch (const std::exception& e) {
               log::error("Unhandled route error for {} {}: {}", req.methodText, req.target, e.what());
               return internalServerError(e.what());
            } catch (...) {
               log::error("Unhandled route error for {} {}", req.methodText, req.target);
               return internalServerError("internal server error");
            }
         }
      }

      return notFound();
   }

   void Router::ensureMutable() const {
      if (frozen_) {
         throw RouterError{"router is frozen and cannot be modified"};
      }
   }

} // namespace c2server
