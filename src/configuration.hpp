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

#include <pegtl.hpp>
#include <pegtl/contrib/parse_tree.hpp>


namespace pegtl = tao::TAO_PEGTL_NAMESPACE;



namespace config
{
    using namespace tao::pegtl;
    using namespace tao::pegtl::ascii;


    // remove content mixin
    struct no_content : std::true_type
    {
        static void transform(std::unique_ptr<pegtl::parse_tree::node> &n)
        {
            n->remove_content();
        }
    };


    // end of statement
    struct eos : one<';'> {};

    // comments
    struct comment : seq<one<'#'>, star<not_at<eol>, not_at<eof>, any>> {};
    struct ignored : sor<space, comment> {};

    // pad
    template <typename N>
    struct p : pad<N, ignored> {};

    // statement template
    template <typename K, typename V = success>
    struct t_statement
        : seq<p<K>, must<p<V>>, must<eos>> {};

    // block template
    template <typename K, typename... Statements>
    struct t_block : seq<p<K>, p<one<'{'>>,
        star<p<sor<Statements...>>>, p<one<'}'>>> {};

    // named block template
    template <typename K, typename N, typename... Statements>
        struct t_named_block : seq<p<K>, p<N>, p<one<'{'>>,
        star<p<sor<Statements...>>>, p<one<'}'>>> {};


    // values
    struct yesno : sor<TAO_PEGTL_ISTRING("yes"), TAO_PEGTL_ISTRING("no")> {};
    struct integer : plus<digit> {};
    struct signed_integer : seq<opt<one<'+', '-'>>, integer> {};
    struct port : must<integer> {};
    // struct ip_address : list_must<integer, one<'.'>> {};
    struct ip : must<seq<integer, rep<3, seq<one<'.'>, integer>>>> {};
    struct device : must<plus<sor<alnum, one<'_', '/'>>>> {};
    struct baudrate : must<integer> {};
    struct flow_control : must<yesno> {};
    // struct device_name : plus<sor<alnum, one<'_', '/'>>> {};


    struct name : identifier {};

    // actions
    struct accept : TAO_PEGTL_STRING("accept") {};
    struct reject : TAO_PEGTL_STRING("reject") {};
    struct call : seq<TAO_PEGTL_STRING("call"), p<name>> {};
    struct goto_ : seq<TAO_PEGTL_STRING("goto"), p<name>> {};
    struct action : sor<accept, reject, call, goto_> {};

    struct mavaddr : seq<integer, one<'.'>, integer> {};
    struct full_mask : seq<one<':'>, mavaddr> {};
    struct forward_mask : seq<one<'/'>, integer> {};
    struct backward_mask : seq<one<'\\'>, integer> {};
    struct mavmask
        : seq<mavaddr, sor<full_mask, forward_mask, backward_mask, success>> {};

    // conditional
    struct packet_type : plus<sor<upper, one<'_'>>> {};
    struct source : mavmask {};
    struct source_command
        : seq<TAO_PEGTL_STRING("from"), must<p<source>>> {};
    struct dest : mavmask {};
    struct dest_command
        : seq<TAO_PEGTL_STRING("to"), must<p<dest>>> {};
    struct conditional
        : seq<TAO_PEGTL_STRING("if"), opt<p<packet_type>>,
        opt<p<source_command>>, opt<p<dest_command>>> {};

    // priority
    struct priority : signed_integer {};
    struct priority_command
        : seq<TAO_PEGTL_STRING("with"), p<TAO_PEGTL_STRING("priority")>,
          p<priority>> {};

    // chain block
    struct rule
        : seq<p<action>, opt<p<priority_command>>, opt<p<conditional>>, eos> {};
    struct rules : plus<p<rule>> {};
    struct chain : t_named_block<TAO_PEGTL_STRING("chain"), name, rules> {};


    // global settings
    struct default_action
        : t_statement<TAO_PEGTL_STRING("default_action"),
          sor<accept, reject>> {};

    // catch unsuported statements
    // TODO: Consider changing failure to raise.
    struct s_catch : if_must<t_statement<identifier, star<alnum>>, failure> {};

    // udp block
    struct s_port : t_statement<TAO_PEGTL_STRING("port"), port> {};
    struct s_address : t_statement<TAO_PEGTL_STRING("address"), ip> {};
    struct udp
        : t_block<TAO_PEGTL_STRING("udp"), s_port, s_address, s_catch> {};

    // serial block
    struct s_device : t_statement<TAO_PEGTL_STRING("device"), device> {};
    struct s_baudrate : t_statement<TAO_PEGTL_STRING("baudrate"), baudrate> {};
    struct s_flow_control
        : t_statement<TAO_PEGTL_STRING("flow_control"), flow_control> {};
    struct serial
        : t_block<TAO_PEGTL_STRING("serial"),
        s_device, s_baudrate, s_flow_control, s_catch> {};



    // combine grammar
    struct block : sor<udp, serial, chain> {};
    struct statement : sor<default_action, s_catch> {};
    struct element : sor<comment, block, statement> {};
    struct grammar : seq<star<pad<element, ignored>>, eof> {};


    // nodes to store
    template<typename> struct store : std::false_type {};

    template<> struct store<chain> : no_content {};
    template<> struct store<name> : std::true_type {};

    template<> struct store<default_action> : no_content {};
    // template<> struct store<action> : std::true_type {};
    template<> struct store<accept> : no_content {};
    template<> struct store<reject> : no_content {};
    template<> struct store<call> : no_content {};
    template<> struct store<goto_> : no_content {};
    template<> struct store<rules> : no_content {};
    template<> struct store<rule> : no_content {};
    template<> struct store<conditional> : std::true_type {};
    template<> struct store<dest> : std::true_type {};
    template<> struct store<source> : std::true_type {};
    template<> struct store<packet_type> : std::true_type {};
    template<> struct store<priority> : std::true_type {};

    template<> struct store<udp> : no_content {};
    template<> struct store<ip> : std::true_type {};
    template<> struct store<port> : std::true_type {};

    template<> struct store<serial> : no_content {};
    template<> struct store<device> : std::true_type {};
    template<> struct store<baudrate> : std::true_type {};
    template<> struct store<flow_control> : std::true_type {};
    // template<> struct store<comment> : std::true_type {};
    // template<> struct store<integer> : std::true_type {};

    template<> struct store<s_catch> : std::true_type {};

}


void parse_file(std::string filename);
void parse_file2(std::string filename);




#endif  // CONFIGURATION_HPP_
