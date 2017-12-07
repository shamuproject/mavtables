Contributing {#contributing}
============================

When contributing to this repository, please first discuss the change you wish
to make via issue, email, or any other method with the owners of this repository
before making a change. 


## Pull Request Process

1. Create a pull request early in development processes with a checkbox list of
   features/changes you plan to make along with a description of what you intend
   to do.  Prepend (WIP:) for "work in progress" to the title of this pull request
   until you are ready for review and merging.
2. Update README.rst and the Sphinx documentation with detains of any changes to
   the interface, this includes new environment variables, exposed ports, and
   useful file locations.
3. Ensure test coverage did not drop below 90% and that all unit tests pass.
4. Ensure the package builds, installs, and runs using `pip install .` in
   a clean `virtualenv`.  List any new external dependencies in the README.rst
   file.
5. Increase the version numbers in any examples files and the README.rst to the
   new version that this Pull Request would represent. The versioning scheme we
   use is [SemVer](http://semver.org/).
6. You may merge the Pull Request in once you have the sign-off of a core
   developer, or if you do not have permission to do that, you may request the
   reviewer merge it for you.


## Git Etiquette

* Do not work on the main repository unless you are merging in changes, 
  preparing for a version bump, or other repository management.
  * Create a fork, make a new branch and work on your changes there.  Merge
    these changes in using the _Pull Request Process_ above.
* Do not commit IDE/Editor specific files or any generated files.  You should
  add these files to the `.gitignore` file to ensure you do not accidentally
  commit them.
* Do not commit application specific configuration files.  If you wish to submit
  an example place it in the examples directory.
* Do not commit assets such as pictures or videos.  The exception is small
  (under 300 kB) images used in the documentation.
* Do not commit other project's documentation such as PDF files.
  * Link to that projects documentation instead.
* Do not commit another project's source code.
  * Use a [Git submodule](https://git-scm.com/docs/git-submodule) instead.


## Programming Style

* All C++17 features are allowed.
* Simply running the `style` rule of the Makefile will fix most style issues and
  should be used to ensure consistent style.  This target will be ran before
  a merge to check for style consistency but does not check naming conventions.
* All type names such as classes, structs, type aliases, enums, and type
  template parameters should be `CamelCase`.
* Functions, variables, and namespaces should be `snake_case`.
* No name may begin with a single `_` (underscore) or contain `__` (double
  underscore).
* Private data members and functions should end in an underscore.
  have an `autodoc` entry in the Sphinx documentation.
* Each class should have it's own source and header file.
* When a file contains a class the file should be named for the class.  All
  other file names should be lower case and should only include letters,
  numbers, and underscores.
* C++ source code files should use the `.cpp` extension and `.hpp` for header
  files.  `.c` and `.h` should only be used for C files.
* All header files must have include guards and the name should be the file name
  (all uppercase) with `_HPP_` appended.
* All public functions and classes should contain [Doxygen style
  comments](https://www.stack.nl/~dimitri/doxygen/manual/docblocks.html) and the
  recommended block format is

      \**
       *
       */

  And the `\` (backslash) is recommended over the `@` (at symbol).
