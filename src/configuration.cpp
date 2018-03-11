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



namespace config
{
    void print_node(const pegtl::parse_tree::node &n, const std::string &s = "");
    void print_node(const pegtl::parse_tree::node &n, const std::string &s)
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
}

#include <cstdio>
#include <fstream>

void parse_file(std::string filename)
{
    // std::cout << filename << std::endl;
    // pegtl::string_input<> in("3 + 4", filename);
    pegtl::read_input<> in(filename);
    // std::ifstream ifs("test.conf");
    // pegtl::istream_input<> in(ifs, 1024, filename);

    // std::ifstream t(filename);
    // std::string str((std::istreambuf_iterator<char>(t)),
    //                  std::istreambuf_iterator<char>());
    //
    // std::cout << str << std::endl;
    // pegtl::string_input<> in(str, filename);
    const auto root = pegtl::parse_tree::parse<config::grammar, config::store>( in );
    if (root != nullptr)
    {
        config::print_node(*root);
    }
    else
    {
        std::cout << "Configuration file is invalid." << std::endl;
    }
}


void parse_file2(std::string filename)
{
    // std::cout << filename << std::endl;
    // pegtl::string_input<> in("3 + 4", filename);
    pegtl::read_input<> in(filename);
    tao::pegtl::parse<config::grammar>( in );
}
