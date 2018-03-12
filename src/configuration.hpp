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


#ifndef CONFIGURATION_HPP_
#define CONFIGURATION_HPP_


#include <iostream>

#include <memory>
#include <optional>
#include <string>
#include <type_traits>

#include <pegtl.hpp>
#include <pegtl/contrib/parse_tree.hpp>


/// @cond INTERNAL
namespace config
{

    using namespace tao::pegtl;
    using namespace tao::pegtl::ascii;

    // Remove content mixin for store object's.
    struct no_content : std::true_type
    {
        static void transform(
            std::unique_ptr<tao::pegtl::parse_tree::node> &n)
        {
            n->remove_content();
        }
    };

    // Do not store AST node content by default
    template<typename> struct store : std::false_type {};

    // Comments and ignored components.
    struct comment : seq<one<'#'>, star<not_at<eol>, not_at<eof>, any>> {};
    struct ignored : sor<space, comment> {};

    // Pad a rule.
    template <typename N>
    struct p : pad<N, ignored> {};

    // End of statement terminator.
    struct eos : one<';'> {};

    // Generic statement with optional value.
    template <typename K, typename V = success>
    struct t_statement
        : seq<p<K>, must<p<V>>, must<eos>> {};

    // Generic block.
    template <typename K, typename... Statements>
    struct t_block : seq<p<K>, p<one<'{'>>,
        star<p<sor<Statements...>>>, p<one<'}'>>> {};

    // Generic named block.
    template <typename K, typename N, typename... Statements>
        struct t_named_block : seq<p<K>, p<N>, p<one<'{'>>,
        star<p<sor<Statements...>>>, p<one<'}'>>> {};

    // Yes/No (boolean) value.
    struct yesno
        : sor<TAO_PEGTL_ISTRING("yes"), TAO_PEGTL_ISTRING("no")> {};

    // Unsigned integer value.
    struct integer : plus<digit> {};

    // Signed integer value.
    struct signed_integer : seq<opt<one<'+', '-'>>, integer> {};

    // IP address.
    struct ip : must<seq<integer, rep<3, seq<one<'.'>, integer>>>> {};
    template<> struct store<ip> : std::true_type {};

    // Port number.
    struct port : must<integer> {};
    template<> struct store<port> : std::true_type {};

    // Serial port device name.
    struct device : must<plus<sor<alnum, one<'_', '/'>>>> {};
    template<> struct store<device> : std::true_type {};

    // Serial port baud rate.
    struct baudrate : must<integer> {};
    template<> struct store<baudrate> : std::true_type {};

    // Serial port flow control.
    struct flow_control : must<yesno> {};
    template<> struct store<flow_control> : std::true_type {};

    // Identifier name (keep node and content).
    struct name : identifier {};
    template<> struct store<name> : std::true_type {};

    // Rule actions.
    struct accept : TAO_PEGTL_STRING("accept") {};
    template<> struct store<accept> : no_content {};
    struct reject : TAO_PEGTL_STRING("reject") {};
    template<> struct store<reject> : no_content {};
    struct call : seq<TAO_PEGTL_STRING("call"), p<name>> {};
    template<> struct store<call> : no_content {};
    struct goto_ : seq<TAO_PEGTL_STRING("goto"), p<name>> {};
    template<> struct store<goto_> : no_content {};
    struct action : sor<accept, reject, call, goto_> {};

    // MAVLink addresses and masks.
    struct mavaddr : seq<integer, one<'.'>, integer> {};
    struct full_mask : seq<one<':'>, mavaddr> {};
    struct forward_mask : seq<one<'/'>, integer> {};
    struct backward_mask : seq<one<'\\'>, integer> {};
    struct mavmask
        : seq<mavaddr, opt<sor<full_mask, forward_mask, backward_mask>>> {};

    // Conditional.
    struct packet_type : plus<sor<upper, one<'_'>>> {};
    template<> struct store<packet_type> : std::true_type {};
    struct source : mavmask {};
    template<> struct store<source> : std::true_type {};
    struct source_command
        : seq<TAO_PEGTL_STRING("from"), must<p<source>>> {};
    struct dest : mavmask {};
    template<> struct store<dest> : std::true_type {};
    struct dest_command
        : seq<TAO_PEGTL_STRING("to"), must<p<dest>>> {};
    struct conditional
        : seq<TAO_PEGTL_STRING("if"), opt<p<packet_type>>,
        opt<p<source_command>>, opt<p<dest_command>>> {};
    template<> struct store<conditional> : std::true_type {};

    // Priority.
    struct priority : signed_integer {};
    template<> struct store<priority> : std::true_type {};
    struct priority_command
        : seq<TAO_PEGTL_STRING("with"), p<TAO_PEGTL_STRING("priority")>,
          p<priority>> {};

    // Catch unsuported statements.
    // TODO: Consider changing failure to raise.
    struct s_catch : if_must<t_statement<identifier, star<alnum>>, failure> {};
    template<> struct store<s_catch> : std::true_type {};

    // Filter chain.
    struct rule
        : seq<p<action>, opt<p<priority_command>>, opt<p<conditional>>, eos> {};
    template<> struct store<rule> : no_content {};
    struct rules : plus<p<rule>> {};
    template<> struct store<rules> : no_content {};
    struct chain : t_named_block<TAO_PEGTL_STRING("chain"), name, rules> {};
    template<> struct store<chain> : no_content {};

    // Default filter action (keep node, no content).
    struct default_action
        : t_statement<TAO_PEGTL_STRING("default_action"),
          sor<accept, reject>> {};
    template<> struct store<default_action> : no_content {};

    // UDP connection block.
    struct s_port : t_statement<TAO_PEGTL_STRING("port"), port> {};
    struct s_address : t_statement<TAO_PEGTL_STRING("address"), ip> {};
    struct udp
        : t_block<TAO_PEGTL_STRING("udp"), s_port, s_address, s_catch> {};
    template<> struct store<udp> : no_content {};

    // Serial port block.
    struct s_device : t_statement<TAO_PEGTL_STRING("device"), device> {};
    struct s_baudrate : t_statement<TAO_PEGTL_STRING("baudrate"), baudrate> {};
    struct s_flow_control
        : t_statement<TAO_PEGTL_STRING("flow_control"), flow_control> {};
    struct serial
        : t_block<TAO_PEGTL_STRING("serial"),
        s_device, s_baudrate, s_flow_control, s_catch> {};
    template<> struct store<serial> : no_content {};

    // Combine grammar.
    struct block : sor<udp, serial, chain> {};
    struct statement : sor<default_action, s_catch> {};
    struct element : sor<comment, block, statement> {};
    struct grammar : seq<star<pad<element, ignored>>, eof> {};

}
/// @endcond


void parse_file(std::string filename);

void print_node(
    const tao::pegtl::parse_tree::node &n, const std::string &s = "");


#endif  // CONFIGURATION_HPP_
