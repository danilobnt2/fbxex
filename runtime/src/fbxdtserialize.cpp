#include "fbxdtserialize.hpp"

#include <array>
#include <stdexcept>

namespace {

nlohmann::json ToJson(const FbxDouble2& v) {
    return nlohmann::json::array({v[0], v[1]});
}

nlohmann::json ToJson(const FbxDouble3& v) {
    return nlohmann::json::array({v[0], v[1], v[2]});
}

nlohmann::json ToJson(const FbxDouble4& v) {
    return nlohmann::json::array({v[0], v[1], v[2], v[3]});
}

nlohmann::json ToJson(const FbxDouble4x4& m) {
    nlohmann::json rows = nlohmann::json::array();
    for (int i = 0; i < 4; ++i) {
        nlohmann::json row = nlohmann::json::array();
        for (int j = 0; j < 4; ++j) {
            row.push_back(m[i][j]);
        }
        rows.push_back(row);
    }
    return rows;
}

template <size_t N, typename TVector>
nlohmann::json ToObject(const TVector& v, const std::array<const char*, N>& keys) {
    nlohmann::json obj;
    for (size_t i = 0; i < N; ++i) {
        obj[keys[i]] = v[i];
    }
    return obj;
}

} // namespace

nlohmann::json SerializeFbxProperty(const FbxProperty& property) {
    nlohmann::json result;
    result["name"] = std::string(property.GetNameAsCStr());

    const FbxDataType data_type = property.GetPropertyDataType();
    const char* raw_type_name = data_type.GetName();
    std::string type_name = raw_type_name ? std::string(raw_type_name) : std::string();
    if (type_name.empty() && data_type.GetType() == FBXSDK_NAMESPACE::eFbxUndefined) {
        type_name = "FbxUndefined";
    }
    result["type"] = type_name;

    auto matches = [&type_name](std::initializer_list<const char*> names) {
        for (const char* n : names) {
            if (type_name == n) {
                return true;
            }
        }
        return false;
    };

    // Object-like data types with named components.
    if (matches({"FbxColor3", "FbxMaterialEmissive", "FbxMaterialAmbient",
                 "FbxMaterialDiffuse", "FbxMaterialTransparentColor",
                 "FbxMaterialSpecular", "FbxMaterialReflection",
                 "FbxMaterialVectorDisplacement"}))
    {
        result["value"] = ToObject(
            property.Get<FbxDouble3>(),
            std::array<const char*, 3>{"r", "g", "b"});
        return result;
    }

    if (matches({"FbxColor4"})) {
        result["value"] = ToObject(
            property.Get<FbxDouble4>(),
            std::array<const char*, 4>{"r", "g", "b", "a"});
        return result;
    }

    if (matches({"FbxHSB"})) {
        result["value"] = ToObject(
            property.Get<FbxDouble3>(),
            std::array<const char*, 3>{"h", "s", "b"});
        return result;
    }

    if (matches({"FbxScalingUV", "FbxTextureRotation", "FbxTranslationUV"}))
    {
        result["value"] = ToObject(
            property.Get<FbxDouble2>(),
            std::array<const char*, 2>{"u", "v"});
        return result;
    }

    if (matches({"FbxTranslation", "FbxRotation", "FbxScaling",
                 "FbxLocalTranslation", "FbxLocalRotation", "FbxLocalScaling"}))
    {
        result["value"] = ToObject(
            property.Get<FbxDouble3>(),
            std::array<const char*, 3>{"x", "y", "z"});
        return result;
    }

    if (matches({"FbxQuaternion", "FbxLocalQuaternion"}))
    {
        result["value"] = ToObject(
            property.Get<FbxDouble4>(),
            std::array<const char*, 4>{"w", "x", "y", "z"});
        return result;
    }

    // Handle explicit null data type by name (no enum value exists for it).
    if (type_name == "FbxNull") {
        result["value"] = nullptr;
        return result;
    }

    if (data_type.GetType() == FBXSDK_NAMESPACE::eFbxUndefined) {
        return result;
    }

    switch (data_type.GetType()) {
        case FBXSDK_NAMESPACE::eFbxBool:
            result["value"] = static_cast<bool>(property.Get<FbxBool>());
            break;
        case FBXSDK_NAMESPACE::eFbxChar:
            result["value"] = static_cast<int>(property.Get<FbxChar>());
            break;
        case FBXSDK_NAMESPACE::eFbxUChar:
            result["value"] = static_cast<unsigned int>(property.Get<FbxUChar>());
            break;
        case FBXSDK_NAMESPACE::eFbxShort:
            result["value"] = static_cast<int>(property.Get<FbxShort>());
            break;
        case FBXSDK_NAMESPACE::eFbxUShort:
            result["value"] = static_cast<unsigned int>(property.Get<FbxUShort>());
            break;
        case FBXSDK_NAMESPACE::eFbxInt:
            result["value"] = static_cast<int>(property.Get<FbxInt>());
            break;
        case FBXSDK_NAMESPACE::eFbxUInt:
            result["value"] = static_cast<unsigned int>(property.Get<FbxUInt>());
            break;
        case FBXSDK_NAMESPACE::eFbxLongLong:
            result["value"] = static_cast<long long>(property.Get<FbxLongLong>());
            break;
        case FBXSDK_NAMESPACE::eFbxULongLong:
            result["value"] = static_cast<unsigned long long>(property.Get<FbxULongLong>());
            break;
        case FBXSDK_NAMESPACE::eFbxFloat:
            result["value"] = static_cast<double>(property.Get<FbxFloat>());
            break;
        case FBXSDK_NAMESPACE::eFbxDouble:
            result["value"] = static_cast<double>(property.Get<FbxDouble>());
            break;
        case FBXSDK_NAMESPACE::eFbxDouble2:
            result["value"] = ToJson(property.Get<FbxDouble2>());
            break;
        case FBXSDK_NAMESPACE::eFbxDouble3:
            result["value"] = ToJson(property.Get<FbxDouble3>());
            break;
        case FBXSDK_NAMESPACE::eFbxDouble4:
            result["value"] = ToJson(property.Get<FbxDouble4>());
            break;
        case FBXSDK_NAMESPACE::eFbxDouble4x4:
            result["value"] = ToJson(property.Get<FbxDouble4x4>());
            break;
        case FBXSDK_NAMESPACE::eFbxString: {
            const FbxString str = property.Get<FbxString>();
            result["value"] = std::string(str.Buffer());
            break;
        }
        default:
            break;
    }

    return result;
}
