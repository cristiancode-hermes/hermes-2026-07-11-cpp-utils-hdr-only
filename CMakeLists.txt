cmake_minimum_required(VERSION 3.28)
project(cpp_utils VERSION 1.0.0 LANGUAGES CXX
  DESCRIPTION "Header-only C++20 utility library — scope_guard, enum_flags, str, expected_ext, overloaded"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Dependencies (via FetchContent — no vcpkg/Conan needed) ──
include(FetchContent)

# spdlog (header-only logging)
FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.15.1
  SYSTEM
)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "Use bundled fmt")
FetchContent_MakeAvailable(spdlog)

# nlohmann_json (header-only JSON for demo/test)
FetchContent_Declare(nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
  SYSTEM
)
FetchContent_MakeAvailable(nlohmann_json)

# Catch2 (test framework)
FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.0
  SYSTEM
)
FetchContent_MakeAvailable(Catch2)

# ── Header-only library target (INTERFACE = no compile, just propagate) ──
add_library(cpp_utils INTERFACE)
target_include_directories(cpp_utils INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(cpp_utils INTERFACE
  spdlog::spdlog
  nlohmann_json::nlohmann_json
)
# Ensure C++20 concepts are available
target_compile_features(cpp_utils INTERFACE cxx_std_20)

# ── Demo executable ──
add_executable(cpp_utils_demo src/main.cpp)
target_link_libraries(cpp_utils_demo PRIVATE cpp_utils)

# ── Tests ──
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ── Install ──
install(TARGETS cpp_utils_demo DESTINATION bin)
install(DIRECTORY include/cpp_utils DESTINATION include)

# ── Packaging ──
include(CPack)
set(CPACK_PACKAGE_NAME "cpp-utils")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
