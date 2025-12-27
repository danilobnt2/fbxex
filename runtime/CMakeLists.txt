cmake_minimum_required(VERSION 3.20)

project(fbxex CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

set(FBXEX_VERSION "" CACHE STRING "Override app version string (default: git working tree hash)")

if(FBXEX_VERSION STREQUAL "")
  find_package(Git QUIET)

  if(GIT_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --show-toplevel
      WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
      OUTPUT_VARIABLE _fbxex_git_root
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE _fbxex_git_root_result
    )

    if(_fbxex_git_root_result EQUAL 0 AND NOT "${_fbxex_git_root}" STREQUAL "")
      execute_process(
        COMMAND ${GIT_EXECUTABLE} write-tree
        WORKING_DIRECTORY "${_fbxex_git_root}"
        OUTPUT_VARIABLE _fbxex_saved_index
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _fbxex_save_index_result
      )

      if(_fbxex_save_index_result EQUAL 0)
        execute_process(
          COMMAND ${GIT_EXECUTABLE} add runtime
          WORKING_DIRECTORY "${_fbxex_git_root}"
          OUTPUT_QUIET
          ERROR_QUIET
          RESULT_VARIABLE _fbxex_git_add_result
        )

        execute_process(
          COMMAND ${GIT_EXECUTABLE} write-tree
          WORKING_DIRECTORY "${_fbxex_git_root}"
          OUTPUT_VARIABLE _fbxex_worktree_tree
          OUTPUT_STRIP_TRAILING_WHITESPACE
          RESULT_VARIABLE _fbxex_worktree_result
        )

        # Restore the original index regardless of the staging outcome
        execute_process(
          COMMAND ${GIT_EXECUTABLE} read-tree ${_fbxex_saved_index}
          WORKING_DIRECTORY "${_fbxex_git_root}"
          OUTPUT_QUIET
          ERROR_QUIET
        )

        if(_fbxex_git_add_result EQUAL 0 AND _fbxex_worktree_result EQUAL 0)
          execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short ${_fbxex_worktree_tree}
            WORKING_DIRECTORY "${_fbxex_git_root}"
            OUTPUT_VARIABLE _fbxex_git_short_hash
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE _fbxex_git_short_result
          )

          if(_fbxex_git_short_result EQUAL 0 AND NOT "${_fbxex_git_short_hash}" STREQUAL "")
            set(FBXEX_VERSION "${_fbxex_git_short_hash}")
          endif()
        endif()
      endif()
    endif()
  endif()

  if(FBXEX_VERSION STREQUAL "")
    set(FBXEX_VERSION "unknown")
  endif()
endif()

message(STATUS "fbxex version: ${FBXEX_VERSION}")

set(FBXEX_DEV_PORT "5173" CACHE STRING "Port used for the run-dev helper target")

include(FetchContent)

option(FBXEX_ENABLE_COVERAGE "Enable code coverage flags for test targets (non-MSVC toolchains)" OFF)

function(fbxex_enable_coverage target)
  if(FBXEX_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
      target_compile_options(${target} PRIVATE -O0 -g --coverage)
      target_link_options(${target} PRIVATE --coverage)
    elseif(MSVC)
      # Coverage flags not configured for MSVC; skip quietly.
      message(STATUS "Coverage skipped for target ${target} (MSVC toolchain).")
    endif()
  endif()
endfunction()

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
)

FetchContent_MakeAvailable(Catch2)

# Ultralight SDK path - modify this or set via -DULTRALIGHT_SDK_PATH
if(NOT DEFINED ULTRALIGHT_SDK_PATH)
  # Try common locations
  if(EXISTS "$ENV{PROGRAMFILES}/Ultralight/SDK")
    set(ULTRALIGHT_SDK_PATH "$ENV{PROGRAMFILES}/Ultralight/SDK")
  elseif(EXISTS "$ENV{HOME}/Ultralight/SDK")
    set(ULTRALIGHT_SDK_PATH "$ENV{HOME}/Ultralight/SDK")
  else()
    message(FATAL_ERROR 
      "Ultralight SDK not found. Please set ULTRALIGHT_SDK_PATH:\n"
      "  cmake -B build -DULTRALIGHT_SDK_PATH=/path/to/ultralight-sdk\n"
      "\n"
      "You can download the SDK from: https://ultralig.ht/download"
    )
  endif()
endif()

if(NOT EXISTS "${ULTRALIGHT_SDK_PATH}/include/Ultralight")
  message(FATAL_ERROR 
    "Invalid Ultralight SDK path: ${ULTRALIGHT_SDK_PATH}\n"
    "Expected to find: ${ULTRALIGHT_SDK_PATH}/include/Ultralight"
  )
endif()

message(STATUS "Ultralight SDK: ${ULTRALIGHT_SDK_PATH}")

# Autodesk FBX SDK path - modify this or set via -DFBX_SDK_PATH
if(NOT DEFINED FBX_SDK_PATH)
  set(_fbx_candidate_roots
    "$ENV{HOME}/Autodesk/FBX/FBX SDK"
    "$ENV{PROGRAMFILES}/Autodesk/FBX/FBX SDK"
  )
  foreach(_root IN LISTS _fbx_candidate_roots)
    if(NOT _root)
      continue()
    endif()
    if(EXISTS "${_root}/include/fbxsdk.h")
      set(FBX_SDK_PATH "${_root}")
      break()
    endif()
  endforeach()
