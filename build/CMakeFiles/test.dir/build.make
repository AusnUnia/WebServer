# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ausn/Desktop/Server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ausn/Desktop/Server/build

# Include any dependencies generated for this target.
include CMakeFiles/test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/test.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test.dir/flags.make

CMakeFiles/test.dir/test.cpp.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/test.cpp.o: ../test.cpp
CMakeFiles/test.dir/test.cpp.o: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ausn/Desktop/Server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test.dir/test.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/test.cpp.o -MF CMakeFiles/test.dir/test.cpp.o.d -o CMakeFiles/test.dir/test.cpp.o -c /home/ausn/Desktop/Server/test.cpp

CMakeFiles/test.dir/test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/test.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ausn/Desktop/Server/test.cpp > CMakeFiles/test.dir/test.cpp.i

CMakeFiles/test.dir/test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/test.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ausn/Desktop/Server/test.cpp -o CMakeFiles/test.dir/test.cpp.s

CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.o: ../mysql_connection_pool/mysql_connection_pool.cpp
CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.o: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ausn/Desktop/Server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.o -MF CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.o.d -o CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.o -c /home/ausn/Desktop/Server/mysql_connection_pool/mysql_connection_pool.cpp

CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ausn/Desktop/Server/mysql_connection_pool/mysql_connection_pool.cpp > CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.i

CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ausn/Desktop/Server/mysql_connection_pool/mysql_connection_pool.cpp -o CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.s

CMakeFiles/test.dir/timer/timer.cpp.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/timer/timer.cpp.o: ../timer/timer.cpp
CMakeFiles/test.dir/timer/timer.cpp.o: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ausn/Desktop/Server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/test.dir/timer/timer.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/timer/timer.cpp.o -MF CMakeFiles/test.dir/timer/timer.cpp.o.d -o CMakeFiles/test.dir/timer/timer.cpp.o -c /home/ausn/Desktop/Server/timer/timer.cpp

CMakeFiles/test.dir/timer/timer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/timer/timer.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ausn/Desktop/Server/timer/timer.cpp > CMakeFiles/test.dir/timer/timer.cpp.i

CMakeFiles/test.dir/timer/timer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/timer/timer.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ausn/Desktop/Server/timer/timer.cpp -o CMakeFiles/test.dir/timer/timer.cpp.s

CMakeFiles/test.dir/http/http_connection.cpp.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/http/http_connection.cpp.o: ../http/http_connection.cpp
CMakeFiles/test.dir/http/http_connection.cpp.o: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ausn/Desktop/Server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/test.dir/http/http_connection.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/http/http_connection.cpp.o -MF CMakeFiles/test.dir/http/http_connection.cpp.o.d -o CMakeFiles/test.dir/http/http_connection.cpp.o -c /home/ausn/Desktop/Server/http/http_connection.cpp

CMakeFiles/test.dir/http/http_connection.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/http/http_connection.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ausn/Desktop/Server/http/http_connection.cpp > CMakeFiles/test.dir/http/http_connection.cpp.i

CMakeFiles/test.dir/http/http_connection.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/http/http_connection.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ausn/Desktop/Server/http/http_connection.cpp -o CMakeFiles/test.dir/http/http_connection.cpp.s

CMakeFiles/test.dir/server.cpp.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/server.cpp.o: ../server.cpp
CMakeFiles/test.dir/server.cpp.o: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ausn/Desktop/Server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/test.dir/server.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/server.cpp.o -MF CMakeFiles/test.dir/server.cpp.o.d -o CMakeFiles/test.dir/server.cpp.o -c /home/ausn/Desktop/Server/server.cpp

CMakeFiles/test.dir/server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/server.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ausn/Desktop/Server/server.cpp > CMakeFiles/test.dir/server.cpp.i

CMakeFiles/test.dir/server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/server.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ausn/Desktop/Server/server.cpp -o CMakeFiles/test.dir/server.cpp.s

CMakeFiles/test.dir/config.cpp.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/config.cpp.o: ../config.cpp
CMakeFiles/test.dir/config.cpp.o: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ausn/Desktop/Server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/test.dir/config.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/config.cpp.o -MF CMakeFiles/test.dir/config.cpp.o.d -o CMakeFiles/test.dir/config.cpp.o -c /home/ausn/Desktop/Server/config.cpp

CMakeFiles/test.dir/config.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/config.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ausn/Desktop/Server/config.cpp > CMakeFiles/test.dir/config.cpp.i

CMakeFiles/test.dir/config.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/config.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ausn/Desktop/Server/config.cpp -o CMakeFiles/test.dir/config.cpp.s

# Object files for target test
test_OBJECTS = \
"CMakeFiles/test.dir/test.cpp.o" \
"CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.o" \
"CMakeFiles/test.dir/timer/timer.cpp.o" \
"CMakeFiles/test.dir/http/http_connection.cpp.o" \
"CMakeFiles/test.dir/server.cpp.o" \
"CMakeFiles/test.dir/config.cpp.o"

# External object files for target test
test_EXTERNAL_OBJECTS =

test: CMakeFiles/test.dir/test.cpp.o
test: CMakeFiles/test.dir/mysql_connection_pool/mysql_connection_pool.cpp.o
test: CMakeFiles/test.dir/timer/timer.cpp.o
test: CMakeFiles/test.dir/http/http_connection.cpp.o
test: CMakeFiles/test.dir/server.cpp.o
test: CMakeFiles/test.dir/config.cpp.o
test: CMakeFiles/test.dir/build.make
test: CMakeFiles/test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ausn/Desktop/Server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX executable test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test.dir/build: test
.PHONY : CMakeFiles/test.dir/build

CMakeFiles/test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test.dir/clean

CMakeFiles/test.dir/depend:
	cd /home/ausn/Desktop/Server/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ausn/Desktop/Server /home/ausn/Desktop/Server /home/ausn/Desktop/Server/build /home/ausn/Desktop/Server/build /home/ausn/Desktop/Server/build/CMakeFiles/test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test.dir/depend

