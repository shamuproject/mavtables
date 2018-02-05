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


ActionResult::ActionResult(
    ActionResult::Action action_, std::optional<int> priority_)
    : action(action_), priority(std::move(priority_))
{
}


ActionResult ActionResult::make_accept(std::optional<int> priority_)
{
    return ActionResult(ActionResult::ACCEPT, priority_);
}


ActionResult ActionResult::make_reject()
{
    return ActionResult(ActionResult::REJECT);
}


ActionResult ActionResult::make_continue()
{
    return ActionResult(ActionResult::CONTINUE);
}


ActionResult ActionResult::make_default()
{
    return ActionResult(ActionResult::DEFAULT);
}


bool operator==(const ActionResult &lhs, const ActionResult &rhs)
{
    return (lhs.action == rhs.action) && (lhs.priority == rhs.priority);
}


bool operator!=(const ActionResult &lhs, const ActionResult &rhs)
{
    return (lhs.action != rhs.action) || (lhs.priority != rhs.priority);
}


std::ostream &operator<<(std::ostream &os, const ActionResult &action_option)
{
    switch (action_option.action)
    {
        case ActionResult::ACCEPT:
            os << "accept";

            if (action_option.priority)
            {
                os << " with priority" << action_option.priority.value();
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
