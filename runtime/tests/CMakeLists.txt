add_executable(fbxex_tests
  fbxnode_tests.cpp
  winfio_tests.cpp
  fbxdtserialize_tests.cpp
  fbxclienteager_tests.cpp
  binders_tests.cpp
  fbxexappmain_tests.cpp
  ../src/winfio.cpp
  ../src/binders.cpp
  ../src/fbxdtserialize.cpp
  ../src/fbxclienteager.cpp
  ../src/ultralight_adapters.cpp
  ../src/fbxexappmain.cpp
  ../src/embedded_filesystem.cpp
)

target_include_directories(fbxex_tests PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../src"
  "${FBXEX_GENERATED_DIR}"
)
target_include_directories(fbxex_tests SYSTEM PRIVATE
  "${FBX_SDK_PATH}/include"
  "${ULTRALIGHT_SDK_PATH}/include"
)

if(MSVC)
  target_compile_options(fbxex_tests PRIVATE
    /external:I"${FBX_SDK_PATH}/include"
    /external:I"${ULTRALIGHT_SDK_PATH}/include"
    /external:W0
    /experimental:external
  )
endif()

target_compile_definitions(fbxex_tests PRIVATE FBXSDK_SHARED)

target_link_libraries(fbxex_tests PRIVATE
  Catch2::Catch2WithMain
  nlohmann_json::nlohmann_json
  Ole32
  Comdlg32
  Shell32
  Dwmapi
  AppCore
  UltralightCore
  Ultralight
  WebCore
)

target_link_directories(fbxex_tests PRIVATE "${ULTRALIGHT_SDK_PATH}/lib")

if(DEFINED FBX_SDK_LIBRARY_RELEASE)
  target_link_libraries(fbxex_tests PRIVATE
    ${FBX_SDK_LIBRARY_RELEASE}
    ${FBX_SDK_XML_RELEASE}
    ${FBX_SDK_ZLIB_RELEASE}
  )
endif()

add_custom_command(TARGET fbxex_tests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ULTRALIGHT_SDK_PATH}/bin/AppCore.dll"
    "$<TARGET_FILE_DIR:fbxex_tests>/"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ULTRALIGHT_SDK_PATH}/bin/UltralightCore.dll"
    "$<TARGET_FILE_DIR:fbxex_tests>/"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ULTRALIGHT_SDK_PATH}/bin/Ultralight.dll"
    "$<TARGET_FILE_DIR:fbxex_tests>/"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ULTRALIGHT_SDK_PATH}/bin/WebCore.dll"
    "$<TARGET_FILE_DIR:fbxex_tests>/"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FBX_SDK_LIB_BASE}/release/libfbxsdk.dll"
    "$<TARGET_FILE_DIR:fbxex_tests>/"
)

fbxex_enable_coverage(fbxex_tests)

include(Catch)
# Each Catch2 test case runs as its own CTest process; use %m in the profile
# path so every case writes a unique .profraw instead of overwriting the same file.
if(FBXEX_ENABLE_COVERAGE)
  catch_discover_tests(fbxex_tests
    PROPERTIES
      ENVIRONMENT "LLVM_PROFILE_FILE=$<SHELL_PATH:${CMAKE_CURRENT_BINARY_DIR}/fbxex_tests_%m.profraw>"
  )
else()
  catch_discover_tests(fbxex_tests)
endif()
