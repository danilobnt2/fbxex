#pragma once

#include <memory>
#include <AppCore/AppCore.h>
#include <Ultralight/Ultralight.h>

#include "fbxexappmain.hpp"

class UltralightAppAdapter : public IApp {
public:
    explicit UltralightAppAdapter(ultralight::RefPtr<ultralight::App> app);
    void Run() override;
    void Quit() override;
    ultralight::Monitor* main_monitor() override;
    void set_listener(ultralight::AppListener* listener) override;
    ultralight::App* raw();
private:
    ultralight::RefPtr<ultralight::App> app_;
};

class UltralightWindowAdapter : public IWindow {
public:
    explicit UltralightWindowAdapter(ultralight::RefPtr<ultralight::Window> window);
    uint32_t width() const override;
    uint32_t height() const override;
    void* native_handle() override;
    void SetCursor(ultralight::Cursor cursor) override;
    void SetTitle(const char* title) override;
    void Close() override;
    void set_listener(ultralight::WindowListener* listener) override;
    ultralight::Window* raw();
private:
    ultralight::RefPtr<ultralight::Window> window_;
};

class UltralightViewAdapter : public IView {
public:
    explicit UltralightViewAdapter(ultralight::RefPtr<ultralight::View> view);
    void LoadURL(const std::string& url) override;
    void set_load_listener(ultralight::LoadListener* listener) override;
    void set_view_listener(ultralight::ViewListener* listener) override;
    void EvaluateScript(const std::string& script) override;
    ultralight::RefPtr<ultralight::JSContext> LockJSContext() override;
    ultralight::View* raw();
private:
    ultralight::RefPtr<ultralight::View> view_;
};

class UltralightOverlayAdapter : public IOverlay {
public:
    explicit UltralightOverlayAdapter(ultralight::RefPtr<ultralight::Overlay> overlay, std::unique_ptr<IView> view);
    void Resize(uint32_t width, uint32_t height) override;
    IView* view() override;
private:
    ultralight::RefPtr<ultralight::Overlay> overlay_;
    std::unique_ptr<IView> view_;
};

class DefaultUiFactory : public IUiFactory {
public:
    std::unique_ptr<IApp> CreateApp(const ultralight::Settings& settings, const ultralight::Config& config) override;
    std::unique_ptr<IWindow> BuildWindow(IApp& app, uint32_t width, uint32_t height, bool fullscreen, uint32_t flags) override;
    std::unique_ptr<IOverlay> CreateOverlay(IWindow& window) override;
    std::string OpenFileDialog(void* native_window) override;
    void ShowMessageBox(const std::string& title, const std::string& message) override;
};

class DefaultFBXClientFactory : public IFBXClientFactory {
public:
    std::unique_ptr<IFBXClient> Create(const std::string& path) override;
};
