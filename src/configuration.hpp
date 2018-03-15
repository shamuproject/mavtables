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
#include <parse_tree.hpp>


/// @cond INTERNAL
namespace config
{

    using namespace tao::pegtl;
    using namespace tao::pegtl::ascii;


    // Error message mixin.
    template<typename>
    struct error
    {
        static const std::string error_message;
    };

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wglobal-constructors"
    #pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

    // Default error message.
    template<typename T>
    const std::string error<T>::error_message =
       "parse error matching " + internal::demangle<T>();

#ifdef __clang__
    #pragma clang diagnostic pop
#endif


    // Store node mixin.
    template<typename T>
    struct yes : std::true_type, error<T> {};

    // Do not store node mixin.
    template<typename T>
    struct no : std::false_type, error<T> {};

    // Remove content mixin for store object's.
    template<typename T>
    struct yes_without_content : std::true_type, error<T>
    {
        static void transform(
            std::unique_ptr<config::parse_tree::node> &n)
        {
            n->remove_content();
        }
    };

    // Do not store AST node content by default
    template<typename T> struct store : no<T> {};


    // Comments and ignored components.
    struct comment : seq<one<'#'>, star<not_at<eol>, not_at<eof>, any>> {};
    struct ignored : sor<space, comment> {};

    // Pad a rule.
    template <typename N>
    struct p : pad<N, ignored> {};

    // End of statement terminator.
    struct eos : one<';'> {};

    // Generic statement with 0 or 1 values.
    template <typename K>
    struct a0_statement : seq<K, p<must<eos>>> {};
    template <typename K, typename V = success>
    struct a1_statement : seq<K, p<must<V>>, p<must<eos>>> {};

    // Generic block.
    struct opening_bracket : one<'{'> {};
    struct closing_bracket : one<'}'> {};
    template <typename K, typename... Statements>
    struct t_block : seq<K,
        p<must<opening_bracket>>,
        star<p<sor<Statements...>>>,
        p<must<closing_bracket>>> {};

    // Generic named block.
    template <typename K, typename N, typename... Statements>
    struct t_named_block
        : seq<K, p<must<N>>, p<must<opening_bracket>>,
        star<p<sor<Statements...>>>, p<must<closing_bracket>>> {};

    // Yes/No (boolean) value.
    struct yesno
    : sor<TAO_PEGTL_ISTRING("yes"), TAO_PEGTL_ISTRING("no")> {};

    // Unsigned integer value.
    struct integer : plus<digit> {};

    // Signed integer value.
    struct signed_integer : seq<opt<one<'+', '-'>>, integer> {};

    // IP address.
    struct ip : seq<integer, rep<3, seq<one<'.'>, integer>>> {};
    template<> struct store<ip> : yes<ip> {};

    // Port number.
    struct port : integer {};
    template<> struct store<port> : yes<port> {};

    // Serial port device name.
    struct device : plus<sor<alnum, one<'_', '/'>>> {};
    template<> struct store<device> : yes<device> {};

    // Serial port baud rate.
    struct baudrate : integer {};
    template<> struct store<baudrate> : yes<baudrate> {};

    // Serial port flow control.
    struct flow_control : yesno {};
    template<> struct store<flow_control> : yes<flow_control> {};

    // Chain name.
    struct chain_name : identifier {};
    template<> struct store<chain_name> : yes<chain_name> {};

    // Rule actions.
    struct accept : TAO_PEGTL_STRING("accept") {};
    template<> struct store<accept> : yes_without_content<accept> {};
    struct reject : TAO_PEGTL_STRING("reject") {};
    template<> struct store<reject> : yes_without_content<reject> {};
    struct call : seq<TAO_PEGTL_STRING("call"), p<must<chain_name>>> {};
    template<> struct store<call> : yes_without_content<call> {};
    struct goto_ : seq<TAO_PEGTL_STRING("goto"), p<must<chain_name>>> {};
    template<> struct store<goto_> : yes_without_content<goto_> {};
    struct action : sor<accept, reject, call, goto_> {};

    // MAVLink addresses and masks.
    struct mavaddr : seq<integer, one<'.'>, integer> {};
    struct full_mask : if_must<one<':'>, mavaddr> {};
    struct forward_mask : if_must<one<'/'>, integer> {};
    struct backward_mask : if_must<one<'\\'>, integer> {};
    struct mavmask
        : seq<mavaddr, opt<sor<full_mask, forward_mask, backward_mask>>> {};

