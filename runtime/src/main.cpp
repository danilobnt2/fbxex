#include <string>
#include <memory>

#include <AppCore/AppCore.h>
#include <JavaScriptCore/JavaScript.h>

#include <fbxsdk.h>

#include "binders.hpp"
#include "fbxclienteager.hpp"
#include "fbxnode.hpp"

#ifdef _WIN32
#include "winfio.hpp"
#endif

static constexpr uint32_t WINDOW_WIDTH = 1024u;
static constexpr uint32_t WINDOW_HEIGHT = 768u;


struct FbxVersion {
  int major = 0;
  int minor = 0;
  int revision = 0;
  bool ok = false;
};

FbxVersion QueryFbxVersion() {
  FbxVersion version;
  FbxManager* fbx_manager = FbxManager::Create();
  if (!fbx_manager) {
    return version;
  }
  fbx_manager->GetFileFormatVersion(version.major, version.minor, version.revision);
  version.ok = true;
  fbx_manager->Destroy();
  return version;
}

JSValueRef getFbxFileFormatVersion(JSContextRef ctx, JSObjectRef function,
  JSObjectRef thisObject, size_t argumentCount, 
  const JSValueRef arguments[], JSValueRef* exception) {

  auto fbx_version_ = QueryFbxVersion();
  if (!fbx_version_.ok) {
    return JSValueMakeNull(ctx);
  }

  std::string version_text = 
    std::to_string(fbx_version_.major) + "." +
    std::to_string(fbx_version_.minor) + "." + 
    std::to_string(fbx_version_.revision);
  
  return JSValueMakeString(ctx, JSStringCreateWithUTF8CString(version_text.c_str()));
}

class MyApp final : public ultralight::AppListener,
                    public ultralight::WindowListener,
                    public ultralight::LoadListener,
                    public ultralight::ViewListener {
 public:
  MyApp() {
    instance_ = this;
    fbx_client_eager_ = nullptr;
    app_ = ultralight::App::Create();
    auto window_flags = 
        ultralight::kWindowFlags_Titled 
      | ultralight::kWindowFlags_Resizable;
    window_ = ultralight::Window::Create(
      app_->main_monitor(), 
      WINDOW_WIDTH, 
      WINDOW_HEIGHT, 
      false, 
      window_flags);
    overlay_ = ultralight::Overlay::Create(window_, 1, 1, 0, 0);
    OnResize(window_.get(), window_->width(), window_->height());
    overlay_->view()->LoadURL("file:///index.html");
    app_->set_listener(this);
    window_->set_listener(this);
    overlay_->view()->set_load_listener(this);
    overlay_->view()->set_view_listener(this);
  }

  ~MyApp() override = default;

  void Run() { app_->Run(); }

  void OnUpdate() override {}

  void OnClose(ultralight::Window* window) override { app_->Quit(); }

  void OnResize(ultralight::Window* window, uint32_t width, uint32_t height) override { overlay_->Resize(width, height); }

  void OnFinishLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url) override {}

  void OnDOMReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url) override {

    auto scoped_context = caller->LockJSContext(); 
    JSContextRef ctx = (*scoped_context);

    BindGlobals(ctx, {
      {"__ul_SelectFbxFile", selectFbxFile},
      {"__ul_CloseWindow", CloseWindow},
      {"__ul_OpenAboutDialog", OpenAboutDialog},
      {"__ul_getFbxFileFormatVersion", getFbxFileFormatVersion},
      {"__ul_getFBXNode", getFBXNode},
      {"__ul_getFBXNodeChildren", getFBXNodeChildren}
    });

    caller->EvaluateScript("window.__ultralight._isAvailable = true;");
    caller->EvaluateScript("window.__remountApp();");

  }

  static JSValueRef CloseWindow(
      JSContextRef ctx,
      JSObjectRef /*function*/,
      JSObjectRef /*thisObject*/,
      size_t /*argumentCount*/,
      const JSValueRef /*arguments*/[],
      JSValueRef* /*exception*/) 
  {
    if (instance_ && instance_->window_) {
      instance_->window_->Close();
    }
    return JSValueMakeUndefined(ctx);
  }

  static JSValueRef OpenAboutDialog(
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

  static JSValueRef selectFbxFile(
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
    std::wstring wpath = OpenFileDialogWin32((HWND)instance_->window_->native_handle());
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

  static JSValueRef getFBXNode(
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

  static JSValueRef getFBXNodeChildren(
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
    for (size_t child_id : children_ids) {
      JSValueRef js_child_id = JSValueMakeNumber(ctx, static_cast<double>(child_id));
      JSObjectSetPropertyAtIndex(ctx, (JSObjectRef)js_array, idx++, js_child_id, nullptr);
    }
    return js_array;
  }

  inline static MyApp* instance_ = nullptr;

  void OnChangeCursor(ultralight::View* caller, ultralight::Cursor cursor) override { window_->SetCursor(cursor); }

  void OnChangeTitle(ultralight::View* caller, const ultralight::String& title) override { window_->SetTitle(title.utf8().data()); }

 private:
  ultralight::RefPtr<ultralight::App> app_;
  ultralight::RefPtr<ultralight::Window> window_;
  ultralight::RefPtr<ultralight::Overlay> overlay_;
  std::unique_ptr<FBXClientEager> fbx_client_eager_;

};

int main() {
  MyApp app;
  app.Run();
  return 0;
}
