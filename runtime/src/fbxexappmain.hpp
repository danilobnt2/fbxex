#pragma once
#include <string>
#include <memory>
#include <cstdlib>

#include <AppCore/AppCore.h>
#include <JavaScriptCore/JavaScript.h>

#include <fbxsdk.h>

#include "binders.hpp"
#include "fbxclienteager.hpp"
#include "fbxnode.hpp"
#include "embedded_filesystem.hpp"

#ifdef _WIN32
#include "winfio.hpp"
#endif


inline static constexpr uint32_t WINDOW_WIDTH = 1024u;
inline static constexpr uint32_t WINDOW_HEIGHT = 768u;

class IApp {
    public:
        virtual ~IApp() = default;
        virtual void Run() = 0;
        virtual void Quit() = 0;
        virtual ultralight::Monitor* main_monitor() = 0;
        virtual void set_listener(ultralight::AppListener* listener) = 0;
};

class IWindow {
    public:
        virtual ~IWindow() = default;
        virtual uint32_t width() const = 0;
        virtual uint32_t height() const = 0;
        virtual void* native_handle() = 0;
        virtual void SetCursor(ultralight::Cursor cursor) = 0;
        virtual void SetTitle(const char* title) = 0;
        virtual void Close() = 0;
        virtual void set_listener(ultralight::WindowListener* listener) = 0;
};

class IView {
    public:
        virtual ~IView() = default;
        virtual void LoadURL(const std::string& url) = 0;
        virtual void set_load_listener(ultralight::LoadListener* listener) = 0;
        virtual void set_view_listener(ultralight::ViewListener* listener) = 0;
        virtual void EvaluateScript(const std::string& script) = 0;
        virtual ultralight::RefPtr<ultralight::JSContext> LockJSContext() = 0;
};

class IOverlay {
    public:
        virtual ~IOverlay() = default;
        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual IView* view() = 0;
};

class IUiFactory {
    public:
        virtual ~IUiFactory() = default;
        virtual std::unique_ptr<IApp> CreateApp(const ultralight::Settings& settings, const ultralight::Config& config) = 0;
        virtual std::unique_ptr<IWindow> BuildWindow(IApp& app, uint32_t width, uint32_t height, bool fullscreen, uint32_t flags) = 0;
        virtual std::unique_ptr<IOverlay> CreateOverlay(IWindow& window) = 0;
        virtual std::string OpenFileDialog(void* native_window) = 0;
        virtual void ShowMessageBox(const std::string& title, const std::string& message) = 0;
};

class IFBXClientFactory {
    public:
        virtual ~IFBXClientFactory() = default;
        virtual std::unique_ptr<IFBXClient> Create(const std::string& path) = 0;
};

class DefaultUiFactory;
class DefaultFBXClientFactory;


class FbxexAppMain : public ultralight::AppListener,
                    public ultralight::WindowListener,
                    public ultralight::LoadListener,
                    public ultralight::ViewListener {
    public:
        explicit FbxexAppMain(const std::string& start_url);
        explicit FbxexAppMain(
            const std::string& start_url,
            std::unique_ptr<IUiFactory> ui_factory, 
            std::unique_ptr<IFBXClientFactory> client_factory);
        ~FbxexAppMain() override;

        inline void Run() { app_->Run(); }
  
        inline void OnUpdate() override {}
  
        inline void OnClose(ultralight::Window* /*window*/) override { if (app_) { app_->Quit(); } }
  
        inline void OnResize(
                ultralight::Window* window, 
                uint32_t width, 
                uint32_t height) override 
        { 
            if (overlay_) { overlay_->Resize(width, height); }
        }
  
        inline void OnFinishLoading(
            ultralight::View* caller, 
            uint64_t frame_id, 
            bool is_main_frame, 
            const ultralight::String& url) override 
        {}
  
        inline void OnChangeCursor(
            ultralight::View* caller, 
            ultralight::Cursor cursor) override 
        { 
            if (window_) { window_->SetCursor(cursor); }
        }

        inline void OnChangeTitle(
            ultralight::View* caller, 
            const ultralight::String& title) override 
        { 
            if (window_) { window_->SetTitle(title.utf8().data()); }
        }

        void OnDOMReady(
              ultralight::View* caller, 
              uint64_t frame_id, 
              bool is_main_frame, 
              const ultralight::String& url) override;

        static JSValueRef CloseWindow(
            JSContextRef ctx,
            JSObjectRef /*function*/,
            JSObjectRef /*thisObject*/,
            size_t /*argumentCount*/,
            const JSValueRef /*arguments*/[],
            JSValueRef* /*exception*/);

        static JSValueRef OpenAboutDialog(
            JSContextRef ctx,
            JSObjectRef /*function*/,
            JSObjectRef /*thisObject*/,
            size_t /*argumentCount*/,
            const JSValueRef /*arguments*/[],
            JSValueRef* /*exception*/);

        static JSValueRef selectFbxFile(
            JSContextRef ctx,
            JSObjectRef /*function*/,
            JSObjectRef /*thisObject*/,
            size_t /*argumentCount*/,
            const JSValueRef /*arguments*/[],
            JSValueRef* /*exception*/);

        static JSValueRef getAppVersion(
            JSContextRef ctx,
            JSObjectRef /*function*/,
            JSObjectRef /*thisObject*/,
            size_t /*argumentCount*/,
            const JSValueRef /*arguments*/[],
            JSValueRef* /*exception*/);

        static JSValueRef getFBXNode(
            JSContextRef ctx,
            JSObjectRef /*function*/,
            JSObjectRef /*thisObject*/,
            size_t /*argumentCount*/,
            const JSValueRef arguments[],
            JSValueRef* /*exception*/);

        static JSValueRef getFBXNodeChildren(
            JSContextRef ctx,
            JSObjectRef /*function*/,
            JSObjectRef /*thisObject*/,
            size_t /*argumentCount*/,
            const JSValueRef arguments[],
            JSValueRef* /*exception*/);

        inline static FbxexAppMain* instance_ = nullptr;
    
    private:
    
        void InitializeUi();
        
        std::unique_ptr<IFBXClient> fbx_client_;
        std::string start_url_;
        std::unique_ptr<IUiFactory> ui_factory_;
        std::unique_ptr<IFBXClientFactory> client_factory_;
        std::unique_ptr<IApp> app_;
        std::unique_ptr<IWindow> window_;
        std::unique_ptr<IOverlay> overlay_;
        std::unique_ptr<ultralight::FileSystem> file_system_;

};
