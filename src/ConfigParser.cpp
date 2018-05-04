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


#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Accept.hpp"
#include "App.hpp"
#include "Call.hpp"
#include "Chain.hpp"
#include "ConfigParser.hpp"
#include "config_grammar.hpp"
#include "ConnectionFactory.hpp"
#include "ConnectionPool.hpp"
#include "Filter.hpp"
#include "GoTo.hpp"
#include "If.hpp"
#include "IPAddress.hpp"
#include "parse_tree.hpp"
#include "Reject.hpp"
#include "SerialInterface.hpp"
#include "SerialPort.hpp"
#include "UDPInterface.hpp"
#include "UnixSerialPort.hpp"
#include "UnixUDPSocket.hpp"
#include "utility.hpp"


/** Construct a map of non default chains.
 *
 *  \relates ConfigParser
 *  \param root Root of configuration AST.
 *  \returns Map of chain names to chains.
 */
std::map<std::string, std::shared_ptr<Chain>> init_chains(
            const config::parse_tree::node &root)
{
    std::map<std::string, std::shared_ptr<Chain>> chains;

    // Loop through top AST nodes creating chains for.
    for (auto &node : root.children)
    {
        if (node->name() == "config::chain")
        {
            if (node->has_content() && node->content() != "default")
            {
                chains[node->content()]
                    = std::make_shared<Chain>(node->content());
            }
        }
    }

    return chains;
}


/** Construct a \ref Rule with action from AST, priority, and condition.
 *
 *  \relates ConfigParser
 *  \param root AST action node.
 *  \param priority The priority to use when constructing the action.  No
 *      priority if {}.  If the AST node is a reject action the priority will be
 *      ignored.
 *  \param condition The condition to use when constructing the action.
 *  \param chains Map of chain names to chains for call and goto actions.
 *  \returns The action (or rule) parsed from the given AST node, priority, and
 *      condition.
 *  \throws std::invalid_argument if the action attempts to `call` or `goto` the
 *      default chain.
 *  \throws std::runtime_error if the action is not one of `accept`, `reject`,
 *      `call`, or `goto`.
 */
std::unique_ptr<Rule> parse_action(
    const config::parse_tree::node &root,
    std::optional<int> priority,
    std::optional<If> condition,
    const std::map<std::string, std::shared_ptr<Chain>> &chains)
{
    // Parse accept rule.
    if (root.name() == "config::accept")
    {
        if (priority)
        {
            return std::make_unique<Accept>(
                       priority.value(), std::move(condition));
        }
        else
        {
            return std::make_unique<Accept>(std::move(condition));
        }
    }
    // Parse reject rule.
    else if (root.name() == "config::reject")
    {
        return std::make_unique<Reject>(std::move(condition));
    }
    // Parse call rule.
    else if (root.name() == "config::call")
    {
        if (root.content() == "default")
        {
            throw std::invalid_argument("cannot 'call' the default chain");
        }

        if (priority)
        {
            return std::make_unique<Call>(
                       chains.at(root.content()),
                       priority.value(),
                       std::move(condition));
        }
        else
        {
            return std::make_unique<Call>(
                       chains.at(root.content()),
                       std::move(condition));
        }
    }
    // Parse call goto.
    else if (root.name() == "config::goto_")
    {
        if (root.content() == "default")
        {
            throw std::invalid_argument("cannot 'goto' the default chain");
        }

        if (priority)
        {
            return std::make_unique<GoTo>(
                       chains.at(root.content()),
                       priority.value(),
                       std::move(condition));
        }
        else
        {
            return std::make_unique<GoTo>(
                       chains.at(root.content()),
                       std::move(condition));
        }
    }

    // Only called if the AST is invalid, this can't be called as long as
    // config_grammar.hpp does not contain any bugs.
    // LCOV_EXCL_START
    throw std::runtime_error("unknown action " + root.name());
    // LCOV_EXCL_STOP
}


/** Add \ref Rule's from AST to a \ref Chain.
 *
 *  \relates ConfigParser
 *  \param chain Chain to add rules to.
 *  \param root AST chain node containing rules.
 *  \param chains Map of chain names to chains for call and goto actions.
 */
void parse_chain(
    Chain &chain,
    const config::parse_tree::node &root,
    const std::map<std::string, std::shared_ptr<Chain>> &chains)
{
    // Loop through each children.
    for (auto &node : root.children)
    {
        std::optional<int> priority;
        std::optional<If> condition;

        // Loop through rule options.
        for (auto &child : node->children)
        {
            // Extract priority.
            if (child->name() == "config::priority")
            {
                priority = std::stoi(child->content());
            }
            // Extract condition.
            else if (child->name() == "config::condition")
            {
                condition = parse_condition(*child);
            }
        }

        // Create and add the new rule.
        chain.append(
            parse_action(
                *node, std::move(priority), std::move(condition), chains));
    }
}


