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


#include <iostream>
#include <memory>
#include <sstream>
#include <string>
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


/** RAII class to replace std::cout with a mocked buffer.
 */
class MockCOut
{
    public:
        /** Replace std::cout with this mock.
         */
        MockCOut()
            : sbuf_(std::cout.rdbuf())
        {
            std::cout.rdbuf(buffer_.rdbuf());
        }
        /** Restore std::cout.
         */
        ~MockCOut()
        {
            std::cout.rdbuf(sbuf_);
        }
        /** Return the contents of the mocked std::cout buffer as a string.
         *
         *  \returns The contents of the mocked buffer.
         */
        std::string buffer()
        {
            return buffer_.str();
        }

    private:
        std::stringstream buffer_;
        std::streambuf *sbuf_;
};


/** RAII class to replace std::cerr with a mocked buffer.
 */
class MockCErr
{
    public:
        /** Replace std::cerr with this mock.
         */
        MockCErr()
            : sbuf_(std::cerr.rdbuf())
        {
            std::cerr.rdbuf(buffer_.rdbuf());
        }
        /** Restore std::cerr.
         */
        ~MockCErr()
        {
            std::cerr.rdbuf(sbuf_);
        }
        /** Return the contents of the mocked std::cerr buffer as a string.
         *
         *  \returns The contents of the mocked buffer.
         */
        std::string buffer()
        {
            return buffer_.str();
        }

    private:
        std::stringstream buffer_;
        std::streambuf *sbuf_;
};
