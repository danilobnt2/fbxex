#include "winfio.hpp"

#ifdef _WIN32
#include <shobjidl.h>

std::wstring OpenFileDialogWin32(HWND owner) {
    std::wstring result;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool co_init = SUCCEEDED(hr);

    IFileOpenDialog* dialog = nullptr;
    hr = CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&dialog)
    );

    if (SUCCEEDED(hr)) {
        DWORD options;
        dialog->GetOptions(&options);
        dialog->SetOptions(options | FOS_FORCEFILESYSTEM);

        hr = dialog->Show(owner);
        if (SUCCEEDED(hr)) {
            IShellItem* item = nullptr;
            hr = dialog->GetResult(&item);
            if (SUCCEEDED(hr)) {
                PWSTR path = nullptr;
                item->GetDisplayName(SIGDN_FILESYSPATH, &path);
                if (path) {
                    result = path;
                    CoTaskMemFree(path);
                }
                item->Release();
            }
        }
        dialog->Release();
    }

    if (co_init) {
        CoUninitialize();
    }

    return result;
}

std::string WStringToUtf8(const std::wstring& w) {
    if (w.empty()) {
        return {};
    }

    int size = WideCharToMultiByte(
        CP_UTF8,
        0,
        w.data(),
        static_cast<int>(w.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    std::string result(size, 0);
    WideCharToMultiByte(
        CP_UTF8,
        0,
        w.data(),
        static_cast<int>(w.size()),
        result.data(),
        size,
        nullptr,
        nullptr
    );
    return result;
}
#endif
