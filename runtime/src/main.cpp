#include <cstdlib>
#include <string>

#include <AppCore/AppCore.h>

#include "fbxexappmain.hpp"

struct AppOptions {
  bool dev_mode = false;
  int dev_port = 5173;
};

AppOptions ParseAppOptions(int argc, char* argv[]) {
  AppOptions opts;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--dev") {
      opts.dev_mode = true;
    } 
    else if (arg == "--dev-port" && i + 1 < argc) {
      try {
        int parsed_port = std::stoi(argv[i + 1]);
        if (parsed_port > 0 && parsed_port <= 65535) {
          opts.dev_port = parsed_port;
        }
        else {
          throw std::runtime_error("Invalid port number specified");
        }
      } catch (...) {
        throw std::runtime_error("Invalid port number specified");
      }
      ++i;
    }
  }
  return opts;
}


int main(int argc, char* argv[]) {
  try {
    const auto options = ParseAppOptions(argc, argv);
    std::string start_url = "file:///index.html";
    if (options.dev_mode) {
      start_url = "http://localhost:" + std::to_string(options.dev_port);
    }
    FbxexAppMain app(start_url);
    app.Run();
    return 0;
  } catch (const std::exception& ex) {
    std::fprintf(stderr, "Fatal Error: %s\n", ex.what());
    ultralight::ShowMessageBox("Fatal Error", ex.what());
    return EXIT_FAILURE;
  }
}
