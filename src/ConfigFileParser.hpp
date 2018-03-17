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


#ifndef CONFIGFILEPARSER_HPP_
#define CONFIGFILEPARSER_HPP_


#include <string>

#include <pegtl.hpp>

#include "ConfigParser.hpp"


/** Construct a configuration parser that parses a file.
 */
class ConfigFileParser : public ConfigParser
{
    public:
        ConfigFileParser(std::string filename);
        ConfigFileParser(const ConfigFileParser &other) = delete;
        ConfigFileParser(ConfigFileParser &&other) = delete;
        ConfigFileParser &operator=(const ConfigFileParser &other) = delete;
        ConfigFileParser &operator=(ConfigFileParser &&other) = delete;
        virtual ~ConfigFileParser();

    private:
        tao::pegtl::read_input<> in_;
};


#endif // CONFIGFILEPARSER_HPP_
