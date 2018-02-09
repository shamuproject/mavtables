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


// See https://github.com/eranpeer/FakeIt/issues/27
template<typename T>
void blank_deleter(T *ptr) {(void)ptr;}


template<typename T>
std::unique_ptr<T, void(*)(T*)> mock_unique(T &mock){
    std::unique_ptr<T, void(*)(T*)> ptr(&mock, blank_deleter<T>);
    return std::move(ptr);
}


template<typename T>
std::shared_ptr<T> mock_shared(T &mock){
    std::shared_ptr<T> ptr(&mock, blank_deleter<T>);
    return std::move(ptr);
}
