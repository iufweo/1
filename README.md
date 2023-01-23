# Lox1

Lox1 is a tree-walking interpreter written in C++20, it implements
a garbage-collected language based on the Lox language
described in Bob Nystrom's book
[Crafting Interpreters](https://craftinginterpreters.com).

## Building

### Non-Windows targets
Supported compilers are GNU C++ and Clang. Code is known to build and function
correctly when compiled with gcc-12 or clang-15.

	cmake -S path_to_source_directory/ -B path_to_build_directory/
	cmake --build path_to_build_directory/

Tests can be run by executing

	path_to_build_directory/testdriver path_to_build_directory/lox1 path_to_source_directory/test/

### Windows and MSYS2

Only GNU C++ is supported with MSYS2.
Code is known to build and function correctly when compiled with gcc-12
on Windows 10.

	cmake -S path_to_source_directory/ -B path_to_build_directory/
	cmake --build path_to_build_directory/

**Tests must not be run using msys runtime**.

Some tests may "fail" due to Windows reordering stderr with stdout.

	path_to_build_directory/testdriver.exe path_to_build_directory/lox1.exe path_to_source_directory/test/

### Windows and MSVC

Code is known to build and function correctly when compiled with MSVC v142
for Windows 10.

Select MSVC by specifying Visual Studio edition as the generator.

	cmake -S path_to_source_directory/ -B path_to_build_directory/ -G "Visual Studio 16 2019"
	cmake --build path_to_build_directory/ --config Release

Building the test driver used for tests is not supported with MSVC.

## Usage

Give no arguments for REPL mode, or a path to file to execute it.