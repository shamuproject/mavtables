// MAVLink router and firewall.
// Copyright (C) 2017  Michael R. Shannon <mrshannon.aerospace@gmail.com>
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


#ifndef COMPARATORS_HPP_
#define COMPARATORS_HPP_


/** \defgroup comparators
 *
 *  Default comparison operators.
 */


/** Equality comparison.
 *
 *  \note Requires != to be defined for type \p T.
 *
 *  \ingroup comparators
 *  \param a The first object.
 *  \param b The second object.
 *  \retval true if \p a is equal to \p b.
 *  \retval false if \p a is not equal to \p b.
 */
template <typename T>
bool operator==(const T &a, const T &b)
{
    return !(a != b);
}


/** Inequality comparison.
 *
 *  \note Requires == to be defined for type \p T.
 *
 *  \ingroup comparators
 *  \param a The first object.
 *  \param b The second object.
 *  \retval true if \p a is not equal to \p b.
 *  \retval false if \p a is equal to \p b.
 */
template <typename T>
bool operator!=(const T &a, const T &b)
{
    return !(a == b);
}


/** Less than comparison.
 *
 *  \note Requires > to be defined for type \p T.
 *
 *  \ingroup comparators
 *  \param a The first object.
 *  \param b The second object.
 *  \retval true if \p a is less than \p b.
 *  \retval false if \p a is greater than or equal to \p b.
 */
template <typename T>
bool operator<(const T &a, const T &b)
{
    return b > a;
}


/** Greater than comparison.
 *
 *  \note Requires < to be defined for type \p T.
 *
 *  \ingroup comparators
 *  \param a The first object.
 *  \param b The second object.
 *  \retval true if \p a is greater than \p b.
 *  \retval false if \p a is less than or equal to \p b.
 */
template <typename T>
bool operator>(const T &a, const T &b)
{
    return b < a;
}



/** Less than or equal comparison.
 *
 *  \note Requires > to be defined for type \p T.
 *
 *  \ingroup comparators
 *  \param a The first object.
 *  \param b The second object.
 *  \retval true if \p a is less than or equal to \p b.
 *  \retval false if \p a is greater than \p b.
 */
template <typename T>
bool operator<=(const T &a, const T &b)
{
    return !(a > b);
}


/** Greater than or equal comparison.
 *
 *  \note Requires < to be defined for type \p T.
 *
 *  \ingroup comparators
 *  \param a The first object.
 *  \param b The second object.
 *  \retval true if \p a is greater than or equal to \p b.
 *  \retval false if \p a is less than \p b.
 */
template <typename T>
bool operator>=(const T &a, const T &b)
{
    return !(a < b);
}


#endif // COMPARATORS_HPP_
