#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <sys/time.h>
#include <istream>
#include <iostream>
#include <ostream>
#include <sstream>

#include "../include/ipv4_header.hpp"
#include "../include/icmp_header.hpp"
#include "../include/icmp_time_body.hpp"


using namespace std;
using namespace boost;
using namespace boost::asio::ip;

typedef boost::shared_ptr<icmp::socket> socket_ptr;

class IcmpTimeServer {
public:

    IcmpTimeServer(boost::asio::io_service& io_service) : 
    socket_(io_service, icmp::v4()), ep_(icmp::v4(), 0), resolver_(io_service) {
        process_all();
    }

    void process_all() {
//        while (1) {
            process_once();
//        }
    }

    void add_time_body(icmp_time_body& body) {

        timeval tim;
        gettimeofday(&tim, NULL);

        uint32_t otime = (tim.tv_sec % (24 * 60 * 60)) * 1000
                + tim.tv_usec / 1000;

        body.reciveTime(otime);
        body.transmitTime(otime);

    }

    void process_once() {

        boost::asio::streambuf reply_buffer_;
        socket_.receive(reply_buffer_.prepare(54));
        reply_buffer_.commit(54);

        std::istream is(&reply_buffer_);
        ipv4_header ipv4_hdr;
        icmp_header icmp_hdr;
        icmp_time_body icmp_body;
        is >> ipv4_hdr >> icmp_hdr >> icmp_body;

        if (icmp_hdr.type() != icmp_header::timestamp_request) {
            return;
        }
        cout << "time request from " << ipv4_hdr.source_address() << endl;

        add_time_body(icmp_body);

        icmp_hdr.type(icmp_header::timestamp_reply);

        boost::asio::streambuf request_buffer;
        std::ostream os(&request_buffer);
        compute_checksum(icmp_hdr, icmp_body.begin(), icmp_body.end());
        
        os << icmp_hdr << icmp_body;
        
        cout << "send " << ipv4_hdr.source_address() << endl;
        
        icmp::endpoint destination(ipv4_hdr.source_address(), 0);
        
//        icmp::resolver::query query(icmp::v4(), "mail.ru", "");
//        destination = *resolver_.resolve(query);
        
        socket_.send_to(request_buffer.data(), destination);

    }

private:
    icmp::endpoint ep_;
    icmp::socket socket_;
    icmp::resolver resolver_;

};

int main(int argc, char* argv[]) {
    boost::asio::io_service io_service;
    IcmpTimeServer s(io_service);
    io_service.run();
}