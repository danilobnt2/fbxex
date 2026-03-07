#include <catch2/catch_test_macros.hpp>

#include <JavaScriptCore/JavaScript.h>

#include "fbxexappmain.hpp"

#include <nlohmann/json.hpp>
#include <memory>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <atomic>

namespace {

std::string JsValueToString(JSContextRef ctx, JSValueRef value) {
    JSStringRef js_str = JSValueToStringCopy(ctx, value, nullptr);
    size_t max_length = JSStringGetMaximumUTF8CStringSize(js_str);
    std::string buffer(max_length, '\0');
    size_t written = JSStringGetUTF8CString(js_str, buffer.data(), max_length);
    JSStringRelease(js_str);
    if (written == 0) {
        return {};
    }
    buffer.resize(written - 1); // drop terminator
    return buffer;
}

class StubClient : public IFBXClient {
public:
    const FBXNodeProps* getNodeProps(size_t id) const override {
        auto it = props.find(id);
        if (it == props.end()) {
            return nullptr;
        }
        return &it->second;
    }
    std::vector<size_t> getNodeChildren(size_t id) const override {
        auto it = children.find(id);
        if (it == children.end()) {
            return {};
        }
        return it->second;
    }
    FBXFormat getFormat() const override { return format; }
    FBXFormat format = FBXFormat::Binary;
    std::unordered_map<size_t, FBXNodeProps> props;
    std::unordered_map<size_t, std::vector<size_t>> children;
};

class MockView : public IView {
public:
    void LoadURL(const std::string& url) override { last_url = url; load_called++; }
    void set_load_listener(ultralight::LoadListener* listener) override { load_listener = listener; }
    void set_view_listener(ultralight::ViewListener* listener) override { view_listener = listener; }
    void EvaluateScript(const std::string& script) override { scripts.push_back(script); }
    ultralight::RefPtr<ultralight::JSContext> LockJSContext() override { return context_ref; }

    std::string last_url;
    int load_called = 0;
    ultralight::LoadListener* load_listener = nullptr;
    ultralight::ViewListener* view_listener = nullptr;
    std::vector<std::string> scripts;
    ultralight::RefPtr<ultralight::JSContext> context_ref;
};

class MockOverlay : public IOverlay {
public:
    explicit MockOverlay(std::unique_ptr<MockView> v) : view_(std::move(v)) {}
    void Resize(uint32_t width, uint32_t height) override {
        resize_calls.emplace_back(width, height);
    }
    IView* view() override { return view_.get(); }
    std::vector<std::pair<uint32_t, uint32_t>> resize_calls;
    std::unique_ptr<MockView> view_;
};

class MockWindow : public IWindow {
public:
    MockWindow(uint32_t w, uint32_t h, void* handle) : width_(w), height_(h), handle_(handle) {}
    uint32_t width() const override { return width_; }
    uint32_t height() const override { return height_; }
    void* native_handle() override { return handle_; }
    void SetCursor(ultralight::Cursor cursor) override { last_cursor = cursor; }
    void SetTitle(const char* title) override { last_title = title ? std::string(title) : std::string(); }
    void Close() override { close_called++; }
    void set_listener(ultralight::WindowListener* listener) override { listener_ = listener; }

