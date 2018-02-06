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


#include <utility>
#include <optional>
#include <ostream>

#include "ActionResult.hpp"


/** Construct an ActionResult.
 *
 *  \param action_ The action this result represents.
 *  \param priority The priority, only used with ActionResult::ACCEPT.
 *  \sa ActionResult::Action
 */
ActionResult::ActionResult(
    ActionResult::Action action_, std::optional<int> priority)
    : action(action_), priority_(std::move(priority))
{
}


/** Set the priority of the action result.
 *
 *  This only has an effect if the result is ActionResult::ACCEPT and the
 *  priority has never been set before.
 *
 *  The default priority is 0.  A higher priority will result in the
 *  packet being prioritized over other packets.
 *
 *  \param priority The priority to apply to the accept action.
 */
void ActionResult::priority(int priority)
{
    if (action == ActionResult::ACCEPT)
    {
        if (!priority_)
        {
            priority_ = priority;
        }
    }
}


/** Return the priority if it exists.
 *
 *  \returns The priority of the action result.  This will always be empty if
 *  the result is not ActionResult::ACCEPT.
 */
std::optional<int> ActionResult::priority() const
{
    return priority_;
}


/** Return a new action result with the ActionResult::ACCEPT action.
 *
 *  \param priority The priority to accept the packet with.  The default is to
 *      not apply a priority.
 *  \returns The new 'accept' action result
 */
ActionResult ActionResult::make_accept(std::optional<int> priority)
{
    return ActionResult(ActionResult::ACCEPT, priority);
}


/** Return a new action result with the ActionResult::REJECT action.
 *
 *  \returns The new 'reject' action result
 */
ActionResult ActionResult::make_reject()
{
    return ActionResult(ActionResult::REJECT);
}


/** Return a new action result with the ActionResult::CONTINUE action.
 *
 *  \returns The new 'continue' action result
 */
ActionResult ActionResult::make_continue()
{
    return ActionResult(ActionResult::CONTINUE);
}


/** Return a new action result with the ActionResult::DEFAULT action.
 *
 *  \returns The new 'default' action result
 */
ActionResult ActionResult::make_default()
{
    return ActionResult(ActionResult::DEFAULT);
}


/** Equality comparison.
 *
 *  \relates ActionResult
 *  \param lhs The left hand side action result.
 *  \param rhs The right hand side action result.
 *  \retval true if \p lhs is the same as rhs.
 *  \retval false if \p lhs is not the same as rhs.
 */
bool operator==(const ActionResult &lhs, const ActionResult &rhs)
{
    return (lhs.action == rhs.action) && (lhs.priority() == rhs.priority());
}


/** Inequality comparison.
 *
 *  \relates ActionResult
 *  \param lhs The left hand side action result.
 *  \param rhs The right hand side action result.
 *  \retval true if \p lhs is not the same as rhs.
 *  \retval false if \p lhs is the same as rhs.
 */
bool operator!=(const ActionResult &lhs, const ActionResult &rhs)
{
    return (lhs.action != rhs.action) || (lhs.priority() != rhs.priority());
}


/** Print the action. to the given output stream.
 *
 *  \relates ActionResult
 *  \param os The output stream to print to.
 *  \param action_result The action result to print.
 *  \return The output stream.
 */
std::ostream &operator<<(std::ostream &os, const ActionResult &action_result)
{
    switch (action_result.action)
    {
        case ActionResult::ACCEPT:
            os << "accept";

            if (action_result.priority())
            {
                os << " with priority " << action_result.priority().value();
            }

            break;

        case ActionResult::REJECT:
            os << "reject";
            break;

        case ActionResult::CONTINUE:
            os << "continue";
            break;

        case ActionResult::DEFAULT:
            os << "default";
            break;
    }

    return os;
}