    // Conditional.
    struct packet_type : plus<sor<upper, one<'_'>>> {};
    template<> struct store<packet_type> : yes<packet_type> {};
    struct source : mavmask {};
    template<> struct store<source> : yes<source> {};
    struct source_command
    : seq<TAO_PEGTL_STRING("from"), p<must<source>>> {};
    struct dest : mavmask {};
    template<> struct store<dest> : yes<dest> {};
    struct dest_command
    : seq<TAO_PEGTL_STRING("to"), p<must<dest>>> {};
    struct start_with_packet_type
        : seq<packet_type, opt<p<source_command>>, opt<p<dest_command>>> {};
    struct start_with_source : seq<source_command, opt<p<dest_command>>> {};
    struct start_with_dest : dest_command {};
    struct condition
        : p<sor<start_with_packet_type, start_with_source, start_with_dest>> {};
    struct conditional : if_must<TAO_PEGTL_STRING("if"), condition> {};
    template<> struct store<conditional> : yes_without_content<conditional> {};

    // Priority.
    struct priority : signed_integer {};
    template<> struct store<priority> : yes<priority> {};
    struct priority_keyword : TAO_PEGTL_STRING("priority") {};
    struct priority_command
    : seq<TAO_PEGTL_STRING("with"), p<must<priority_keyword>>,
      p<must<priority>>> {};

    // Catch unsuported statements.
    struct unsupported_statement : failure {};
    struct sa0_catch
        : if_must<a0_statement<identifier>, unsupported_statement> {};
    struct sa1_catch
        : if_must<a1_statement<identifier,
        star<not_one<';'>>>, unsupported_statement> {};
    struct s_catch : sor<sa0_catch, sa1_catch> {};

    // Catch unsuported rules.
    struct invalid_rule : failure {};
    struct sa0_rule_catch : if_must<a0_statement<identifier>, invalid_rule> {};
    struct sa1_rule_catch
        : if_must<a1_statement<identifier,
        star<not_one<';'>>>, invalid_rule> {};
    struct rule_catch : sor<sa0_rule_catch, sa1_rule_catch> {};

    // Filter chain.
    struct rule
        : if_must<action, opt<p<priority_command>>,
          opt<p<conditional>>, eos> {};
    template<> struct store<rule> : yes_without_content<rule> {};
    struct rules : plus<p<sor<rule, rule_catch>>> {};
    template<> struct store<rules> : yes_without_content<rules> {};
    struct chain
        : t_named_block<TAO_PEGTL_STRING("chain"), chain_name, rules> {};
    template<> struct store<chain> : yes_without_content<chain> {};

    // Default filter action (keep node, no content).
    struct default_action_option : sor<accept, reject> {};
    struct default_action
    : a1_statement<TAO_PEGTL_STRING("default_action"), default_action_option> {};
    template<> struct store<default_action>
        : yes_without_content<default_action> {};

    // UDP connection block.
    struct s_port : a1_statement<TAO_PEGTL_STRING("port"), port> {};
    struct s_address : a1_statement<TAO_PEGTL_STRING("address"), ip> {};
    struct udp
    : t_block<TAO_PEGTL_STRING("udp"), s_port, s_address, s_catch> {};
    template<> struct store<udp> : yes_without_content<udp> {};

    // Serial port block.
    struct s_device : a1_statement<TAO_PEGTL_STRING("device"), device> {};
    struct s_baudrate : a1_statement<TAO_PEGTL_STRING("baudrate"), baudrate> {};
    struct s_flow_control
    : a1_statement<TAO_PEGTL_STRING("flow_control"), flow_control> {};
    struct serial
    : t_block<TAO_PEGTL_STRING("serial"),
      s_device, s_baudrate, s_flow_control, s_catch> {};
    template<> struct store<serial> : yes_without_content<serial> {};

    // Combine grammar.
    struct block : sor<udp, serial, chain> {};
    struct statement : sor<default_action, s_catch> {};
    struct element : sor<comment, block, statement> {};
    struct grammar : seq<star<pad<element, ignored>>, eof> {};

}
/// @endcond


void parse_file(std::string filename);

void print_node(
    const config::parse_tree::node &n, const std::string &s = "");


#endif  // CONFIGURATION_HPP_
