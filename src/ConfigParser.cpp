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


#include <ostream>
#include <string>
#include <stdexcept>

#include "parse_tree.hpp"
#include "config_grammar.hpp"
#include "ConfigParser.hpp"


/** Construct a configuration parser from a file.
 *
 *  \param filename The path to the configuration file to parse.
 */
ConfigParser::ConfigParser(std::string filename)
    : in_(filename)
{
    root_ = config::parse(in_);
    if (root_ == nullptr)
    {
        // It is technically impossible parsing errors should be raised as a
        // parse_error.
        // LCOV_EXCL_START
        throw std::runtime_error(
            "Unexpected error while parsing configuration file.");
        // LCOV_EXCL_STOP
    }
}


/** Print the configuration settings to the given output stream.
 *
 *  An example (that of examples/test.conf) is:
 *
 *  ```
 *  examples/test.conf:001:  default_action
 *  examples/test.conf:001:  |  accept
 *  examples/test.conf:004:  udp
 *  examples/test.conf:005:  |  port 14500
 *  examples/test.conf:006:  |  address 127.0.0.1
 *  examples/test.conf:010:  serial
 *  examples/test.conf:011:  |  device /dev/ttyUSB0
 *  examples/test.conf:012:  |  baudrate 115200
 *  examples/test.conf:013:  |  flow_control yes
 *  examples/test.conf:017:  serial
 *  examples/test.conf:018:  |  device COM1
 *  examples/test.conf:019:  |  baudrate 9600
 *  examples/test.conf:020:  |  flow_control no
 *  examples/test.conf:025:  chain default
 *  examples/test.conf:027:  |  call some_chain10
 *  examples/test.conf:027:  |  |  condition
 *  examples/test.conf:027:  |  |  |  source 127.1
 *  examples/test.conf:027:  |  |  |  dest 192.0
 *  examples/test.conf:028:  |  reject
 *  examples/test.conf:033:  chain some_chain10
 *  examples/test.conf:035:  |  accept
 *  examples/test.conf:035:  |  |  priority 99
 *  examples/test.conf:035:  |  |  condition
 *  examples/test.conf:035:  |  |  |  dest 192.0
 *  examples/test.conf:036:  |  accept
 *  examples/test.conf:036:  |  |  condition
 *  examples/test.conf:036:  |  |  |  packet_type PING
 *  examples/test.conf:037:  |  accept
 *  ```
 *
 *  \relates MAVAddress
 *  \param os The output stream to print to.
 *  \param config_parser The MAVLink address to print.
 *  \return The output stream.
 */
std::ostream &operator<<(std::ostream &os, const ConfigParser &config_parser)
{
    os << *config_parser.root_;
    return os;
}
