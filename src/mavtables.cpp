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
#include "config_grammar.hpp"
#include "ConfigParser.hpp"
#include "Logger.hpp"
#include "Options.hpp"
#include "util.hpp"


int main(int argc, const char *argv[])
{
    try
    {
        Options options(argc, argv);

        if (options)
        {
            auto config = std::make_unique<ConfigParser>(options.config_file());
            if (options.ast())
            {
                std::cout << "===== " << options.config_file()
                    << " =====" << std::endl;
                std::cout << *config;
            }
            if (options.run())
            {
                Logger::level(options.loglevel());
                auto app = config->make_app();
                app->run();
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