    uint32_t width_;
    uint32_t height_;
    void* handle_;
    int close_called = 0;
    ultralight::Cursor last_cursor = ultralight::kCursor_Hand;
    std::string last_title;
    ultralight::WindowListener* listener_ = nullptr;
};

class MockApp : public IApp {
public:
    void Run() override { run_called++; }
    void Quit() override { quit_called++; }
    ultralight::Monitor* main_monitor() override { return nullptr; }
    void set_listener(ultralight::AppListener* listener) override { listener_ = listener; }
    int run_called = 0;
    int quit_called = 0;
    ultralight::AppListener* listener_ = nullptr;
};

class DummyJSContext : public ultralight::JSContext {
public:
    explicit DummyJSContext(JSContextRef ctx) : ctx_(ctx) {}
    JSContextRef ctx() override { return ctx_; }
    void AddRef() const override { ref_count_++; }
    void Release() const override { if (--ref_count_ == 0) { delete this; } }
    int ref_count() const override { return static_cast<int>(ref_count_.load()); }
private:
    mutable std::atomic<int> ref_count_{1};
    JSContextRef ctx_;
};

class MockUiFactory : public IUiFactory {
public:
    std::unique_ptr<IApp> CreateApp(const ultralight::Settings&, const ultralight::Config&) override {
        auto created = std::make_unique<MockApp>();
        app = created.get();
        return created;
    }
    std::unique_ptr<IWindow> BuildWindow(IApp&, uint32_t width, uint32_t height, bool, uint32_t) override {
        auto created = std::make_unique<MockWindow>(width, height, fake_handle);
        window = created.get();
        return created;
    }
    std::unique_ptr<IOverlay> CreateOverlay(IWindow&) override {
        auto view = std::make_unique<MockView>();
        auto created = std::make_unique<MockOverlay>(std::move(view));
        overlay = created.get();
        overlay_view = static_cast<MockView*>(overlay->view());
        return created;
    }
    std::string OpenFileDialog(void* native_window) override {
        last_dialog_handle = native_window;
        return dialog_result;
    }
    void ShowMessageBox(const std::string& title, const std::string& message) override {
        last_message_title = title;
        last_message_body = message;
    }

    void* fake_handle = reinterpret_cast<void*>(0xDEADBEEF);
    MockApp* app = nullptr;
    MockWindow* window = nullptr;
    MockOverlay* overlay = nullptr;
    MockView* overlay_view = nullptr;
    void* last_dialog_handle = nullptr;
    std::string dialog_result;
    std::string last_message_title;
    std::string last_message_body;
};

class MockFBXClientFactory : public IFBXClientFactory {
public:
    std::unique_ptr<IFBXClient> Create(const std::string& path) override {
        last_path = path;
        if (throw_on_create) {
            throw std::runtime_error("fail");
        }
        auto produced = std::make_unique<StubClient>(*prototype);
        last_client = produced.get();
        return produced;
    }
    std::string last_path;
    bool throw_on_create = false;
    std::unique_ptr<StubClient> prototype = std::make_unique<StubClient>();
    StubClient* last_client = nullptr;
};

} // namespace

TEST_CASE("FbxexAppMain static helpers guard when instance is missing") {
    // Ensure no instance is set.
    FbxexAppMain::instance_ = nullptr;

    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    JSValueRef args[1];
    args[0] = JSValueMakeNumber(ctx, 1);

    SECTION("getFBXNode returns null without instance") {
        JSValueRef result = FbxexAppMain::getFBXNode(ctx, nullptr, nullptr, 1, args, nullptr);
        REQUIRE(JSValueIsNull(ctx, result));
    }

    SECTION("getFBXNodeChildren returns null without instance") {
        JSValueRef result = FbxexAppMain::getFBXNodeChildren(ctx, nullptr, nullptr, 1, args, nullptr);
        REQUIRE(JSValueIsNull(ctx, result));
    }

    SECTION("selectFbxFile returns false without instance") {
        JSValueRef result = FbxexAppMain::selectFbxFile(ctx, nullptr, nullptr, 0, nullptr, nullptr);
        REQUIRE(JSValueIsBoolean(ctx, result));
        REQUIRE(JSValueToBoolean(ctx, result) == false);
    }

    SECTION("getFBXFormat returns null without instance") {
        JSValueRef result = FbxexAppMain::getFBXFormat(ctx, nullptr, nullptr, 0, nullptr, nullptr);
        REQUIRE(JSValueIsNull(ctx, result));
    }

    JSGlobalContextRelease(ctx);
}

