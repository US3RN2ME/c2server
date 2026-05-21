module;
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>

module c2server.server;

import c2server.error;
import c2server.http;
import std;

#include "server_detail.hpp"

namespace c2server::detail {

   ssl::context makeSslContext(const SslSettings& settings) {
      ssl::context ctx{ssl::context::tls_server};
      ctx.set_options(ssl::context::default_workarounds | ssl::context::no_sslv2 | ssl::context::no_sslv3 |
                      ssl::context::single_dh_use);

      if (!settings.privateKeyPassword.empty()) {
         ctx.set_password_callback([password = settings.privateKeyPassword](std::size_t, ssl::context::password_purpose) {
            return password;
         });
      }

      try {
         ctx.use_certificate_chain_file(settings.certificateChainFile);
         ctx.use_private_key_file(settings.privateKeyFile, ssl::context::pem);
         if (!settings.dhParamsFile.empty()) {
            ctx.use_tmp_dh_file(settings.dhParamsFile);
         }
      } catch (const std::exception& e) {
         throw ServerError{"failed to configure SSL context: {}", e.what()};
      }

      return ctx;
   }

} // namespace c2server::detail
