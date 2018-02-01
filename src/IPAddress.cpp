// MAVLink router and firewall.
// Copyright (C) 2017-2018  Michael R. Shannon <mrshannon.aerospace@gmail.com>
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
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include <cstring>
#include <netdb.h>
#include <netinet/in.h>

#include <boost/algorithm/string.hpp>

#include "config.hpp"
#include "util.hpp"
#include "DNSLookupError.hpp"
#include "IPAddress.hpp"


// Private functions.
#ifdef UNIX
namespace
{
    IPAddress unix_dnslookup(const std::string &url, unsigned int port);
}
#elif WINDOWS
#endif


// Construct IP address from address and port number.
void IPAddress::construct_(unsigned long address, unsigned int port)
{
    if (address > 0xFFFFFFFF)
    {
        std::stringstream ss;
        ss << "Address (0x"
           << std::uppercase << std::hex << address << std::nouppercase
           << ") is outside of the allowed range (0x00000000 - 0xFFFFFFFF).";
        throw std::out_of_range(ss.str());
    }

    if (port > 65535)
    {
        throw std::out_of_range(
            "port number (" + std::to_string(port) +
            ") is outside of the allowed range (0 - 65535).");
    }

    address_ = address;
    port_ = port;
}


/** Construct IP from another IP address, changing the port number.
 *
 *  Copy constructor that also changes the port.
 *
 *  \param other IP address to copy from.
 *  \param port Port number (0 - 65535).  A port of 0 means no specific port.
 *  \throws std::out_of_range If the port number is out of range.
 */
IPAddress::IPAddress(const IPAddress &other, unsigned int port)
{
    construct_(other.address_, port);
}


/** Construct IP address from address and port number.
 *
 *  \param address 32-bit IP address in system byte order (0x00000000 -
 *      0xFFFFFFFF).
 *  \param port Port number (0 - 65535).  A port of 0 means no specific port.
 *  \throws std::out_of_range If the address or port number is out of range.
 */