TEST_CASE("FbxexAppMain getFBXNode helpers use injected client") {
    auto ui_factory = std::make_unique<MockUiFactory>();
    MockUiFactory* ui_factory_ptr = ui_factory.get();
    auto client_factory = std::make_unique<MockFBXClientFactory>();
    MockFBXClientFactory* client_factory_ptr = client_factory.get();
    FBXNodeProps props;
    props.name = "Stubbed";
    props.properties.push_back(nlohmann::json::object({{"foo", 1}}));
    client_factory->prototype->props.emplace(5U, props);
    client_factory->prototype->children.emplace(5U, std::vector<size_t>{10U, 11U});

    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    {
        FbxexAppMain app("about:blank", std::move(ui_factory), std::move(client_factory));
        // Seed client via selectFbxFile using mock dialog and factory.
        ui_factory_ptr->dialog_result = "fakepath";
        JSValueRef select_result = FbxexAppMain::selectFbxFile(ctx, nullptr, nullptr, 0, nullptr, nullptr);
        REQUIRE(JSValueIsBoolean(ctx, select_result));
        REQUIRE(JSValueToBoolean(ctx, select_result));
        REQUIRE(client_factory_ptr->last_path == "fakepath");

        JSValueRef args[1];
        args[0] = JSValueMakeNumber(ctx, 5);

        JSValueRef node_value = FbxexAppMain::getFBXNode(ctx, nullptr, nullptr, 1, args, nullptr);
        REQUIRE(JSValueIsObject(ctx, node_value));
        JSObjectRef node_obj = JSValueToObject(ctx, node_value, nullptr);
        JSStringRef id_key = JSStringCreateWithUTF8CString("id");
        JSValueRef id_value = JSObjectGetProperty(ctx, node_obj, id_key, nullptr);
        JSStringRelease(id_key);
        REQUIRE(JSValueIsNumber(ctx, id_value));
        REQUIRE(JSValueToNumber(ctx, id_value, nullptr) == 5.0);

        JSStringRef props_key = JSStringCreateWithUTF8CString("props");
        JSValueRef props_value = JSObjectGetProperty(ctx, node_obj, props_key, nullptr);
        JSStringRelease(props_key);
        REQUIRE(JSValueIsObject(ctx, props_value));

        JSObjectRef props_obj = JSValueToObject(ctx, props_value, nullptr);
        JSStringRef name_key = JSStringCreateWithUTF8CString("name");
        JSValueRef name_value = JSObjectGetProperty(ctx, props_obj, name_key, nullptr);
        JSStringRelease(name_key);
        REQUIRE(JSValueIsString(ctx, name_value));
        REQUIRE(JsValueToString(ctx, name_value) == "Stubbed");

        JSValueRef children_value = FbxexAppMain::getFBXNodeChildren(ctx, nullptr, nullptr, 1, args, nullptr);
        REQUIRE(JSValueIsObject(ctx, children_value));
        JSObjectRef children_array = JSValueToObject(ctx, children_value, nullptr);
        JSValueRef child0 = JSObjectGetPropertyAtIndex(ctx, children_array, 0, nullptr);
        JSValueRef child1 = JSObjectGetPropertyAtIndex(ctx, children_array, 1, nullptr);
        REQUIRE(JSValueIsNumber(ctx, child0));
        REQUIRE(JSValueIsNumber(ctx, child1));
        REQUIRE(JSValueToNumber(ctx, child0, nullptr) == 10.0);
        REQUIRE(JSValueToNumber(ctx, child1, nullptr) == 11.0);

        SECTION("non-number arguments return null") {
            JSStringRef s = JSStringCreateWithUTF8CString("bad");
            JSValueRef bad_arg = JSValueMakeString(ctx, s);
            JSStringRelease(s);
            JSValueRef args_bad[1] = {bad_arg};
            JSValueRef null_node = FbxexAppMain::getFBXNode(ctx, nullptr, nullptr, 1, args_bad, nullptr);
            JSValueRef null_children = FbxexAppMain::getFBXNodeChildren(ctx, nullptr, nullptr, 1, args_bad, nullptr);
            REQUIRE(JSValueIsNull(ctx, null_node));
            REQUIRE(JSValueIsNull(ctx, null_children));
        }
    }

    JSGlobalContextRelease(ctx);
}

