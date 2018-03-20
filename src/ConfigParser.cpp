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


std::ostream &operator<<(std::ostream &os, const ConfigParser &config_parser)
{
    os << *config_parser.root_;
    return os;
}
