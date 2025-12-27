#pragma once

#include "fbxnode.hpp"
#include <fbxsdk.h>
#include <memory>
#include <string>
#include <utility>

class FBXClientEager : public IFBXClient {
    public:
        explicit FBXClientEager(const std::string& path);
        FBXClientEager(
            std::shared_ptr<FbxManager> manager,
            std::shared_ptr<FbxScene> scene);
        ~FBXClientEager() override = default;

        const FBXNodeProps* getNodeProps(size_t id) const override;
        std::vector<size_t> getNodeChildren(size_t id) const override;

    private:
        struct NodeData {
            FBXNodeProps props;
            std::vector<size_t> children;
        };
        explicit FBXClientEager(
            std::pair<std::shared_ptr<FbxManager>, std::shared_ptr<FbxScene>> resources);

        void buildNodeMap(FbxNode* fbx_node, size_t parent_id);
        void populateNodeData(FbxNode* fbx_node, NodeData& node_data);

        std::vector<NodeData> nodes_;
        std::shared_ptr<FbxManager> manager_;
        std::shared_ptr<FbxScene> scene_;
};
