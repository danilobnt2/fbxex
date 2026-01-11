#include <catch2/catch_test_macros.hpp>

#include <Ultralight/String.h>

#include "embedded_filesystem.hpp"

TEST_CASE("EmbeddedFileSystem normalizes paths for lookups") {
    auto fs = CreateEmbeddedFileSystem();
    REQUIRE(fs);

    REQUIRE(fs->FileExists(ultralight::String("fbxex.ico")));
    REQUIRE(fs->FileExists(ultralight::String("./fbxex.ico")));
    REQUIRE(fs->FileExists(ultralight::String(".\\fbxex.ico")));
    REQUIRE(fs->FileExists(ultralight::String("file:///fbxex.ico")));

    REQUIRE_FALSE(fs->FileExists(ultralight::String("missing.txt")));
    REQUIRE_FALSE(fs->FileExists(ultralight::String("")));
}

TEST_CASE("EmbeddedFileSystem exposes MIME types and charset") {
    auto fs = CreateEmbeddedFileSystem();
    REQUIRE(fs);

    REQUIRE(fs->GetFileMimeType(ultralight::String("fbxex.ico")) == "image/x-icon");
    REQUIRE(fs->GetFileMimeType(ultralight::String("missing.bin")) == "application/octet-stream");
    REQUIRE(fs->GetFileMimeType(ultralight::String("")) == "application/octet-stream");
    REQUIRE(fs->GetFileCharset(ultralight::String("fbxex.ico")) == "utf-8");
}

TEST_CASE("EmbeddedFileSystem opens embedded assets") {
    auto fs = CreateEmbeddedFileSystem();
    REQUIRE(fs);

    auto buffer = fs->OpenFile(ultralight::String("fbxex.ico"));
    REQUIRE(buffer);

    auto missing = fs->OpenFile(ultralight::String("missing.txt"));
    REQUIRE_FALSE(missing);
}
