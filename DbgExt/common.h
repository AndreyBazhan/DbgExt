#pragma once


#define APPNAME     _T("DbgExt")


HRESULT
IsKernelMode(
    _In_ PDEBUG_CLIENT DebugClient,
    _In_ PCSTR CommandName
    );
