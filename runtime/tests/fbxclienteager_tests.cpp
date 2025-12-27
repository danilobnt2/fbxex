#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fbxsdk.h>

#include "fbxclienteager.hpp"

TEST_CASE("FBXClientEager throws on invalid file path") {
    const std::string invalid_path = "nonexistent_file_path_that_should_fail.fbx";
    REQUIRE_THROWS_AS(FBXClientEager(invalid_path), std::runtime_error);
}

TEST_CASE("FBXClientEager can build from an in-memory scene") {
    auto manager = std::shared_ptr<FbxManager>(
        FbxManager::Create(),
        [](FbxManager* m) {
            if (m) {
                m->Destroy();
            }
        });
    REQUIRE(manager);
    manager->SetIOSettings(FbxIOSettings::Create(manager.get(), IOSROOT));

    auto scene = std::shared_ptr<FbxScene>(
        FbxScene::Create(manager.get(), "testScene"),
        [](FbxScene* s) {
            if (s) {
                s->Destroy();
            }
        });
    REQUIRE(scene);

    FbxNode* root = scene->GetRootNode();
    FbxNode* child_a = FbxNode::Create(scene.get(), "ChildA");
    FbxNode* child_b = FbxNode::Create(scene.get(), "ChildB");
    root->AddChild(child_a);
    child_a->AddChild(child_b);

    FbxProperty int_prop = FbxProperty::Create(child_a, FbxIntDT, "MyInt");
    int_prop.Set<FbxInt>(7);

    FBXClientEager client(manager, scene);

    const FBXNodeProps* root_props = client.getNodeProps(0);
    REQUIRE(root_props);
    REQUIRE(root_props->name == root->GetName());

    const auto root_children = client.getNodeChildren(0);
    REQUIRE(root_children.size() == 1);
    const size_t child_a_id = root_children.front();

    const FBXNodeProps* child_a_props = client.getNodeProps(child_a_id);
    REQUIRE(child_a_props);
    REQUIRE(child_a_props->name == child_a->GetName());

    const auto prop_match = std::find_if(
        child_a_props->properties.begin(),
        child_a_props->properties.end(),
        [](const nlohmann::json& prop) {
            return prop.value("name", "") == "MyInt"
                && prop.value("type", "") == "eFbxInt"
                && prop.value("value", 0) == 7;
        });
    REQUIRE(prop_match != child_a_props->properties.end());

    const auto child_a_children = client.getNodeChildren(child_a_id);
    REQUIRE(child_a_children.size() == 1);
    const size_t child_b_id = child_a_children.front();

    const FBXNodeProps* child_b_props = client.getNodeProps(child_b_id);
    REQUIRE(child_b_props);
    REQUIRE(child_b_props->name == child_b->GetName());

    REQUIRE(client.getNodeProps(9999) == nullptr);
    REQUIRE(client.getNodeChildren(9999).empty());
}

TEST_CASE("FBXClientEager loads an FBX file from path") {
    const std::filesystem::path asset_path =
        std::filesystem::path(__FILE__).parent_path() / "assets" / "test.fbx";
    REQUIRE(std::filesystem::exists(asset_path));

    FBXClientEager client(asset_path.string());
    const FBXNodeProps* root_props = client.getNodeProps(0);
    REQUIRE(root_props);
    REQUIRE_FALSE(root_props->name.empty());
}
