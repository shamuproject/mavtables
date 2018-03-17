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


#ifndef CONFIGSTRINGPARSER_HPP_
#define CONFIGSTRINGPARSER_HPP_


#include <string>

#include <pegtl.hpp>

#include "ConfigParser.hpp"


/** Construct a configuration parser that parses a string.
 */
class ConfigStringParser : public ConfigParser
{
    public:
        ConfigStringParser(std::string data);
        ConfigStringParser(const ConfigStringParser &other) = delete;
        ConfigStringParser(ConfigStringParser &&other) = delete;
        ConfigStringParser &operator=(const ConfigStringParser &other) = delete;
        ConfigStringParser &operator=(ConfigStringParser &&other) = delete;
        virtual ~ConfigStringParser();

    private:
        tao::pegtl::string_input<> in_;
};


#endif // CONFIGSTRINGPARSER_HPP_
