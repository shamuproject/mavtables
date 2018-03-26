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


#include <iostream>

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <utility>

#include "If.hpp"
#include "Filter.hpp"
#include "parse_tree.hpp"
#include "config_grammar.hpp"
#include "ConfigParser.hpp"
#include "Chain.hpp"
#include "Accept.hpp"
#include "Reject.hpp"
#include "Call.hpp"
#include "GoTo.hpp"
#include "IPAddress.hpp"
#include "UDPInterface.hpp"
#include "ConnectionFactory.hpp"
#include "ConnectionPool.hpp"
// #include "UDPSocket.hpp"
#include "UnixUDPSocket.hpp"
#include "SerialInterface.hpp"
#include "SerialPort.hpp"
#include "UnixSerialPort.hpp"
#include "App.hpp"


/** Construct map of non default chains.
 *
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


/** Construct conditional from AST.
 *
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
        if (child->name() == "packet_type")
        {
            condition.type(child->content());
        }
        // Parse source MAVLink address.
        else if (child->name() == "source")
        {
            condition.from(child->content());
        }
        // Parse destination MAVLink address.
        else if (child->name() == "dest")
        {
            condition.to(child->content());
        }
    }
    return condition;
}


/** Construct action from AST, priority, and condition.
 *
 *  \param root AST action node.
 *  \param priority The priority to use when constructing the action.  No
 *      priority if {}.  If the AST node is a reject action the priority will be
 *      ignored.
 *  \param condition The condition to use when constructing the action.
 *  \param chains Map of chain names to chains for call and goto actions.
 *  \returns The action (or rule) parsed from the given AST node, priority, and
 *      condition.
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
    // Only called if the AST is invalid.
    throw std::runtime_error("unknown action " + root.name());
}


/** Add rules from AST to a chain.
 *
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
            if (child->name() == "priority")
            {
                priority = std::stoi(child->content());
            }
            // Extract condition.
            else if (child->name() == "condition")
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


/** Parse filter from AST.
 *
 *  \param root Root of configuration AST.
 *  \returns The filter parsed from the AST.
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
            default_action = node->name() == "accept";
        }
    }

    // Construct the filter.
    return std::make_unique<Filter>(std::move(default_chain), default_action);
}


/** Parse a UPD interface from an AST.
 *
 *  \param root The UDP node to parse.
 *  \param filter The filter to use for the interface.
 *  \param pool The connection pool to add the interface's connection to.
 *  \returns The UDP interface parsed from the AST and using the given filter
 *      and connection pool.
 */
std::unique_ptr<UDPInterface> parse_udp(
    const config::parse_tree::node &root,
    std::shared_ptr<Filter> filter,
    std::shared_ptr<ConnectionPool> pool)
{
    unsigned int port = 14444;
    std::optional<IPAddress> address;

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
    }

    // Override port number with that provided with IP address.
    if (address.has_value() && address->port() != 0)
    {
        port = address->port();
    }

    // Construct the UDP interface.
    auto socket = std::make_unique<UnixUDPSocket>(port, address);
    auto factory = std::make_unique<ConnectionFactory<>>(filter, false);
    return std::make_unique<UDPInterface>(
        std::move(socket), pool, std::move(factory));
}


/** Parse a serial port interface from an AST.
 *
 *  \param root The serial port node to parse.
 *  \param filter The filter to use for the interface.
 *  \param pool The connection pool to add the interface's connection to.
 *  \returns The serial port interface parsed from the AST and using the given
 *      filter and connection pool.
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
            features = SerialPort::FLOW_CONTROL;
        }
    }

    // Throw error if no device was given.
    if (!device.has_value())
    {
        throw std::invalid_argument("AST does not contain a device string");
    }

    // Construct serial interface.
    auto port = std::make_unique<UnixSerialPort>(
        device.value(), baud_rate, features);
    auto connection = std::make_unique<Connection>(filter, false);
    return std::make_unique<SerialInterface>(
        std::move(port), pool, std::move(connection));
}


/** Parse UDP and serial port interfaces from AST root.
 *
 *  \param root The root of the AST to create interfaces from.
 *  \param filter The packet filter to use for the interfaces.
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


/** Construct a configuration parser from a file.
 *
 *  \param filename The path to the configuration file to parse.
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

    // identify chains
    // create default chain and named chains
    // add rules to default chain
    // add rules to each named chain
    // create ConnectionPool
    // create Filter with default chain and default filter action
    // create UDPSocket
    // create ConnectionFactory
    // create UDPInterface with socket, factory, and pool
    // create SerialPort
    // create a Connection for the serial port
    // create SerialInteface with port, pool, and connection
    // load interfaces into Interface threaders.
    // construct an App from the interface threaders
}


std::unique_ptr<App> ConfigParser::make_app()
{
    auto filter = parse_filter(*root_);
    auto interfaces = parse_interfaces(*root_, std::move(filter));
    return std::make_unique<App>(std::move(interfaces));
}



/** Print the configuration settings to the given output stream.
 *
 *  An example (that of examples/test.conf) is:
 *
 *  ```
 *  examples/test.conf:001:  default_action
 *  examples/test.conf:001:  |  accept
 *  examples/test.conf:004:  udp
 *  examples/test.conf:005:  |  port 14500
 *  examples/test.conf:006:  |  address 127.0.0.1
 *  examples/test.conf:010:  serial
 *  examples/test.conf:011:  |  device /dev/ttyUSB0
 *  examples/test.conf:012:  |  baudrate 115200
 *  examples/test.conf:013:  |  flow_control yes
 *  examples/test.conf:017:  serial
 *  examples/test.conf:018:  |  device COM1
 *  examples/test.conf:019:  |  baudrate 9600
 *  examples/test.conf:020:  |  flow_control no
 *  examples/test.conf:025:  chain default
 *  examples/test.conf:027:  |  call some_chain10
 *  examples/test.conf:027:  |  |  condition
 *  examples/test.conf:027:  |  |  |  source 127.1
 *  examples/test.conf:027:  |  |  |  dest 192.0
 *  examples/test.conf:028:  |  reject
 *  examples/test.conf:033:  chain some_chain10
 *  examples/test.conf:035:  |  accept
 *  examples/test.conf:035:  |  |  priority 99
 *  examples/test.conf:035:  |  |  condition
 *  examples/test.conf:035:  |  |  |  dest 192.0
 *  examples/test.conf:036:  |  accept
 *  examples/test.conf:036:  |  |  condition
 *  examples/test.conf:036:  |  |  |  packet_type PING
 *  examples/test.conf:037:  |  accept
 *  ```
 *
 *  \relates MAVAddress
 *  \param os The output stream to print to.
 *  \param config_parser The MAVLink address to print.
 *  \return The output stream.
 */
std::ostream &operator<<(std::ostream &os, const ConfigParser &config_parser)
{
    os << *config_parser.root_;
    return os;
}
