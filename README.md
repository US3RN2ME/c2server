# c2server

C++23 HTTP and HTTPS server library built on Boost.Asio and Boost.Beast.

Features:

- route handlers and Express-style middleware
- generated OpenAPI JSON and Swagger UI endpoints
- CORS, request IDs, security headers, access logs, and rate limiting
- JSON configuration
- graceful shutdown
- synchronous HTTP and HTTPS client
- public C++ modules

## Requirements

- CMake 3.30 or newer
- a C++23 compiler with module dependency scanning
- standard library module support for `import std;`
- vcpkg manifest mode, or installed CMake packages for `boost-beast`, `nlohmann-json`, `openssl`, and `spdlog`

MSVC builds require a Visual Studio developer environment. Linux builds require a compiler and standard library
combination that supports C++23 modules and `import std;`.

## Build

With vcpkg:

```sh
cmake -B ./build \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DC2SERVER_BUILD_EXAMPLES=ON \
  -DC2SERVER_BUILD_TESTS=ON \
  -DC2SERVER_BUILD_DOCS=OFF \
  -DC2SERVER_INSTALL=ON
cmake --build ./build --parallel
```

CMake options:

| Option | Default | Description |
| --- | --- | --- |
| `C2SERVER_BUILD_EXAMPLES` | `ON` | Build example executables. |
| `C2SERVER_BUILD_TESTS` | `ON` | Build Boost.UT tests. |
| `C2SERVER_BUILD_DOCS` | `ON` | Enable the `c2server_docs` target when Doxygen is available. |
| `C2SERVER_INSTALL` | `ON` | Enable install rules and package generation. |

Run tests:

```sh
ctest --test-dir ./build --output-on-failure
```

Generate API documentation:

```sh
cmake --build ./build --target c2server_docs
```

## Install

```sh
cmake --install ./build --prefix /desired/prefix
```

The install contains:

- `c2server::core` CMake target
- public `.cppm` module interfaces
- library
- HTML API documentation when `c2server_docs` was generated

## Consume Modules

Consumers must enable C++23 module scanning and `import std` before creating targets. The experimental gate must be set
before `project(...)`.

```cmake
cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
    # CMake experimental feature gate for `import std`.
    # This UUID is defined by CMake, not c2server. It may change between
    # CMake releases. Use the value from Help/dev/experimental.rst in the
    # source tree for the CMake version used to configure the project.
    "d0edc3af-4c50-42ea-a356-e2862fe7a444"
    CACHE STRING "CMake experimental import std gate")

if(CMAKE_HOST_WIN32)
   set(CMAKE_CXX_FLAGS_INIT "/utf-8")
endif()

project(example LANGUAGES CXX)

find_package(c2server CONFIG REQUIRED)

add_executable(example main.cpp)
target_link_libraries(example PRIVATE c2server::core)
set_target_properties(
   example
   PROPERTIES CXX_STANDARD 23
              CXX_STANDARD_REQUIRED ON
              CXX_EXTENSIONS OFF
              CXX_SCAN_FOR_MODULES ON
              CXX_MODULE_STD ON)
```

Configure the consumer with the installation prefix:

```sh
cmake -B ./build \
  -DCMAKE_PREFIX_PATH=/desired/prefix \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build ./build --parallel
```

Public modules:

- `c2server.client`
- `c2server.config`
- `c2server.error`
- `c2server.http`
- `c2server.logger`
- `c2server.middleware`
- `c2server.payload`
- `c2server.router`
- `c2server.server`
- `c2server.version`

## Example

```c++
import c2server.config;
import c2server.http;
import c2server.middleware;
import c2server.router;
import c2server.server;
import c2server.version;
import std;

int main() {
   auto router = std::make_shared<c2server::Router>();

   router->use(c2server::middleware::requestId())
       .use(c2server::middleware::securityHeaders())
       .use(c2server::middleware::cors());

   router->get("/health", [](const c2server::HttpRequest&) {
      return c2server::jsonOk({{"status", "ok"}});
   }, {
      .summary = "Health check",
      .tags = {"system"},
      .responses = {{.status = 200, .description = "Service status", .contentType = "application/json"}},
   });

   router->serveOpenApi({
      .info = {
         .title = "Example API",
         .version = std::string{c2server::kVersion},
         .description = "Generated OpenAPI documentation.",
      },
   });

   c2server::Server{c2server::loadServerSettings("config.json"), router}.run();
}
```

Register routes and middleware before constructing `c2server::Server`. Server construction freezes the router.

`serveOpenApi()` registers:

- `/docs` for Swagger UI
- `/openapi.json` for the generated OpenAPI document

Both paths can be customized:

```c++
router->serveOpenApi({
   .info = {.title = "Example API", .version = "1.0.0"},
   .specTarget = "/api/openapi.json",
   .docsTarget = "/api/docs",
});
```

Example targets:

| Target | Description |
| --- | --- |
| `c2server_payload_server` | Original payload server example without Swagger UI. |
| `c2server_swagger_server` | Documented API example with `/docs` and `/openapi.json`. |
| `c2server_hello_world` | Minimal single-route HTTP server. |
| `c2server_middleware_server` | Request ID, security headers, CORS, and rate-limit example. |

Examples load `config.json` from their working directory by default. Pass a custom path as the first argument:

```sh
./build/examples/c2server_swagger_server ./config/config.json
```
