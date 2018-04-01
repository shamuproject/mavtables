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


#include <chrono>
#include <memory>
#include <utility>
#include <vector>


#include "App.hpp"
#include "Interface.hpp"
#include "InterfaceThreader.hpp"


using namespace std::chrono_literals;


/** Construct mavtables application from a vector of interfaces.
 *
 *  Neither the interfaces, nor the application will be started until the \ref
 *  run method is called.
 *
 *  \param interfaces A vector of interfaces.
 */
App::App(std::vector<std::unique_ptr<Interface>> interfaces)
{
    // Create threader for each interface.
    for (auto &interface : interfaces)
    {
        threaders_.push_back(
            std::make_unique<InterfaceThreader>(
                std::move(interface), 100ms, InterfaceThreader::DELAY_START));
    }
}


/** Start the application.
 */
void App::run()
{
    for (auto &interface : threaders_)
    {
        interface->start();
    }
}
