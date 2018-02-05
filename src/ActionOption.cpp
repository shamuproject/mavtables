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

#include "ActionOption.hpp"


ActionOption::ActionOption(
    ActionOption::Choice choice_, std::optional<int> priority_)
    : choice(choice_), priority(std::move(priority_))
{
}


ActionOption ActionOption::make_accept(std::optional<int> priority_)
{
    return ActionOption(ActionOption::ACCEPT, priority_);
}


ActionOption ActionOption::make_reject()
{
    return ActionOption(ActionOption::REJECT);
}


ActionOption ActionOption::make_continue()
{
    return ActionOption(ActionOption::CONTINUE);
}


ActionOption ActionOption::make_default()
{
    return ActionOption(ActionOption::DEFAULT);
}


bool operator==(const ActionOption &lhs, const ActionOption &rhs)
{
    return (lhs.choice == rhs.choice) && (lhs.priority == rhs.priority);
}


bool operator!=(const ActionOption &lhs, const ActionOption &rhs)
{
    return (lhs.choice != rhs.choice) || (lhs.priority != rhs.priority);
}


std::ostream &operator<<(std::ostream &os, const ActionOption &action_option)
{
    switch (action_option.choice)
    {
        case ActionOption::ACCEPT:
            os << "accept";

            if (action_option.priority)
            {
                os << " with priority" << action_option.priority.value();
            }

            break;

        case ActionOption::REJECT:
            os << "reject";
            break;

        case ActionOption::CONTINUE:
            os << "continue";
            break;

        case ActionOption::DEFAULT:
            os << "default";
            break;
    }

    return os;
}
