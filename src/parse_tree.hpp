// Copyright (c) 2007-2018 Dr. Colin Hirsch and Daniel Frey
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


// Modified by Michael R. Shannon for use in mavtables.


#ifndef PARSE_TREE_HPP_
#define PARSE_TREE_HPP_


#include <cassert>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

#include <pegtl/config.hpp>
#include <pegtl/normal.hpp>
#include <pegtl/nothing.hpp>
#include <pegtl/parse.hpp>
#include <pegtl/internal/demangle.hpp>
#include <pegtl/internal/iterator.hpp>


// Not included in documentation because it is mostly copied from PEGTL.

/// @cond INTERNAL
namespace config
{
    namespace parse_tree
    {
        namespace pegtl = tao::pegtl;

        template<typename T>
        struct default_node_children
        {
            using node_t = T;
            using children_t = std::vector< std::unique_ptr< node_t > >;
            children_t children;

            // each node will be default constructed
            default_node_children() = default;

            // no copy/move is necessary
            // (nodes are always owned/handled by a std::unique_ptr)
            default_node_children(const default_node_children &) = delete;
            default_node_children(default_node_children &&) = delete;

            ~default_node_children() = default;

            // no assignment either
            default_node_children &operator=(
                const default_node_children &) = delete;
            default_node_children &operator=(
                default_node_children &&) = delete;

            typename children_t::reference at(
                const typename children_t::size_type pos)
            {
                return children.at(pos);
            }

            typename children_t::const_reference at(
                const typename children_t::size_type pos) const
            {
                return children.at(pos);
            }

            typename children_t::reference front()
            {
                return children.front();
            }

            typename children_t::const_reference front() const
            {
                return children.front();
            }

            typename children_t::reference back()
            {
                return children.back();
            }

            typename children_t::const_reference back() const
            {
                return children.back();
            }

            bool empty() const noexcept
            {
                return children.empty();
            }

            typename children_t::size_type size() const noexcept
            {
                return children.size();
            }

            // if parsing succeeded and the (optional) transform call
            // did not discard the node, it is appended to its parent.
            // note that "child" is the node whose Rule just succeeded
            // and "*this" is the parent where the node should be appended.
            template<typename... States>
            void emplace_back(
                std::unique_ptr< node_t > child, States &&... /*unused*/)
            {
                assert(child);
                children.emplace_back(std::move(child));
            }
        };

        struct default_node_content
        {
            const std::type_info *id_ = nullptr;
            pegtl::internal::iterator begin_;
            pegtl::internal::iterator end_;
            std::string source_;

            bool is_root() const noexcept
            {
                return id_ == nullptr;
            }

            template<typename T>
            bool is() const noexcept
            {
                return id_ == &typeid(T);
            }

            std::string name() const
            {
                assert(!is_root());
                return pegtl::internal::demangle(id_->name());
            }

            pegtl::position begin() const
            {
                return pegtl::position(begin_, source_);
            }

            pegtl::position end() const
            {
                return pegtl::position(end_, source_);
            }

            const std::string &source() const noexcept
            {
                return source_;
            }

            bool has_content() const noexcept
            {
                return end_.data != nullptr;
            }

            std::string content() const
            {
                assert(has_content());
                return std::string(begin_.data, end_.data);
            }

            template< typename... States >
            void remove_content(States &&... /*unused*/) noexcept
            {
                end_.reset();
            }

            // all non-root nodes are initialized by calling this method
            template<typename Rule, typename Input, typename... States>
            void start(const Input &in, States &&... /*unused*/)
            {
                id_ = &typeid(Rule);
                begin_ = in.iterator();
                source_ = in.source();
            }

            // if parsing of the rule succeeded, this method is called
            template<typename Rule, typename Input, typename... States>
            void success(const Input &in, States &&... /*unused*/) noexcept
            {
                end_ = in.iterator();
            }

            // if parsing of the rule failed, this method is called
            template<typename Rule, typename Input, typename... States>
            void failure(
                const Input & /*unused*/, States &&... /*unused*/) noexcept
            {
            }
        };

        template<typename T>
        struct basic_node
            : default_node_children<T>, default_node_content
        {
        };

        struct node
            : basic_node< node >
        {
        };

        namespace internal
        {
            template<typename Node>
            struct state
            {
                std::vector<std::unique_ptr<Node>> stack;

                state()
                {
                    emplace_back();
                }

