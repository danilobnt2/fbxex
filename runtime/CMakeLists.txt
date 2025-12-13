cmake_minimum_required(VERSION 3.20)

project(fbxex CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
  )
else()
  message(FATAL_ERROR "This application currently only supports Windows.")
endif()

# Set up include directories
target_include_directories(fbxex PRIVATE
  "${ULTRALIGHT_SDK_PATH}/include"
  "${FBX_SDK_PATH}/include"
)

# Link libraries
if(WIN32)
  target_link_directories(fbxex PRIVATE "${ULTRALIGHT_SDK_PATH}/lib")
  target_link_libraries(fbxex PRIVATE
    AppCore.lib
    UltralightCore.lib
    Ultralight.lib
    WebCore.lib
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

# For MSVC, set startup project
if(MSVC)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT fbxex)
endif()
