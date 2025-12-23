#pragma once

#include "fbxnode.hpp"
#include <fbxsdk.h>
#include <string>

class FBXClientEager : public IFBXClient {
    public:
        explicit FBXClientEager(const std::string& path);
        ~FBXClientEager() override;

        const FBXNodeProps* getNodeProps(size_t id) const override;
        std::vector<size_t> getNodeChildren(size_t id) const override;

    private:
        struct NodeData {
            FBXNodeProps props;
            std::vector<size_t> children;
        };

        void buildNodeMap(FbxNode* fbx_node, size_t parent_id);
        void populateNodeData(FbxNode* fbx_node, NodeData& node_data);

        std::vector<NodeData> nodes_;
        FbxManager* manager_ = nullptr;
        FbxScene* scene_ = nullptr;
        FbxImporter* importer_ = nullptr;
};