TEST_CASE("FbxexAppMain getFBXFormat returns format string from client") {
    auto ui_factory = std::make_unique<MockUiFactory>();
    MockUiFactory* ui_factory_ptr = ui_factory.get();
    auto client_factory = std::make_unique<MockFBXClientFactory>();
    MockFBXClientFactory* client_factory_ptr = client_factory.get();

    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    {
        FbxexAppMain app("about:blank", std::move(ui_factory), std::move(client_factory));

        SECTION("returns null when no client is loaded") {
            JSValueRef result = FbxexAppMain::getFBXFormat(ctx, nullptr, nullptr, 0, nullptr, nullptr);
            REQUIRE(JSValueIsNull(ctx, result));
        }

        SECTION("returns \"binary\" when client reports binary format") {
            client_factory_ptr->prototype->format = FBXFormat::Binary;
            ui_factory_ptr->dialog_result = "fakepath";
            FbxexAppMain::selectFbxFile(ctx, nullptr, nullptr, 0, nullptr, nullptr);

            JSValueRef result = FbxexAppMain::getFBXFormat(ctx, nullptr, nullptr, 0, nullptr, nullptr);
            REQUIRE(JSValueIsString(ctx, result));
            REQUIRE(JsValueToString(ctx, result) == "binary");
        }

        SECTION("returns \"ascii\" when client reports ASCII format") {
            client_factory_ptr->prototype->format = FBXFormat::ASCII;
            ui_factory_ptr->dialog_result = "fakepath";
            FbxexAppMain::selectFbxFile(ctx, nullptr, nullptr, 0, nullptr, nullptr);

            JSValueRef result = FbxexAppMain::getFBXFormat(ctx, nullptr, nullptr, 0, nullptr, nullptr);
            REQUIRE(JSValueIsString(ctx, result));
            REQUIRE(JsValueToString(ctx, result) == "ascii");
        }
    }

    JSGlobalContextRelease(ctx);
}

TEST_CASE("FbxexAppMain getAppVersion returns a string") {
    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    JSValueRef result = FbxexAppMain::getAppVersion(ctx, nullptr, nullptr, 0, nullptr, nullptr);
    REQUIRE(JSValueIsString(ctx, result));

    JSStringRef js_str = JSValueToStringCopy(ctx, result, nullptr);
    size_t length = JSStringGetMaximumUTF8CStringSize(js_str);
    std::string version(length, '\0');
    JSStringGetUTF8CString(js_str, version.data(), length);
    JSStringRelease(js_str);
    if (!version.empty() && version.back() == '\0') {
        version.pop_back();
    }
    REQUIRE_FALSE(version.empty());

    JSGlobalContextRelease(ctx);
}

TEST_CASE("FbxexAppMain OnDOMReady binds globals and runs scripts") {
    auto ui_factory = std::make_unique<MockUiFactory>();
    MockUiFactory* ui_factory_ptr = ui_factory.get();
    auto client_factory = std::make_unique<MockFBXClientFactory>();

    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    FbxexAppMain app("about:blank", std::move(ui_factory), std::move(client_factory));
    REQUIRE(ui_factory_ptr->overlay_view != nullptr);
    ui_factory_ptr->overlay_view->context_ref = ultralight::AdoptRef(*new DummyJSContext(ctx));

    app.OnDOMReady(nullptr, 0, true, ultralight::String("about:blank"));

    REQUIRE(ui_factory_ptr->overlay_view->scripts.size() == 2);
    REQUIRE(ui_factory_ptr->overlay_view->scripts[0].find("_isAvailable") != std::string::npos);
    REQUIRE(ui_factory_ptr->overlay_view->scripts[1].find("__remountApp") != std::string::npos);

    JSGlobalContextRelease(ctx);
}

