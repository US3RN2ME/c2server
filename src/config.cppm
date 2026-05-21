export module c2server.config;

import std;

export namespace c2server {

   struct InitialPayload {
      std::string value;
   };

   struct Host {
      std::string value;
   };

   struct Port {
      std::uint16_t value;
   };

} // namespace c2server
