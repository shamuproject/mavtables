// MAVLink router and firewall.
// Copyright (C) 2017-2018  Michael R. Shannon <mrshannon.aerospace@gmail.com>
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


#ifndef UTILITY_HPP_
#define UTILITY_HPP_


#include <array>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <boost/range/irange.hpp>


template <typename T>
typename std::vector<T>::iterator append(
    std::vector<T> &dest, const std::vector<T> &source);

template <typename T>
typename std::vector<T>::iterator append(
    std::vector<T> &dest, std::vector<T> &&source);

template <class T>
std::string str(const T &object);

template <class ByteType = unsigned char, class T>
std::array<ByteType, sizeof(T)> to_bytes(T number);

std::string to_lower(std::string string);

template <class T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vector);


/** Append one vector to another.
 *
 *  Taken from https://stackoverflow.com/a/37210097
 *
 *  \ingroup utility
 *  \param dest Vector to append to.
 *  \param source Vector to append the elements from.
 *  \returns Iterator pointing to the first element appended, or the end of the
 *      destination vector if the source vector is empty.
 */
template <typename T>
typename std::vector<T>::iterator append(
    std::vector<T> &dest, const std::vector<T> &source)
{
    typename std::vector<T>::iterator result;

    if (dest.empty())
    {
        dest = source;
        result = std::begin(dest);
    }
    else
    {
        result =
            dest.insert(std::end(dest), std::cbegin(source), std::cend(source));
    }

    return result;
}


/** Append one vector to another (move from source).
 *
 *  Taken from https://stackoverflow.com/a/37210097
 *
 *  \ingroup utility
 *  \param dest Vector to append to.
 *  \param source Vector to append the elements from.  \p source will be a valid
 *      empty vector after this call.
 *  \returns Iterator pointing to the first element appended, or the end of the
 *      destination vector if the source vector is empty.
 */
template <typename T>
typename std::vector<T>::iterator append(
    std::vector<T> &dest, std::vector<T> &&source)
{
    typename std::vector<T>::iterator result;

    if (dest.empty())
    {
        dest = std::move(source);
        result = std::begin(dest);
    }
    else
    {
        result = dest.insert(
                     std::end(dest),
                     std::make_move_iterator(std::begin(source)),
                     std::make_move_iterator(std::end(source)));
    }

    source.clear();
    source.shrink_to_fit();
    return result;
}


/** Convert any object supporting the output stream operator (<<) to a string.
 *
 *  \ingroup utility
 *  \tparam T Type of the object to convert to a string.
 *  \param object The object to convert to a string.
 *  \returns The string representing the object.
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
 *  \returns The array of bytes from the given number, in LSB order.
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
 *  \ingroup utility
 *  \tparam T The type stored in the vector, it must support the << operator.
 *  \param os The output stream to print to.
 *  \param vector The vector of elements to print.
 *  \returns The output stream.
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


#endif // UTILITY_HPP_
