#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <cstring>

#include <ufbx.h>

#include "fbxdtserialize.hpp"

namespace {

ufbx_string MakeUfbxString(const char* value) {
    return {value, std::strlen(value)};
}

} // namespace

TEST_CASE("SerializeFbxProperty handles core ufbx types") {
    SECTION("boolean property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("visible");
        prop.type = UFBX_PROP_BOOLEAN;
        prop.value_int = 1;

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "visible");
        REQUIRE(json.at("type") == "UFBX_PROP_BOOLEAN");
        REQUIRE(json.at("value").get<bool>() == true);
    }

    SECTION("integer property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("count");
        prop.type = UFBX_PROP_INTEGER;
        prop.value_int = 42;

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "count");
        REQUIRE(json.at("type") == "UFBX_PROP_INTEGER");
        REQUIRE(json.at("value").get<long long>() == 42);
    }

    SECTION("number property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("weight");
        prop.type = UFBX_PROP_NUMBER;
        prop.value_real = 2.5;

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "weight");
        REQUIRE(json.at("type") == "UFBX_PROP_NUMBER");
        REQUIRE(json.at("value").get<double>() == 2.5);
    }

    SECTION("vector property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("translation");
        prop.type = UFBX_PROP_VECTOR;
        prop.value_vec3 = {1.0, 2.0, 3.5};

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "translation");
        REQUIRE(json.at("type") == "UFBX_PROP_VECTOR");
        REQUIRE(json.at("value").is_array());
        REQUIRE(json.at("value").size() == 3);
        REQUIRE(json.at("value")[0].get<double>() == 1.0);
        REQUIRE(json.at("value")[1].get<double>() == 2.0);
        REQUIRE(json.at("value")[2].get<double>() == 3.5);
    }

    SECTION("color with alpha property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("albedo");
        prop.type = UFBX_PROP_COLOR_WITH_ALPHA;
        prop.value_vec4 = {0.0, 0.25, 0.5, 1.0};

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "albedo");
        REQUIRE(json.at("type") == "UFBX_PROP_COLOR_WITH_ALPHA");
        REQUIRE(json.at("value").is_array());
        REQUIRE(json.at("value").size() == 4);
        REQUIRE(json.at("value")[0].get<double>() == 0.0);
        REQUIRE(json.at("value")[1].get<double>() == 0.25);
        REQUIRE(json.at("value")[2].get<double>() == 0.5);
        REQUIRE(json.at("value")[3].get<double>() == 1.0);
    }

    SECTION("string property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("label");
        prop.type = UFBX_PROP_STRING;
        prop.value_str = MakeUfbxString("fbxex");

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "label");
        REQUIRE(json.at("type") == "UFBX_PROP_STRING");
        REQUIRE(json.at("value").get<std::string>() == "fbxex");
    }

    SECTION("date time property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("timestamp");
        prop.type = UFBX_PROP_DATE_TIME;
        prop.value_str = MakeUfbxString("2025-12-31T23:59:59");

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "timestamp");
        REQUIRE(json.at("type") == "UFBX_PROP_DATE_TIME");
        REQUIRE(json.at("value").get<std::string>() == "2025-12-31T23:59:59");
    }
}

TEST_CASE("SerializeFbxProperty returns null for missing value") {
    ufbx_prop prop = {};
    prop.name = MakeUfbxString("missing");
    prop.type = UFBX_PROP_NUMBER;
    prop.flags = UFBX_PROP_FLAG_NO_VALUE;

    const auto json = SerializeFbxProperty(prop);
    REQUIRE(json.at("name") == "missing");
    REQUIRE(json.at("type") == "UFBX_PROP_NUMBER");
    REQUIRE(json.at("value").is_null());
}
