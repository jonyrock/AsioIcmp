#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <sys/time.h>
#include <istream>
#include <iostream>
#include <ostream>

#include "../include/ipv4_header.hpp"
#include "../include/icmp_header.hpp"
#include "../include/icmp_time_body.hpp"

using boost::asio::ip::icmp;
using boost::asio::deadline_timer;

using namespace std;

namespace posix_time = boost::posix_time;

class DateRequester {
public:

    DateRequester(boost::asio::io_service& io_service, const char* destination) :
    resolver_(io_service), socket_(io_service, icmp::v4()), timer_(io_service) {
        icmp::resolver::query query(icmp::v4(), destination, "");
        destination_ = *resolver_.resolve(query);
        send();
        receive();
    }

private:

    icmp_time_body add_time_body(icmp_header& timestamp_request) {

        timeval tim;
        gettimeofday(&tim, NULL);

        uint32_t otime = (tim.tv_sec % (24 * 60 * 60)) * 1000
                + tim.tv_usec / 1000;

        icmp_time_body body;

        body.originateTime(otime);
        body.reciveTime(0);
        body.transmitTime(0);

        //        cout << body.originateTime() << endl;
        //        cout << body.reciveTime() << endl;
        //        cout << body.transmitTime() << endl;
        //        
        //        cout << " -------- " << endl;

        compute_checksum(timestamp_request, body.begin(), body.end());

        return body;

    }

    void send() {

        icmp_header timestamp_request;
        timestamp_request.type(icmp_header::timestamp_request);
        timestamp_request.code(0);
        timestamp_request.identifier(get_identifier());
        timestamp_request.sequence_number(0);

        icmp_time_body body = add_time_body(timestamp_request);

        // Encode the request packet.
        boost::asio::streambuf request_buffer;
        std::ostream os(&request_buffer);
        
        cout << body.originateTime() << endl;
        cout << "--------" << endl;
        
        os << timestamp_request << body;


        // Send the request.
        time_sent_ = posix_time::microsec_clock::universal_time();

        socket_.send_to(request_buffer.data(), destination_);
        
    }


    void receive() {

        socket_.receive(reply_buffer_.prepare(54));
        reply_buffer_.commit(54);

        std::istream is(&reply_buffer_);
        ipv4_header ipv4_hdr;
        icmp_header icmp_hdr;
        icmp_time_body icmp_body;
        is >> ipv4_hdr >> icmp_hdr >> icmp_body;

        if (is && icmp_hdr.type() == icmp_header::timestamp_reply
                && icmp_hdr.identifier() == get_identifier()) {
            cout << icmp_body.originateTime() << endl;
            cout << icmp_body.reciveTime() << endl;
            cout << icmp_body.transmitTime() << endl;
        }
        exit(0);
    }

    static inline unsigned short get_identifier() {
        return static_cast<unsigned short> (::getpid());
    }

    icmp::resolver resolver_;
    icmp::endpoint destination_;
    icmp::socket socket_;
    deadline_timer timer_;
    posix_time::ptime time_sent_;
    boost::asio::streambuf reply_buffer_;
    
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: ping <host>" << std::endl;
            return 1;
        }
        boost::asio::io_service io_service;
        DateRequester p(io_service, argv[1]);
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}