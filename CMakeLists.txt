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

# Verify SDK exists
if(NOT EXISTS "${ULTRALIGHT_SDK_PATH}/include/Ultralight")
  message(FATAL_ERROR 
    "Invalid Ultralight SDK path: ${ULTRALIGHT_SDK_PATH}\n"
    "Expected to find: ${ULTRALIGHT_SDK_PATH}/include/Ultralight"
  )
endif()

message(STATUS "Ultralight SDK: ${ULTRALIGHT_SDK_PATH}")

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
target_include_directories(fbxex PRIVATE "${ULTRALIGHT_SDK_PATH}/include")

# Link libraries
if(WIN32)
  target_link_directories(fbxex PRIVATE "${ULTRALIGHT_SDK_PATH}/lib")
  target_link_libraries(fbxex PRIVATE
    AppCore.lib
    UltralightCore.lib
    Ultralight.lib
    WebCore.lib
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
