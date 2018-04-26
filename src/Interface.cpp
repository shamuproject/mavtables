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


#include <memory>
#include <utility>

#include "ConnectionPool.hpp"
#include "Interface.hpp"


// Placed here to avoid weak-vtables error.
// LCOV_EXCL_START
Interface::~Interface()
{
}
// LCOV_EXCL_STOP


/** Print the give interface to the given output stream.
 *
 *  Some examples are:
 *  ```
 *  serial {
 *      device /dev/ttyUSB0;
 *      baudrate 115200;
 *      flow_control yes;
 *  }
 *  ```
 *  ```
 *  udp {
 *      port 14500;
 *      address 127.0.0.1;
 *  }
 *  ```

 *  \relates Interface
 *  \param os The output stream to print to.
 *  \param interface The interface (or any child of Interface) to print.
 *  \return The output stream.
 */
std::ostream &operator<<(std::ostream &os, const Interface &interface)
{
    return interface.print_(os);
}
