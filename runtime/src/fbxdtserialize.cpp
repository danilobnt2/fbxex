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

} // namespace

nlohmann::json SerializeFbxProperty(const FbxProperty& property) {
    nlohmann::json result;
    
    auto name = property.GetNameAsCStr();
    result["name"] = name ? std::string(name) : std::string();

    const FbxDataType data_type = property.GetPropertyDataType();

    switch (data_type.GetType()) {
        case FBXSDK_NAMESPACE::eFbxUndefined:
            result["type"] = "eFbxUndefined";
            result["value"] = nullptr;
            break;
        case FBXSDK_NAMESPACE::eFbxChar:
            result["type"] = "eFbxChar";
            result["value"] = static_cast<int>(property.Get<FbxChar>());
            break;
        case FBXSDK_NAMESPACE::eFbxUChar:
            result["type"] = "eFbxUChar";
            result["value"] = static_cast<unsigned int>(property.Get<FbxUChar>());
            break;
        case FBXSDK_NAMESPACE::eFbxShort:
            result["type"] = "eFbxShort";
            result["value"] = static_cast<int>(property.Get<FbxShort>());
            break;
        case FBXSDK_NAMESPACE::eFbxUShort:
            result["type"] = "eFbxUShort";
            result["value"] = static_cast<unsigned int>(property.Get<FbxUShort>());
            break;
        case FBXSDK_NAMESPACE::eFbxUInt:
            result["type"] = "eFbxUInt";
            result["value"] = static_cast<unsigned int>(property.Get<FbxUInt>());
            break;
        case FBXSDK_NAMESPACE::eFbxLongLong:
            result["type"] = "eFbxLongLong";
            result["value"] = static_cast<long long>(property.Get<FbxLongLong>());
            break;
        case FBXSDK_NAMESPACE::eFbxULongLong:
            result["type"] = "eFbxULongLong";
            result["value"] = static_cast<unsigned long long>(property.Get<FbxULongLong>());
            break;
        case FBXSDK_NAMESPACE::eFbxHalfFloat:
            result["type"] = "eFbxHalfFloat";
            result["value"] = static_cast<double>(property.Get<FbxHalfFloat>().value());
            break;
        case FBXSDK_NAMESPACE::eFbxBool:
            result["type"] = "eFbxBool";
            result["value"] = static_cast<bool>(property.Get<FbxBool>());
            break;
        case FBXSDK_NAMESPACE::eFbxInt:
            result["type"] = "eFbxInt";
            result["value"] = static_cast<int>(property.Get<FbxInt>());
            break;
        case FBXSDK_NAMESPACE::eFbxFloat:
            result["type"] = "eFbxFloat";
            result["value"] = static_cast<double>(property.Get<FbxFloat>());
            break;
        case FBXSDK_NAMESPACE::eFbxDouble:
            result["type"] = "eFbxDouble";
            result["value"] = static_cast<double>(property.Get<FbxDouble>());
            break;
        case FBXSDK_NAMESPACE::eFbxDouble2:
            result["type"] = "eFbxDouble2";
            result["value"] = ToJson(property.Get<FbxDouble2>());
            break;
        case FBXSDK_NAMESPACE::eFbxDouble3:
            result["type"] = "eFbxDouble3";
            result["value"] = ToJson(property.Get<FbxDouble3>());
            break;
        case FBXSDK_NAMESPACE::eFbxDouble4:
            result["type"] = "eFbxDouble4";
            result["value"] = ToJson(property.Get<FbxDouble4>());
            break;
        case FBXSDK_NAMESPACE::eFbxDouble4x4:
            result["type"] = "eFbxDouble4x4";
            result["value"] = ToJson(property.Get<FbxDouble4x4>());
            break;
        case FBXSDK_NAMESPACE::eFbxEnum:
            result["type"] = "eFbxEnum";
            result["value"] = static_cast<int>(property.Get<FbxEnum>());
            break;
        case FBXSDK_NAMESPACE::eFbxEnumM:
            result["type"] = "eFbxEnumM";
            result["value"] = static_cast<int>(property.Get<FbxEnum>());
            break;
        case FBXSDK_NAMESPACE::eFbxString:
            result["type"] = "eFbxString";
            result["value"] = std::string(property.Get<FbxString>().Buffer());
            break;
        case FBXSDK_NAMESPACE::eFbxTime:
            result["type"] = "eFbxTime";
            result["value"] = std::string(property.Get<FbxTime>().GetTimeString());
            break;
        case FBXSDK_NAMESPACE::eFbxReference:
            // Unsupported
        case FBXSDK_NAMESPACE::eFbxBlob:
            // Unsupported
        case FBXSDK_NAMESPACE::eFbxDistance:
            // Unsupported
        case FBXSDK_NAMESPACE::eFbxDateTime:
            result["type"] = "eFbxDateTime";
            result["value"] = std::string(property.Get<FbxDateTime>().toString().Buffer());
            break;
        case FBXSDK_NAMESPACE::eFbxTypeCount:
            result["type"] = "eFbxTypeCount";
            result["value"] = static_cast<int>(property.Get<FbxInt>());
            break;
        default:
            break;
    }

    return result;
}
