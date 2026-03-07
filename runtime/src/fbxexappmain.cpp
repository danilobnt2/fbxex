#include <string>
#include <memory>
#include <cstdlib>
#include <utility>
#include <stdexcept>

#include <AppCore/AppCore.h>
#include <JavaScriptCore/JavaScript.h>
#include <Ultralight/platform/Platform.h>

#include "binders.hpp"
#include "fbxnode.hpp"
#include "ultralight_adapters.hpp"

#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif
#ifndef DWMWA_TEXT_COLOR
#define DWMWA_TEXT_COLOR 36
#endif
#pragma comment(lib, "Dwmapi.lib")
#include "winfio.hpp"
#include "resource.h"
#endif

#include "fbxexappmain.hpp"

#ifndef FBXEX_VERSION
#define FBXEX_VERSION "unknown"
#endif

namespace {
constexpr const char* kAppVersion = FBXEX_VERSION;
}

FbxexAppMain::FbxexAppMain(const std::string& start_url) 
    : start_url_(start_url)
    , ui_factory_(std::make_unique<DefaultUiFactory>())
    , client_factory_(std::make_unique<DefaultFBXClientFactory>())
{
    instance_ = this;
    InitializeUi();
}

FbxexAppMain::FbxexAppMain(
        const std::string& start_url,
        std::unique_ptr<IUiFactory> ui_factory, 
        std::unique_ptr<IFBXClientFactory> client_factory)
    : start_url_(start_url)
    , ui_factory_(std::move(ui_factory))
    , client_factory_(std::move(client_factory))
{
    instance_ = this;
    InitializeUi();
}

FbxexAppMain::~FbxexAppMain() {
    instance_ = nullptr;
}

void FbxexAppMain::InitializeUi() {
    if (!ui_factory_) {
        ui_factory_ = std::make_unique<DefaultUiFactory>();
    }
    if (!client_factory_) {
        client_factory_ = std::make_unique<DefaultFBXClientFactory>();
    }

    if (!file_system_) {
        file_system_ = CreateEmbeddedFileSystem();
        ultralight::Platform::instance().set_file_system(file_system_.get());
    }

    ultralight::Settings settings;
    settings.app_name = "fbxex";
    ultralight::Config config;
    config.resource_path_prefix = "resources/";
    config.force_repaint = true; // needed for css scrollbars to work properly
    app_ = ui_factory_->CreateApp(settings, config);
    if (!app_) { throw std::runtime_error("Failed to create Ultralight App"); }
    window_ = ui_factory_->BuildWindow(*app_, WINDOW_WIDTH, WINDOW_HEIGHT, false,
        ultralight::kWindowFlags_Titled | ultralight::kWindowFlags_Resizable);
    if (!window_) { throw std::runtime_error("Failed to create Ultralight Window"); }

#ifdef _WIN32
    const auto hwnd = static_cast<HWND>(window_->native_handle());
    if (hwnd) {
        const HICON icon = static_cast<HICON>(LoadImage(
            GetModuleHandle(nullptr),
            MAKEINTRESOURCE(IDI_APP_ICON),
            IMAGE_ICON,
            0,
            0,
            LR_DEFAULTSIZE));
        if (icon) {
            SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
        }

        const COLORREF caption = RGB(20, 21, 24);
        const COLORREF text = RGB(232, 234, 237);
        const BOOL dark = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
        DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &caption, sizeof(caption));
        DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &text, sizeof(text));

        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style |= WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        SetWindowLong(hwnd, GWL_STYLE, style);
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
#endif

    overlay_ = ui_factory_->CreateOverlay(*window_);
    if (!overlay_) { throw std::runtime_error("Failed to create Ultralight Overlay"); }
    overlay_->Resize(window_->width(), window_->height());
    if (auto* view = overlay_->view()) {
        view->LoadURL(start_url_);
        view->set_load_listener(this);
        view->set_view_listener(this);
    }
    app_->set_listener(this);
    window_->set_listener(this);
}