IPAddress::IPAddress(unsigned long address, unsigned int port)
{
    construct_(address, port);
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
 *  \throws std::invalid_argument If the string does not represent a valid IP
 *      address.
 *  \throws std::out_of_range If an address octet or the port number is out of
 *      range.
 */
IPAddress::IPAddress(std::string address)
{
    // Separate port from address.
    unsigned int port = 0;
    std::vector<std::string> parts;
    boost::split(parts, address, [](char c)
    {
        return c == ':';
    });

    // Read port.
    if (parts.size() == 2)
    {
        std::istringstream(parts.back()) >> port;
        parts.pop_back();
    }

    if (parts.size() != 1)
    {
        throw std::invalid_argument("Invalid IP address string.");
    }

    address = parts.back();

    // Check validity of address string.
    if (address.size() < 7 || !(isdigit(address.front()))
            || !isdigit(address.back()))
    {
        throw std::invalid_argument("Invalid IP address string.");
    }

    for (auto c : address)
    {
        if (!(c == '.' || isdigit(c)))
        {
            throw std::invalid_argument("Invalid IP address string.");
        }
    }

    // Read address.
    std::replace(address.begin(), address.end(), '.', ' ');
    std::vector<unsigned long> octets;
    std::istringstream ss(address);
    unsigned long octet;

    while (ss >> octet)
    {
        octets.push_back(octet);
    }

    // Ensure proper number of octets.
    if (octets.size() != 4)
    {
        throw std::invalid_argument("Invalid IP address string.");
    }

    // Ensure octets are between 0 and 255.
    for (auto i : octets)
        if (i > 255)
        {
            throw std::out_of_range(
                "Address octet (" + std::to_string(i) +
                ") is outside of the allowed range (0 - 255).");
        }

    {
    }

    construct_((octets[0] << 8 * 3) | (octets[1] << 8 * 2) |
               (octets[2] << 8) | octets[3], port);
}


/** Return the IP address.
 *
 *  \return The 32-bit IP address (in system byte order) as an integer
 *      (0x00000000 - 0xFFFFFFFF).
 *  \complexity \f$O(1)\f$
 */
unsigned long IPAddress::address() const
{
    return address_;
}


/** Return the port.
 *
 *  \return The port number (0 - 65535).
 *  \complexity \f$O(1)\f$
 */
unsigned int IPAddress::port() const
{
    return port_;
}


/** Equality comparison.
 *
 *  \relates IPAddress
 *  \param lhs The left hand side IP address.
 *  \param rhs The right hand side IP address.
 *  \retval true if \p lhs and \p rhs have the same address and port.
 *  \retval false if \p lhs and \p rhs do not have the same address and port.
 *  \complexity \f$O(1)\f$
 */
bool operator==(const IPAddress &lhs, const IPAddress &rhs)
{
    return (lhs.address() == rhs.address()) && (lhs.port() == rhs.port());
}


/** Inequality comparison.
 *
 *  \relates IPAddress
 *  \param lhs The left hand side IP address.
 *  \param rhs The right hand side IP address.
 *  \retval true if \p lhs and \p rhs do not have the same address and port.
 *  \retval false if \p lhs and \p rhs have the same address and port.
 *  \complexity \f$O(1)\f$
 */
bool operator!=(const IPAddress &lhs, const IPAddress &rhs)
{
    return (lhs.address() != rhs.address()) || (lhs.port() != rhs.port());
}


/** Less than comparison.
 *
 *  \note The address is considered first followed by the port.
 *
 *  \relates IPAddress
 *  \param lhs The left hand side IP address.
 *  \param rhs The right hand side IP address.
 *  \retval true if \p lhs is less than \p rhs.
 *  \retval false if \p lhs is not less than \p rhs.
 *  \complexity \f$O(1)\f$
 */
bool operator<(const IPAddress &lhs, const IPAddress &rhs)
{
    return (lhs.address() < rhs.address()) || ((lhs.address() == rhs.address())
            && (lhs.port() < rhs.port()));
}


/** Greater than comparison.
 *
 *  \note The address is considered first followed by the port.
 *
 *  \relates IPAddress
 *  \param lhs The left hand side IP address.
 *  \param rhs The right hand side IP address.
 *  \retval true if \p lhs is greater than \p rhs.
 *  \retval false if \p lhs is not greater than \p rhs.
 *  \complexity \f$O(1)\f$
 */
bool operator>(const IPAddress &lhs, const IPAddress &rhs)
{
    return (lhs.address() > rhs.address()) || ((lhs.address() == rhs.address())
            && (lhs.port() > rhs.port()));
}


/** Less than or equal comparison.
 *
 *  \note The address is considered first followed by the port.
 *
 *  \relates IPAddress
 *  \param lhs The left hand side IP address.
 *  \param rhs The right hand side IP address.
 *  \retval true if \p lhs is less than or eqaul to \p rhs.
 *  \retval false if \p lhs is greater than \p rhs.
 *  \complexity \f$O(1)\f$
 */
bool operator<=(const IPAddress &lhs, const IPAddress &rhs)
{
    return (lhs < rhs) || (lhs == rhs);
}


/** Greater than comparison.
 *
 *  \note The address is considered first followed by the port.
 *
 *  \relates IPAddress
 *  \param lhs The left hand side IP address.
 *  \param rhs The right hand side IP address.
 *  \retval true if \p lhs is greater than or equal to \p rhs.
 *  \retval false if \p lhs is less than \p rhs.
 *  \complexity \f$O(1)\f$
 */
bool operator>=(const IPAddress &lhs, const IPAddress &rhs)
{
    return (lhs > rhs) || (lhs == rhs);
}


/** Print the IP address to the given output stream.
 *
 *  The format is "<IP Address>" or "<IP Address>:<Port Number>" if the port
 *  number is nonzero an "<IP Address>" if the port is 0.
 *
 *  Some examples are:
 *      - `127.0.0.1`
 *      - `127.0.0.1:8888`
 *      - `183.125.120.42:443`
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
 *  \throws DNSLookupError If the address cannot be found.
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

namespace
{

    /* Lookup an IP address based on a hostname.
     *
     * This version is unix only and will be called by \ref dnslookup on UNIX
     * systems.
     */
    IPAddress unix_dnslookup(const std::string &url, unsigned int port)
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

        std::unique_ptr<struct addrinfo, void(*)(struct addrinfo *)>
        result(result_ptr, freeaddrinfo);
        result_ptr = nullptr;

        for (result_ptr = result.get(); result_ptr != nullptr;
                result_ptr = result_ptr->ai_next)
        {
            struct sockaddr_in *address_ptr =
                    reinterpret_cast<struct sockaddr_in *>(result_ptr->ai_addr);
            unsigned long address = ntohl(address_ptr->sin_addr.s_addr);
            addresses.insert(IPAddress(address, port));
        }

        // This should never be true but it's here just in case.
        if (addresses.empty())
        {
            // LCOV_EXCL_START
            throw DNSLookupError(url);
            // LCOV_EXCL_STOP
        }

        return *(addresses.begin());
    }

}

#elif WINDOWS

#endif
