
#include <boost/asio.hpp>
#include <boost/array.hpp>
//#include <boost/thread.hpp> 
#include <iostream>
#include <string>

using namespace boost;
using namespace std;

asio::io_service io_service;
asio::ip::tcp::resolver resolver(io_service);
asio::ip::tcp::socket sock(io_service);
array<char, 4096> buffer;

void read_handler(const system::error_code& ec, size_t bytes_transferred) {
    if (!ec) {
        cout << string(buffer.data(), bytes_transferred) << endl;
        sock.async_read_some(asio::buffer(buffer), read_handler);
    }
}

void connect_handler(const system::error_code& ec) {
    if (!ec) {
        asio::write(sock,
                asio::buffer("GET / HTTP 1.1\r\r\nHost: www.highscore.de\r\n\r\n"));
        sock.async_read_some(asio::buffer(buffer), read_handler);
    }
}

void resolve_handler(const system::error_code & ec, 
        asio::ip::tcp::resolver::iterator it) {
    if(!ec) {
        sock.async_connect(*it, connect_handler);
    }
}

int main() {
    asio::ip::tcp::resolver::query query("www.highscore.de", "80");
    resolver.async_resolve(query, resolve_handler);
    io_service.run();
}