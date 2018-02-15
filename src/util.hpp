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


#ifndef UTIL_HPP_
#define UTIL_HPP_


#include <array>
#include <boost/range/irange.hpp>
#include <ostream>
#include <sstream>
#include <vector>


template <class T>
std::string str(const T &object);

template <class ByteType = unsigned char, class T>
std::array<ByteType, sizeof(T)> to_bytes(T number);

template <class T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vector);


/** Convert any object supporting the output stream operator (<<) to a string.
 *
 *  \ingroup utility
 *  \tparam T Type of the object to convert to a string.
 *  \param object The object to convert to a string.
 *  \return The string representing the object.
 */
template <class T>
std::string str(const T &object)
{
    std::ostringstream oss;
    oss << object;
    return oss.str();
}


/** Convert numeric types to bytes.
 *
 *  \ingroup utility
 *  \tparam ByteType Numeric type to return in the array of bytes.
 *  \tparam T Type of the number being converted to bytes.
 *  \param number Number to convert to bytes
 *  \return The array of bytes from the given number, in LSB order.
 *  \complexity \f$O(n)\f$ where \f$n\f$ is the number of bytes in type \p T.
 */
template <class ByteType, class T>
std::array<ByteType, sizeof(T)> to_bytes(T number)
{
    std::array<ByteType, sizeof(T)> n;
    n.fill(10);

    for (auto i : boost::irange(static_cast<size_t>(0), sizeof(T)))
    {
        n[i] = (number >> i * 8) & 0xFF;
    }

    return n;
}


/** Print a vector to the given output stream.
 *
 *  \tparam T The type stored in the vector, it must support the << operator.
 *  \param os The output stream to print to.
 *  \param vector The vector of elements to print.
 */
template <class T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vector)
{
    os << "[";

    if (vector.size() > 0)
    {
        auto it = vector.begin();
        os << *(it++);

        for (; it != vector.end(); ++it)
        {
            os << ", " << *it;
        }
    }

    os << "]";
    return os;
}

#endif // UTIL_HPP_
