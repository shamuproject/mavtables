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


#include <cstdlib>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "Filesystem.hpp"
#include "configuration.hpp"



void print_node(const tao::pegtl::parse_tree::node &n, const std::string &s)
{
    // detect the root node:
    if (n.is_root())
    {
        std::cout << "ROOT" << std::endl;
    }
    else
    {
        if (n.has_content())
        {
            std::cout << s << n.name() << " \"" << n.content() << "\" at " << n.begin() <<
                      " to " << n.end() << std::endl;
        }
        else
        {
            std::cout << s << n.name() << " at " << n.begin() << std::endl;
        }
    }

    // print all child nodes
    if (!n.children.empty())
    {
        const auto s2 = s + "  ";

        for (auto &up : n.children)
        {
            print_node(*up, s2);
        }
    }
}


void parse_file(std::string filename)
{
    tao::pegtl::read_input<> in(filename);
    const auto root = tao::pegtl::parse_tree::parse<config::grammar, config::store>( in );
    if (root != nullptr)
    {
        print_node(*root);
    }
    else
    {
        std::cout << "Configuration file is invalid." << std::endl;
    }
}
