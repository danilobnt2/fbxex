#include "binders.hpp"

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>


// Helper to convert nlohmann::json to JSValueRef
JSValueRef JsonToJSValueStep(JSContextRef ctx, const nlohmann::json& j) {
  switch (j.type()) {
    case nlohmann::json::value_t::null:
      return JSValueMakeNull(ctx);
    case nlohmann::json::value_t::boolean:
      return JSValueMakeBoolean(ctx, j.get<bool>());
    case nlohmann::json::value_t::number_integer:
      return JSValueMakeNumber(ctx, j.get<int>());
    case nlohmann::json::value_t::number_unsigned:
      return JSValueMakeNumber(ctx, j.get<unsigned int>());
    case nlohmann::json::value_t::number_float:
      return JSValueMakeNumber(ctx, j.get<double>());
    case nlohmann::json::value_t::string: {
      const std::string& str = j.get<std::string>();
      return JSValueMakeString(ctx, JSStringCreateWithUTF8CString(str.c_str()));
    }
    case nlohmann::json::value_t::array: {
      const auto& arr = j;
      JSObjectRef js_array = JSObjectMakeArray(ctx, 0, nullptr, nullptr);
      for (size_t i = 0; i < arr.size(); ++i) {
        JSValueRef element = JsonToJSValueStep(ctx, arr[i]);
        JSObjectSetPropertyAtIndex(ctx, js_array, i, element, nullptr);
      }
      return js_array;
    }
    case nlohmann::json::value_t::object: {
      const auto& obj = j;
      JSObjectRef js_obj = JSObjectMake(ctx, nullptr, nullptr);
      for (auto it = obj.begin(); it != obj.end(); ++it) {
        JSValueRef value = JsonToJSValueStep(ctx, it.value());
        JSStringRef key = JSStringCreateWithUTF8CString(it.key().c_str());
        JSObjectSetProperty(ctx, js_obj, key, value, kJSPropertyAttributeNone, nullptr);
        JSStringRelease(key);
      }
      return js_obj;
    }
    default:
      throw std::runtime_error("Unsupported JSON value type");
  }
}

JSObjectRef JsonToJSObject(JSContextRef ctx, const nlohmann::json& j) {
  auto js_value = JsonToJSValueStep(ctx, j);
  if (JSValueIsObject(ctx, js_value)) {
    return JSValueToObject(ctx, js_value, nullptr);
  } else {
    throw std::runtime_error("JSON value is not an object");
  }
}


JSValueRef BindFBXNodeProps(JSContextRef ctx, const FBXNodeProps& props) {
  // Serialize props via nlohmann::json (extend once FBXNodeProps holds data).
  const auto name = props.name;
  const auto properties = props.properties;
  JSObjectRef js_pros = JSObjectMake(ctx, nullptr, nullptr);
  JSStringRef name_key = JSStringCreateWithUTF8CString("name");
  JSObjectSetProperty(
      ctx,
      js_pros,
      name_key,
      JSValueMakeString(ctx, JSStringCreateWithUTF8CString(name.c_str())),
      kJSPropertyAttributeNone,
      nullptr);
  JSStringRelease(name_key);
  JSStringRef props_key = JSStringCreateWithUTF8CString("properties");
  JSObjectSetProperty(
      ctx,
      js_pros,
      props_key,
      JsonToJSObject(ctx, properties),
      kJSPropertyAttributeNone,
      nullptr);
  JSStringRelease(props_key);
  return js_pros;
}

JSValueRef BindFBXNode(JSContextRef ctx, const FBXNode& node) {
  JSObjectRef js_node = JSObjectMake(ctx, nullptr, nullptr);

  JSStringRef id_key = JSStringCreateWithUTF8CString("id");
  JSObjectSetProperty(
      ctx,
      js_node,
      id_key,
      JSValueMakeNumber(ctx, static_cast<double>(node.getID())),
      kJSPropertyAttributeNone,
      nullptr);
  JSStringRelease(id_key);

  JSStringRef props_key = JSStringCreateWithUTF8CString("props");
  JSObjectSetProperty(
      ctx,
      js_node,
      props_key,
      BindFBXNodeProps(ctx, node.getProps()),
      kJSPropertyAttributeNone,
      nullptr);
  JSStringRelease(props_key);

  return js_node;
}
