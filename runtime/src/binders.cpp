#include "binders.hpp"

#include <vector>

JSValueRef BindFBXNodeProps(JSContextRef ctx, const FBXNodeProps& /*props*/) {
  // Currently no properties are exposed, return an empty object placeholder.
  return JSObjectMake(ctx, nullptr, nullptr);
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
