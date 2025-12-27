#include <catch2/catch_test_macros.hpp>

#include <JavaScriptCore/JavaScript.h>

#include "fbxexappmain.hpp"

#include <nlohmann/json.hpp>
#include <memory>
#include <unordered_map>

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
    std::unordered_map<size_t, FBXNodeProps> props;
    std::unordered_map<size_t, std::vector<size_t>> children;
};

class TestFbxexAppMain : public FbxexAppMain {
public:
    explicit TestFbxexAppMain(std::unique_ptr<IFBXClient> client)
        : FbxexAppMain(NoUiInitTag{}, std::move(client)) {}
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

    JSGlobalContextRelease(ctx);
}

TEST_CASE("FbxexAppMain getFBXNode helpers use injected client") {
    auto client = std::make_unique<StubClient>();
    FBXNodeProps props;
    props.name = "Stubbed";
    props.properties.push_back(nlohmann::json::object({{"foo", 1}}));
    client->props.emplace(5U, props);
    client->children.emplace(5U, std::vector<size_t>{10U, 11U});

    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    {
        TestFbxexAppMain app(std::move(client));

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