TEST_CASE("FbxexAppMain initializes UI through factories and wires listeners") {
    auto ui_factory = std::make_unique<MockUiFactory>();
    MockUiFactory* factory_ptr = ui_factory.get();
    auto client_factory = std::make_unique<MockFBXClientFactory>();

    FbxexAppMain app("https://example.test/", std::move(ui_factory), std::move(client_factory));

    REQUIRE(factory_ptr->app != nullptr);
    REQUIRE(factory_ptr->window != nullptr);
    REQUIRE(factory_ptr->overlay != nullptr);
    REQUIRE(factory_ptr->overlay->resize_calls.size() == 1);
    REQUIRE(factory_ptr->overlay->resize_calls[0].first == factory_ptr->window->width_);
    REQUIRE(factory_ptr->overlay->resize_calls[0].second == factory_ptr->window->height_);
    REQUIRE(factory_ptr->overlay_view != nullptr);
    REQUIRE(factory_ptr->overlay_view->load_called == 1);
    REQUIRE(factory_ptr->overlay_view->last_url == "https://example.test/");

    // listener forwards
    app.OnResize(nullptr, 640, 480);
    REQUIRE(factory_ptr->overlay->resize_calls.size() == 2);
    app.OnChangeTitle(nullptr, ultralight::String("NewTitle"));
    REQUIRE(factory_ptr->window->last_title == "NewTitle");
    app.OnChangeCursor(nullptr, ultralight::kCursor_Cross);
    REQUIRE(factory_ptr->window->last_cursor == ultralight::kCursor_Cross);
    app.OnClose(nullptr);
    REQUIRE(factory_ptr->app->quit_called == 1);
}

#ifdef _WIN32
TEST_CASE("FbxexAppMain selectFbxFile uses factories for dialog and client creation") {
    auto ui_factory = std::make_unique<MockUiFactory>();
    MockUiFactory* factory_ptr = ui_factory.get();
    auto client_factory = std::make_unique<MockFBXClientFactory>();
    MockFBXClientFactory* client_factory_ptr = client_factory.get();

    FbxexAppMain app("about:blank", std::move(ui_factory), std::move(client_factory));

    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    SECTION("returns false when dialog cancelled") {
        factory_ptr->dialog_result.clear();
        JSValueRef result = FbxexAppMain::selectFbxFile(ctx, nullptr, nullptr, 0, nullptr, nullptr);
        REQUIRE(JSValueIsBoolean(ctx, result));
        REQUIRE(JSValueToBoolean(ctx, result) == false);
    }

    SECTION("returns false and shows message when client creation throws") {
        factory_ptr->dialog_result = "fakepath";
        client_factory_ptr->throw_on_create = true;
        JSValueRef result = FbxexAppMain::selectFbxFile(ctx, nullptr, nullptr, 0, nullptr, nullptr);
        REQUIRE(JSValueIsBoolean(ctx, result));
        REQUIRE(JSValueToBoolean(ctx, result) == false);
        REQUIRE(factory_ptr->last_message_title == "Error");
        REQUIRE_FALSE(factory_ptr->last_message_body.empty());
    }

    SECTION("returns true and stores client when dialog returns path") {
        factory_ptr->dialog_result = "fakepath";
        JSValueRef result = FbxexAppMain::selectFbxFile(ctx, nullptr, nullptr, 0, nullptr, nullptr);
        REQUIRE(JSValueIsBoolean(ctx, result));
        REQUIRE(JSValueToBoolean(ctx, result) == true);
        REQUIRE(client_factory_ptr->last_path == "fakepath");
    }

    JSGlobalContextRelease(ctx);
}
#endif

TEST_CASE("FbxexAppMain CloseWindow uses current window instance") {
    auto ui_factory = std::make_unique<MockUiFactory>();
    MockUiFactory* factory_ptr = ui_factory.get();
    auto client_factory = std::make_unique<MockFBXClientFactory>();

    FbxexAppMain app("about:blank", std::move(ui_factory), std::move(client_factory));
    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    JSValueRef result = FbxexAppMain::CloseWindow(ctx, nullptr, nullptr, 0, nullptr, nullptr);
    REQUIRE(JSValueIsUndefined(ctx, result));
    REQUIRE(factory_ptr->window->close_called == 1);

    JSGlobalContextRelease(ctx);
}
