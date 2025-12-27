#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <fbxsdk.h>

#include "fbxdtserialize.hpp"

namespace {

class FbxTestContext {
public:
    FbxTestContext() {
        manager = FbxManager::Create();
        REQUIRE(manager != nullptr);
    }

    ~FbxTestContext() {
        if (manager) {
            manager->Destroy();
            manager = nullptr;
        }
    }

    FbxObject* makeObject(const char* name) {
        auto* obj = FbxObject::Create(manager, name);
        REQUIRE(obj != nullptr);
        return obj;
    }

    FbxManager* manager = nullptr;
};

} // namespace

TEST_CASE("SerializeFbxProperty handles scalar and vector types") {
    FbxTestContext ctx;
    auto* obj = ctx.makeObject("prop-holder");

    SECTION("boolean property") {
        auto bool_prop = FbxProperty::Create(obj, FbxBoolDT, "visible");
        bool_prop.Set<bool>(true);

        const auto json = SerializeFbxProperty(bool_prop);
        REQUIRE(json.at("name") == "visible");
        REQUIRE(json.at("type") == "eFbxBool");
        REQUIRE(json.at("value").get<bool>() == true);
    }

    SECTION("char property") {
        auto char_prop = FbxProperty::Create(obj, FbxCharDT, "char");
        char_prop.Set<FbxChar>(-12);

        const auto json = SerializeFbxProperty(char_prop);
        REQUIRE(json.at("name") == "char");
        REQUIRE(json.at("type") == "eFbxChar");
        REQUIRE(json.at("value").get<int>() == -12);
    }

    SECTION("unsigned char property") {
        auto uchar_prop = FbxProperty::Create(obj, FbxUCharDT, "uchar");
        uchar_prop.Set<FbxUChar>(250U);

        const auto json = SerializeFbxProperty(uchar_prop);
        REQUIRE(json.at("name") == "uchar");
        REQUIRE(json.at("type") == "eFbxUChar");
        REQUIRE(json.at("value").get<unsigned int>() == 250U);
    }

    SECTION("short property") {
        auto short_prop = FbxProperty::Create(obj, FbxShortDT, "short");
        short_prop.Set<FbxShort>(-1024);

        const auto json = SerializeFbxProperty(short_prop);
        REQUIRE(json.at("name") == "short");
        REQUIRE(json.at("type") == "eFbxShort");
        REQUIRE(json.at("value").get<int>() == -1024);
    }

    SECTION("unsigned short property") {
        auto ushort_prop = FbxProperty::Create(obj, FbxUShortDT, "ushort");
        ushort_prop.Set<FbxUShort>(60000U);

        const auto json = SerializeFbxProperty(ushort_prop);
        REQUIRE(json.at("name") == "ushort");
        REQUIRE(json.at("type") == "eFbxUShort");
        REQUIRE(json.at("value").get<unsigned int>() == 60000U);
    }

    SECTION("unsigned int property") {
        auto uint_prop = FbxProperty::Create(obj, FbxUIntDT, "uint");
        uint_prop.Set<FbxUInt>(123456789U);

        const auto json = SerializeFbxProperty(uint_prop);
        REQUIRE(json.at("name") == "uint");
        REQUIRE(json.at("type") == "eFbxUInt");
        REQUIRE(json.at("value").get<unsigned int>() == 123456789U);
    }

    SECTION("long long property") {
        auto ll_prop = FbxProperty::Create(obj, FbxLongLongDT, "longlong");
        ll_prop.Set<FbxLongLong>(-900000000000LL);

        const auto json = SerializeFbxProperty(ll_prop);
        REQUIRE(json.at("name") == "longlong");
        REQUIRE(json.at("type") == "eFbxLongLong");
        REQUIRE(json.at("value").get<long long>() == -900000000000LL);
    }

    SECTION("unsigned long long property") {
        auto ull_prop = FbxProperty::Create(obj, FbxULongLongDT, "ulonglong");
        ull_prop.Set<FbxULongLong>(1800000000000ULL);

        const auto json = SerializeFbxProperty(ull_prop);
        REQUIRE(json.at("name") == "ulonglong");
        REQUIRE(json.at("type") == "eFbxULongLong");
        REQUIRE(json.at("value").get<unsigned long long>() == 1800000000000ULL);
    }

    SECTION("half float property") {
        auto half_prop = FbxProperty::Create(obj, FbxHalfFloatDT, "half");
        FbxHalfFloat half_val(1.25f);
        half_prop.Set<FbxHalfFloat>(half_val);

        const auto json = SerializeFbxProperty(half_prop);
        REQUIRE(json.at("name") == "half");
        REQUIRE(json.at("type") == "eFbxHalfFloat");
        REQUIRE(json.at("value").get<double>() == Catch::Approx(1.25));
    }

    SECTION("string property") {
        auto str_prop = FbxProperty::Create(obj, FbxStringDT, "label");
        str_prop.Set<FbxString>("fbxex");

        const auto json = SerializeFbxProperty(str_prop);
        REQUIRE(json.at("name") == "label");
        REQUIRE(json.at("type") == "eFbxString");
        REQUIRE(json.at("value").get<std::string>() == "fbxex");
    }

    SECTION("integer property") {
        auto int_prop = FbxProperty::Create(obj, FbxIntDT, "count");
        int_prop.Set<FbxInt>(42);

        const auto json = SerializeFbxProperty(int_prop);
        REQUIRE(json.at("name") == "count");
        REQUIRE(json.at("type") == "eFbxInt");
        REQUIRE(json.at("value").get<int>() == 42);
    }

    SECTION("float property") {
        auto float_prop = FbxProperty::Create(obj, FbxFloatDT, "weight");
        float_prop.Set<FbxFloat>(2.5f);

        const auto json = SerializeFbxProperty(float_prop);
        REQUIRE(json.at("name") == "weight");
        REQUIRE(json.at("type") == "eFbxFloat");
        REQUIRE(json.at("value").get<double>() == Catch::Approx(2.5));
    }

    SECTION("double property") {
        auto double_prop = FbxProperty::Create(obj, FbxDoubleDT, "scale");
        double_prop.Set<FbxDouble>(3.14);

        const auto json = SerializeFbxProperty(double_prop);
        REQUIRE(json.at("name") == "scale");
        REQUIRE(json.at("type") == "eFbxDouble");
        REQUIRE(json.at("value").get<double>() == 3.14);
    }

    SECTION("vector2 property") {
        auto vec2_prop = FbxProperty::Create(obj, FbxDouble2DT, "uv");
        vec2_prop.Set<FbxDouble2>(FbxDouble2(0.5, 0.75));

        const auto json = SerializeFbxProperty(vec2_prop);
        REQUIRE(json.at("name") == "uv");
        REQUIRE(json.at("type") == "eFbxDouble2");
        REQUIRE(json.at("value").is_array());
        REQUIRE(json.at("value").size() == 2);
        REQUIRE(json.at("value")[0].get<double>() == 0.5);
        REQUIRE(json.at("value")[1].get<double>() == 0.75);
    }

    SECTION("vector3 property") {
        auto vec_prop = FbxProperty::Create(obj, FbxDouble3DT, "translation");
        vec_prop.Set<FbxDouble3>(FbxDouble3(1.0, 2.0, 3.5));

        const auto json = SerializeFbxProperty(vec_prop);
        REQUIRE(json.at("name") == "translation");
        REQUIRE(json.at("type") == "eFbxDouble3");
        REQUIRE(json.at("value").is_array());
        REQUIRE(json.at("value").size() == 3);
        REQUIRE(json.at("value")[0].get<double>() == 1.0);
        REQUIRE(json.at("value")[1].get<double>() == 2.0);
        REQUIRE(json.at("value")[2].get<double>() == 3.5);
    }

    SECTION("vector4 property") {
        auto vec4_prop = FbxProperty::Create(obj, FbxDouble4DT, "quaternion");
        vec4_prop.Set<FbxDouble4>(FbxDouble4(0.0, 0.0, 0.7071, 0.7071));

        const auto json = SerializeFbxProperty(vec4_prop);
        REQUIRE(json.at("name") == "quaternion");
        REQUIRE(json.at("type") == "eFbxDouble4");
        REQUIRE(json.at("value").is_array());
        REQUIRE(json.at("value").size() == 4);
        REQUIRE(json.at("value")[0].get<double>() == 0.0);
        REQUIRE(json.at("value")[1].get<double>() == 0.0);
        REQUIRE(json.at("value")[2].get<double>() == 0.7071);
        REQUIRE(json.at("value")[3].get<double>() == 0.7071);
    }

    SECTION("double4x4 property") {
        auto matrix_prop = FbxProperty::Create(obj, FbxDouble4x4DT, "transform");
        FbxDouble4x4 matrix;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                matrix[row][col] = static_cast<double>((row * 4) + col + 1);
            }
        }
        matrix_prop.Set<FbxDouble4x4>(matrix);

        const auto json = SerializeFbxProperty(matrix_prop);
        REQUIRE(json.at("name") == "transform");
        REQUIRE(json.at("type") == "eFbxDouble4x4");
        REQUIRE(json.at("value").is_array());
        REQUIRE(json.at("value").size() == 4);
        for (int row = 0; row < 4; ++row) {
            REQUIRE(json.at("value")[static_cast<size_t>(row)].is_array());
            REQUIRE(json.at("value")[static_cast<size_t>(row)].size() == 4);
            for (int col = 0; col < 4; ++col) {
                REQUIRE(json.at("value")[static_cast<size_t>(row)][static_cast<size_t>(col)].get<double>() == matrix[row][col]);
            }
        }
    }

    SECTION("enum property") {
        auto enum_prop = FbxProperty::Create(obj, FbxEnumDT, "mode");
        enum_prop.Set<FbxEnum>(3);

        const auto json = SerializeFbxProperty(enum_prop);
        REQUIRE(json.at("name") == "mode");
        REQUIRE(json.at("type") == "eFbxEnum");
        REQUIRE(json.at("value").get<int>() == 3);
    }

    SECTION("time property") {
        auto time_prop = FbxProperty::Create(obj, FbxTimeDT, "time");
        FbxTime fbx_time;
        fbx_time.SetMilliSeconds(1500);
        time_prop.Set<FbxTime>(fbx_time);

        const auto expected_time = std::string(fbx_time.GetTimeString());
        const auto json = SerializeFbxProperty(time_prop);
        REQUIRE(json.at("name") == "time");
        REQUIRE(json.at("type") == "eFbxTime");
        REQUIRE(json.at("value").get<std::string>() == expected_time);
    }

    SECTION("datetime property") {
        auto datetime_prop = FbxProperty::Create(obj, FbxDateTimeDT, "timestamp");
        FbxDateTime fbx_datetime(2025, 12, 31, 23, 59, 59, 0);
        datetime_prop.Set<FbxDateTime>(fbx_datetime);

        const auto expected_datetime = std::string(fbx_datetime.toString().Buffer());
        const auto json = SerializeFbxProperty(datetime_prop);
        REQUIRE(json.at("name") == "timestamp");
        REQUIRE(json.at("type") == "eFbxDateTime");
        REQUIRE(json.at("value").get<std::string>() == expected_datetime);
    }

}

TEST_CASE("SerializeFbxProperty returns null for undefined type") {
    // Default constructed property is invalid/undefined; creating FbxUndefinedDT crashes.
    FbxProperty undefined_prop;
    const auto json = SerializeFbxProperty(undefined_prop);
    REQUIRE(json.at("name") == "");
    REQUIRE(json.at("type") == "eFbxUndefined");
    REQUIRE(json.at("value").is_null());
}