void FbxexAppMain::OnDOMReady(
    ultralight::View* caller, 
    uint64_t frame_id, 
    bool is_main_frame, 
    const ultralight::String& url) 
{
    IView* view_iface = overlay_ ? overlay_->view() : nullptr;
    if (!view_iface) { return; }
    auto scoped_context = view_iface->LockJSContext(); 
    if (!scoped_context) { return; }
    JSContextRef ctx = (*scoped_context);

    BindGlobals(ctx, {
        {"__ul_SelectFbxFile", selectFbxFile},
        {"__ul_CloseWindow", CloseWindow},
        {"__ul_OpenAboutDialog", OpenAboutDialog},
        {"__ul_getAppVersion", getAppVersion},
        {"__ul_getFBXNode", getFBXNode},
        {"__ul_getFBXNodeChildren", getFBXNodeChildren},
        {"__ul_getFBXFormat", getFBXFormat}
    });
    
    view_iface->EvaluateScript("window.__ultralight._isAvailable = true;");
    view_iface->EvaluateScript("window.__remountApp();");
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
    const std::string message = std::string("fbxex\nVersion ") + kAppVersion;
    ultralight::ShowMessageBox("About", message.c_str());
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
    if (!instance_ || 
        !instance_->window_ || 
        !instance_->ui_factory_ || 
        !instance_->client_factory_) {
      return JSValueMakeBoolean(ctx, false);
    }
    std::string path;
#ifdef _WIN32
    path = instance_->ui_factory_->OpenFileDialog(instance_->window_->native_handle());
#endif
    if (path.empty()) {
      return JSValueMakeBoolean(ctx, false);
    }
    try {
      instance_->fbx_client_ = instance_->client_factory_->Create(path);
    } catch (const std::exception& ex) {
      instance_->ui_factory_->ShowMessageBox("Error", ex.what());
      return JSValueMakeBoolean(ctx, false);
    }
    return JSValueMakeBoolean(ctx, true);
}

JSValueRef FbxexAppMain::getAppVersion(
    JSContextRef ctx,
    JSObjectRef /*function*/,
    JSObjectRef /*thisObject*/,
    size_t /*argumentCount*/,
    const JSValueRef /*arguments*/[],
    JSValueRef* /*exception*/)
{
    JSStringRef version = JSStringCreateWithUTF8CString(kAppVersion);
    JSValueRef result = JSValueMakeString(ctx, version);
    JSStringRelease(version);
    return result;
}

JSValueRef FbxexAppMain::getFBXNode(
      JSContextRef ctx,
      JSObjectRef /*function*/,
      JSObjectRef /*thisObject*/,
      size_t /*argumentCount*/,
      const JSValueRef arguments[],
      JSValueRef* /*exception*/) 
{
    if (!instance_ || !instance_->fbx_client_) {
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
    FBXNode requested_node(requested_node_id, *(instance_->fbx_client_));
    return BindFBXNode(ctx, requested_node);
}

JSValueRef FbxexAppMain::getFBXFormat(
    JSContextRef ctx,
    JSObjectRef /*function*/,
    JSObjectRef /*thisObject*/,
    size_t /*argumentCount*/,
    const JSValueRef /*arguments*/[],
    JSValueRef* /*exception*/)
{
    if (!instance_ || !instance_->fbx_client_) {
        return JSValueMakeNull(ctx);
    }
    const char* format_str = instance_->fbx_client_->getFormat() == FBXFormat::ASCII
        ? "ascii"
        : "binary";
    JSStringRef js_str = JSStringCreateWithUTF8CString(format_str);
    JSValueRef result = JSValueMakeString(ctx, js_str);
    JSStringRelease(js_str);
    return result;
}

JSValueRef FbxexAppMain::getFBXNodeChildren(
    JSContextRef ctx,
    JSObjectRef /*function*/,
    JSObjectRef /*thisObject*/,
    size_t /*argumentCount*/,
    const JSValueRef arguments[],
    JSValueRef* /*exception*/) 
{
    if (!instance_ || !instance_->fbx_client_) {
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
    FBXNode requested_node(requested_node_id, *(instance_->fbx_client_));
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
