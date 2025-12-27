#include "fbxclienteager.hpp"
#include "fbxdtserialize.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

using ManagerPtr = std::shared_ptr<FbxManager>;
using ScenePtr = std::shared_ptr<FbxScene>;

ManagerPtr CreateManager() {
    auto manager = ManagerPtr(
        FbxManager::Create(), 
        [](FbxManager* m) { if (m) m->Destroy(); });
    if (!manager) {
        throw std::runtime_error("Failed to create FBX manager");
    }
    FbxIOSettings* ios = FbxIOSettings::Create(manager.get(), IOSROOT);
    manager->SetIOSettings(ios);
    return manager;
}

std::pair<ManagerPtr, ScenePtr> LoadSceneFromPath(const std::string& path) {
    auto manager = CreateManager();

    auto importer_deleter = [](FbxImporter* imp) { if (imp) imp->Destroy(); };
    std::unique_ptr<FbxImporter, decltype(importer_deleter)> importer(
        FbxImporter::Create(manager.get(), ""), importer_deleter);
    if (!importer) {
        throw std::runtime_error("Failed to create FBX importer");
    }

    if (!importer->Initialize(path.c_str(), -1, manager->GetIOSettings())) {
        throw std::runtime_error(
            std::string("FBX importer initialization failed: ") +
            importer->GetStatus().GetErrorString());
    }

    ScenePtr scene(
        FbxScene::Create(manager.get(), "scene"), 
        [](FbxScene* s) { if (s) s->Destroy(); });
    if (!scene) {
        throw std::runtime_error("Failed to create FBX scene");
    }

    if (!importer->Import(scene.get())) {
        throw std::runtime_error(
            std::string("FBX import failed: ") +
            importer->GetStatus().GetErrorString());
    }

    return {std::move(manager), std::move(scene)};
}

} // namespace

FBXClientEager::FBXClientEager(const std::string& path)
    : FBXClientEager(LoadSceneFromPath(path)) {}

FBXClientEager::FBXClientEager(
    std::pair<std::shared_ptr<FbxManager>, std::shared_ptr<FbxScene>> resources)
    : FBXClientEager(
          std::move(resources.first),
          std::move(resources.second)) {}

FBXClientEager::FBXClientEager(
    std::shared_ptr<FbxManager> manager,
    std::shared_ptr<FbxScene> scene)
    : manager_(std::move(manager)),
      scene_(std::move(scene)) {
    if (!manager_ || !scene_) {
        throw std::invalid_argument("Manager and scene must not be null");
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
