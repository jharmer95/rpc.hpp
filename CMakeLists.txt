cmake_minimum_required(VERSION 3.1...3.19)

project(
  "rpc.hpp"
  VERSION 0.4.1
  DESCRIPTION "Simple RPC Header-Only Library"
  LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
  set(TARGET_WINDOWS TRUE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CXX_MSVC TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_MINGW TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_CLANG TRUE)
  endif()
elseif(UNIX AND NOT APPLE)
  set(TARGET_LINUX TRUE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_GCC TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_CLANG TRUE)
  endif()
elseif(APPLE)
  set(TARGET_APPLE TRUE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_GCC TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_CLANG TRUE)
  endif()
endif()

if(CXX_MSVC)
  set(FULL_WARNING
      /W4
      /experimental:external
      /external:W0
      /external:anglebrackets
      /permissive-
      /wd4619
      /w14242
      /w14254
      /w14263
      /w14265
      /w14287
      /we4289
      /w14296
      /w14311
      /w14545
      /w14546
      /w14547
      /w14549
      /w14555
      /w14640
      /w14826
      /w14905
      /w14906
      /w14928
      /we4834
      /bigobj
      -D_WIN32_WINNT=0x0A00)
  set(MIN_WARNING
      /W1
      /experimental:external
      /external:W0
      /external:anglebrackets
      /wd4619
      /bigobj
      -D_WIN32_WINNT=0x0A00)
elseif(CXX_GCC OR CXX_MINGW)
  set(FULL_WARNING
      -Wall
      -Wextra
      -Wpedantic
      -Wshadow
      -Wnon-virtual-dtor
      -Wold-style-cast
      -Wcast-align
      -Wunused
      -Woverloaded-virtual
      -Wconversion
      -Wsign-conversion
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
      -Weffc++
      -Wuseless-cast
      -Wlogical-op
      -Wduplicated-branches
      -Wduplicated-cond
      -Wmisleading-indentation)
  set(MIN_WARNING -Wall)
  if(CXX_MINGW)
    list(APPEND FULL_WARNING -Wa,-mbig-obj)
    list(APPEND MIN_WARNING -Wa,-mbig-obj)
  endif()
elseif(CXX_CLANG)
  set(FULL_WARNING
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-c++98-c++11-compat
      -Wno-c++98-c++11-compat-binary-literal
      -Wno-c++98-c++11-c++14-compat
      -Wno-padded
      -Wno-exit-time-destructors
      -Wno-missing-prototypes
      -Wno-documentation
      -Wno-documentation-unknown-command
      -Wno-weak-vtables
      -Wno-reserved-id-macro)
  set(MIN_WARNING -Wall)
endif()

include(CTest)
include(Catch)

option(GENERATE_DOXYGEN "Generate Doxygen documentation from comments" OFF)

if(${GENERATE_DOXYGEN})
  find_package(Doxygen OPTIONAL_COMPONENTS dot)

  if(DOXYGEN_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_HTML_OUTPUT "docs")
    set(DOXYGEN_EXCLUDE_PATTERNS "*/tests/*")
    set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "README.md")
    doxygen_add_docs(docs ${PROJECT_SOURCE_DIR}
                     COMMENT "Generate doxygen docs for rpc.hpp")
  endif()
endif()

option(USE_CONAN "Use Conan to manage C/C++ dependencies" OFF)
option(USE_VCPKG "Use vcpkg to manage C/C++ dependencies" OFF)

if(${USE_CONAN})
  if(${USE_VCPKG})
    message(FATAL_ERROR "Using both vcpkg and Conan is not supported!")
  endif()

  set(CONAN_EXTRA_REQUIRES "")
  set(CONAN_EXTRA_OPTIONS "")
  include(cmake/Conan.cmake)
endif()

add_library(coverage_config INTERFACE)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(coverage_config INTERFACE -O0 -g --coverage)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(coverage_config INTERFACE --coverage)
  else()
    target_link_libraries(coverage_config INTERFACE --coverage)
  endif()
endif()

find_package(Threads)

add_library(rpc_hpp INTERFACE)
target_include_directories(rpc_hpp INTERFACE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(rpc_hpp INTERFACE ${CMAKE_THREAD_LIBS_INIT})
install(FILES "${PROJECT_SOURCE_DIR}/include/rpc.hpp"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/include")

option(BUILD_NJSON_ADAPTER "Build the adapter for nlohmann/json" ON)
option(BUILD_RAPIDJSON_ADAPTER "Build the adapter for rapidjson" ON)
option(BUILD_BOOST_JSON_ADAPTER "Build the adapter for Boost.JSON" OFF)

add_subdirectory(include/rpc_adapters)

option(BUILD_TESTING "Build the testing tree." ON)
if(BUILD_TESTING AND (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
  enable_testing()
  add_subdirectory(tests)
endif()
