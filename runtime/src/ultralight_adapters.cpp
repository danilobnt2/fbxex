#include "ultralight_adapters.hpp"
#include "winfio.hpp"
#include "fbxclienteager.hpp"

UltralightAppAdapter::UltralightAppAdapter(ultralight::RefPtr<ultralight::App> app) : app_(std::move(app)) {}
void UltralightAppAdapter::Run() { app_->Run(); }
void UltralightAppAdapter::Quit() { app_->Quit(); }
ultralight::Monitor* UltralightAppAdapter::main_monitor() { return app_->main_monitor(); }
void UltralightAppAdapter::set_listener(ultralight::AppListener* listener) { app_->set_listener(listener); }
ultralight::App* UltralightAppAdapter::raw() { return app_.get(); }

UltralightWindowAdapter::UltralightWindowAdapter(ultralight::RefPtr<ultralight::Window> window) : window_(std::move(window)) {}
uint32_t UltralightWindowAdapter::width() const { return window_->width(); }
uint32_t UltralightWindowAdapter::height() const { return window_->height(); }
void* UltralightWindowAdapter::native_handle() { return window_->native_handle(); }
void UltralightWindowAdapter::SetCursor(ultralight::Cursor cursor) { window_->SetCursor(cursor); }
void UltralightWindowAdapter::SetTitle(const char* title) { window_->SetTitle(title); }
void UltralightWindowAdapter::Close() { window_->Close(); }
void UltralightWindowAdapter::set_listener(ultralight::WindowListener* listener) { window_->set_listener(listener); }
ultralight::Window* UltralightWindowAdapter::raw() { return window_.get(); }

UltralightViewAdapter::UltralightViewAdapter(ultralight::RefPtr<ultralight::View> view) : view_(std::move(view)) {}
void UltralightViewAdapter::LoadURL(const std::string& url) { view_->LoadURL(url.c_str()); }
void UltralightViewAdapter::set_load_listener(ultralight::LoadListener* listener) { view_->set_load_listener(listener); }
void UltralightViewAdapter::set_view_listener(ultralight::ViewListener* listener) { view_->set_view_listener(listener); }
void UltralightViewAdapter::EvaluateScript(const std::string& script) { view_->EvaluateScript(script.c_str()); }
ultralight::RefPtr<ultralight::JSContext> UltralightViewAdapter::LockJSContext() { return view_->LockJSContext(); }
ultralight::View* UltralightViewAdapter::raw() { return view_.get(); }

UltralightOverlayAdapter::UltralightOverlayAdapter(ultralight::RefPtr<ultralight::Overlay> overlay, std::unique_ptr<IView> view)
    : overlay_(std::move(overlay)), view_(std::move(view)) {}
void UltralightOverlayAdapter::Resize(uint32_t width, uint32_t height) { overlay_->Resize(width, height); }
IView* UltralightOverlayAdapter::view() { return view_.get(); }

std::unique_ptr<IApp> DefaultUiFactory::CreateApp(const ultralight::Settings& settings, const ultralight::Config& config) {
    return std::make_unique<UltralightAppAdapter>(ultralight::App::Create(settings, config));
}

std::unique_ptr<IWindow> DefaultUiFactory::BuildWindow(IApp& app, uint32_t width, uint32_t height, bool fullscreen, uint32_t flags) {
    auto* app_adapter = dynamic_cast<UltralightAppAdapter*>(&app);
    if (!app_adapter) { return nullptr; }
    auto window = ultralight::Window::Create(app_adapter->raw()->main_monitor(), width, height, fullscreen, flags);
    return std::make_unique<UltralightWindowAdapter>(window);
}

std::unique_ptr<IOverlay> DefaultUiFactory::CreateOverlay(IWindow& window) {
    auto* window_adapter = dynamic_cast<UltralightWindowAdapter*>(&window);
    if (!window_adapter) { return nullptr; }
    auto overlay = ultralight::Overlay::Create(window_adapter->raw(), 1, 1, 0, 0);
    auto view_adapter = std::make_unique<UltralightViewAdapter>(overlay->view());
    return std::make_unique<UltralightOverlayAdapter>(overlay, std::move(view_adapter));
}

std::string DefaultUiFactory::OpenFileDialog(void* native_window) {
#ifdef _WIN32
    std::wstring wpath = OpenFileDialogWin32(static_cast<HWND>(native_window));
    return WStringToUtf8(wpath);
#else
    (void)native_window;
    return {};
#endif
}

void DefaultUiFactory::ShowMessageBox(const std::string& title, const std::string& message) {
    ultralight::ShowMessageBox(title.c_str(), message.c_str());
}

std::unique_ptr<IFBXClient> DefaultFBXClientFactory::Create(const std::string& path) {
    return std::make_unique<FBXClientEager>(path);
}
