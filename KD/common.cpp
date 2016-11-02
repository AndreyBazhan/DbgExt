/*++

Copyright (c) Andrey Bazhan

--*/

#include "stdafx.h"


HRESULT
IsKernelMode(
    _In_ PDEBUG_CLIENT DebugClient,
    _In_ PCSTR CommandName
    )
{
    HRESULT Status = S_OK;
    PDEBUG_CONTROL DebugControl = NULL;
    ULONG Class;
    ULONG Qualifier;

    __try {

        if ((Status = DebugClient->QueryInterface(__uuidof(IDebugControl), (PVOID *)&DebugControl)) != S_OK) {

            __leave;
        }

        if ((Status = DebugControl->GetDebuggeeType(&Class, &Qualifier)) != S_OK) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not get debuggee type.\n");
            __leave;
        }

        if (Class != DEBUG_CLASS_KERNEL) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "%s : Only works in kernel-mode debugging mode.\n", CommandName);

            Status = S_FALSE;

            __leave;
        }
    }
    __finally {

        if (DebugControl) {

            DebugControl->Release();
        }
    }

    return Status;
}
