#pragma once
#ifdef _WIN32
#include <windows.h>
#include <string>

// Opens a native file dialog and returns the selected path (empty on cancel).
std::wstring OpenFileDialogWin32(HWND owner = nullptr);

// Converts a wide string to UTF-8.
std::string WStringToUtf8(const std::wstring& w);
#endif
