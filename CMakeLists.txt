cmake_minimum_required(VERSION 3.27)
project(Dodo_Lang)

set(CMAKE_CXX_STANDARD 23)

file (GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "Dodo-lang/src/*.cpp")
file (GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "Dodo-lang/src/*.hpp")

set (INCLUDE_DIRS "")
foreach (_headerFile ${HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list (REMOVE_DUPLICATES INCLUDE_DIRS)

include_directories(${INCLUDE_DIRS})

add_executable(Dodo_Lang ${SOURCES})

add_custom_command(
        OUTPUT Dodo-lang/src/Misc/Increment.hpp
        PRE_BUILD
        COMMAND ./increment.sh
        COMMAND echo running
        DEPENDS BuildNumberCache.txt
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_custom_target(IncrementBuildNumber ALL DEPENDS Dodo-lang/src/Misc/Increment.hpp)

add_dependencies(Dodo_Lang IncrementBuildNumber)

# run this in build folder to create the executable and get the return value
# as out.s -o out.o && ld out.o -o out && ./out || echo $?

# example compilation cli arguments
# dodoc dodotest.dodo -o dodotest -l3 -import=../
# to create a stdlib link when in cmake dir
# sudo ln -s <PATH>/Dodo-lang/Stdlib/ /usr/include/DodoLang