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

   /** @brief OpenAPI request body documentation for a route. */
   struct RequestBodyDoc {
      /** @brief Human-readable request body description. */
      std::string description;
      /** @brief Request body content type. */
      std::string contentType = "text/plain";
      /** @brief Whether the request body is required. */
      bool required = false;
   };

   /** @brief OpenAPI response documentation for a route. */
   struct ResponseDoc {
      /** @brief HTTP response status code. */
      unsigned status = 200;
      /** @brief Human-readable response description. */
      std::string description = "OK";
      /** @brief Response content type. */
      std::string contentType = "text/plain";
   };

   /** @brief OpenAPI documentation attached to a route. */
   struct RouteDoc {
      /** @brief Short operation summary. */
      std::string summary;
      /** @brief Longer operation description. */
      std::string description;
      /** @brief Operation tags. */
      std::vector<std::string> tags;
      /** @brief Optional request body documentation. */
      std::optional<RequestBodyDoc> requestBody;
      /** @brief Documented responses. */
      std::vector<ResponseDoc> responses;
   };

   /** @brief Registered route information used for OpenAPI generation. */
   struct RouteInfo {
      /** @brief Accepted HTTP method. */
      HttpMethod method = HttpMethod::Unknown;
      /** @brief Accepted request target. */
      std::string target;
      /** @brief Route documentation. */
      RouteDoc doc;
   };

   /** @brief Top-level OpenAPI document information. */
   struct OpenApiInfo {
      /** @brief API title. */
      std::string title = "c2server";
      /** @brief API version. */
      std::string version = "0.0.1";
      /** @brief Optional API description. */
      std::string description;
   };

   /** @brief Options for serving OpenAPI JSON and Swagger UI routes. */
   struct OpenApiOptions {
      /** @brief Top-level OpenAPI document information. */
      OpenApiInfo info;
      /** @brief Route target that serves the OpenAPI JSON document. */
      std::string specTarget = "/openapi.json";
      /** @brief Route target that serves the Swagger UI page. */
      std::string docsTarget = "/docs";
   };

   /** @brief Endpoint that dispatches one HTTP method and target. */
   struct RouteEndpoint final : EndpointBase {
      /**
       * @brief Construct a route endpoint.
       *
       * @param method Accepted HTTP method.
       * @param target Accepted request target.
       * @param handler Request handler.
       * @param doc Route documentation.
       */
      RouteEndpoint(HttpMethod method, std::string target, HttpHandler handler, RouteDoc doc = {});

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
      /**
       * @brief Return route metadata.
       *
       * @return Registered route information.
       */
      [[nodiscard]] RouteInfo routeInfo() const;

   private:
      HttpMethod method_;
      std::string target_;
      HttpHandler handler_;
      RouteDoc doc_;
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
       * @brief Add a documented route handler.
       *
       * @param method Accepted HTTP method.
       * @param target Accepted request target.
       * @param handler Request handler.
       * @param doc Route documentation.
       *
       * @return This router.
       */
      Router& addRoute(HttpMethod method, std::string target, HttpHandler handler, RouteDoc doc);
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
       * @brief Add a documented GET route.
       *
       * @param target Request target.
       * @param handler Request handler.
       * @param doc Route documentation.
       *
       * @return This router.
       */
      Router& get(std::string target, HttpHandler handler, RouteDoc doc);
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
       * @brief Add a documented POST route.
       *
       * @param target Request target.
       * @param handler Request handler.
       * @param doc Route documentation.
       *
       * @return This router.
       */
      Router& post(std::string target, HttpHandler handler, RouteDoc doc);
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
       * @brief Add a documented PUT route.
       *
       * @param target Request target.
       * @param handler Request handler.
       * @param doc Route documentation.
       *
       * @return This router.
       */
      Router& put(std::string target, HttpHandler handler, RouteDoc doc);
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
       * @brief Add a documented DELETE route.
       *
       * @param target Request target.
       * @param handler Request handler.
       * @param doc Route documentation.
       *
       * @return This router.
       */
      Router& delete_(std::string target, HttpHandler handler, RouteDoc doc);
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
       * @brief Add a documented PATCH route.
       *
       * @param target Request target.
       * @param handler Request handler.
       * @param doc Route documentation.
       *
       * @return This router.
       */
      Router& patch(std::string target, HttpHandler handler, RouteDoc doc);
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
       * @brief Add a documented HEAD route.
       *
       * @param target Request target.
       * @param handler Request handler.
       * @param doc Route documentation.
       *
       * @return This router.
       */
      Router& head(std::string target, HttpHandler handler, RouteDoc doc);
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
       * @brief Add a documented OPTIONS route.
       *
       * @param target Request target.
       * @param handler Request handler.
       * @param doc Route documentation.
       *
       * @return This router.
       */
      Router& options(std::string target, HttpHandler handler, RouteDoc doc);
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
       * @brief Return metadata for route endpoints registered with this router.
       *
       * @return Registered route information.
       */
      [[nodiscard]] std::vector<RouteInfo> routes() const;
      /**
       * @brief Generate an OpenAPI 3.0 JSON document for registered routes.
       *
       * @param info Top-level API document information.
       *
       * @return Serialized OpenAPI JSON.
       */
      [[nodiscard]] std::string openApiJson(OpenApiInfo info = {}) const;
      /**
       * @brief Register OpenAPI JSON and Swagger UI endpoints.
       *
       * @param options OpenAPI serving options.
       *
       * @return This router.
       */
      Router& serveOpenApi(OpenApiOptions options = {});
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
