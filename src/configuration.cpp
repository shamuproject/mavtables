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
#include "parse_tree.hpp"


// custom error messages
namespace config
{

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wglobal-constructors"
    #pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

    template<>
    const std::string error<eos>::error_message =
           "expected end of statement ';' character";

    template<>
    const std::string error<opening_bracket>::error_message =
           "expected opening bracket '{'";

    template<>
    const std::string error<closing_bracket>::error_message =
           "expected closing bracket '}'";

    template<>
    const std::string error<unsupported_statement>::error_message =
           "unsupported statement";

    template<>
    const std::string error<default_action_option>::error_message =
           "expected 'accept' or 'reject'";

    template<>
    const std::string error<port>::error_message =
           "expected a valid port number";

    template<>
    const std::string error<ip>::error_message =
           "expected a valid IP address";

    template<>
    const std::string error<device>::error_message =
           "expected a valid serial port device name";

    template<>
    const std::string error<baudrate>::error_message =
           "expected a valid baudrate";

    template<>
    const std::string error<flow_control>::error_message =
           "expected 'yes' or 'no'";

    template<>
    const std::string error<chain_name>::error_message =
           "expected a valid chain name";

    template<>
    const std::string error<invalid_rule>::error_message =
           "expected a valid rule";

    template<>
    const std::string error<condition>::error_message =
           "conditional is empty";

    template<>
    const std::string error<dest>::error_message =
           "expected a valid MAVLink subnet";

    template<>
    const std::string error<source>::error_message =
           "expected a valid MAVLink subnet";

    template<>
    const std::string error<mavaddr>::error_message =
           "expected a valid MAVLink address";

    template<>
    const std::string error<integer>::error_message =
           "expected an integer";

    template<>
    const std::string error<priority>::error_message =
           "expected priority level";

    template<>
    const std::string error<priority_keyword>::error_message =
           "'with' keyword must be followed by the 'priority' keyword";

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

    void print_node(
        const config::parse_tree::node &node,
        bool print_location, const std::string &prefix)
    {
        // detect the root node
        if (node.is_root())
        {
            std::cout << "ROOT" << std::endl;
        }
        else
        {
            auto name = node.name();
            name.erase(0, 8);
            const auto begin = name.find_first_not_of("_");
            const auto end = name.find_last_not_of("_");
            name = name.substr(begin, end - begin + 1);
            std::cout << prefix << name;
            if (node.has_content())
            {
                std::cout << " \"" << node.content() << "\"";
            }
            if (print_location)
            {
                std::cout << " at " << node.begin();
                if (node.has_content())
                {
                    std::cout << " to " << node.end();
                }
            }
            std::cout << std::endl;
        }

        // print all child nodes
        if (!node.children.empty())
        {
            const auto prefix2 = prefix + "    ";

            for (auto &up : node.children)
            {
                print_node(*up, print_location, prefix2);
            }
        }
    }

}


void parse_file(std::string filename)
{
    tao::pegtl::read_input<> in(filename);
    const auto root =
        config::parse_tree::parse<config::grammar, config::store>(in);

    if (root != nullptr)
    {
        config::print_node(*root);
    }
    else
    {
        std::cout << "Configuration file is invalid." << std::endl;
    }
}
