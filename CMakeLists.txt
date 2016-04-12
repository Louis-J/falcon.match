project(ctmatch)
cmake_minimum_required(VERSION 2.8)


if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  include(CMakeDefinitions.txt)

  if (DEFINED ASAN)
    add_definitions(-fsanitize=address)
    link_libraries(-lasan)
  endif()
  if (DEFINED DEBUG_FULL)
    add_definitions(-D_GLIBCXX_DEBUG)
  endif()
endif()

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -iquote ${PROJECT_SOURCE_DIR}/")

# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

include_directories(include/)

# aux_source_directory(. SRC_LIST)
# file(GLOB headers_SRC "*.hpp")


# Tests
function(add_executable_test name)
  add_executable(${name}_test test/${name}.cpp)
  add_test(match_test ${name}_test)
endfunction()

add_executable_test(match)

enable_testing()


install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/falcon/ DESTINATION .)
