#pragma once

#include <JavaScriptCore/JavaScript.h>
#include <string>
#include <string_view>

#include "fbxnode.hpp"


struct NativeFn {
  std::string_view name;
  JSObjectCallAsFunctionCallback cb;
};


inline void BindGlobalFunction(
    JSContextRef ctx,
    std::string_view name,
    JSObjectCallAsFunctionCallback cb,
    JSPropertyAttributes attrs = kJSPropertyAttributeNone)
{
  JSStringRef jsName = JSStringCreateWithUTF8CString(name.data());

  try {
      JSObjectRef fn = JSObjectMakeFunctionWithCallback(ctx, jsName, cb);
      JSObjectRef global = JSContextGetGlobalObject(ctx);
      JSObjectSetProperty(ctx, global, jsName, fn, attrs, nullptr);
      JSStringRelease(jsName);
  } catch (const std::exception& e) {
    JSStringRelease(jsName);
    throw;
  }
}

inline void BindGlobals(JSContextRef ctx, std::initializer_list<NativeFn> fns)
{
  for (const auto& f : fns) {
    BindGlobalFunction(ctx, f.name, f.cb);
  }
}

JSValueRef BindFBXNode(JSContextRef ctx, const FBXNode& node);
JSValueRef BindFBXNodeProps(JSContextRef ctx, const FBXNodeProps& props);
