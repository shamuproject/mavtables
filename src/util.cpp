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


#include <algorithm>
#include <cctype>
#include <string>

#include "util.hpp"


/** \defgroup utility Utility Functions
 *
 *  Utility functions that don't warrant their own file.
 */


/** Convert string to lower case.
 *
 *  \ingroup utility
 *  \param string The string to convert to lower case.
 *  \returns The \a string converted to lower case.
 */
std::string to_lower(std::string string)
{
    std::transform(string.begin(), string.end(), string.begin(), ::tolower);
    return string;
}
