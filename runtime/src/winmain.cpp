// Small WinMain shim so we can build a WIN32 (GUI) exe
#ifdef _WIN32
#include <windows.h>

// Forward declaration of the normal main()
int main();

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/) {
  return main();
}
#endif
