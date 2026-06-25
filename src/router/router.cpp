module;
#include <nlohmann/json.hpp>

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

      std::string methodName(HttpMethod method) {
         switch (method) {
            case HttpMethod::Get:
               return "get";
            case HttpMethod::Post:
               return "post";
            case HttpMethod::Put:
               return "put";
            case HttpMethod::Delete:
               return "delete";
            case HttpMethod::Patch:
               return "patch";
            case HttpMethod::Head:
               return "head";
            case HttpMethod::Options:
               return "options";
            case HttpMethod::Unknown:
               return "unknown";
         }
         return "unknown";
      }

      nlohmann::json stringSchema() {
         return {
             {"type", "string"},
         };
      }

      nlohmann::json contentDoc(std::string_view contentType) {
         return {
             {std::string{contentType}, {{"schema", stringSchema()}}},
         };
      }

      std::string swaggerUiHtml(std::string_view specTarget) {
         return std::format(
             R"(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>API Docs</title>
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/swagger-ui-dist@5/swagger-ui.css">
  <style>
    body {{ margin: 0; background: #f6f8fb; }}
  </style>
</head>
<body>
  <div id="swagger-ui"></div>
  <script src="https://cdn.jsdelivr.net/npm/swagger-ui-dist@5/swagger-ui-bundle.js"></script>
  <script>
    window.addEventListener("load", () => {{
      SwaggerUIBundle({{
        url: {},
        dom_id: "#swagger-ui",
        deepLinking: true,
        presets: [SwaggerUIBundle.presets.apis],
        layout: "BaseLayout"
      }});
    }});
  </script>
</body>
</html>)",
             nlohmann::json{std::string{specTarget}}.dump());
      }

      void applySwaggerUiCsp(HttpResponse& response) {
         setHeader(response, "Content-Security-Policy",
                   "default-src 'none'; "
                   "connect-src 'self'; "
                   "img-src 'self' data: https://validator.swagger.io; "
                   "script-src 'self' 'unsafe-inline' https://cdn.jsdelivr.net; "
                   "style-src 'self' 'unsafe-inline' https://cdn.jsdelivr.net; "
                   "font-src https://cdn.jsdelivr.net; "
                   "frame-ancestors 'none'");
      }

   } // namespace

   RouteEndpoint::RouteEndpoint(HttpMethod method, std::string target, HttpHandler handler, RouteDoc doc)
       : method_{std::move(method)}
       , target_{std::move(target)}
       , handler_{std::move(handler)}
       , doc_{std::move(doc)} {
      if (!handler_) {
         throw EndpointError{"route handler must not be empty"};
      }
   }

   bool RouteEndpoint::matches(const HttpRequest& req) const {
      return req.method == method_ && req.path == target_;
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

   RouteInfo RouteEndpoint::routeInfo() const {
      return {.method = method_, .target = target_, .doc = doc_};
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
      return addRoute(method, std::move(target), std::move(handler), {});
   }

   Router& Router::addRoute(HttpMethod method, std::string target, HttpHandler handler, RouteDoc doc) {
      return addEndpoint(std::make_shared<RouteEndpoint>(method, std::move(target), std::move(handler), std::move(doc)));
   }

   Router& Router::use(Middleware middleware) {
      ensureMutable();
      if (!middleware) {
         throw RouterError{"middleware must not be empty"};
      }
      middleware_.push_back(std::move(middleware));
      return *this;
   }

   Router& Router::get(std::string target, HttpHandler handler) {
      return get(std::move(target), std::move(handler), {});
   }

   Router& Router::get(std::string target, HttpHandler handler, RouteDoc doc) {
      return addRoute(HttpMethod::Get, std::move(target), std::move(handler), std::move(doc));
   }

   Router& Router::post(std::string target, HttpHandler handler) {
      return post(std::move(target), std::move(handler), {});
   }

   Router& Router::post(std::string target, HttpHandler handler, RouteDoc doc) {
      return addRoute(HttpMethod::Post, std::move(target), std::move(handler), std::move(doc));
   }

   Router& Router::put(std::string target, HttpHandler handler) {
      return put(std::move(target), std::move(handler), {});
   }

   Router& Router::put(std::string target, HttpHandler handler, RouteDoc doc) {
      return addRoute(HttpMethod::Put, std::move(target), std::move(handler), std::move(doc));
   }

   Router& Router::delete_(std::string target, HttpHandler handler) {
      return delete_(std::move(target), std::move(handler), {});
   }

   Router& Router::delete_(std::string target, HttpHandler handler, RouteDoc doc) {
      return addRoute(HttpMethod::Delete, std::move(target), std::move(handler), std::move(doc));
   }

   Router& Router::patch(std::string target, HttpHandler handler) {
      return patch(std::move(target), std::move(handler), {});
   }

   Router& Router::patch(std::string target, HttpHandler handler, RouteDoc doc) {
      return addRoute(HttpMethod::Patch, std::move(target), std::move(handler), std::move(doc));
   }

   Router& Router::head(std::string target, HttpHandler handler) {
      return head(std::move(target), std::move(handler), {});
   }

   Router& Router::head(std::string target, HttpHandler handler, RouteDoc doc) {
      return addRoute(HttpMethod::Head, std::move(target), std::move(handler), std::move(doc));
   }

   Router& Router::options(std::string target, HttpHandler handler) {
      return options(std::move(target), std::move(handler), {});
   }

   Router& Router::options(std::string target, HttpHandler handler, RouteDoc doc) {
      return addRoute(HttpMethod::Options, std::move(target), std::move(handler), std::move(doc));
   }

   Router& Router::freeze() {
      frozen_ = true;
      return *this;
   }

   bool Router::frozen() const {
      return frozen_;
   }

   std::vector<RouteInfo> Router::routes() const {
      std::vector<RouteInfo> result;
      result.reserve(endpoints_.size());
      for (const auto& endpoint : endpoints_) {
         if (const auto* route = dynamic_cast<const RouteEndpoint*>(endpoint.get())) {
            result.push_back(route->routeInfo());
         }
      }
      return result;
   }

   std::string Router::openApiJson(OpenApiInfo info) const {
      nlohmann::json document = {
          {"openapi", "3.0.3"},
          {"info", {{"title", std::move(info.title)}, {"version", std::move(info.version)}}},
          {"paths", nlohmann::json::object()},
      };

      if (!info.description.empty()) {
         document["info"]["description"] = std::move(info.description);
      }

      for (const auto& route : routes()) {
         if (route.method == HttpMethod::Unknown) {
            continue;
         }

         const auto method = methodName(route.method);
         nlohmann::json operation = nlohmann::json::object();
         if (!route.doc.summary.empty()) {
            operation["summary"] = route.doc.summary;
         }
         if (!route.doc.description.empty()) {
            operation["description"] = route.doc.description;
         }
         if (!route.doc.tags.empty()) {
            operation["tags"] = route.doc.tags;
         }
         if (route.doc.requestBody) {
            operation["requestBody"] = {
                {"description", route.doc.requestBody->description},
                {"required", route.doc.requestBody->required},
                {"content", contentDoc(route.doc.requestBody->contentType)},
            };
         }

         auto responses = nlohmann::json::object();
         const auto routeResponses =
             route.doc.responses.empty() ? std::vector<ResponseDoc>{ResponseDoc{}} : route.doc.responses;
         for (const auto& response : routeResponses) {
            auto responseDoc = nlohmann::json{{"description", response.description}};
            if (!response.contentType.empty()) {
               responseDoc["content"] = contentDoc(response.contentType);
            }
            responses[std::to_string(response.status)] = std::move(responseDoc);
         }
         operation["responses"] = std::move(responses);
         document["paths"][route.target][method] = std::move(operation);
      }

      return document.dump(2);
   }

   Router& Router::serveOpenApi(OpenApiOptions options) {
      auto specTarget = options.specTarget;
      auto docsTarget = options.docsTarget;
      auto info = std::move(options.info);

      get(specTarget,
          [this, info = std::move(info)](const HttpRequest&) {
             return HttpResponse{
                 .status = 200,
                 .body = openApiJson(info),
                 .contentType = "application/json",
             };
          },
          {
              .summary = "OpenAPI specification",
              .description = "Returns the generated OpenAPI JSON document for this server.",
              .tags = {"docs"},
              .responses = {{.status = 200, .description = "OpenAPI JSON", .contentType = "application/json"}},
          });

      get(docsTarget,
          [specTarget = std::move(specTarget)](const HttpRequest&) {
             auto response = HttpResponse{
                 .status = 200,
                 .body = swaggerUiHtml(specTarget),
                 .contentType = "text/html",
             };
             applySwaggerUiCsp(response);
             return response;
          },
          {
              .summary = "Swagger UI",
              .description = "Interactive API documentation powered by Swagger UI.",
              .tags = {"docs"},
              .responses = {{.status = 200, .description = "Swagger UI HTML", .contentType = "text/html"}},
          });

      return *this;
   }

   HttpResponse Router::handle(const HttpRequest& req) const {
      try {
         return dispatchMiddleware(0, req);
      } catch (const Error& e) {
         log::error("Request pipeline error for {} {}: {}", req.methodText, req.target, e.what());
      } catch (const std::exception& e) {
         log::error("Unhandled request pipeline error for {} {}: {}", req.methodText, req.target, e.what());
      } catch (...) {
         log::error("Unhandled request pipeline error for {} {}", req.methodText, req.target);
      }
      return internalServerError("internal server error");
   }

   HttpResponse Router::dispatch(const HttpRequest& req) const {
      for (const auto& endpoint : endpoints_) {
         if (endpoint->matches(req)) {
            try {
               return endpoint->handle(req);
            } catch (const EndpointError& e) {
               log::error("Endpoint handler error for {} {}: {}", req.methodText, req.target, e.what());
            } catch (const Error& e) {
               log::error("Router error for {} {}: {}", req.methodText, req.target, e.what());
            } catch (const std::exception& e) {
               log::error("Unhandled route error for {} {}: {}", req.methodText, req.target, e.what());
            } catch (...) {
               log::error("Unhandled route error for {} {}", req.methodText, req.target);
            }
            return internalServerError("internal server error");
         }
      }

      return notFound();
   }

   HttpResponse Router::dispatchMiddleware(std::size_t index, const HttpRequest& req) const {
      if (index == middleware_.size()) {
         return dispatch(req);
      }
      return middleware_[index](req, [this, index](const HttpRequest& nextReq) {
         return dispatchMiddleware(index + 1, nextReq);
      });
   }

   void Router::ensureMutable() const {
      if (frozen_) {
         throw RouterError{"router is frozen and cannot be modified"};
      }
   }

} // namespace c2server
