#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>
#include <ufbx.h>

#include "fbxclienteager.hpp"
#include "fbxdtserialize.hpp"

namespace {

std::shared_ptr<ufbx_scene> LoadUfbxScene(const std::filesystem::path& path) {
    ufbx_load_opts opts = {};
    ufbx_error error = {};
    ufbx_scene* scene = ufbx_load_file(path.string().c_str(), &opts, &error);
    REQUIRE(scene != nullptr);
    return std::shared_ptr<ufbx_scene>(scene, [](ufbx_scene* s) { ufbx_free_scene(s); });
}

void CollectNodes(const ufbx_node* node, std::vector<const ufbx_node*>& out) {
    if (!node) {
        return;
    }
    out.push_back(node);
    for (size_t i = 0; i < node->children.count; ++i) {
        CollectNodes(node->children.data[i], out);
    }
}

bool ShouldIncludeProp(const ufbx_prop& prop) {
    return (prop.flags & UFBX_PROP_FLAG_SYNTHETIC) == 0;
}

} // namespace

TEST_CASE("FBXClientEager throws on invalid file path") {
    const std::string invalid_path = "nonexistent_file_path_that_should_fail.fbx";
    REQUIRE_THROWS_AS(FBXClientEager(invalid_path), std::runtime_error);
}

TEST_CASE("FBXClientEager loads an FBX file from path") {
    const std::filesystem::path asset_path =
        std::filesystem::path(__FILE__).parent_path() / "assets" / "test.fbx";
    REQUIRE(std::filesystem::exists(asset_path));

    FBXClientEager client(asset_path.string());
    const FBXNodeProps* root_props = client.getNodeProps(0);
    REQUIRE(root_props);
    auto scene = LoadUfbxScene(asset_path);
    REQUIRE(scene);
    REQUIRE(root_props->name == UfbxStringToString(scene->root_node->name));
    REQUIRE(client.getNodeProps(9999) == nullptr);
    REQUIRE(client.getNodeChildren(9999).empty());
}

TEST_CASE("FBXClientEager mirrors ufbx node hierarchy and metadata") {
    const std::filesystem::path asset_path =
        std::filesystem::path(__FILE__).parent_path() / "assets" / "test.fbx";
    REQUIRE(std::filesystem::exists(asset_path));

    auto scene = LoadUfbxScene(asset_path);
    REQUIRE(scene);

    std::vector<const ufbx_node*> expected_nodes;
    CollectNodes(scene->root_node, expected_nodes);
    REQUIRE_FALSE(expected_nodes.empty());

    std::unordered_map<const ufbx_node*, size_t> node_ids;
    node_ids.reserve(expected_nodes.size());
    for (size_t i = 0; i < expected_nodes.size(); ++i) {
        node_ids.emplace(expected_nodes[i], i);
    }

    FBXClientEager client(asset_path.string());

    for (size_t i = 0; i < expected_nodes.size(); ++i) {
        const ufbx_node* node = expected_nodes[i];
        const FBXNodeProps* props = client.getNodeProps(i);
        REQUIRE(props);
        REQUIRE(props->name == UfbxStringToString(node->name));

        std::vector<size_t> expected_children;
        expected_children.reserve(node->children.count);
        for (size_t child_idx = 0; child_idx < node->children.count; ++child_idx) {
            const ufbx_node* child = node->children.data[child_idx];
            auto found = node_ids.find(child);
            REQUIRE(found != node_ids.end());
            expected_children.push_back(found->second);
        }
        const auto actual_children = client.getNodeChildren(i);
        REQUIRE(actual_children.size() == expected_children.size());
        REQUIRE(actual_children == expected_children);

        std::vector<nlohmann::json> expected_props;
        expected_props.reserve(node->props.props.count);
        for (size_t prop_idx = 0; prop_idx < node->props.props.count; ++prop_idx) {
            const ufbx_prop& prop = node->props.props.data[prop_idx];
            if (!ShouldIncludeProp(prop)) {
                continue;
            }
            expected_props.push_back(SerializeFbxProperty(prop));
        }
        REQUIRE(props->properties.size() == expected_props.size());
        for (size_t prop_idx = 0; prop_idx < expected_props.size(); ++prop_idx) {
            REQUIRE(props->properties[prop_idx] == expected_props[prop_idx]);
        }

        std::vector<nlohmann::json> expected_attrs;
        expected_attrs.reserve(node->all_attribs.count);
        for (size_t attr_idx = 0; attr_idx < node->all_attribs.count; ++attr_idx) {
            const ufbx_element* attr = node->all_attribs.data[attr_idx];
            REQUIRE(attr != nullptr);
            nlohmann::json attr_json = SerializeUfbxAttribute(*attr);
            if (attr_json.contains("properties") && attr_json["properties"].is_array()) {
                nlohmann::json filtered = nlohmann::json::array();
                for (size_t prop_idx = 0; prop_idx < attr->props.props.count; ++prop_idx) {
                    const ufbx_prop& prop = attr->props.props.data[prop_idx];
                    if (!ShouldIncludeProp(prop)) {
                        continue;
                    }
                    filtered.push_back(SerializeFbxProperty(prop));
                }
                attr_json["properties"] = std::move(filtered);
            }
            expected_attrs.push_back(std::move(attr_json));
        }
        REQUIRE(props->attributes.size() == expected_attrs.size());
        for (size_t attr_idx = 0; attr_idx < expected_attrs.size(); ++attr_idx) {
            REQUIRE(props->attributes[attr_idx] == expected_attrs[attr_idx]);
        }
    }

    REQUIRE(client.getNodeProps(expected_nodes.size()) == nullptr);
    REQUIRE(client.getNodeChildren(expected_nodes.size()).empty());
}
