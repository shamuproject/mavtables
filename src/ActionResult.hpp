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


#ifndef ACTION_HPP_
#define ACTION_HPP_


#include <optional>
#include <ostream>


/** A result to applying an action method to a packet/address combination.
 *
 *  This is used as a return value to determine what to do with a packet.
 */
class ActionResult
{
    private:
        ActionResult(
            ActionResult::Action action, std::optional<int> priority = {});

    public:
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
        /** The priority associated with an action.
         *
         *  This is only valid if the action is ActionResult::ACCEPT.
         *
         *  The default priority is 0.  A higher priority will result in the
         *  packet being prioritized over other packets.
         */
        std::optional<int> priority;
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
        ActionResult make_accept(std::optional<int> priority = {});
        ActionResult make_reject();
        ActionResult make_continue();
        ActionResult make_default();
        /** Assignment operator.
         *
         *  \param other ActionResult to copy.
         */
        ActionResult &operator=(const ActionResult &other) = default;
        /** Assignment operator (by move semantics).
         *
         *  \param other ActionResult to move from.
         */
        ActionResult &operator=(ActionResult &&other) = default;

};


bool operator==(const ActionResult &lhs, const ActionResult &rhs);
bool operator!=(const ActionResult &lhs, const ActionResult &rhs);
std::ostream &operator<<(std::ostream &os, const ActionResult &action_option);


#endif // ACTION_HPP_
