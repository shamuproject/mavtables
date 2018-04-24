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


#include <stdexcept>
#include <string>

#include <boost/program_options.hpp>

#include "Options.hpp"
#include "config.hpp"


namespace po = boost::program_options;


/** Construct an options object.
 *
 *  \param os The output stream to use for printing the help message.
 *  \param argc The number of command line arguments given.
 *  \param argv The command line arguments, as given in the arguments to the
 *      main function.
 *  \param filesystem A filesystem instance.  The default is to construct an
 *      instance.
 */
Options::Options(
    int argc, const char *argv[], std::ostream &os,
    const Filesystem &filesystem)
    : continue_(true), loglevel_(0)
{
    // Command line options.
    po::options_description options(
        "usage: " + std::string(argv[0]));
    options.add_options()
    ("help,h", "print this message")
    ("config", po::value<std::string>(), "specify configuration file")
    ("ast", "print AST of configuration file (do not run)")
    ("version", "print version and license information")
    ("loglevel", po::value<unsigned int>(),
     "level of logging, between 0 and 3");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    // Print help.
    if (vm.count("help"))
    {
        os << options << std::endl;
        continue_ = false;
        return;
    }

    // Print version information.
    if (vm.count("version"))
    {
        os << "mavtables (SHAMU Project) ";
        os << "v" << std::to_string(VERSION_MAJOR);
        os << "." << std::to_string(VERSION_MINOR);
        os << "." << std::to_string(VERSION_PATCH) << "\n";
        os << "Copyright (C) 2018  Michael R. Shannon\n";
        os << "\n";
        os << "License: GPL v2.0 or any later version.\n";
        os << "This is free software; see the source for copying conditions.  ";
        os << "There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS ";
        os << "FOR A PARTICULAR PURPOSE." << std::endl;
        continue_ = false;
        return;
    }

    // Find configuration file.
    if (vm.count("config"))
    {
        if (filesystem.exists(vm["config"].as<std::string>()))
        {
            config_file_ = vm["config"].as<std::string>();
        }
        else
        {
            continue_ = false;
            throw std::runtime_error(
                "mavtables could not locate a configuration file");
        }
    }
    else
    {
        if (auto config_file = find_config(filesystem))
        {
            config_file_ = config_file.value();
        }
        else
        {
            continue_ = false;
            throw std::runtime_error(
                "mavtables could not locate a configuration file");
        }
    }

    if (vm.count("loglevel"))
    {
        loglevel_ = vm["loglevel"].as<unsigned int>();
    }

    // Determine actions.
    print_ast_ = vm.count("ast");
    run_firewall_ = !print_ast_;
}


/** Determine whether to print the configuration file's AST or not.
 *
 *  \retval true Print abstract syntax tree of configuration file.
 *  \retval false Don't print abstract syntax tree of configuration file.
 */
bool Options::ast()
{
    return print_ast_;
}


/** Get path to an existing configuration file.
 *
 *  \returns The path to an existing, but necessarily valid configuration file.
 */
std::string Options::config_file()
{
    return config_file_;
}


/** Get the log level.
 *
 *  \returns The level to log at, between 0 and 3.
 */
unsigned int Options::loglevel()
{
    return loglevel_;
}


/** Determine whether to run the firewall/router or note.
 *
 *  \retval true Run the firewall/router.
 *  \retval false Don't run the firewall/router.
 */
bool Options::run()
{
    return run_firewall_;
}


/** Determine if options object is valid.
 *
 *  \retval true If the options object sucessfully parsed the command line
 *      arguments.
 *  \retval false If the program should exit imediatly.
 */
Options::operator bool() const
{
    return continue_;
}


/** Find the configuration file.
 *
 *  1. The target of the `MAVTABLES_CONFIG_PATH` environment variable.
 *  2. `.mavtablesrc` in the current directory.
 *  3. `.mavtablesrc` at `$HOME/.mavtablesrc`.
 *  4. The main configuration file at `PREFIX/etc/mavtables.conf`.
 *
 *  \param filesystem A filesystem instance.  The default is to construct an
 *      instance.
 *  \returns The path the first configuration file found.  {} if no
 *      configuration file could be found.
 */
std::optional<std::string> find_config(const Filesystem &filesystem)
{
    // Check MATABLES_CONFIG_PATH.
    if (auto config_path = std::getenv("MAVTABLES_CONFIG_PATH"))
    {
        if (filesystem.exists(Filesystem::path(config_path)))
        {
            return config_path;
        }
    }

    // Check for .mavtablesrc in current directory.
    if (filesystem.exists(".mavtablesrc"))
    {
        return ".mavtablesrc";
    }

    // Check for .mavtablesrc in home directory.
    if (auto home = std::getenv("HOME"))
    {
        Filesystem::path config_path(home);
        config_path /= ".mavtablesrc";

        if (filesystem.exists(config_path))
        {
            return config_path.string();
        }
    }

    // Check for PREFIX/etc/mavtables.conf.
    if (filesystem.exists(PREFIX "/etc/mavtables.conf"))
    {
        return PREFIX "/etc/mavtables.conf";
    }

    return {};
}
