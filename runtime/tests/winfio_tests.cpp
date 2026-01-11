#include <catch2/catch_test_macros.hpp>

#include "winfio.hpp"

#ifdef _WIN32

TEST_CASE("WStringToUtf8 converts wide strings to UTF-8") {
    REQUIRE(WStringToUtf8(L"") == "");
    REQUIRE(WStringToUtf8(L"fbx explorer") == "fbx explorer");

    const std::wstring sample = L"Paths/With/Spaces 123";
    REQUIRE(WStringToUtf8(sample) == "Paths/With/Spaces 123");
}

#endif
