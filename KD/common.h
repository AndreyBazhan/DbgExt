#pragma once


#define APPNAME     _T("KD")


HRESULT
IsKernelMode(
    _In_ PDEBUG_CLIENT DebugClient,
    _In_ PCSTR CommandName
    );
