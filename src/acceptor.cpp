module;
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>

module c2server.acceptor;

import c2server.logger;

namespace c2server {

   net::awaitable<void> listen(tcp::acceptor acceptor, HttpHandler handler) {
      log::info("Listening on {}:{}", acceptor.local_endpoint().address().to_string(), acceptor.local_endpoint().port());

      while (true) {
         auto socket = co_await acceptor.async_accept(net::use_awaitable);
         net::co_spawn(acceptor.get_executor(), runSession(std::move(socket), handler), net::detached);
      }
   }

} // namespace c2server
