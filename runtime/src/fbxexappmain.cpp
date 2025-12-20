#include <string>
#include <memory>
#include <cstdlib>

#include <AppCore/AppCore.h>
#include <JavaScriptCore/JavaScript.h>

#include <fbxsdk.h>

#include "binders.hpp"
#include "fbxclienteager.hpp"
#include "fbxnode.hpp"

#ifdef _WIN32
#include "winfio.hpp"
#endif

#include "fbxexappmain.hpp"


FbxexAppMain::FbxexAppMain(const std::string& start_url) 
    : start_url_(start_url)
    , fbx_client_eager_(nullptr)
{
    instance_ = this;
    app_ = ultralight::App::Create();
    window_ = ultralight::Window::Create(
        app_->main_monitor(), 
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        false
        , ultralight::kWindowFlags_Titled 
        | ultralight::kWindowFlags_Resizable);
    overlay_ = ultralight::Overlay::Create(window_, 1, 1, 0, 0);
    OnResize(window_.get(), window_->width(), window_->height());
    overlay_->view()->LoadURL(start_url_.c_str());
    app_->set_listener(this);
    window_->set_listener(this);
    overlay_->view()->set_load_listener(this);
    overlay_->view()->set_view_listener(this);
}

void FbxexAppMain::OnDOMReady(
    ultralight::View* caller, 
    uint64_t frame_id, 
    bool is_main_frame, 
    const ultralight::String& url) 
{
    auto scoped_context = caller->LockJSContext(); 
    JSContextRef ctx = (*scoped_context);

    BindGlobals(ctx, {
        {"__ul_SelectFbxFile", selectFbxFile},
        {"__ul_CloseWindow", CloseWindow},
        {"__ul_OpenAboutDialog", OpenAboutDialog},
        {"__ul_getFBXNode", getFBXNode},
        {"__ul_getFBXNodeChildren", getFBXNodeChildren}
    });
    
    caller->EvaluateScript("window.__ultralight._isAvailable = true;");
    caller->EvaluateScript("window.__remountApp();");
}

JSValueRef FbxexAppMain::CloseWindow(
    JSContextRef ctx,
    JSObjectRef /*function*/,
    JSObjectRef /*thisObject*/,
    size_t /*argumentCount*/,
    const JSValueRef /*arguments*/[],
    JSValueRef* /*exception*/) 
{
  if (instance_ && instance_->window_) { instance_->window_->Close(); }
  return JSValueMakeUndefined(ctx);
}

JSValueRef FbxexAppMain::OpenAboutDialog(
    JSContextRef ctx,
    JSObjectRef /*function*/,
    JSObjectRef /*thisObject*/,
    size_t /*argumentCount*/,
    const JSValueRef /*arguments*/[],
    JSValueRef* /*exception*/) 
{
    ultralight::ShowMessageBox("About", "FBX Explorer\nVersion 1.0.0");
    return JSValueMakeUndefined(ctx);
}

JSValueRef FbxexAppMain::selectFbxFile(
    JSContextRef ctx,
    JSObjectRef /*function*/,
    JSObjectRef /*thisObject*/,
    size_t /*argumentCount*/,
    const JSValueRef /*arguments*/[],
    JSValueRef* /*exception*/) 
{
    std::string path;
#ifdef _WIN32
    if (!instance_ || !instance_->window_) {
      return JSValueMakeBoolean(ctx, false);
    }
    std::wstring wpath = OpenFileDialogWin32(
        (HWND)instance_->window_->native_handle()
    );
    path = WStringToUtf8(wpath);
#endif
    if (path.empty()) {
      return JSValueMakeBoolean(ctx, false);
    }
    try {
      instance_->fbx_client_eager_ = std::make_unique<FBXClientEager>(path);
    } catch (const std::exception& ex) {
      ultralight::ShowMessageBox("Error", ex.what());
      return JSValueMakeBoolean(ctx, false);
    }
    return JSValueMakeBoolean(ctx, true);
}

JSValueRef FbxexAppMain::getFBXNode(
      JSContextRef ctx,
      JSObjectRef /*function*/,
      JSObjectRef /*thisObject*/,
      size_t /*argumentCount*/,
      const JSValueRef arguments[],
      JSValueRef* /*exception*/) 
{
    if (!instance_ || !instance_->fbx_client_eager_) {
      return JSValueMakeNull(ctx);
    }
    try {
      if (JSValueGetType(ctx, arguments[0]) != kJSTypeNumber) {
        return JSValueMakeNull(ctx);
      }
    } catch (...) {
      return JSValueMakeNull(ctx);
    }
    size_t requested_node_id = JSValueToNumber(ctx, arguments[0], nullptr);
    FBXNode requested_node(requested_node_id, *(instance_->fbx_client_eager_));
    return BindFBXNode(ctx, requested_node);
}

JSValueRef FbxexAppMain::getFBXNodeChildren(
    JSContextRef ctx,
    JSObjectRef /*function*/,
    JSObjectRef /*thisObject*/,
    size_t /*argumentCount*/,
    const JSValueRef arguments[],
    JSValueRef* /*exception*/) 
{
    if (!instance_ || !instance_->fbx_client_eager_) {
      return JSValueMakeNull(ctx);
    }
    try {
      if (JSValueGetType(ctx, arguments[0]) != kJSTypeNumber) {
        return JSValueMakeNull(ctx);
      }
    } catch (...) {
      return JSValueMakeNull(ctx);
    }
    size_t requested_node_id = JSValueToNumber(ctx, arguments[0], nullptr);
    FBXNode requested_node(requested_node_id, *(instance_->fbx_client_eager_));
    const auto children_ids = requested_node.getChildren();
    JSValueRef js_array = JSObjectMakeArray(ctx, 0, nullptr, nullptr);
    unsigned int idx = 0;
    for (size_t child_id : children_ids) 
    {
        JSValueRef js_child_id = JSValueMakeNumber(
            ctx, 
            static_cast<double>(child_id));
        JSObjectSetPropertyAtIndex(
            ctx, 
            (JSObjectRef)js_array, 
            idx++, 
            js_child_id, 
            nullptr);
    }
    return js_array;
}