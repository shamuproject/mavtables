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


class ActionOption
{
    private:

    public:
        enum Choice
        {
            ACCEPT,     //!< The packet has been accepted.
            REJECT,     //!< The packet has been rejected.
            CONTINUE,   //!< Continue evaluating rules.
            DEFAULT     //!< Use the default rule.
        };
        const Choice choice;
        std::optional<int> priority;
        ActionOption(
            ActionOption::Choice choice, std::optional<int> priority = {});
        ActionOption make_accept(std::optional<int> priority = {});
        ActionOption make_reject();
        ActionOption make_continue();
        ActionOption make_default();

};


#endif // ACTION_HPP_
