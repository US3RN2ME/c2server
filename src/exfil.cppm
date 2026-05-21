export module c2server.exfil;

import std;

export namespace c2server {

   using ExfilFn = std::function<void(std::string_view ip, std::string_view data)>;

   ExfilFn makeFileExfil(std::string_view path);

} // namespace c2server
