/* 
 * File:   icmp_time_body.hpp
 * Author: alex
 *
 * Created on December 31, 2013, 2:34 AM
 */

#ifndef ICMP_TIME_BODY_HPP
#define	ICMP_TIME_BODY_HPP

class icmp_time_body {

    union {
        uint32_t timee[3];
        char originateTimeB[4 * 3];
    };

public:

    uint32_t originateTime() const {
        return timee[0];
    }

    void originateTime(uint32_t v) {
        timee[0] = v;
    }

    uint32_t reciveTime() const {
        return timee[1];
    }

    void reciveTime(uint32_t v) {
        timee[1] = v;
    }

    uint32_t transmitTime() const {
        return timee[2];
    }

    void transmitTime(uint32_t v) {
        timee[2] = v;
    }

    char* begin() {
        return originateTimeB;
    }

    const char* begin() const {
        return originateTimeB;
    }

    size_t size() const {
        return 12;
    }

    char* end() {
        return originateTimeB + (4 * 3);
    }

    const char* end() const {
        return originateTimeB + (4 * 3);
    }

    friend std::istream& operator>>(std::istream& is, icmp_time_body& body) {
        return is.read(reinterpret_cast<char*>(body.begin()), body.size());
    }

    friend std::ostream& operator<<(std::ostream& os, const icmp_time_body& body) {
        return os.write(body.begin(), body.size());
    }

};


#endif	/* ICMP_TIME_BODY_HPP */

