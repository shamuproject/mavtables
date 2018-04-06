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
#include <iostream>
#include <iomanip>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "Filesystem.hpp"
#include "config_grammar.hpp"
#include "parse_tree.hpp"


/** \defgroup config Configuration functions.
 *
 *  These functions are used to parse the configuration file.
 */


// custom error messages
namespace config
{

/// @cond INTERNAL

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wglobal-constructors"
    #pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

    template<>
    const std::string error<eos>::error_message =
           "expected end of statement ';' character";

    template<>
    const std::string error<opening_brace>::error_message =
           "expected opening brace '{'";

    template<>
    const std::string error<closing_brace>::error_message =
           "expected closing brace '}'";

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
    const std::string error<address>::error_message =
           "expected a valid IP address";

    template<>
    const std::string error<device>::error_message =
           "expected a valid serial port device name";

    template<>
    const std::string error<baudrate>::error_message =
           "expected a valid baud rate";

    template<>
    const std::string error<flow_control>::error_message =
           "expected 'yes' or 'no'";

    template<>
    const std::string error<chain_name>::error_message =
           "expected a valid chain name";

    template<>
    const std::string error<chain>::error_message =
           "expected a valid chain name";

    template<>
    const std::string error<call>::error_message =
           "expected a valid chain name";

    template<>
    const std::string error<goto_>::error_message =
           "expected a valid chain name";

    template<>
    const std::string error<invalid_rule>::error_message =
           "expected a valid rule";

    template<>
    const std::string error<condition_value>::error_message =
           "condition is empty or invalid";

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

    template<>
    const std::string error<elements>::error_message =
           "expected at least one valid statement or block";

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

/// @endcond


    /** Print an AST node and all its children.
     *
     *  \ingroup config
     *  \param os The output stream to print to.
     *  \param node The node to print, also prints it's children.
     *  \param print_location Set to true to print file and line numbers of each
     *      AST node.
     *  \param prefix A string to prefix to each AST element.  This is reserved
     *      for internal use.
     *  \returns The output stream.
     */
    std::ostream &print_node(
        std::ostream &os,
        const config::parse_tree::node &node,
        bool print_location, const std::string &prefix)
    {
        auto new_prefix = prefix;

        // If not the root node.
        if (!node.is_root())
        {
            // Add 2 spaces to the indent.
            new_prefix = prefix + "|  ";

            // Remove "config::" prefix from node name.
            auto node_name = node.name();
            node_name.erase(0, 8);
            const auto begin = node_name.find_first_not_of("_");
            const auto end = node_name.find_last_not_of("_");
            node_name = node_name.substr(begin, end - begin + 1);

            // Print location.
            if (print_location)
            {
                os << ":" << std::setfill('0') << std::setw(3)
                   << node.begin().line << ":  ";
            }

            // Print name.
            os << prefix << node_name;

            // Print node content.
            if (node.has_content())
            {
                os << " " << node.content() << "";
            }
            os << std::endl;
        }

        // Print all child nodes
        if (!node.children.empty())
        {
            for (auto &up : node.children)
            {
                print_node(os, *up, print_location, new_prefix);
            }
        }
        return os;
    }

}


/** Print AST node to the given output stream.
 *
 *  This is the same as calling \ref config::print_node with the location option
 *  set to true.
 *
 *  \ingroup config
 *  \param os The output stream to print to.
 *  \param node The node to print, also prints it's children.
 *  \return The output stream.
 */
std::ostream &operator<<(
    std::ostream &os, const config::parse_tree::node &node)
{
    return config::print_node(os, node, true);
}
