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


#ifndef CONFIGPARSER_HPP_
#define CONFIGPARSER_HPP_


#include <memory>
#include <ostream>

#include "parse_tree.hpp"


class ConfigParser
{
    public:
        ConfigParser();
        ConfigParser(const ConfigParser &other) = delete;
        ConfigParser(ConfigParser &&other) = delete;
        ConfigParser &operator=(const ConfigParser &other) = delete;
        ConfigParser &operator=(ConfigParser &&other) = delete;
        virtual ~ConfigParser();

        friend std::ostream &operator<<(
                std::ostream &os, const ConfigParser &config_parser);

    protected:
        std::unique_ptr<config::parse_tree::node> root_;
};


std::ostream &operator<<(std::ostream &os, const ConfigParser &config_parser);


#endif // CONFIGPARSER_HPP_
