#include <string>

#include <AppCore/AppCore.h>
#include <JavaScriptCore/JavaScript.h>

#include <fbxsdk.h>

#include "binders.hpp"

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
  explicit MyApp(FbxVersion fbx_version) : fbx_version_(fbx_version) {
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
      {"__ul_getFbxFileFormatVersion", getFbxFileFormatVersion}
    });

    caller->EvaluateScript("window.__ultralight._isAvailable = true;");
    caller->EvaluateScript("window.__remountApp();");

  }

  void OnChangeCursor(ultralight::View* caller, ultralight::Cursor cursor) override { window_->SetCursor(cursor); }

  void OnChangeTitle(ultralight::View* caller, const ultralight::String& title) override { window_->SetTitle(title.utf8().data()); }

 private:
  FbxVersion fbx_version_;
  ultralight::RefPtr<ultralight::App> app_;
  ultralight::RefPtr<ultralight::Window> window_;
  ultralight::RefPtr<ultralight::Overlay> overlay_;
};

int main() {
  FbxVersion fbx_version = QueryFbxVersion();
  MyApp app(fbx_version);
  app.Run();
  return 0;
}