endif()

if(NOT DEFINED FBX_SDK_PATH OR NOT EXISTS "${FBX_SDK_PATH}/include/fbxsdk.h")
  message(FATAL_ERROR
    "Autodesk FBX SDK not found. Please set FBX_SDK_PATH:\n"
    "  cmake -B build -DFBX_SDK_PATH=C:/path/to/Autodesk/FBX_SDK\n"
    "\n"
    "Download the SDK from: https://aps.autodesk.com/developer/overview/fbx-sdk"
  )
endif()

set(FBX_SDK_LIB_BASE "${FBX_SDK_PATH}/lib/x64")

function(_fbx_pick_library out_var base_dir config)
  set(_names ${ARGN})
  foreach(_name IN LISTS _names)
    set(_candidate "${base_dir}/${config}/${_name}")
    if(EXISTS "${_candidate}")
      set(${out_var} "${_candidate}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  set(${out_var} "" PARENT_SCOPE)
endfunction()

_fbx_pick_library(FBX_SDK_LIBRARY_RELEASE "${FBX_SDK_LIB_BASE}" "release" "libfbxsdk.lib" "libfbxsdk-md.lib" "libfbxsdk-mt.lib")
_fbx_pick_library(FBX_SDK_XML_RELEASE "${FBX_SDK_LIB_BASE}" "release" "libxml2.lib" "libxml2-md.lib" "libxml2-mt.lib")
_fbx_pick_library(FBX_SDK_ZLIB_RELEASE "${FBX_SDK_LIB_BASE}" "release" "zlib.lib" "zlib-md.lib" "zlib-mt.lib")

foreach(_required_var IN ITEMS FBX_SDK_LIBRARY_RELEASE FBX_SDK_XML_RELEASE FBX_SDK_ZLIB_RELEASE)
  if(NOT DEFINED ${_required_var} OR "${${_required_var}}" STREQUAL "")
    message(FATAL_ERROR "Missing FBX SDK library (${_required_var}). Checked under ${FBX_SDK_LIB_BASE}")
  endif()
endforeach()

message(STATUS "FBX SDK: ${FBX_SDK_PATH}")
message(STATUS "FBX SDK toolchain: ${FBX_SDK_TOOLCHAIN_DIR}")

# Create executable
if(WIN32)
  add_executable(fbxex WIN32
    src/main.cpp
    src/winmain.cpp
    src/fbxclienteager.cpp
    src/fbxexappmain.cpp
    src/fbxdtserialize.cpp
    src/winfio.cpp
    src/binders.cpp
    src/resource.rc
  )
else()
  message(FATAL_ERROR "This application currently only supports Windows.")
endif()

# Set up include directories
target_include_directories(fbxex PRIVATE
  "${ULTRALIGHT_SDK_PATH}/include"
  "${FBX_SDK_PATH}/include"
)

target_compile_definitions(fbxex PRIVATE
  FBXEX_VERSION="${FBXEX_VERSION}"
  FBXSDK_SHARED
)

# Link libraries
if(WIN32)
  target_link_directories(fbxex PRIVATE "${ULTRALIGHT_SDK_PATH}/lib")
  target_link_libraries(fbxex PRIVATE
    AppCore.lib
    UltralightCore.lib
    Ultralight.lib
    WebCore.lib
    nlohmann_json::nlohmann_json
    ${FBX_SDK_LIBRARY_RELEASE}
    ${FBX_SDK_XML_RELEASE}
    ${FBX_SDK_ZLIB_RELEASE}
  )
  
  # Copy DLLs to build output directory
  add_custom_command(TARGET fbxex POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ULTRALIGHT_SDK_PATH}/bin/AppCore.dll"
    "$<TARGET_FILE_DIR:fbxex>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ULTRALIGHT_SDK_PATH}/bin/UltralightCore.dll"
    "$<TARGET_FILE_DIR:fbxex>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ULTRALIGHT_SDK_PATH}/bin/Ultralight.dll"
    "$<TARGET_FILE_DIR:fbxex>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ULTRALIGHT_SDK_PATH}/bin/WebCore.dll"
    "$<TARGET_FILE_DIR:fbxex>/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${ULTRALIGHT_SDK_PATH}/resources"
    "$<TARGET_FILE_DIR:fbxex>/assets/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FBX_SDK_LIB_BASE}/release/libfbxsdk.dll"
    "$<TARGET_FILE_DIR:fbxex>/"
  )
  else()
    # error for unsupported platforms
    message(FATAL_ERROR "This application currently only supports Windows.")
endif()

# Copy assets to build directory
add_custom_command(TARGET fbxex POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/assets"
  "$<TARGET_FILE_DIR:fbxex>/assets"
  COMMENT "Copying assets...")

if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()

add_custom_target(run-dev
  DEPENDS fbxex
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} -E echo "Starting fbxex in dev mode on http://localhost:${FBXEX_DEV_PORT}"
  COMMAND $<TARGET_FILE:fbxex> --dev --dev-port ${FBXEX_DEV_PORT}
  VERBATIM
  COMMENT "Build and run the app against the dev UI server")

# For MSVC, set startup project
if(MSVC)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT fbxex)
endif()
