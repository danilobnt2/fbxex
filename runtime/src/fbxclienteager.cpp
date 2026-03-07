#include "fbxclienteager.hpp"
#include "fbxdtserialize.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

std::shared_ptr<ufbx_scene> LoadSceneFromPath(const std::string& path) {
    ufbx_load_opts opts = {};
    ufbx_error error = {};
    ufbx_scene* scene = ufbx_load_file(path.c_str(), &opts, &error);
    if (!scene) {
        std::string message = UfbxStringToString(error.description);
        if (message.empty() && error.info_length > 0) {
            message.assign(error.info, error.info_length);
        }
        if (message.empty()) {
            message = "UFBX load failed";
        }
        throw std::runtime_error(message);
    }
    return std::shared_ptr<ufbx_scene>(scene, [](ufbx_scene* s) { ufbx_free_scene(s); });
}

bool ShouldIncludeProp(const ufbx_prop& prop) {
    return (prop.flags & UFBX_PROP_FLAG_SYNTHETIC) == 0;
}

} // namespace

FBXClientEager::FBXClientEager(const std::string& path)
    : FBXClientEager(LoadSceneFromPath(path)) {}

FBXClientEager::FBXClientEager(
    std::shared_ptr<ufbx_scene> scene)
    : scene_(std::move(scene)) {
    if (!scene_) {
        throw std::invalid_argument("Scene must not be null");
    }

    nodes_.clear();
    nodes_.reserve(scene_->nodes.count + 1);

    // id 0 is reserved for the root node.
    nodes_.push_back(NodeData{});
    const ufbx_node* root = scene_->root_node;
    populateNodeData(root, nodes_[0]);
    if (root) {
        for (size_t i = 0; i < root->children.count; ++i) {
            buildNodeMap(root->children.data[i], 0);
        }
    }
}

const FBXNodeProps* FBXClientEager::getNodeProps(size_t id) const {
    if (id >= nodes_.size()) {
        return nullptr;
    }
    return &nodes_[id].props;
}

FBXFormat FBXClientEager::getFormat() const {
    return scene_->metadata.ascii ? FBXFormat::ASCII : FBXFormat::Binary;
}

std::vector<size_t> FBXClientEager::getNodeChildren(size_t id) const {
    if (id >= nodes_.size()) {
        return {};
    }
    return nodes_[id].children;
}

void FBXClientEager::populateNodeData(const ufbx_node* fbx_node, NodeData& node_data) {
    if (!fbx_node) {
        return;
    }

    node_data.props.name = UfbxStringToString(fbx_node->name);

    node_data.props.properties.clear();
    for (size_t idx = 0; idx < fbx_node->props.props.count; ++idx) {
        const ufbx_prop& prop = fbx_node->props.props.data[idx];
        if (!ShouldIncludeProp(prop)) {
            continue;
        }
        node_data.props.properties.push_back(SerializeFbxProperty(prop));
    }

    node_data.props.attributes.clear();
    for (size_t idx = 0; idx < fbx_node->all_attribs.count; ++idx) {
        const ufbx_element* attrib = fbx_node->all_attribs.data[idx];
        if (!attrib) {
            continue;
        }
        nlohmann::json attr = SerializeUfbxAttribute(*attrib);
        if (attr.contains("properties") && attr["properties"].is_array()) {
            nlohmann::json filtered = nlohmann::json::array();
            for (size_t prop_idx = 0; prop_idx < attrib->props.props.count; ++prop_idx) {
                const ufbx_prop& prop = attrib->props.props.data[prop_idx];
                if (!ShouldIncludeProp(prop)) {
                    continue;
                }
                filtered.push_back(SerializeFbxProperty(prop));
            }
            attr["properties"] = std::move(filtered);
        }
        node_data.props.attributes.push_back(std::move(attr));
    }
}

void FBXClientEager::buildNodeMap(const ufbx_node* fbx_node, size_t parent_id) {
    if (!fbx_node) {
        return;
    }

    size_t current_id = nodes_.size();
    nodes_.push_back(NodeData{});
    populateNodeData(fbx_node, nodes_.back());

    if (parent_id < nodes_.size()) {
        nodes_[parent_id].children.push_back(current_id);
    }

    for (size_t i = 0; i < fbx_node->children.count; ++i) {
        buildNodeMap(fbx_node->children.data[i], current_id);
    }
}