                void emplace_back()
                {
                    stack.emplace_back(std::unique_ptr< Node >(new Node));
                }

                std::unique_ptr<Node> &back() noexcept
                {
                    return stack.back();
                }

                void pop_back() noexcept
                {
                    return stack.pop_back();
                }
            };

            template<typename Node, typename S, typename = void>
            struct transform
            {
                template<typename... States>
                static void call(std::unique_ptr< Node > & /*unused*/,
                                 States &&... /*unused*/) noexcept
                {
                }
            };

            template<typename Node, typename S>
            struct transform <
            Node, S, decltype(
                S::transform(std::declval<std::unique_ptr<Node>& >()), void()) >
            {
                template<typename... States>
                static void call(
                    std::unique_ptr<Node> &n, States &&... st)
                noexcept(noexcept(S::transform(n)))
                {
                    S::transform(n, st...);
                }
            };

            // https://stackoverflow.com/a/16000226
            template <typename T, typename = int>
            struct has_error_message : std::false_type {};

            // https://stackoverflow.com/a/16000226
            template <typename T>
            struct has_error_message <T, decltype((void) T::error_message, 0)>
                : std::true_type {};

            template<template<typename> class S>
            struct make_control
            {
                template<typename Rule, bool = S<Rule>::value>
                struct control;

                template<typename Rule>
                using type = control<Rule>;
            };

            template<template<typename> class S>
            template<typename Rule>
            struct make_control< S >::control< Rule, false >
                : pegtl::normal< Rule >
            {

                // custom error message
                template< typename Input, typename... States >
                static void raise( const Input& in, States&&... )
                {
                    if constexpr (has_error_message<S<Rule>>::value)
                    {
                        throw pegtl::parse_error(S<Rule>::error_message, in);
                    }
                    else
                    {
                        throw pegtl::parse_error(
                            "Parse error matching " +
                            pegtl::internal::demangle<Rule>(), in);
                    }
                }

            };

            template<template<typename> class S>
            template<typename Rule>
            struct make_control<S>::control< Rule, true >
                : pegtl::normal<Rule>
            {

                // custom error message
                template< typename Input, typename... States >
                static void raise( const Input& in, States&&... )
                {
                    if constexpr (has_error_message<S<Rule>>::value)
                    {
                        throw pegtl::parse_error(S<Rule>::error_message, in);
                    }
                    else
                    {
                        throw pegtl::parse_error(
                            "Parse error matching " +
                            pegtl::internal::demangle<Rule>(), in);
                    }
                }

                template<typename Input, typename Node, typename... States>
                static void start(
                    const Input &in, state< Node > &state, States &&... st)
                {
                    state.emplace_back();
                    state.back()->template start<Rule>(in, st...);
                }

                template< typename Input, typename Node, typename... States >
                static void success(
                    const Input &in, state<Node> &state, States &&... st)
                {
                    auto n = std::move(state.back());
                    state.pop_back();
                    n->template success<Rule>(in, st...);
                    transform<Node, S<Rule>>::call(n, st...);

                    if (n)
                    {
                        state.back()->emplace_back(std::move(n), st...);
                    }
                }

                template<typename Input, typename Node, typename... States>
                static void failure(
                    const Input &in, state< Node > &state, States &&... st)
                noexcept(noexcept(std::declval<node &>().template
                                  failure<Rule>(in, st...)))
                {
                    state.back()->template failure< Rule >(in, st...);
                    state.pop_back();
                }
            };

            template<typename>
            struct store_all : std::true_type
            {
            };

        }  // namespace internal

        template <
            typename Rule,
            typename Node,
            template<typename> class S = internal::store_all,
            typename Input,
            typename... States >
        std::unique_ptr<Node> parse(Input &in, States && ... st)
        {
            internal::state< Node > state;

            if (!pegtl::parse <
                    Rule, pegtl::nothing,
                    internal::make_control<S>::template type>
                    (in, state, st...))
            {
                return nullptr;
            }

            assert(state.stack.size() == 1);
            return std::move(state.back());
        }

        template <
            typename Rule,
            template<typename> class S = internal::store_all,
            typename Input,
            typename... States >
        std::unique_ptr<node> parse(Input &in, States && ... st)
        {
            return parse<Rule, node, S>(in, st...);
        }

    }  // namespace parse_tree

}  // namespace config
/// @endcond

#endif
