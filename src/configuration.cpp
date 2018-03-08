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


#include <cstdlib>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "Filesystem.hpp"
#include "configuration.hpp"


/** Find the configuration file.
 *
 *  The order of checking is:
 *  1. The given path.
 *  2. The target of the `MAVTABLES_CONFIG_PATH` environment variable.
 *  3. `.mavtablesrc` in the current directory.
 *  4. `.mavtablesrc` at `$HOME/.mavtablesrc`.
 *  5. The main configuration file at `/etc/mavtables.conf`.
 *
 *  \param config Path to the configuration file specified on the command line.
 *  \param filesystem A filesystem instance.  The default is to construct an
 *      instance.
 *  \returns The path the first configuration file found.
 */
std::string find_config(
    std::optional<std::string> config, std::unique_ptr<Filesystem> filesystem)
{
    // Check given path.
    if (config)
    {
        if (filesystem->exists(config.value()))
        {
            return config.value();
        }
    }

    // Check MATABLES_CONFIG_PATH.
    if (auto config_path = std::getenv("MAVTABLES_CONFIG_PATH"))
    {
        if (filesystem->exists(Filesystem::path(config_path)))
        {
            return config_path;
        }
    }

    // Check for .mavtablesrc in current directory.
    if (filesystem->exists(".mavtablesrc"))
    {
        return ".mavtablesrc";
    }

    // Check for .mavtablesrc in home directory.
    if (auto home = std::getenv("HOME"))
    {
        Filesystem::path config_path(home);
        config_path /= ".mavtablesrc";

        if (filesystem->exists(config_path))
        {
            return config_path.string();
        }
    }

    // Check for /etc/mavtables.conf.
    if (filesystem->exists("/etc/mavtables.conf"))
    {
        return "/etc/mavtables.conf";
    }

    throw std::runtime_error("Could not locate any configuration files.");
}
