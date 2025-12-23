#include "fbxclienteager.hpp"
#include "fbxdtserialize.hpp"

#include <stdexcept>
#include <string>
#include <vector>

FBXClientEager::FBXClientEager(const std::string& path) {
    manager_ = FbxManager::Create();
    if (!manager_) {
        throw std::runtime_error("Failed to create FBX manager");
    }

    FbxIOSettings* ios = FbxIOSettings::Create(manager_, IOSROOT);
    manager_->SetIOSettings(ios);

    importer_ = FbxImporter::Create(manager_, "");
    if (!importer_) {
        throw std::runtime_error("Failed to create FBX importer");
    }

    if (!importer_->Initialize(path.c_str(), -1, manager_->GetIOSettings())) {
        throw std::runtime_error(
            std::string("FBX importer initialization failed: ") +
            importer_->GetStatus().GetErrorString());
    }

    scene_ = FbxScene::Create(manager_, "scene");
    if (!scene_) {
        throw std::runtime_error("Failed to create FBX scene");
    }

    if (!importer_->Import(scene_)) {
        throw std::runtime_error(
            std::string("FBX import failed: ") +
            importer_->GetStatus().GetErrorString());
    }

    nodes_.clear();
    nodes_.reserve(static_cast<size_t>(scene_->GetNodeCount()));

    // id 0 is reserved for the root node.
    nodes_.push_back(NodeData{});
    FbxNode* root = scene_->GetRootNode();
    populateNodeData(root, nodes_[0]);
    if (root) {
        for (int i = 0; i < root->GetChildCount(); ++i) {
            buildNodeMap(root->GetChild(i), 0);
        }
    }
}

FBXClientEager::~FBXClientEager() {
    if (importer_) {
        importer_->Destroy();
        importer_ = nullptr;
    }
    if (scene_) {
        scene_->Destroy();
        scene_ = nullptr;
    }
    if (manager_) {
        manager_->Destroy();
        manager_ = nullptr;
    }
}

const FBXNodeProps* FBXClientEager::getNodeProps(size_t id) const {
    if (id >= nodes_.size()) {
        return nullptr;
    }
    return &nodes_[id].props;
}

std::vector<size_t> FBXClientEager::getNodeChildren(size_t id) const {
    if (id >= nodes_.size()) {
        return {};
    }
    return nodes_[id].children;
}

void FBXClientEager::populateNodeData(FbxNode* fbx_node, NodeData& node_data) {
    if (!fbx_node) {
        return;
    }

    node_data.props.name = fbx_node->GetName();

    FbxProperty property = fbx_node->GetFirstProperty();
    while (property.IsValid()) {
        try {
            node_data.props.properties.push_back(SerializeFbxProperty(property));
        } catch (const std::exception&) {
            // Unsupported property types are skipped.
        }
        property = fbx_node->GetNextProperty(property);
    }
}

void FBXClientEager::buildNodeMap(FbxNode* fbx_node, size_t parent_id) {
    if (!fbx_node) {
        return;
    }

    size_t current_id = nodes_.size();
    nodes_.push_back(NodeData{});
    populateNodeData(fbx_node, nodes_.back());

    if (parent_id < nodes_.size()) {
        nodes_[parent_id].children.push_back(current_id);
    }

    for (int i = 0; i < fbx_node->GetChildCount(); ++i) {
        buildNodeMap(fbx_node->GetChild(i), current_id);
    }
}
