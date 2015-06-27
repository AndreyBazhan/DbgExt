/*++

Copyright (c) Andrey Bazhan

--*/

#include "stdafx.h"


PDEBUG_CONTROL DebugControl;
PDEBUG_SYMBOLS DebugSymbols;
PDEBUG_DATA_SPACES4 DebugDataSpaces;
PDEBUG_REGISTERS DebugRegisters;


HRESULT
QueryInterfaces(
    _In_ PDEBUG_CLIENT DebugClient
    )
{
    HRESULT Result;

    if ((Result = DebugClient->QueryInterface(__uuidof(IDebugControl), (PVOID *)&DebugControl)) != S_OK) {

        return Result;
    }

    if ((Result = DebugClient->QueryInterface(__uuidof(IDebugSymbols), (PVOID *)&DebugSymbols)) != S_OK) {

        return Result;
    }

    if ((Result = DebugClient->QueryInterface(__uuidof(IDebugDataSpaces), (PVOID *)&DebugDataSpaces)) != S_OK) {

        return Result;
    }

    if ((Result = DebugClient->QueryInterface(__uuidof(IDebugRegisters), (PVOID *)&DebugRegisters)) != S_OK) {

        return Result;
    }

    return S_OK;
}

VOID
ReleaseInterfaces(
    VOID
    )
{
    if (DebugRegisters) {

        DebugRegisters->Release();
        DebugRegisters = NULL;
    }

    if (DebugDataSpaces) {

        DebugDataSpaces->Release();
        DebugDataSpaces = NULL;
    }

    if (DebugSymbols) {

        DebugSymbols->Release();
        DebugSymbols = NULL;
    }

    if (DebugControl) {

        DebugControl->Release();
        DebugControl = NULL;
    }
}

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
