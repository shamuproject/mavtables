// MAVLink router and firewall.
// Copyright (C) 2017  Michael R. Shannon <mrshannon.aerospace@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include <set>
#include <memory>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include <cstring>
#include <netdb.h>
#include <netinet/in.h>

#include <boost/algorithm/string.hpp>

#include "config.hpp"
#include "DNSLookupError.hpp"
#include "IPAddress.hpp"
#include "comparators.hpp"
#include "util.hpp"


// Private functions.
#ifdef UNIX
    static IPAddress unix_dnslookup(const std::string &url, unsigned int port);
#elif WINDOWS
#endif


/** Construct IP from another IP address, changing the port number.
 *
 *  Copy constructor that also changes the port.
 *
 *  \param other IP address to copy from.
 *  \param port Port number (0 - 65535).  A port of 0 means no specific port.
 */
IPAddress::IPAddress(const IPAddress &other, unsigned int port)
{
    address_ = other.address_;
    port_ = port;
}


/** Construct IP address from a string.
 *
 *  Parse a string of the form "<IP Address>" or "<IP Address>:<Port Number>".
 *
 *  Some examples are:
 *      - "127.0.0.1"
 *      - "127.0.0.1:8888"
 *      - "183.125.120.42:443"
 *
 *  If no port is given the 0 port is used which represents no specific port.
 *
 *  \param address String representing the IP address and optionally the port
 *      number.
 */
IPAddress::IPAddress(std::string address)
{
    port_ = 0;
    std::vector<std::string> parts;
    boost::split(parts, address, [](char c)
    {
        return c == ':';
    });

    if (parts.size() == 2)
    {
        std::istringstream(parts.back()) >> port_;
        parts.pop_back();
    }

    if (parts.size() != 1)
    {
        throw std::invalid_argument("Invalid IP address string.");
    }

    address = parts.back();
    std::replace(address.begin(), address.end(), '.', ' ');
    std::vector<unsigned long> octets;
    std::istringstream ss(address);
    unsigned long i;

    while (ss >> i)
    {
        octets.push_back(i);
    }

    if (octets.size() != 4)
    {
        throw std::invalid_argument("Invalid IP address string.");
    }

    address_ = (octets[0] << 8 * 3) | (octets[1] << 8 * 2) |
               (octets[2] << 8) | octets[3];
}


/** Construct IP address from address and port number.
 *
 *  \param address 32-bit IP address in system byte order (0x00000000 -
 *      0xFFFFFFFF).
 *  \param port Port number (0 - 65535).  A port of 0 means no specific port.
 */
IPAddress::IPAddress(unsigned long address, unsigned int port)
{
    address_ = address;
    port_ = port;
}


/** Return the IP address.
 *
 *  \return The 32-bit IP address (in system byte order) as an integer
 *      (0x00000000 - 0xFFFFFFFF).
 */
unsigned long IPAddress::address() const
{
    return address_;
}


/** Return the port.
 *
 *  \return The port number (0 - 65535).
 */
unsigned int IPAddress::port() const
{
    return port_;
}

/** Less than comparison.
 *
 *  \note The address is considered first followed by the port.
 *
 *  \relates IPAddress
 *  \param a The first IP address.
 *  \param b The second IP address.
 *  \retval true if \p a is less than \p b.
 *  \retval false if \p a is not less than \p b.
 */
bool operator<(const IPAddress &a, const IPAddress &b)
{
    return (a.address() < b.address()) || ((a.address() == b.address())
                                           && (a.port() < b.port()));
}


/** Equality comparison.
 *
 *  \relates IPAddress
 *  \param a The first IP address.
 *  \param b The second IP address.
 *  \retval true if \p a and \p b have the same address and port.
 *  \retval true if \p a and \p b do not have the same address and port.
 */
bool operator==(const IPAddress &a, const IPAddress &b)
{
    return (a.address() == b.address()) && (a.port() == b.port());
}


/** Print the IP address to the given output stream.
 *
 *  The format is "<IP Address>" or "<IP Address>:<Port Number>" if the port
 *  number is nonzero an "<IP Address>" if the port is 0.
 *
 *  Some examples are:
 *      - "127.0.0.1"
 *      - "127.0.0.1:8888"
 *      - "183.125.120.42:443"
 *
 *  \param os The output stream to print to.
 *  \param ipaddress The IP address to print.
 *  \return The output stream.
 */
std::ostream &operator<<(std::ostream &os, const IPAddress &ipaddress)
{
    const auto bytes = to_bytes(ipaddress.address_);

    for (size_t i = 3; i > 0; --i)
    {
        os << static_cast<int>(bytes[i]) << ".";
    }

    os << static_cast<int>(bytes[0]);

    if (ipaddress.port_ != 0)
    {
        os << ":" << ipaddress.port_;
    }

    return os;
}


/** Lookup an IP address based on a hostname.
 *
 *  \relates IPAddress
 *  \param url The URL to get an IP address for.
 *  \return IP addresses corrensponding to the given URL.
 *  \throws DNSLookupError if the address cannot be found
 *
 */
IPAddress dnslookup(const std::string &url)
{
    #ifdef UNIX
    return unix_dnslookup(url, 0);
    #elif WINDOWS
    return win32_dnslookup(url, 0);
    #endif
}


#ifdef UNIX

/* Lookup an IP address based on a hostname.
 *
 * This version is unix only and will be called by \ref dnslookup on UNIX
 * systems.
 */
static IPAddress unix_dnslookup(const std::string &url, unsigned int port)
{
    std::set<IPAddress> addresses;
    // Setup hints.
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 only (for now)
    hints.ai_protocol = static_cast<int>(port);
    // Get IP addresses.
    struct addrinfo *result_ptr = nullptr;

    if (getaddrinfo(url.c_str(), nullptr, &hints, &result_ptr))
    {
        throw DNSLookupError(url);
    }

    std::unique_ptr<struct addrinfo, void(*)(struct addrinfo *)> result(result_ptr,
            freeaddrinfo);
    result_ptr = nullptr;

    for (result_ptr = result.get(); result_ptr != nullptr;
            result_ptr = result_ptr->ai_next)
    {
        struct sockaddr_in *address_ptr = reinterpret_cast<struct sockaddr_in *>
                                              (result_ptr->ai_addr);
        unsigned long address = ntohl(address_ptr->sin_addr.s_addr);
        // unsigned int port_ = ntohs(address_ptr->sin_port);
        addresses.insert(IPAddress(address, port));
    }

    if (addresses.empty())
    {
        throw DNSLookupError(url);
    }

    return *(addresses.begin());
}

#elif WINDOWS

#endif
