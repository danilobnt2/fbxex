#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nlohmann/json.hpp>

#include <JavaScriptCore/JavaScript.h>

#include "binders.hpp"

// Forward declarations for internal helpers under test (defined in binders.cpp).
JSValueRef JsonToJSValueStep(JSContextRef ctx, const nlohmann::json& j);
JSObjectRef JsonToJSObject(JSContextRef ctx, const nlohmann::json& j);

namespace {

using Catch::Approx;

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

} // namespace

TEST_CASE("JsonToJSValueStep converts primitive JSON types") {
    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    auto null_value = JsonToJSValueStep(ctx, nullptr);
    REQUIRE(JSValueIsNull(ctx, null_value));

    auto bool_value = JsonToJSValueStep(ctx, true);
    REQUIRE(JSValueIsBoolean(ctx, bool_value));
    REQUIRE(JSValueToBoolean(ctx, bool_value) == true);

    auto int_value = JsonToJSValueStep(ctx, -3);
    REQUIRE(JSValueIsNumber(ctx, int_value));
    REQUIRE(JSValueToNumber(ctx, int_value, nullptr) == Approx(-3.0));

    auto unsigned_value = JsonToJSValueStep(ctx, nlohmann::json(7u));
    REQUIRE(JSValueIsNumber(ctx, unsigned_value));
    REQUIRE(JSValueToNumber(ctx, unsigned_value, nullptr) == Approx(7.0));

    auto float_value = JsonToJSValueStep(ctx, 2.5);
    REQUIRE(JSValueIsNumber(ctx, float_value));
    REQUIRE(JSValueToNumber(ctx, float_value, nullptr) == Approx(2.5));

    auto string_value = JsonToJSValueStep(ctx, "hello");
    REQUIRE(JsValueToString(ctx, string_value) == "hello");

    JSGlobalContextRelease(ctx);
}

TEST_CASE("JsonToJSValueStep throws on unsupported JSON types") {
    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    nlohmann::json unsupported = nlohmann::json::binary({0x00, 0x01});
    REQUIRE_THROWS_AS(JsonToJSValueStep(ctx, unsupported), std::runtime_error);

    JSGlobalContextRelease(ctx);
}

TEST_CASE("JsonToJSObject rejects non-object JSON roots") {
    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    nlohmann::json not_object = 123;
    REQUIRE_THROWS_AS(JsonToJSObject(ctx, not_object), std::runtime_error);

    JSGlobalContextRelease(ctx);
}

TEST_CASE("BindFBXNodeProps converts FBXNodeProps to JS object") {
    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    FBXNodeProps props;
    props.name = "ExampleNode";
    props.properties.push_back(nlohmann::json::object({{"key", "value"}}));
    props.properties.push_back(nlohmann::json::array({1, 2, 3}));

    JSValueRef js_props = BindFBXNodeProps(ctx, props);
    REQUIRE(JSValueIsObject(ctx, js_props));

    JSObjectRef obj = JSValueToObject(ctx, js_props, nullptr);

    JSStringRef name_key = JSStringCreateWithUTF8CString("name");
    JSValueRef name_value = JSObjectGetProperty(ctx, obj, name_key, nullptr);
    JSStringRelease(name_key);
    REQUIRE(JsValueToString(ctx, name_value) == "ExampleNode");

    JSStringRef props_key = JSStringCreateWithUTF8CString("properties");
    JSValueRef props_value = JSObjectGetProperty(ctx, obj, props_key, nullptr);
    JSStringRelease(props_key);
    REQUIRE(JSValueIsObject(ctx, props_value));

    JSObjectRef props_array = JSValueToObject(ctx, props_value, nullptr);
    JSValueRef first_element = JSObjectGetPropertyAtIndex(ctx, props_array, 0, nullptr);
    JSValueRef second_element = JSObjectGetPropertyAtIndex(ctx, props_array, 1, nullptr);

    REQUIRE(JSValueIsObject(ctx, first_element));
    JSObjectRef first_obj = JSValueToObject(ctx, first_element, nullptr);
    JSStringRef key = JSStringCreateWithUTF8CString("key");
    JSValueRef stored_value = JSObjectGetProperty(ctx, first_obj, key, nullptr);
    JSStringRelease(key);
    REQUIRE(JsValueToString(ctx, stored_value) == "value");

    REQUIRE(JSValueIsObject(ctx, second_element));
    JSObjectRef second_array = JSValueToObject(ctx, second_element, nullptr);
    JSValueRef second_array_first = JSObjectGetPropertyAtIndex(ctx, second_array, 0, nullptr);
    REQUIRE(JSValueIsNumber(ctx, second_array_first));
    REQUIRE(JSValueToNumber(ctx, second_array_first, nullptr) == 1.0);

    JSGlobalContextRelease(ctx);
}

TEST_CASE("BindFBXNode wraps node id and props into JS object") {
    JSGlobalContextRef ctx = JSGlobalContextCreate(nullptr);
    REQUIRE(ctx != nullptr);

    class StubClient : public IFBXClient {
    public:
        const FBXNodeProps* getNodeProps(size_t id) const override {
            return id == 9 ? &props : nullptr;
        }
        std::vector<size_t> getNodeChildren(size_t id) const override {
            return id == 9 ? std::vector<size_t>{2, 4} : std::vector<size_t>{};
        }
        FBXNodeProps props;
    };

    StubClient client;
    client.props.name = "BoundNode";

    FBXNode node(9, client);
    JSValueRef js_node = BindFBXNode(ctx, node);
    REQUIRE(JSValueIsObject(ctx, js_node));

    JSObjectRef obj = JSValueToObject(ctx, js_node, nullptr);

    JSStringRef id_key = JSStringCreateWithUTF8CString("id");
    JSValueRef id_value = JSObjectGetProperty(ctx, obj, id_key, nullptr);
    JSStringRelease(id_key);
    REQUIRE(JSValueIsNumber(ctx, id_value));
    REQUIRE(JSValueToNumber(ctx, id_value, nullptr) == 9.0);

    JSStringRef props_key = JSStringCreateWithUTF8CString("props");
    JSValueRef props_value = JSObjectGetProperty(ctx, obj, props_key, nullptr);
    JSStringRelease(props_key);
    REQUIRE(JSValueIsObject(ctx, props_value));

    JSObjectRef props_obj = JSValueToObject(ctx, props_value, nullptr);
    JSStringRef name_key = JSStringCreateWithUTF8CString("name");
    JSValueRef name_value = JSObjectGetProperty(ctx, props_obj, name_key, nullptr);
    JSStringRelease(name_key);
    REQUIRE(JsValueToString(ctx, name_value) == "BoundNode");

    JSGlobalContextRelease(ctx);
}
