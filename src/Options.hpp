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


#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_


#include <iostream>
#include <optional>
#include <ostream>
#include <string>

#include "Filesystem.hpp"


/** An options class which is used to parse the command line arguments.
 */
class Options
{
    public:
        Options(
            int argc, const char *argv[],
            const Filesystem &filesystem = Filesystem());
        bool ast();
        unsigned int loglevel();
        std::string config_file();
        bool run();
        explicit operator bool() const;

    private:
        bool continue_;
        unsigned int loglevel_;
        std::string config_file_;
        bool print_ast_;
        bool run_firewall_;
};


std::optional<std::string> find_config(
    const Filesystem &filesystem = Filesystem());


#endif  // OPTIONS_HPP_
