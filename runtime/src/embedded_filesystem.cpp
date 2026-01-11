#include "embedded_filesystem.hpp"

#include <algorithm>
#include <string>
#include <string_view>

#include "embedded_files_data.inc"

namespace {
std::string NormalizePath(std::string_view path) {
    std::string normalized(path.begin(), path.end());
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    const std::string_view file_scheme = "file://";
    if (normalized.compare(0, file_scheme.size(), file_scheme) == 0) {
        normalized.erase(0, file_scheme.size());
    }
    while (!normalized.empty() && normalized.front() == '/') {
        normalized.erase(normalized.begin());
    }
    if (normalized.rfind("./", 0) == 0) {
        normalized.erase(0, 2);
    }
    return normalized;
}

const EmbeddedFileEntry* FindEmbeddedFile(std::string_view path) {
    for (size_t i = 0; i < kEmbeddedFileCount; ++i) {
        if (path == kEmbeddedFiles[i].path) {
            return &kEmbeddedFiles[i];
        }
    }
    return nullptr;
}

ultralight::RefPtr<ultralight::Buffer> CreateBufferFromEmbedded(const EmbeddedFileEntry& entry) {
    auto* data = const_cast<unsigned char*>(entry.data);
    return ultralight::Buffer::Create(data, entry.size, nullptr, nullptr);
}

std::string_view GetExtension(std::string_view path) {
    const auto dot = path.find_last_of('.');
    if (dot == std::string_view::npos || dot + 1 >= path.size()) {
        return {};
    }
    return path.substr(dot + 1);
}

ultralight::String MimeTypeFromPath(std::string_view path) {
    const auto ext = GetExtension(path);
    if (ext == "html" || ext == "htm") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "json") return "application/json";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "ico") return "image/x-icon";
    if (ext == "webp") return "image/webp";
    if (ext == "woff") return "font/woff";
    if (ext == "woff2") return "font/woff2";
    if (ext == "ttf") return "font/ttf";
    if (ext == "otf") return "font/otf";
    if (ext == "wasm") return "application/wasm";
    if (ext == "txt") return "text/plain";
    return "application/octet-stream";
}
} // namespace

class EmbeddedFileSystem final : public ultralight::FileSystem {
public:
    bool FileExists(const ultralight::String& file_path) override {
        std::string utf8_path = file_path.utf8().data();
        std::string normalized = NormalizePath(utf8_path);
        if (normalized.empty()) {
            return false;
        }
        return FindEmbeddedFile(normalized) != nullptr;
    }

    ultralight::String GetFileMimeType(const ultralight::String& file_path) override {
        std::string utf8_path = file_path.utf8().data();
        std::string normalized = NormalizePath(utf8_path);
        if (normalized.empty()) {
            return "application/octet-stream";
        }
        return MimeTypeFromPath(normalized);
    }

    ultralight::String GetFileCharset(const ultralight::String& /*file_path*/) override {
        return "utf-8";
    }

    ultralight::RefPtr<ultralight::Buffer> OpenFile(const ultralight::String& path) override {
        std::string utf8_path = path.utf8().data();
        std::string normalized = NormalizePath(utf8_path);
        if (normalized.empty()) {
            return nullptr;
        }
        if (const auto* entry = FindEmbeddedFile(normalized)) {
            return CreateBufferFromEmbedded(*entry);
        }
        return nullptr;
    }
};

std::unique_ptr<ultralight::FileSystem> CreateEmbeddedFileSystem() {
    return std::make_unique<EmbeddedFileSystem>();
}
