project(falcon.regex-dfa)
cmake_minimum_required(VERSION 2.8)


include(CMakeDefinitions.txt)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -iquote ${PROJECT_SOURCE_DIR}/")

# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

if (DEFINED ASAN)
  add_definitions(-fsanitize=address)
  link_libraries(-lasan)
endif()
if (DEFINED DEBUG_FULL)
  add_definitions(-D_GLIBCXX_DEBUG)
endif()

include_directories(include/)

# aux_source_directory(. SRC_LIST)
# file(GLOB headers_SRC "*.hpp")


# Tests
function(add_executable_test name)
  add_executable(test_${name} test/${name}.cpp)
  add_test(match_test test_${name})
endfunction()

add_executable_test(match)

enable_testing()
