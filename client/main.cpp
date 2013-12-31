#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <sys/time.h>
#include <istream>
#include <iostream>
#include <ostream>

#include "ipv4_header.hpp"
#include "icmp_header.hpp"
#include "icmp_time_body.hpp"

using boost::asio::ip::icmp;
// using namepsace boost::posix_time;
using boost::asio::deadline_timer;

using namespace std;

namespace posix_time = boost::posix_time;

class DateRequester {
public:

    DateRequester(boost::asio::io_service& io_service, const char* destination) :
    resolver_(io_service), socket_(io_service, icmp::v4()), timer_(io_service) {
        icmp::resolver::query query(icmp::v4(), destination, "");
        destination_ = *resolver_.resolve(query);
        start_send();
        start_receive();
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

        compute_checksum(timestamp_request, body.begin(), body.end());

        return body;


    }

    void start_send() {

        icmp_header timestamp_request;
        timestamp_request.type(icmp_header::timestamp_request);
        timestamp_request.code(0);
        timestamp_request.identifier(get_identifier());
        timestamp_request.sequence_number(0);

        icmp_time_body body = add_time_body(timestamp_request);


        // Encode the request packet.
        boost::asio::streambuf request_buffer;
        std::ostream os(&request_buffer);
        os << timestamp_request << body;


        // Send the request.
        time_sent_ = posix_time::microsec_clock::universal_time();


        socket_.send_to(request_buffer.data(), destination_);

        // Wait up to five seconds for a reply.
        timer_.expires_at(time_sent_ + posix_time::seconds(3));
        timer_.async_wait(boost::bind(&DateRequester::handle_timeout, this));

    }

    void handle_timeout() {
        cout << "TIMEOOUT" << endl;
        exit(1);
        // Requests must be sent no less than one second apart.
        timer_.expires_at(time_sent_ + posix_time::seconds(1));
        timer_.async_wait(boost::bind(&DateRequester::start_send, this));
    }

    void start_receive() {
        cout << "start_receive" << endl;
        // Discard any data already in the buffer.
        reply_buffer_.consume(reply_buffer_.size());

        // Wait for a reply. We prepare the buffer to receive up to 64KB.
        socket_.async_receive(reply_buffer_.prepare(65536),
                boost::bind(&DateRequester::handle_receive, this, _2));
    }

    void handle_receive(std::size_t length) {
        // The actual number of bytes received is committed to the buffer so that we
        // can extract it using a std::istream object.
        reply_buffer_.commit(length);

        // Decode the reply packet.
        std::istream is(&reply_buffer_);
        ipv4_header ipv4_hdr;
        icmp_header icmp_hdr;
        icmp_time_body icmp_body;
        is >> ipv4_hdr >> icmp_hdr >> icmp_body;

        // We can receive all ICMP packets received by the host, so we need to
        // filter out only the echo replies that match the our identifier and
        // expected sequence number.
        if (is && icmp_hdr.type() == icmp_header::timestamp_reply
                && icmp_hdr.identifier() == get_identifier()) {
            // If this is the first reply, interrupt the five second timeout.
            // Print out some information about the reply packet.
            posix_time::ptime now = posix_time::microsec_clock::universal_time();
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