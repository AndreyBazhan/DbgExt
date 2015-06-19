/*++

Copyright (c) Andrey Bazhan

--*/

#include "stdafx.h"

#define APPNAME     _T("KD")


HRESULT
CALLBACK
help(
    _In_ PDEBUG_CLIENT DebugClient,
    _In_opt_ PCSTR args
    )
{
    PDEBUG_CONTROL4 DebugControl;

    UNREFERENCED_PARAMETER(args);

    if (DebugClient->QueryInterface(__uuidof(IDebugControl), (PVOID *)&DebugControl) == S_OK) {

        HMODULE hModule = GetModuleHandle(APPNAME);

        if (hModule) {

            TCHAR FileName[MAX_PATH];
            TCHAR Text[MAX_PATH] = APPNAME _T(" ");
            DWORD Handle;
            PVOID Block;
            DWORD BlockSize;
            PVOID String;
            UINT Length;

            DebugControl->OutputWide(DEBUG_OUTPUT_NORMAL, _T("\n"));

            if (GetModuleFileName(hModule, FileName, _countof(FileName))) {

                BlockSize = GetFileVersionInfoSize(FileName, &Handle);

                if (BlockSize) {

                    Block = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BlockSize);

                    if (Block) {

                        GetFileVersionInfo(FileName, 0, BlockSize, Block);

                        if (VerQueryValue(Block, _T("\\StringFileInfo\\040904b0\\FileVersion"), &String, &Length)) {

                            _tcsncpy_s(&Text[_tcslen(Text)], _countof(Text) - _tcslen(Text), (PCTSTR)String, Length);

                            DebugControl->OutputWide(DEBUG_OUTPUT_NORMAL, Text);
                            DebugControl->OutputWide(DEBUG_OUTPUT_NORMAL, _T("\n"));
                        }

                        if (VerQueryValue(Block, _T("\\StringFileInfo\\040904b0\\LegalCopyright"), &String, &Length)) {

                            _tcsncpy_s(Text, _countof(Text), (PCTSTR)String, Length);

                            DebugControl->OutputWide(DEBUG_OUTPUT_NORMAL, Text);
                            DebugControl->OutputWide(DEBUG_OUTPUT_NORMAL, _T("\n"));
                        }

                        HeapFree(GetProcessHeap(), 0, Block);
                    }
                }
            }

            DebugControl->ControlledOutputWide(DEBUG_OUTCTL_ALL_CLIENTS | DEBUG_OUTCTL_DML,
                                               DEBUG_OUTPUT_NORMAL,
                                               _T("<link cmd=\".shell -x start http://www.andreybazhan.com\">http://www.andreybazhan.com</link>\n\n")
                                               _T("help - Displays this list\n")
                                               _T("st   - Displays system service table\n")
                                               _T("idt  - Displays interrupt descriptor table\n\n"));
        }

        DebugControl->Release();
    }

    return S_OK;
}
