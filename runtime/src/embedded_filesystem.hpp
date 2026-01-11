#pragma once

#include <memory>
#include <Ultralight/platform/FileSystem.h>

std::unique_ptr<ultralight::FileSystem> CreateEmbeddedFileSystem();
