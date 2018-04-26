// MAVLink router and firewall.
// Copyright (C) 2018  Michael R. Shannon <mrshannon.aerospace@gmail.com>
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


#ifndef PARTIALSENDERROR_HPP_
#define PARTIALSENDERROR_HPP_


#include <exception>
#include <string>


/** Exception type emmited when an interface failes to send a complete packet.
 */
class PartialSendError : public std::exception
{
    public:
        PartialSendError(unsigned long bytes_sent, unsigned long total_bytes);
        const char *what() const noexcept;

    private:
        std::string message_;
};


#endif // PARTIALSENDRROR_HPP_
