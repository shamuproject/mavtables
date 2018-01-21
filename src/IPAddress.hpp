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


#ifndef IPADDRESS_HPP_
#define IPADDRESS_HPP_


#include <string>
#include <ostream>
#include <stdexcept>
#include "DNSLookupError.hpp"


/** An IP address with optional port number.
 */
class IPAddress
{
    private:
        unsigned long address_;
        unsigned int port_;
        void construct_(unsigned long address, unsigned int port);

    public:
        /** Copy constructor.
         *
         * \param other IP address to copy.
         */
        IPAddress(const IPAddress &other) = default;
        IPAddress(const IPAddress &other, unsigned int port);
        IPAddress(unsigned long address, unsigned int port = 0);
        IPAddress(std::string address);
        unsigned long address() const;
        unsigned int port() const;
        /** Assignment operator.
         *
         * \param other IP address to copy.
         */
        IPAddress &operator=(const IPAddress &other) = default;

        friend std::ostream &operator<<(
            std::ostream &os, const IPAddress &ipaddress);
};


bool operator==(const IPAddress &lhs, const IPAddress &rhs);
bool operator!=(const IPAddress &lhs, const IPAddress &rhs);
bool operator<(const IPAddress &lhs, const IPAddress &rhs);
bool operator>(const IPAddress &lhs, const IPAddress &rhs);
bool operator<=(const IPAddress &lhs, const IPAddress &rhs);
bool operator>=(const IPAddress &lhs, const IPAddress &rhs);
std::ostream &operator<<(std::ostream &os, const IPAddress &ipaddress);


IPAddress dnslookup(const std::string &url);


#endif // IPADDRESS_HPP_
