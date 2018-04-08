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


#include <string>
#include <utility>

#include "PartialSendError.hpp"


/** Construct a PartialSendError..
 *
 *  \param bytes_sent Number of bytes there were sent.
 *  \param total_bytes Number of bytes in the packet.
 */
PartialSendError::PartialSendError(
    unsigned long bytes_sent, unsigned long total_bytes)
{
    message_ = "Could only write " + std::to_string(bytes_sent) +
               " of " + std::to_string(total_bytes) + " bytes.";
}


/** Return error message string.
 *
 *  \return Error message string.
 */
const char *PartialSendError::what() const noexcept
{
    return message_.c_str();
}
