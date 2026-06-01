export module c2server.router;

import c2server.error;
import c2server.http;
import std;

export namespace c2server {

   /** @brief Abstract interface for an HTTP endpoint. */
   struct EndpointBase {
      /** @brief Destroy an endpoint. */
      virtual ~EndpointBase() = default;
      /**
       * @brief Check whether this endpoint accepts a request.
       *
       * @param req HTTP request.
       *
       * @return True when the endpoint accepts the request.
       */
      virtual bool matches(const HttpRequest& req) const = 0;
      /**
       * @brief Handle an accepted HTTP request.
       *
       * @param req HTTP request.
       *
       * @return HTTP response.
       */
      virtual HttpResponse handle(const HttpRequest& req) const = 0;
   };

   /** @brief Shared immutable endpoint pointer. */
   using EndpointPtr = std::shared_ptr<const EndpointBase>;

   /** @brief Endpoint that dispatches one HTTP method and target. */
   struct RouteEndpoint final : EndpointBase {
      /**
       * @brief Construct a route endpoint.
       *
       * @param method Accepted HTTP method.
       * @param target Accepted request target.
       * @param handler Request handler.
       */
      RouteEndpoint(HttpMethod method, std::string target, HttpHandler handler);

      /**
       * @brief Check whether the method and target match.
       *
       * @param req HTTP request.
       *
       * @return True when the route accepts the request.
       */
      bool matches(const HttpRequest& req) const override;
      /**
       * @brief Invoke the route handler.
       *
       * @param req HTTP request.
       *
       * @return HTTP response.
       */
      HttpResponse handle(const HttpRequest& req) const override;

   private:
      HttpMethod method_;
      std::string target_;
      HttpHandler handler_;
   };

   /** @brief HTTP endpoint router with middleware support. */
   struct Router {
      /** @brief Construct an empty router. */
      Router();
      /**
       * @brief Construct a router with endpoints.
       *
       * @param endpoints Initial endpoints.
       */
      explicit Router(std::vector<EndpointPtr> endpoints);

      /**
       * @brief Add an endpoint.
       *
       * @param endpoint Endpoint to add.
       *
       * @return This router.
       */
      Router& addEndpoint(EndpointPtr endpoint);
      /**
       * @brief Add a route handler.
       *
       * @param method Accepted HTTP method.
       * @param target Accepted request target.
       * @param handler Request handler.
       *
       * @return This router.
       */
      Router& addRoute(HttpMethod method, std::string target, HttpHandler handler);
      /**
       * @brief Append middleware to the request pipeline.
       *
       * @param middleware Middleware to append.
       *
       * @return This router.
       */
      Router& use(Middleware middleware);
      /**
       * @brief Add a GET route.
       *
       * @param target Request target.
       * @param handler Request handler.
       *
       * @return This router.
       */
      Router& get(std::string target, HttpHandler handler);
      /**
       * @brief Add a POST route.
       *
       * @param target Request target.
       * @param handler Request handler.
       *
       * @return This router.
       */
      Router& post(std::string target, HttpHandler handler);
      /**
       * @brief Add a PUT route.
       *
       * @param target Request target.
       * @param handler Request handler.
       *
       * @return This router.
       */
      Router& put(std::string target, HttpHandler handler);
      /**
       * @brief Add a DELETE route.
       *
       * @param target Request target.
       * @param handler Request handler.
       *
       * @return This router.
       */
      Router& delete_(std::string target, HttpHandler handler);
      /**
       * @brief Add a PATCH route.
       *
       * @param target Request target.
       * @param handler Request handler.
       *
       * @return This router.
       */
      Router& patch(std::string target, HttpHandler handler);
      /**
       * @brief Add a HEAD route.
       *
       * @param target Request target.
       * @param handler Request handler.
       *
       * @return This router.
       */
      Router& head(std::string target, HttpHandler handler);
      /**
       * @brief Add an OPTIONS route.
       *
       * @param target Request target.
       * @param handler Request handler.
       *
       * @return This router.
       */
      Router& options(std::string target, HttpHandler handler);
      /**
       * @brief Prevent further router configuration changes.
       *
       * @return This router.
       */
      Router& freeze();
      /**
       * @brief Check whether the router is frozen.
       *
       * @return True when configuration changes are disabled.
       */
      [[nodiscard]] bool frozen() const;
      /**
       * @brief Dispatch an HTTP request through middleware and endpoints.
       *
       * @param req HTTP request.
       *
       * @return HTTP response.
       */
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
