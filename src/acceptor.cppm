module;
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>

export module c2server.acceptor;

import c2server.http;
import c2server.session;

export namespace c2server {

   net::awaitable<void> listen(tcp::acceptor acceptor, HttpHandler handler);

} // namespace c2server
