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


#include <map>
#include <memory>
#include <ostream>
#include <string>

#include <pegtl.hpp>
#include "parse_tree.hpp"
#include "Chain.hpp"
#include "Filter.hpp"
#include "UDPInterface.hpp"
#include "SerialInterface.hpp"
#include "App.hpp"


void parse_chain(
    Chain &chain,
    const config::parse_tree::node &root,
    const std::map<std::string, std::shared_ptr<Chain>> &chains);



std::unique_ptr<UDPInterface> parse_udp(
    const config::parse_tree::node &root,
    std::shared_ptr<Filter> filter,
    std::shared_ptr<ConnectionPool> pool);

std::unique_ptr<Rule> parse_action(
    const config::parse_tree::node &root,
    std::optional<int> priority,
    std::optional<If> condition,
    const std::map<std::string, std::shared_ptr<Chain>> &chains);


std::vector<std::unique_ptr<Interface>> parse_interfaces(
        const config::parse_tree::node &root, std::unique_ptr<Filter> filter);

If parse_condition(const config::parse_tree::node &root);

int parse_priority(const config::parse_tree::node &root);

std::map<std::string, std::shared_ptr<Chain>> init_chains(
        const config::parse_tree::node &root);

std::unique_ptr<Filter> parse_filter(const config::parse_tree::node &root);


std::unique_ptr<SerialInterface> parse_serial(
    const config::parse_tree::node &root,
    std::shared_ptr<Filter> filter,
    std::shared_ptr<ConnectionPool> pool);


class ConfigParser
{
    public:
        ConfigParser(std::string filename);
        ConfigParser(const ConfigParser &other) = delete;
        ConfigParser(ConfigParser &&other) = delete;
        std::unique_ptr<App> make_app();
        ConfigParser &operator=(const ConfigParser &other) = delete;
        ConfigParser &operator=(ConfigParser &&other) = delete;

        friend std::ostream &operator<<(
                std::ostream &os, const ConfigParser &config_parser);

    protected:
        tao::pegtl::read_input<> in_;
        std::unique_ptr<config::parse_tree::node> root_;
};


std::ostream &operator<<(std::ostream &os, const ConfigParser &config_parser);


#endif // CONFIGPARSER_HPP_
