/*++

Copyright (c) Andrey Bazhan

--*/

#include "stdafx.h"


VOID
CALLBACK
DebugExtensionUninitialize(
    VOID
    )
{
    return;
}

HRESULT
CALLBACK
DebugExtensionInitialize(
    _Out_ PULONG Version,
    _Out_ PULONG Flags
    )
{
    *Version = DEBUG_EXTENSION_VERSION(1, 0);
    *Flags = 0;

    return S_OK;
}

BOOL
WINAPI
DllMain(
    _In_ HINSTANCE hInstance,
    _In_ DWORD Reason,
    _In_ LPVOID Reserved
    )
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(Reserved);

    switch (Reason) {

    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}
