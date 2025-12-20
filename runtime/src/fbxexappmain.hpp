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

#ifdef _WIN32
#include "winfio.hpp"
#endif


inline static constexpr uint32_t WINDOW_WIDTH = 1024u;
inline static constexpr uint32_t WINDOW_HEIGHT = 768u;


class FbxexAppMain final : public ultralight::AppListener,
                    public ultralight::WindowListener,
                    public ultralight::LoadListener,
                    public ultralight::ViewListener {
    public:
        explicit FbxexAppMain(const std::string& start_url);
        ~FbxexAppMain() override = default;

        inline void Run() { app_->Run(); }
  
        inline void OnUpdate() override {}
  
        inline void OnClose(ultralight::Window* window) override { app_->Quit(); }
  
        inline void OnResize(
                ultralight::Window* window, 
                uint32_t width, 
                uint32_t height) override 
        { 
            overlay_->Resize(width, height); 
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
            window_->SetCursor(cursor); 
        }

        inline void OnChangeTitle(
            ultralight::View* caller, 
            const ultralight::String& title) override 
        { 
            window_->SetTitle(title.utf8().data()); 
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
    
        ultralight::RefPtr<ultralight::App> app_;
        ultralight::RefPtr<ultralight::Window> window_;
        ultralight::RefPtr<ultralight::Overlay> overlay_;
        std::unique_ptr<FBXClientEager> fbx_client_eager_;
        std::string start_url_;

};
