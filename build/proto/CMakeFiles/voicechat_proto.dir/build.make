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
CMAKE_SOURCE_DIR = /home/sua/voice_chat

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sua/voice_chat/build

# Include any dependencies generated for this target.
include proto/CMakeFiles/voicechat_proto.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include proto/CMakeFiles/voicechat_proto.dir/compiler_depend.make

# Include the progress variables for this target.
include proto/CMakeFiles/voicechat_proto.dir/progress.make

# Include the compile flags for this target's objects.
include proto/CMakeFiles/voicechat_proto.dir/flags.make

proto/voice_message.pb.h: ../proto/voice_message.proto
proto/voice_message.pb.h: /usr/bin/protoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/sua/voice_chat/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Running cpp protocol buffer compiler on voice_message.proto"
	cd /home/sua/voice_chat/build/proto && /usr/bin/protoc --cpp_out /home/sua/voice_chat/build/proto -I /home/sua/voice_chat/proto /home/sua/voice_chat/proto/voice_message.proto

proto/voice_message.pb.cc: proto/voice_message.pb.h
	@$(CMAKE_COMMAND) -E touch_nocreate proto/voice_message.pb.cc

proto/CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.o: proto/CMakeFiles/voicechat_proto.dir/flags.make
proto/CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.o: proto/voice_message.pb.cc
proto/CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.o: proto/CMakeFiles/voicechat_proto.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sua/voice_chat/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object proto/CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.o"
	cd /home/sua/voice_chat/build/proto && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT proto/CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.o -MF CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.o.d -o CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.o -c /home/sua/voice_chat/build/proto/voice_message.pb.cc

proto/CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.i"
	cd /home/sua/voice_chat/build/proto && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sua/voice_chat/build/proto/voice_message.pb.cc > CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.i

proto/CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.s"
	cd /home/sua/voice_chat/build/proto && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sua/voice_chat/build/proto/voice_message.pb.cc -o CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.s

# Object files for target voicechat_proto
voicechat_proto_OBJECTS = \
"CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.o"

# External object files for target voicechat_proto
voicechat_proto_EXTERNAL_OBJECTS =

lib/libvoicechat_proto.a: proto/CMakeFiles/voicechat_proto.dir/voice_message.pb.cc.o
lib/libvoicechat_proto.a: proto/CMakeFiles/voicechat_proto.dir/build.make
lib/libvoicechat_proto.a: proto/CMakeFiles/voicechat_proto.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sua/voice_chat/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library ../lib/libvoicechat_proto.a"
	cd /home/sua/voice_chat/build/proto && $(CMAKE_COMMAND) -P CMakeFiles/voicechat_proto.dir/cmake_clean_target.cmake
	cd /home/sua/voice_chat/build/proto && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/voicechat_proto.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
proto/CMakeFiles/voicechat_proto.dir/build: lib/libvoicechat_proto.a
.PHONY : proto/CMakeFiles/voicechat_proto.dir/build

proto/CMakeFiles/voicechat_proto.dir/clean:
	cd /home/sua/voice_chat/build/proto && $(CMAKE_COMMAND) -P CMakeFiles/voicechat_proto.dir/cmake_clean.cmake
.PHONY : proto/CMakeFiles/voicechat_proto.dir/clean

proto/CMakeFiles/voicechat_proto.dir/depend: proto/voice_message.pb.cc
proto/CMakeFiles/voicechat_proto.dir/depend: proto/voice_message.pb.h
	cd /home/sua/voice_chat/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sua/voice_chat /home/sua/voice_chat/proto /home/sua/voice_chat/build /home/sua/voice_chat/build/proto /home/sua/voice_chat/build/proto/CMakeFiles/voicechat_proto.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : proto/CMakeFiles/voicechat_proto.dir/depend

