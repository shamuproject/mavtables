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


#include <string>
#include <cctype>
#include "util.hpp"


/** \defgroup utility Utility Function
 *
 *  Utility functions that don't warrant their own file.
 */


/** \brief Capitalize first letter of a string.
 *  \ingroup utility
 *  \bug Index error if given an empty string.
 */
std::string capital_case(std::string str)
{
    str[0] = static_cast<char>(toupper(str[0]));
    return str;
}
