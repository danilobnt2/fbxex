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
  ../src/fbxexappmain.cpp
)

target_include_directories(fbxex_tests PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../src"
  "${FBX_SDK_PATH}/include"
  "${ULTRALIGHT_SDK_PATH}/include"
)

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
catch_discover_tests(fbxex_tests)