/** Construct conditional (\ref If) from AST.
 *
 *  \relates ConfigParser
 *  \param root AST conditional node.
 *  \returns The conditional constructed from the given AST node.
 */
If parse_condition(const config::parse_tree::node &root)
{
    If condition;

    // Parse condition options.
    for (auto &child : root.children)
    {
        // Parse packet type.
        if (child->name() == "config::packet_type")
        {
            condition.type(child->content());
        }
        // Parse source MAVLink address.
        else if (child->name() == "config::source")
        {
            condition.from(child->content());
        }
        // Parse destination MAVLink address.
        else if (child->name() == "config::dest")
        {
            condition.to(child->content());
        }
    }

    return condition;
}


/** Parse \ref Filter from AST.
 *
 *  \relates ConfigParser
 *  \param root Root of configuration AST.
 *  \returns The \ref Filter parsed from the AST.
 */
std::unique_ptr<Filter> parse_filter(const config::parse_tree::node &root)
{
    Chain default_chain("default");
    bool default_action = false;
    std::map<std::string, std::shared_ptr<Chain>> chains = init_chains(root);

    // Look through top nodes.
    for (auto &node : root.children)
    {
        // Parse chain.
        if (node->name() == "config::chain")
        {
            // Parse default chain.
            if (node->content() == "default")
            {
                parse_chain(default_chain, *node, chains);
            }
            // Parse named chain.
            else
            {
                parse_chain(*chains.at(node->content()), *node, chains);
            }
        }
        // Parse default filter action.
        else if (node->name() == "config::default_action")
        {
            default_action = node->children[0]->name() == "config::accept";
        }
    }

    // Construct the filter.
    return std::make_unique<Filter>(std::move(default_chain), default_action);
}


/** Parse UDP and serial port interfaces from AST root.
 *
 *  \relates ConfigParser
 *  \param root The root of the AST to create \ref Interface's from.
 *  \param filter The packet \ref Filter to use for the interfaces.
 *  \returns A vector of UDP and serial port interfaces.
 */
std::vector<std::unique_ptr<Interface>> parse_interfaces(
        const config::parse_tree::node &root, std::unique_ptr<Filter> filter)
{
    std::shared_ptr<Filter> shared_filter = std::move(filter);
    std::vector<std::unique_ptr<Interface>> interfaces;
    auto connection_pool = std::make_shared<ConnectionPool>();

    // Loop over each node of the root AST node.
    for (auto &node : root.children)
    {
        // Parse UDP interface.
        if (node->name() == "config::udp")
        {
            interfaces.push_back(
                parse_udp(*node, shared_filter, connection_pool));
        }
        // Parse serial port interface.
        else if (node->name() == "config::serial")
        {
            interfaces.push_back(
                parse_serial(*node, shared_filter, connection_pool));
        }
    }

    return interfaces;
}


/** Parse a serial port interface from an AST.
 *
 *  \relates ConfigParser
 *  \param root The serial port node to parse.
 *  \param filter The \ref Filter to use for the \ref SerialInterface.
 *  \param pool The connection pool to add the interface's connection to.
 *  \returns The serial port interface parsed from the AST and using the given
 *      filter and connection pool.
 *  \throws std::invalid_argument if the device string is missing.
 */
std::unique_ptr<SerialInterface> parse_serial(
    const config::parse_tree::node &root,
    std::shared_ptr<Filter> filter,
    std::shared_ptr<ConnectionPool> pool)
{
    // Default settings.
    std::optional<std::string> device;
    unsigned long baud_rate = 9600;
    SerialPort::Feature features = SerialPort::DEFAULT;
    std::vector<MAVAddress> preload;

    // Extract settings from AST.
    for (auto &node : root.children)
    {
        // Extract device string.
        if (node->name() == "config::device")
        {
            device = node->content();
        }
        // Extract device baud rate.
        else if (node->name() == "config::baudrate")
        {
            baud_rate = static_cast<unsigned long>(std::stol(node->content()));
        }
        // Extract flow control.
        else if (node->name() == "config::flow_control")
        {
            if (to_lower(node->content()) == "yes")
            {
                features = SerialPort::FLOW_CONTROL;
            }
        }
        // Extract preloaded address.
        else if (node->name() == "config::preload")
        {
            preload.push_back(MAVAddress(node->content()));
        }
    }

    // Throw error if no device was given.
    if (!device.has_value())
    {
        throw std::invalid_argument("missing device string");
    }

    // Construct serial interface.
    auto port = std::make_unique<UnixSerialPort>(
                    device.value(), baud_rate, features);
    auto connection = std::make_unique<Connection>(
                          device.value(), filter, false);

    for (const auto &addr : preload)
    {
        connection->add_address(addr);
    }

    return std::make_unique<SerialInterface>(
               std::move(port), pool, std::move(connection));
}


