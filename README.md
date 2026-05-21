# c2server

Builds a small C++23 REST server library and executable.

Runtime settings are read from JSON with `nlohmann_json`. The executable loads `config.json` from its working
directory by default, or a custom path passed as the first argument:

```powershell
.\c2server.exe .\config.json
```

Library consumers can use `c2server::loadServerSettings(path)` for only the server settings, or
`c2server::loadAppConfig(path)` for the executable's full configuration.

Applications can attach extra shutdown work with `Server::setShutdownCallback`. The callback receives the signal
number and runs during graceful shutdown after the listener is closed.

Routes should be registered before constructing `Server`. The server freezes the router at construction time, so
request handling can read routes concurrently without route-mutation locks.

SSL can be enabled under `server.ssl`. When enabled, the listener detects TLS handshakes and can still allow plain
HTTP on the same port if `allow_plain_http` is true:

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
