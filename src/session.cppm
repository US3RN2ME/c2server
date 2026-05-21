module;
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>

export module c2server.session;

import c2server.http;
import std;

export namespace c2server {

   namespace net = boost::asio;
   using tcp = net::ip::tcp;

   net::awaitable<void> runSession(tcp::socket socket, HttpHandler handler);

} // namespace c2server