/** Parse a UPD interface from an AST.
 *
 *  \relates ConfigParser
 *  \param root The UDP node to parse.
 *  \param filter The \ref Filter to use for the \ref UDPInterface.
 *  \param pool The connection pool to add the interface's connections to.
 *  \returns The UDP interface parsed from the AST and using the given filter
 *      and connection pool.
 */
std::unique_ptr<UDPInterface> parse_udp(
    const config::parse_tree::node &root,
    std::shared_ptr<Filter> filter,
    std::shared_ptr<ConnectionPool> pool)
{
    unsigned int port = 14500;
    std::optional<IPAddress> address;
    unsigned long max_bitrate = 0;

    // Loop over options for UDP interface.
    for (auto &node : root.children)
    {
        // Parse port number.
        if (node->name() == "config::port")
        {
            port = static_cast<unsigned int>(std::stol(node->content()));
        }
        // Parse IP address and optionally port number.
        else if (node->name() == "config::address")
        {
            address = IPAddress(node->content());
        }
        // Parse bitrate limit.
        else if (node->name() == "config::max_bitrate")
        {
            max_bitrate = static_cast<unsigned long>(
                              std::stoll(node->content()));
        }
    }

    // Construct the UDP interface.
    auto socket = std::make_unique<UnixUDPSocket>(port, address, max_bitrate);
    auto factory = std::make_unique<ConnectionFactory<>>(filter, false);
    return std::make_unique<UDPInterface>(
               std::move(socket), pool, std::move(factory));
}


/** Construct a configuration parser from a file.
 *
 *  \param filename The path of the configuration file to parse.
 *  \throws std::runtime_error if the configuration file cannot be parsed.
 */
ConfigParser::ConfigParser(std::string filename)
    : in_(filename)
{
    root_ = config::parse(in_);

    if (root_ == nullptr)
    {
        // It is technically impossible parsing errors should be raised as a
        // parse_error.
        // LCOV_EXCL_START
        throw std::runtime_error(
            "Unexpected error while parsing configuration file.");
        // LCOV_EXCL_STOP
    }
}


/** Build a mavtables application from the AST contained by the parser.
 *
 *  \returns A mavtables application.
 */
std::unique_ptr<App> ConfigParser::make_app()
{
    auto filter = parse_filter(*root_);
    auto interfaces = parse_interfaces(*root_, std::move(filter));
    return std::make_unique<App>(std::move(interfaces));
}


/** Print the configuration settings to the given output stream.
 *
 *  An example (that of test/mavtables.conf) is:
 *
 *  ```
 *  ===== test/mavtables.conf =====
 *  :001:  default_action
 *  :001:  |  accept
 *  :004:  udp
 *  :005:  |  port 14500
 *  :006:  |  address 127.0.0.1
 *  :007:  |  max_bitrate 8388608
 *  :011:  serial
 *  :012:  |  device ./ttyS0
 *  :013:  |  baudrate 115200
 *  :014:  |  flow_control yes
 *  :015:  |  preload 1.1
 *  :016:  |  preload 62.34
 *  :020:  chain default
 *  :022:  |  call some_chain10
 *  :022:  |  |  condition
 *  :022:  |  |  |  source 127.1
 *  :022:  |  |  |  dest 192.0
 *  :023:  |  reject
 *  :027:  chain some_chain10
 *  :029:  |  accept
 *  :029:  |  |  priority 99
 *  :029:  |  |  condition
 *  :029:  |  |  |  dest 192.0
 *  :030:  |  accept
 *  :030:  |  |  condition
 *  :030:  |  |  |  packet_type PING
 *  :031:  |  accept
 *  ```
 *
 *  \relates ConfigParser
 *  \param os The output stream to print to.
 *  \param config_parser The configuration parser to print.
 *  \returns The output stream.
 */
std::ostream &operator<<(std::ostream &os, const ConfigParser &config_parser)
{
    os << *config_parser.root_;
    return os;
}
