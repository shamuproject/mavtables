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


#include <optional>
#include <ostream>
#include <utility>

#include "Action.hpp"


/** Construct an Action.
 *
 *  \param action_ The action this result represents.
 *  \param priority The priority, only used with Action::ACCEPT.
 *  \sa Action::Option
 */
Action::Action(
    Action::Option action_, std::optional<int> priority)
    : action(action_), priority_(std::move(priority))
{
}


/** Set the priority of the action.
 *
 *  This only has an effect if the \ref action is Action::ACCEPT and the
 *  priority has never been set before.
 *
 *  The default priority is 0.  A higher priority will result in the
 *  packet being prioritized over other packets.
 *
 *  \param priority The priority to apply to the accept action.
 */
void Action::priority(int priority)
{
    if (action == Action::ACCEPT)
    {
        if (!priority_)
        {
            priority_ = priority;
        }
    }
}


/** Return the priority if it exists.
 *
 *  \returns The priority of the action.  This will always be 0 if the \ref
 *  action is not Action::ACCEPT.  It will also be 0 (the default priority) if
 *  the priority was never set.
 */
int Action::priority() const
{
    if (priority_)
    {
        return priority_.value();
    }

    return 0;
}


/** Return a new action result with the Action::ACCEPT action.
 *
 *  An accept action indicates that the packet/address combination this action
 *  is the response to should be accepted without any further processing.
 *
 *  \param priority The priority to accept the packet with.  The default is to
 *      not apply a priority.
 *  \returns The new 'accept' action.
 */
Action Action::make_accept(std::optional<int> priority)
{
    return Action(Action::ACCEPT, priority);
}


/** Return a new action result with the Action::REJECT action.
 *
 *  A reject action indicates that the packet/address combination this action is
 *  the response to should be rejected without any further processing.
 *
 *  \returns The new 'reject' action.
 */
Action Action::make_reject()
{
    return Action(Action::REJECT);
}


/** Return a new action result with the Action::CONTINUE action.
 *
 *  A continue action indicates that filtering of the packet/address combination
 *  this action is the response should continue with the next \ref Rule.
 *
 *  \returns The new 'continue' action.
 */
Action Action::make_continue()
{
    return Action(Action::CONTINUE);
}


/** Return a new action result with the Action::DEFAULT action.
 *
 *  A default action indicates that the default action (defined in \ref Filter)
 *  should be taken for the packet/address combination this action is the
 *  response to.
 *
 *  \returns The new 'default' action.
 */
Action Action::make_default()
{
    return Action(Action::DEFAULT);
}


/** Equality comparison.
 *
 *  \relates Action
 *  \param lhs The left hand side action.
 *  \param rhs The right hand side action.
 *  \retval true if \p lhs is the same as rhs.
 *  \retval false if \p lhs is not the same as rhs.
 */
bool operator==(const Action &lhs, const Action &rhs)
{
    return (lhs.action == rhs.action) && (lhs.priority() == rhs.priority());
}


/** Inequality comparison.
 *
 *  \relates Action
 *  \param lhs The left hand side action.
 *  \param rhs The right hand side action.
 *  \retval true if \p lhs is not the same as rhs.
 *  \retval false if \p lhs is the same as rhs.
 */
bool operator!=(const Action &lhs, const Action &rhs)
{
    return (lhs.action != rhs.action) || (lhs.priority() != rhs.priority());
}


/** Print the action. to the given output stream.
 *
 *  \relates Action
 *  \param os The output stream to print to.
 *  \param action The action result to print.
 *  \return The output stream.
 */
std::ostream &operator<<(std::ostream &os, const Action &action)
{
    switch (action.action)
    {
        case Action::ACCEPT:
            os << "accept";

            if (action.priority() != 0)
            {
                os << " with priority " << action.priority();
            }

            break;

        case Action::REJECT:
            os << "reject";
            break;

        case Action::CONTINUE:
            os << "continue";
            break;

        case Action::DEFAULT:
            os << "default";
            break;
    }

    return os;
}
