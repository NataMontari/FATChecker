# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.29

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
CMAKE_COMMAND = /snap/clion/296/bin/cmake/linux/x64/bin/cmake

# The command to remove a file.
RM = /snap/clion/296/bin/cmake/linux/x64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/vbronetskyi/OS/FATChecker

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/vbronetskyi/OS/FATChecker/cmake-build-release

# Include any dependencies generated for this target.
include CMakeFiles/FATChecker.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/FATChecker.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/FATChecker.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/FATChecker.dir/flags.make

CMakeFiles/FATChecker.dir/main.cpp.o: CMakeFiles/FATChecker.dir/flags.make
CMakeFiles/FATChecker.dir/main.cpp.o: /home/vbronetskyi/OS/FATChecker/main.cpp
CMakeFiles/FATChecker.dir/main.cpp.o: CMakeFiles/FATChecker.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/vbronetskyi/OS/FATChecker/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/FATChecker.dir/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/FATChecker.dir/main.cpp.o -MF CMakeFiles/FATChecker.dir/main.cpp.o.d -o CMakeFiles/FATChecker.dir/main.cpp.o -c /home/vbronetskyi/OS/FATChecker/main.cpp

CMakeFiles/FATChecker.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/FATChecker.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/vbronetskyi/OS/FATChecker/main.cpp > CMakeFiles/FATChecker.dir/main.cpp.i

CMakeFiles/FATChecker.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/FATChecker.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/vbronetskyi/OS/FATChecker/main.cpp -o CMakeFiles/FATChecker.dir/main.cpp.s

CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.o: CMakeFiles/FATChecker.dir/flags.make
CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.o: /home/vbronetskyi/OS/FATChecker/includes/AnalyzersFAT16.cpp
CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.o: CMakeFiles/FATChecker.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/vbronetskyi/OS/FATChecker/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.o -MF CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.o.d -o CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.o -c /home/vbronetskyi/OS/FATChecker/includes/AnalyzersFAT16.cpp

CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/vbronetskyi/OS/FATChecker/includes/AnalyzersFAT16.cpp > CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.i

CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/vbronetskyi/OS/FATChecker/includes/AnalyzersFAT16.cpp -o CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.s

CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.o: CMakeFiles/FATChecker.dir/flags.make
CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.o: /home/vbronetskyi/OS/FATChecker/includes/AnalyzersFAT32.cpp
CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.o: CMakeFiles/FATChecker.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/vbronetskyi/OS/FATChecker/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.o -MF CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.o.d -o CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.o -c /home/vbronetskyi/OS/FATChecker/includes/AnalyzersFAT32.cpp

CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/vbronetskyi/OS/FATChecker/includes/AnalyzersFAT32.cpp > CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.i

CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/vbronetskyi/OS/FATChecker/includes/AnalyzersFAT32.cpp -o CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.s

CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.o: CMakeFiles/FATChecker.dir/flags.make
CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.o: /home/vbronetskyi/OS/FATChecker/includes/AnalyzersFAT12.cpp
CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.o: CMakeFiles/FATChecker.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/vbronetskyi/OS/FATChecker/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.o -MF CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.o.d -o CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.o -c /home/vbronetskyi/OS/FATChecker/includes/AnalyzersFAT12.cpp

CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/vbronetskyi/OS/FATChecker/includes/AnalyzersFAT12.cpp > CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.i

CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/vbronetskyi/OS/FATChecker/includes/AnalyzersFAT12.cpp -o CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.s

CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.o: CMakeFiles/FATChecker.dir/flags.make
CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.o: /home/vbronetskyi/OS/FATChecker/includes/FixFAT16.cpp
CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.o: CMakeFiles/FATChecker.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/vbronetskyi/OS/FATChecker/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.o -MF CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.o.d -o CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.o -c /home/vbronetskyi/OS/FATChecker/includes/FixFAT16.cpp

CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/vbronetskyi/OS/FATChecker/includes/FixFAT16.cpp > CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.i

CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/vbronetskyi/OS/FATChecker/includes/FixFAT16.cpp -o CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.s

# Object files for target FATChecker
FATChecker_OBJECTS = \
"CMakeFiles/FATChecker.dir/main.cpp.o" \
"CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.o" \
"CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.o" \
"CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.o" \
"CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.o"

# External object files for target FATChecker
FATChecker_EXTERNAL_OBJECTS =

FATChecker: CMakeFiles/FATChecker.dir/main.cpp.o
FATChecker: CMakeFiles/FATChecker.dir/includes/AnalyzersFAT16.cpp.o
FATChecker: CMakeFiles/FATChecker.dir/includes/AnalyzersFAT32.cpp.o
FATChecker: CMakeFiles/FATChecker.dir/includes/AnalyzersFAT12.cpp.o
FATChecker: CMakeFiles/FATChecker.dir/includes/FixFAT16.cpp.o
FATChecker: CMakeFiles/FATChecker.dir/build.make
FATChecker: CMakeFiles/FATChecker.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/vbronetskyi/OS/FATChecker/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable FATChecker"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/FATChecker.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/FATChecker.dir/build: FATChecker
.PHONY : CMakeFiles/FATChecker.dir/build

CMakeFiles/FATChecker.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/FATChecker.dir/cmake_clean.cmake
.PHONY : CMakeFiles/FATChecker.dir/clean

CMakeFiles/FATChecker.dir/depend:
	cd /home/vbronetskyi/OS/FATChecker/cmake-build-release && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/vbronetskyi/OS/FATChecker /home/vbronetskyi/OS/FATChecker /home/vbronetskyi/OS/FATChecker/cmake-build-release /home/vbronetskyi/OS/FATChecker/cmake-build-release /home/vbronetskyi/OS/FATChecker/cmake-build-release/CMakeFiles/FATChecker.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/FATChecker.dir/depend
