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


#ifndef ACTIONRESULT_HPP_
#define ACTIONRESULT_HPP_


#include <optional>
#include <ostream>


/** A result to applying an action method to a packet/address combination.
 *
 *  This is used as a return value to determine what to do with a packet.
 */
class ActionResult
{
    public:
        /** Possible actions.
         */
        enum Action
        {
            ACCEPT,     //!< The packet has been accepted.
            REJECT,     //!< The packet has been rejected.
            CONTINUE,   //!< Continue evaluating rules.
            DEFAULT     //!< Use the default rule.
        };
        /** The action that has been chosen.
         */
        const ActionResult::Action action;
    private:
        std::optional<int> priority_;
        ActionResult() = delete;
        ActionResult(
            ActionResult::Action action, std::optional<int> priority = {});

    public:
        /** Copy constructor.
         *
         *  \param other ActionResult to copy.
         */
        ActionResult(const ActionResult &other) = default;
        /** Move constructor.
         *
         *  \param other ActionResult to move from.
         */
        ActionResult(ActionResult &&other) = default;
        void priority(int priority);
        std::optional<int> priority() const;
        ActionResult &operator=(const ActionResult &other) = delete;
        ActionResult &operator=(ActionResult &&other) = delete;

        static ActionResult make_accept(std::optional<int> priority = {});
        static ActionResult make_reject();
        static ActionResult make_continue();
        static ActionResult make_default();

};


bool operator==(const ActionResult &lhs, const ActionResult &rhs);
bool operator!=(const ActionResult &lhs, const ActionResult &rhs);
std::ostream &operator<<(std::ostream &os, const ActionResult &action_option);


#endif // ACTIONRESULT_HPP_
