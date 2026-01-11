#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include "fbxnode.hpp"

#include <unordered_map>
#include <vector>

class StubFBXClient : public IFBXClient {
public:
    const FBXNodeProps* getNodeProps(size_t id) const override {
        auto it = props_.find(id);
        if (it == props_.end()) {
            return nullptr;
        }
        return &it->second;
    }

    std::vector<size_t> getNodeChildren(size_t id) const override {
        auto it = children_.find(id);
        if (it == children_.end()) {
            return {};
        }
        return it->second;
    }

    std::unordered_map<size_t, FBXNodeProps> props_;
    std::unordered_map<size_t, std::vector<size_t>> children_;
};

TEST_CASE("FBXNode forwards node data from the client") {
    StubFBXClient client;

    FBXNodeProps props;
    props.name = "RootNode";
    props.properties.push_back(nlohmann::json::object({{"key", "value"}}));
    props.properties.push_back(nlohmann::json::array({1, 2, 3}));

    client.props_.emplace(7U, props);
    client.children_.emplace(7U, std::vector<size_t>{3U, 4U, 5U});

    FBXNode node(7U, client);

    REQUIRE(node.getID() == 7U);
    REQUIRE(node.getName() == "RootNode");

    const auto& retrieved = node.getProps();
    REQUIRE(retrieved.properties.size() == 2);
    REQUIRE(retrieved.properties[0]["key"] == "value");
    REQUIRE(retrieved.properties[1] == nlohmann::json::array({1, 2, 3}));

    REQUIRE(node.getChildren() == std::vector<size_t>{3U, 4U, 5U});
}
