# c2server

`c2server` is a small C++23 REST server library built on Boost.Beast and Boost.Asio. It provides a reusable
`c2server_core` library with routing, JSON-backed server configuration, structured runtime errors, graceful shutdown
handling, optional TLS, and convenience HTTP response helpers.

The project builds the reusable `c2server_core` library and, by default, the `c2server` executable.

The public API is exposed through C++ modules. Internally, the server uses Boost.Asio awaitables and C++ coroutines for
the accept loop, TLS detection, request reads, response writes, and graceful shutdown flow.

## Requirements

Common requirements:

- CMake 3.30 or newer
- A C++23 compiler with C++ module support
- vcpkg manifest mode, or otherwise available CMake packages for:
    - `boost-beast`
    - `nlohmann-json`
    - `openssl`
    - `spdlog`

Windows requirements:

- Visual Studio 2022 or newer with MSVC C++ tools
- Windows SDK

Linux requirements:

- A C++23 compiler and standard library combination with usable C++ module support
- OpenSSL development libraries when not using vcpkg

## Build

Configure and build with CMake:

```sh
cmake -B build \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DC2SERVER_BUILD_APP=ON
cmake --build build --parallel
```

On Windows with the Visual Studio developer environment:

```powershell
cmake -B build `
  -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake `
  -DC2SERVER_BUILD_APP=ON
cmake --build build
```

The build uses C++23 modules and `import std`, so the compiler, standard library, and CMake generator must support
module dependency scanning.

## Modules and Coroutines

`c2server_core` is a module-first library. Public interfaces live in `.cppm` module units such as:

- `c2server.config`
- `c2server.error`
- `c2server.http`
- `c2server.logger`
- `c2server.payload`
- `c2server.router`
- `c2server.server`

Consumers import the modules they use instead of including public headers:

```c++
import c2server.http;
import c2server.router;
import c2server.server;
```

The asynchronous server implementation is coroutine-based. The listener accepts connections with Boost.Asio
`awaitable` operations, sessions read and write HTTP messages with `co_await`, and TLS-capable listeners use Beast's
SSL detection before dispatching to either a plain HTTP or TLS session.

Boost and OpenSSL types are kept in implementation files and private detail headers. They are not part of the public
module API.

## CMake Options

- `C2SERVER_BUILD_APP=ON` builds the `c2server` executable.

## Configuration

The executable loads `config.json` from its working directory by default, or a custom config path passed as the first
argument:

```sh
./build/app/c2server ./config/config.json
```

The default config is copied from `config/config.json` into the build directory as `config.json`.

```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "worker_threads": 0,
    "request_body_limit_bytes": 1048576,
    "request_timeout_seconds": 10,
    "ssl": {
      "enabled": false,
      "allow_plain_http": true,
      "certificate_chain_file": "",
      "private_key_file": "",
      "private_key_password": "",
      "dh_params_file": ""
    }
  },
  "log_file": "c2server.log"
}
```

## Library Usage

Import the modules you need and register routes before constructing the server:

```c++
import c2server.config;
import c2server.http;
import c2server.router;
import c2server.server;

import std;

int main() {
   auto settings = c2server::loadServerSettings("config.json");
   auto router = std::make_shared<c2server::Router>();

   router->get("/health", [](const c2server::HttpRequest&) {
      return c2server::jsonOk({{"status", "ok"}});
   });

   c2server::Server{std::move(settings), router}.run();
}
```

Routes are immutable after server construction. `Server` freezes the router at construction time so request handling can
read routes concurrently without route-mutation locks.

## HTTP Helpers

`c2server.http` provides convenience helpers for common responses:

```c++
return c2server::ok("hello");
return c2server::ok("<h1>hello</h1>", "text/html");
return c2server::jsonOk({{"status", "ok"}});
return c2server::json({{"error", "invalid request"}}, 400);
```

`json` and `jsonOk` accept `c2server::JsonObject`, an alias for `std::unordered_map<std::string, std::string>`, and
return responses with `application/json`.

## TLS

TLS can be enabled under `server.ssl`. When enabled, the listener detects TLS handshakes and can still allow plain HTTP
on the same port if `allow_plain_http` is true:

```json
{
  "server": {
    "ssl": {
      "enabled": true,
      "allow_plain_http": true,
      "certificate_chain_file": "server.crt",
      "private_key_file": "server.key"
    }
  }
}
```

## Shutdown

The server installs Boost.Asio signal handling for graceful shutdown. Stop the executable with `Ctrl+C` or the
platform's normal termination signal.

Applications can attach extra shutdown work with `Server::setShutdownCallback`:

```c++
c2server::Server server{settings, router};
server.setShutdownCallback([](int signalNumber) {
   std::println("shutdown signal: {}", signalNumber);
});
server.run();
```
