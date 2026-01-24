#pragma once

#include "fbxnode.hpp"

#include <memory>
#include <string>
#include <ufbx.h>

class FBXClientEager : public IFBXClient {
    public:
        explicit FBXClientEager(const std::string& path);
        ~FBXClientEager() override = default;

        const FBXNodeProps* getNodeProps(size_t id) const override;
        std::vector<size_t> getNodeChildren(size_t id) const override;

    private:
        struct NodeData {
            FBXNodeProps props;
            std::vector<size_t> children;
        };
        explicit FBXClientEager(std::shared_ptr<ufbx_scene> scene);

        void buildNodeMap(const ufbx_node* fbx_node, size_t parent_id);
        void populateNodeData(const ufbx_node* fbx_node, NodeData& node_data);

        std::vector<NodeData> nodes_;
        std::shared_ptr<ufbx_scene> scene_;
};
