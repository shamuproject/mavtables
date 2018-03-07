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


/** An action that is to be taken with a packet.
 *
 *  This is used as a return value to determine what to do with a packet.
 */
class Action
{
  public:
    /** Possible actions.
     */
    enum Option
    {
        ACCEPT,    //!< The packet has been accepted, possibly with priority.
        REJECT,    //!< The packet has been rejected.
        CONTINUE,  //!< Continue evaluating rules.
        DEFAULT    //!< Use the default rule.
    };
    // Variables
    Action::Option action_;
    // Methods
    /** Copy constructor.
     *
     *  \param other Action to copy.
     */
    Action(const Action &other) = default;
    /** Move constructor.
     *
     *  \param other Action to move from.
     */
    Action(Action &&other) = default;
    Action::Option action() const;
    void priority(int priority);
    int priority() const;
    /** Assignment operator.
     *
     * \param other Action to copy.
     */
    Action &operator=(const Action &other) = default;
    /** Assignment operator (by move semantics).
     *
     * \param other Action to move from.
     */
    Action &operator=(Action &&other) = default;

    static Action make_accept(std::optional<int> priority = {});
    static Action make_reject();
    static Action make_continue();
    static Action make_default();

  private:
    // Note: The reason this is optional is because there is a difference
    //       between {} and 0.  This is because a priority of {} can still
    //       be set to something other than 0 by a higher level rule (see
    //       \ref Call or \ref GoTo) while if the priority has been set to 0
    //       it cannot be set again.
    std::optional<int> priority_;
    Action(Action::Option action, std::optional<int> priority = {});
};


bool operator==(const Action &lhs, const Action &rhs);
bool operator!=(const Action &lhs, const Action &rhs);
std::ostream &operator<<(std::ostream &os, const Action &action);


#endif  // ACTION_HPP_
