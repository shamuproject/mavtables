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


#include <memory>
#include <utility>

#include "fakeit.hpp"


/** Construct a std::shared_ptr from a fakit::Mock object.
 *
 *  \tparam T The type of object being mocked.
 *  \param mock The mock object itself.
 */
template<typename T>
std::shared_ptr<T> mock_shared(fakeit::Mock<T> &mock){
    fakeit::Fake(Dtor(mock));
    std::shared_ptr<T> ptr(&mock.get());
    return std::move(ptr);
}


/** Construct a std::unique_ptr from a fakit::Mock object.
 *
 *  \tparam T The type of object being mocked.
 *  \param mock The mock object itself.
 */
template<typename T>
std::unique_ptr<T> mock_unique(fakeit::Mock<T> &mock){
    fakeit::Fake(Dtor(mock));
    std::unique_ptr<T> ptr(&mock.get());
    return std::move(ptr);
}
