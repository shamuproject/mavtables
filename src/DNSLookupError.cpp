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


#include <string>
#include "DNSLookupError.hpp"


/** Construct a DNSLookupError given the unresolvable URL.
 *
 *  \param url The URL that could not be resolved.
 */
DNSLookupError::DNSLookupError(std::string url)
{
    message_ = "DNSLookupError: Could not find an IP address for \""
        + url + "\"";
}


/** Return error string.
 *
 *  \return Error string containing unresolvable hostname.
 */
const char *DNSLookupError::what() const noexcept
{
    return message_.c_str();
}
