#ifdef _WIN32
#include <windows.h>
#include <shobjidl.h>
#include <string>

std::wstring OpenFileDialogWin32(HWND owner = nullptr) {
    std::wstring result;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool coInit = SUCCEEDED(hr);

    IFileOpenDialog* pDialog = nullptr;
    hr = CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pDialog)
    );

    if (SUCCEEDED(hr)) {
        DWORD options;
        pDialog->GetOptions(&options);
        pDialog->SetOptions(options | FOS_FORCEFILESYSTEM);

        hr = pDialog->Show(owner);
        if (SUCCEEDED(hr)) {
            IShellItem* pItem = nullptr;
            hr = pDialog->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR path = nullptr;
                pItem->GetDisplayName(SIGDN_FILESYSPATH, &path);
                if (path) {
                    result = path;
                    CoTaskMemFree(path);
                }
                pItem->Release();
            }
        }
        pDialog->Release();
    }

    if (coInit)
        CoUninitialize();

    return result;
}

std::string WStringToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int size = WideCharToMultiByte(
        CP_UTF8, 0,
        w.data(), (int)w.size(),
        nullptr, 0,
        nullptr, nullptr
    );

    std::string result(size, 0);
    WideCharToMultiByte(
        CP_UTF8, 0,
        w.data(), (int)w.size(),
        result.data(), size,
        nullptr, nullptr
    );
    return result;
}
#endif
