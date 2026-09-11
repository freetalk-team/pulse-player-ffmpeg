#ifdef _WIN32

#include <windows.h>
#include <delayimp.h>
#include <string.h>

static FARPROC WINAPI delay_load_hook(
    unsigned notification,
    DelayLoadInfo* info)
{
    if (notification != dliNotePreLoadLibrary)
        return NULL;

    if (_stricmp(info->szDll, "node.exe") != 0)
        return NULL;

    return (FARPROC)GetModuleHandleW(NULL);
}

PfnDliHook __pfnDliNotifyHook2 = delay_load_hook;

#endif
