#ifdef _WIN32

#include <windows.h>
#include <delayimp.h>
#include <cstring>

static FARPROC WINAPI delay_load_hook(
    unsigned notification,
    DelayLoadInfo* info)
{
    if (notification != dliNotePreLoadLibrary)
        return nullptr;

    if (_stricmp(info->szDll, "node.exe") != 0)
        return nullptr;

    // The executable that loaded this DLL:
    HMODULE exe = GetModuleHandleW(nullptr);

    return reinterpret_cast<FARPROC>(exe);
}

// MinGW's delayimp implementation uses this symbol.
extern "C" PfnDliHook __pfnDliNotifyHook2 = delay_load_hook;

#endif
