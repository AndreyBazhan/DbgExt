#pragma once

#include "stdafx.h"


extern PDEBUG_CONTROL DebugControl;
extern PDEBUG_SYMBOLS DebugSymbols;
extern PDEBUG_DATA_SPACES DebugDataSpaces;
extern PDEBUG_REGISTERS DebugRegisters;


HRESULT
QueryInterfaces(
    _In_ PDEBUG_CLIENT DebugClient
    );

VOID
ReleaseInterfaces(
    VOID
    );
