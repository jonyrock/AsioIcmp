
#include <boost/asio.hpp>
#include <iostream>

void handler(const boost::system::error_code& ec){
    std::cout << "s" << std::endl;
}

int main() {
    boost::asio::io_service ios;
    boost::asio::deadline_timer timer(ios, boost::posix_time::seconds(1));
    timer.async_wait(handler);
    std::cout << "helloshkin" << std::endl;
    ios.run();
}