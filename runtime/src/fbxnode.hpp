#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

class FBXNodeProps {
    public:
        FBXNodeProps() = default;
        ~FBXNodeProps() = default;

        std::string name;
        std::vector<nlohmann::json> properties;
        std::vector<nlohmann::json> attributes;
};

enum class FBXFormat {
    Binary,
    ASCII
};

class IFBXClient {
    public:
        virtual ~IFBXClient() = default;
        virtual const FBXNodeProps* getNodeProps(size_t id) const = 0;
        virtual std::vector<size_t> getNodeChildren(size_t id) const = 0;
        virtual FBXFormat getFormat() const = 0;
};

class FBXNode {

    public:
        FBXNode(size_t id, const IFBXClient& client) 
            : id_(id)
            , client_(client) {}

        ~FBXNode() = default;

        size_t getID() const 
        { 
            return id_; 
        }
        const std::string& getName() const
        {
            return client_.getNodeProps(id_)->name;
        }
        const FBXNodeProps& getProps() const 
        { 
            return *client_.getNodeProps(id_); 
        }
        std::vector<size_t> getChildren() const 
        { 
            return client_.getNodeChildren(id_); 
        }

    private:
        size_t id_;
        const IFBXClient& client_;
};

