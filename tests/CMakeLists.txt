cmake_minimum_required(VERSION 3.28)
project(tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Catch2 comes from parent FetchContent — don't redeclare
add_executable(test_cpp_utils test_cpp_utils.cpp)
target_link_libraries(test_cpp_utils PRIVATE
  cpp_utils
  Catch2::Catch2WithMain
)

include(CTest)
add_test(NAME test_cpp_utils COMMAND test_cpp_utils)
