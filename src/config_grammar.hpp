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


#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <pegtl.hpp>
#include <parse_tree.hpp>


namespace config
{

    /// @cond INTERNAL

    using namespace tao::pegtl;
    using namespace tao::pegtl::ascii;


    // Error message mixin.
    template<typename>
    struct error
    {
        static const std::string error_message;
    };


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
        static void transform(std::unique_ptr<config::parse_tree::node> &node)
        {
            node->remove_content();
        }
    };

    // Replace with first child storage mixin.
    template <typename T>
    struct replace_with_first_child : std::true_type, error<T>
    {
        static void transform(std::unique_ptr<config::parse_tree::node> &node)
        {
            auto children = std::move(node->children);
            node = std::move(children.front());
            children.erase(children.begin());
            if (node->children.empty())
            {
                node->children = std::move(children);
            }
            else
            {
                node->children.insert(
                    std::end(node->children),
                    std::make_move_iterator(std::begin(children)),
                    std::make_move_iterator(std::end(children)));
            }
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
    struct opening_brace : one<'{'> {};
    struct closing_brace : one<'}'> {};
    template <typename K, typename... Statements>
    struct t_block : seq<K,
        p<must<opening_brace>>,
        star<p<sor<Statements...>>>,
        p<must<closing_brace>>> {};

    // Generic named block.
    template <typename K, typename N, typename... Statements>
    struct t_named_block
        : seq<K, p<must<N>>, p<must<opening_brace>>,
        star<p<sor<Statements...>>>, p<must<closing_brace>>> {};

    // Yes/No (boolean) value.
    struct yesno
    : sor<TAO_PEGTL_STRING("yes"), TAO_PEGTL_STRING("no")> {};

    // Unsigned integer value.
    struct integer : plus<digit> {};

    // Signed integer value.
    struct signed_integer : seq<opt<one<'+', '-'>>, integer> {};

    // IP address.
    struct address : seq<integer, rep<3, seq<one<'.'>, integer>>> {};
    template<> struct store<address> : yes<address> {};

    // Port number.
    struct port : seq<integer> {};
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
    // accept
    struct accept : TAO_PEGTL_STRING("accept") {};
    template<> struct store<accept> : yes_without_content<accept> {};
    // reject
    struct reject : TAO_PEGTL_STRING("reject") {};
    template<> struct store<reject> : yes_without_content<reject> {};
    // call
    struct call : chain_name {};
    template<> struct store<call> : yes<call> {};
    struct call_container : seq<TAO_PEGTL_STRING("call"), p<must<call>>> {};
    template<> struct store<call_container>
        : replace_with_first_child<call_container> {};
    // goto
    struct goto_ : chain_name {};
    template<> struct store<goto_> : yes<goto_> {};
    struct goto_container : seq<TAO_PEGTL_STRING("goto"), p<must<goto_>>> {};
    template<> struct store<goto_container>
        : replace_with_first_child<goto_container> {};
    // generic action
    struct action : sor<accept, reject, call_container, goto_container> {};

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
    struct condition_value
        : p<sor<start_with_packet_type, start_with_source, start_with_dest>> {};
    struct condition : if_must<TAO_PEGTL_STRING("if"), condition_value> {};
    template<> struct store<condition> : yes_without_content<condition> {};

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
        : if_must<sor<action, rule_catch>, opt<p<priority_command>>,
          opt<p<condition>>, eos> {};
    template<> struct store<rule> : replace_with_first_child<rule> {};
    struct chain : chain_name {};
    template<> struct store<chain> : yes<chain> {};
    struct chain_container
        : t_named_block<TAO_PEGTL_STRING("chain"), chain, rule> {};
    template<> struct store<chain_container>
        : replace_with_first_child<chain_container> {};

    // Default filter action (keep node, no content).
    struct default_action_option : sor<accept, reject> {};
    struct default_action
    : a1_statement<TAO_PEGTL_STRING("default_action"), default_action_option> {};
    template<> struct store<default_action>
        : yes_without_content<default_action> {};

    // UDP connection block.
    struct s_port : a1_statement<TAO_PEGTL_STRING("port"), port> {};
    struct s_address : a1_statement<TAO_PEGTL_STRING("address"), address> {};
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
    struct block : sor<udp, serial, chain_container> {};
    struct statement : sor<default_action, s_catch> {};
    struct element : sor<comment, block, statement> {};
    struct grammar : seq<star<pad<element, ignored>>, eof> {};

    /// @endcond


#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wglobal-constructors"
    #pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

    // Error messages

    template<>
    const std::string error<eos>::error_message;

    template<>
    const std::string error<opening_brace>::error_message;

    template<>
    const std::string error<closing_brace>::error_message;

    template<>
    const std::string error<unsupported_statement>::error_message;

    template<>
    const std::string error<default_action_option>::error_message;

    template<>
    const std::string error<port>::error_message;

    template<>
    const std::string error<address>::error_message;

    template<>
    const std::string error<device>::error_message;

    template<>
    const std::string error<baudrate>::error_message;

    template<>
    const std::string error<flow_control>::error_message;

    template<>
    const std::string error<chain_name>::error_message;

    template<>
    const std::string error<chain>::error_message;

    template<>
    const std::string error<call>::error_message;

    template<>
    const std::string error<goto_>::error_message;

    template<>
    const std::string error<invalid_rule>::error_message;

    template<>
    const std::string error<condition_value>::error_message;

    template<>
    const std::string error<dest>::error_message;

    template<>
    const std::string error<source>::error_message;

    template<>
    const std::string error<mavaddr>::error_message;

    template<>
    const std::string error<integer>::error_message;

    template<>
    const std::string error<priority>::error_message;

    template<>
    const std::string error<priority_keyword>::error_message;

    // Default error message.
    template<typename T>
    const std::string error<T>::error_message =
       "parse error matching " + internal::demangle<T>();

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

    // TODO: document
    template <typename Input>
    std::unique_ptr<config::parse_tree::node> parse(Input &in)
    {
        return parse_tree::parse<grammar, config::store>(in);
    }

    std::ostream &print_node(
        std::ostream &os, const config::parse_tree::node &n,
        bool print_location, const std::string &s = "");
}




std::ostream &operator<<(
    std::ostream &os, const config::parse_tree::node &node);


#endif  // CONFIGURATION_HPP_
