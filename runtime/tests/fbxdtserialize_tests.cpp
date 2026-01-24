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

    SECTION("color property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("tint");
        prop.type = UFBX_PROP_COLOR;
        prop.value_vec3 = {0.1, 0.2, 0.3};

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "tint");
        REQUIRE(json.at("type") == "UFBX_PROP_COLOR");
        REQUIRE(json.at("value").is_array());
        REQUIRE(json.at("value").size() == 3);
        REQUIRE(json.at("value")[0].get<double>() == 0.1);
        REQUIRE(json.at("value")[1].get<double>() == 0.2);
        REQUIRE(json.at("value")[2].get<double>() == 0.3);
    }

    SECTION("translation property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("t");
        prop.type = UFBX_PROP_TRANSLATION;
        prop.value_vec3 = {4.0, 5.0, 6.0};

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "t");
        REQUIRE(json.at("type") == "UFBX_PROP_TRANSLATION");
        REQUIRE(json.at("value").is_array());
        REQUIRE(json.at("value").size() == 3);
        REQUIRE(json.at("value")[0].get<double>() == 4.0);
        REQUIRE(json.at("value")[1].get<double>() == 5.0);
        REQUIRE(json.at("value")[2].get<double>() == 6.0);
    }

    SECTION("rotation property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("r");
        prop.type = UFBX_PROP_ROTATION;
        prop.value_vec3 = {10.0, 20.0, 30.0};

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "r");
        REQUIRE(json.at("type") == "UFBX_PROP_ROTATION");
        REQUIRE(json.at("value").is_array());
        REQUIRE(json.at("value").size() == 3);
        REQUIRE(json.at("value")[0].get<double>() == 10.0);
        REQUIRE(json.at("value")[1].get<double>() == 20.0);
        REQUIRE(json.at("value")[2].get<double>() == 30.0);
    }

    SECTION("scaling property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("s");
        prop.type = UFBX_PROP_SCALING;
        prop.value_vec3 = {1.0, 2.0, 3.0};

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "s");
        REQUIRE(json.at("type") == "UFBX_PROP_SCALING");
        REQUIRE(json.at("value").is_array());
        REQUIRE(json.at("value").size() == 3);
        REQUIRE(json.at("value")[0].get<double>() == 1.0);
        REQUIRE(json.at("value")[1].get<double>() == 2.0);
        REQUIRE(json.at("value")[2].get<double>() == 3.0);
    }

    SECTION("distance property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("d");
        prop.type = UFBX_PROP_DISTANCE;
        prop.value_real = 12.5;

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "d");
        REQUIRE(json.at("type") == "UFBX_PROP_DISTANCE");
        REQUIRE(json.at("value").get<double>() == 12.5);
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

    SECTION("compound property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("compound");
        prop.type = UFBX_PROP_COMPOUND;

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "compound");
        REQUIRE(json.at("type") == "UFBX_PROP_COMPOUND");
        REQUIRE(json.at("value").is_null());
    }

    SECTION("blob property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("blob");
        prop.type = UFBX_PROP_BLOB;

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "blob");
        REQUIRE(json.at("type") == "UFBX_PROP_BLOB");
        REQUIRE(json.at("value").is_null());
    }

    SECTION("reference property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("ref");
        prop.type = UFBX_PROP_REFERENCE;

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "ref");
        REQUIRE(json.at("type") == "UFBX_PROP_REFERENCE");
        REQUIRE(json.at("value").is_null());
    }

    SECTION("unknown property") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("unknown");
        prop.type = UFBX_PROP_UNKNOWN;

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "unknown");
        REQUIRE(json.at("type") == "UFBX_PROP_UNKNOWN");
        REQUIRE(json.at("value").is_null());
    }

    SECTION("invalid property type falls back to unknown") {
        ufbx_prop prop = {};
        prop.name = MakeUfbxString("invalid");
        prop.type = static_cast<ufbx_prop_type>(999);

        const auto json = SerializeFbxProperty(prop);
        REQUIRE(json.at("name") == "invalid");
        REQUIRE(json.at("type") == "UFBX_PROP_UNKNOWN");
        REQUIRE(json.at("value").is_null());
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

TEST_CASE("SerializeUfbxAttribute maps element types and properties") {
    ufbx_prop prop = {};
    prop.name = MakeUfbxString("AttrProp");
    prop.type = UFBX_PROP_STRING;
    prop.value_str = MakeUfbxString("value");

    ufbx_prop_list prop_list = {};
    prop_list.data = &prop;
    prop_list.count = 1;

    ufbx_props props = {};
    props.props = prop_list;

    struct ElementCase {
        ufbx_element_type type;
        const char* expected;
    };

    const ElementCase cases[] = {
        {UFBX_ELEMENT_MESH, "Mesh"},
        {UFBX_ELEMENT_LIGHT, "Light"},
        {UFBX_ELEMENT_CAMERA, "Camera"},
        {UFBX_ELEMENT_BONE, "Bone"},
        {UFBX_ELEMENT_EMPTY, "Empty"},
        {UFBX_ELEMENT_LINE_CURVE, "LineCurve"},
        {UFBX_ELEMENT_NURBS_CURVE, "NurbsCurve"},
        {UFBX_ELEMENT_NURBS_SURFACE, "NurbsSurface"},
        {UFBX_ELEMENT_NURBS_TRIM_SURFACE, "NurbsTrimSurface"},
        {UFBX_ELEMENT_NURBS_TRIM_BOUNDARY, "NurbsTrimBoundary"},
        {UFBX_ELEMENT_PROCEDURAL_GEOMETRY, "ProceduralGeometry"},
        {UFBX_ELEMENT_STEREO_CAMERA, "StereoCamera"},
        {UFBX_ELEMENT_CAMERA_SWITCHER, "CameraSwitcher"},
        {UFBX_ELEMENT_MARKER, "Marker"},
        {UFBX_ELEMENT_LOD_GROUP, "LODGroup"},
    };

    for (const auto& entry : cases) {
        ufbx_element element = {};
        element.name = MakeUfbxString("Attr");
        element.type = entry.type;
        element.props = props;

        const auto json = SerializeUfbxAttribute(element);
        REQUIRE(json.at("name") == "Attr");
        REQUIRE(json.at("type") == entry.expected);
        REQUIRE(json.at("properties").is_array());
        REQUIRE(json.at("properties").size() == 1);
        REQUIRE(json.at("properties")[0].at("name") == "AttrProp");
    }
}

TEST_CASE("SerializeUfbxAttribute defaults unknown element type") {
    ufbx_element element = {};
    element.name = MakeUfbxString("Attr");
    element.type = static_cast<ufbx_element_type>(999);

    const auto json = SerializeUfbxAttribute(element);
    REQUIRE(json.at("name") == "Attr");
    REQUIRE(json.at("type") == "Unknown");
    REQUIRE(json.at("properties").is_array());
    REQUIRE(json.at("properties").empty());
}
