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


#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "config.hpp"
#include "configuration.hpp"
#include "util.hpp"
#include "Options.hpp"


namespace po = boost::program_options;


int main(int argc, char *argv[])
{
    try
    {
        Options options(std::cout, argc, argv);

        if (options)
        {
            std::cout << "Configuration file located at "
                      << options.config_file() << std::endl;
            // parse_file2(options.config_file());
            parse_file(options.config_file());
        }
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
    // mavtables --dot
    // mavtables --ast
    // (void)argc;
    // (void)argv;
    // std::cout << NAME << ": v"
    //           << VERSION_MAJOR << "."
    //           << VERSION_MINOR << "."
    //           << VERSION_PATCH << std::endl;
    // std::cout << "A MAVLink router and firewall." << std::endl;
}
