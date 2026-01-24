#include "fbxdtserialize.hpp"

std::string UfbxStringToString(const ufbx_string& value) {
    if (!value.data || value.length == 0) {
        return {};
    }
    return std::string(value.data, value.length);
}

namespace {

nlohmann::json ToJson(const ufbx_vec2& v) {
    return nlohmann::json::array({v.x, v.y});
}

nlohmann::json ToJson(const ufbx_vec3& v) {
    return nlohmann::json::array({v.x, v.y, v.z});
}

nlohmann::json ToJson(const ufbx_vec4& v) {
    return nlohmann::json::array({v.x, v.y, v.z, v.w});
}

const char* PropTypeToString(ufbx_prop_type type) {
    switch (type) {
        case UFBX_PROP_UNKNOWN:
            return "UFBX_PROP_UNKNOWN";
        case UFBX_PROP_BOOLEAN:
            return "UFBX_PROP_BOOLEAN";
        case UFBX_PROP_INTEGER:
            return "UFBX_PROP_INTEGER";
        case UFBX_PROP_NUMBER:
            return "UFBX_PROP_NUMBER";
        case UFBX_PROP_VECTOR:
            return "UFBX_PROP_VECTOR";
        case UFBX_PROP_COLOR:
            return "UFBX_PROP_COLOR";
        case UFBX_PROP_COLOR_WITH_ALPHA:
            return "UFBX_PROP_COLOR_WITH_ALPHA";
        case UFBX_PROP_STRING:
            return "UFBX_PROP_STRING";
        case UFBX_PROP_DATE_TIME:
            return "UFBX_PROP_DATE_TIME";
        case UFBX_PROP_TRANSLATION:
            return "UFBX_PROP_TRANSLATION";
        case UFBX_PROP_ROTATION:
            return "UFBX_PROP_ROTATION";
        case UFBX_PROP_SCALING:
            return "UFBX_PROP_SCALING";
        case UFBX_PROP_DISTANCE:
            return "UFBX_PROP_DISTANCE";
        case UFBX_PROP_COMPOUND:
            return "UFBX_PROP_COMPOUND";
        case UFBX_PROP_BLOB:
            return "UFBX_PROP_BLOB";
        case UFBX_PROP_REFERENCE:
            return "UFBX_PROP_REFERENCE";
        default:
            return "UFBX_PROP_UNKNOWN";
    }
}

const char* ElementTypeToString(ufbx_element_type type) {
    switch (type) {
        case UFBX_ELEMENT_MESH:
            return "Mesh";
        case UFBX_ELEMENT_LIGHT:
            return "Light";
        case UFBX_ELEMENT_CAMERA:
            return "Camera";
        case UFBX_ELEMENT_BONE:
            return "Bone";
        case UFBX_ELEMENT_EMPTY:
            return "Empty";
        case UFBX_ELEMENT_LINE_CURVE:
            return "LineCurve";
        case UFBX_ELEMENT_NURBS_CURVE:
            return "NurbsCurve";
        case UFBX_ELEMENT_NURBS_SURFACE:
            return "NurbsSurface";
        case UFBX_ELEMENT_NURBS_TRIM_SURFACE:
            return "NurbsTrimSurface";
        case UFBX_ELEMENT_NURBS_TRIM_BOUNDARY:
            return "NurbsTrimBoundary";
        case UFBX_ELEMENT_PROCEDURAL_GEOMETRY:
            return "ProceduralGeometry";
        case UFBX_ELEMENT_STEREO_CAMERA:
            return "StereoCamera";
        case UFBX_ELEMENT_CAMERA_SWITCHER:
            return "CameraSwitcher";
        case UFBX_ELEMENT_MARKER:
            return "Marker";
        case UFBX_ELEMENT_LOD_GROUP:
            return "LODGroup";
        default:
            return "Unknown";
    }
}

} // namespace

nlohmann::json SerializeFbxProperty(const ufbx_prop& property) {
    nlohmann::json result;
    result["name"] = UfbxStringToString(property.name);
    result["type"] = PropTypeToString(property.type);

    if ((property.flags & UFBX_PROP_FLAG_NO_VALUE) != 0) {
        result["value"] = nullptr;
        return result;
    }

    switch (property.type) {
        case UFBX_PROP_BOOLEAN:
            result["value"] = static_cast<bool>(property.value_int != 0);
            break;
        case UFBX_PROP_INTEGER:
            result["value"] = static_cast<long long>(property.value_int);
            break;
        case UFBX_PROP_NUMBER:
            result["value"] = static_cast<double>(property.value_real);
            break;
        case UFBX_PROP_VECTOR:
        case UFBX_PROP_COLOR:
        case UFBX_PROP_TRANSLATION:
        case UFBX_PROP_ROTATION:
        case UFBX_PROP_SCALING:
            result["value"] = ToJson(property.value_vec3);
            break;
        case UFBX_PROP_COLOR_WITH_ALPHA:
            result["value"] = ToJson(property.value_vec4);
            break;
        case UFBX_PROP_STRING:
        case UFBX_PROP_DATE_TIME:
            result["value"] = UfbxStringToString(property.value_str);
            break;
        case UFBX_PROP_DISTANCE:
            result["value"] = static_cast<double>(property.value_real);
            break;
        case UFBX_PROP_COMPOUND:
        case UFBX_PROP_BLOB:
        case UFBX_PROP_REFERENCE:
        case UFBX_PROP_UNKNOWN:
        default:
            result["value"] = nullptr;
            break;
    }

    return result;
}

nlohmann::json SerializeUfbxAttribute(const ufbx_element& attribute) {
    nlohmann::json result;
    result["name"] = UfbxStringToString(attribute.name);
    result["type"] = ElementTypeToString(attribute.type);
    result["properties"] = nlohmann::json::array();

    for (size_t idx = 0; idx < attribute.props.props.count; ++idx) {
        const ufbx_prop& prop = attribute.props.props.data[idx];
        result["properties"].push_back(SerializeFbxProperty(prop));
    }

    return result;
}
