/*++

Copyright (c) Andrey Bazhan

--*/

#include "stdafx.h"
#include "common.h"


static PDEBUG_CONTROL4 DebugControl;
static PDEBUG_SYMBOLS3 DebugSymbols;
static PDEBUG_DATA_SPACES4 DebugDataSpaces;


static
HRESULT
QueryInterfaces(
    _In_ PDEBUG_CLIENT DebugClient
    )
{
    HRESULT Status = S_OK;

    if ((Status = DebugClient->QueryInterface(__uuidof(IDebugControl), (PVOID *)&DebugControl)) != S_OK) {

        return Status;
    }

    if ((Status = DebugClient->QueryInterface(__uuidof(IDebugSymbols), (PVOID *)&DebugSymbols)) != S_OK) {

        return Status;
    }

    if ((Status = DebugClient->QueryInterface(__uuidof(IDebugDataSpaces), (PVOID *)&DebugDataSpaces)) != S_OK) {

        return Status;
    }

    return Status;
}

static
VOID
ReleaseInterfaces(
    VOID
    )
{
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

HRESULT
CALLBACK
st(
    _In_ PDEBUG_CLIENT DebugClient,
    _In_opt_ PCSTR args
    )

/*++

Routine Description:

    Displays system service table.

Return Value:

    HRESULT

Environment:

    Kernel mode.

--*/

{
    HRESULT Status = S_OK;
    ULONG ProcessorType;
    ULONG PlatformId;
    ULONG Major;
    ULONG Minor;
    ULONG ServicePackNumber;
    ULONG64 KeServiceDescriptorTable;
    ULONG64 KiServiceLimit;
    ULONG64 ServiceTableBase;
    ULONG64 Address;
    ULONG64 ServiceAddress;
    ULONG64 PsNtosImageBase;
    ULONG64 NtosImageBase;
    ULONG64 NtosImageEnd;
    ULONG Limit;
    ULONG i;
    LONG Offset;
    ULONG BytesRead;
    CHAR ServiceName[MAX_PATH];
    IMAGE_NT_HEADERS64 ImageNtHeaders;

    UNREFERENCED_PARAMETER(args);

    __try {

        if ((Status = QueryInterfaces(DebugClient)) != S_OK) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not query interfaces.\n");
            __leave;
        }

        if ((Status = IsKernelMode(DebugClient, __FUNCTION__)) != S_OK) {

            __leave;
        }

        if ((Status = DebugControl->GetActualProcessorType(&ProcessorType)) != S_OK) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not get processor type.\n");
            __leave;
        }

        if ((Status = DebugControl->GetSystemVersion(&PlatformId, &Major, &Minor, NULL, NULL, NULL, &ServicePackNumber, NULL, NULL, NULL)) != S_OK) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not get system version.\n");
            __leave;
        }

        if ((Status = DebugSymbols->GetOffsetByName("nt!KeServiceDescriptorTable", &KeServiceDescriptorTable)) != S_OK) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not get address of nt!KeServiceDescriptorTable.\n");
            __leave;
        }

        if ((Status = DebugSymbols->GetOffsetByName("nt!KiServiceLimit", &KiServiceLimit)) != S_OK) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not get address of nt!KiServiceLimit.\n");
            __leave;
        }

        if ((Status = DebugDataSpaces->ReadPointersVirtual(1, KeServiceDescriptorTable, &ServiceTableBase)) != S_OK) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not read service table base.\n");
            __leave;
        }

        if ((Status = DebugDataSpaces->ReadVirtual(KiServiceLimit, &Limit, sizeof(ULONG), &BytesRead)) != S_OK) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not read service table limit.\n");
            __leave;
        }

        if ((Status = DebugSymbols->GetOffsetByName("nt!PsNtosImageBase", &PsNtosImageBase)) != S_OK) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not get address of nt!PsNtosImageBase.\n");
            __leave;
        }

        if (DebugDataSpaces->ReadPointersVirtual(1, PsNtosImageBase, &NtosImageBase) != S_OK) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not read nt!PsNtosImageBase.\n");
            __leave;
        }

        if (DebugDataSpaces->ReadImageNtHeaders(NtosImageBase, &ImageNtHeaders) != S_OK) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not read kernel image headers.\n");
            __leave;
        }

        NtosImageEnd = NtosImageBase + ImageNtHeaders.OptionalHeader.SizeOfImage;

        Address = ServiceTableBase;

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "\n");

        if (IMAGE_FILE_MACHINE_I386 == ProcessorType) {

            for (i = 0; i < Limit; i++, Address += sizeof(ULONG)) {

                ServiceName[0] = '\0';

                if ((Status = DebugDataSpaces->ReadPointersVirtual(1, Address, &ServiceAddress)) != S_OK) {

                    DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not read memory.\n");
                    __leave;
                }

                DebugSymbols->GetNameByOffset(ServiceAddress, (PSTR)ServiceName, _countof(ServiceName), &BytesRead, NULL);

                DebugControl->ControlledOutput(DEBUG_OUTCTL_DML,
                                               DEBUG_OUTPUT_NORMAL,
                                               (ServiceAddress >= NtosImageBase && ServiceAddress < NtosImageEnd) ? "%03lx:\t%p\t%s\n" : "%03lx:<col fg=\"changed\">\t%p\t%s</col>\n",
                                               i,
                                               ServiceAddress,
                                               ServiceName);
            }
        }
        else if (IMAGE_FILE_MACHINE_AMD64 == ProcessorType) {

            for (i = 0; i < Limit; i++, Address += sizeof(ULONG)) {

                ServiceName[0] = '\0';

                if ((Status = DebugDataSpaces->ReadVirtual(Address, &Offset, sizeof(Offset), &BytesRead)) != S_OK) {

                    DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Could not read memory.\n");
                    __leave;
                }

                if (Minor < 6000) {

                    Offset &= ~0xF;
                }
                else {

                    Offset >>= 4;
                }

                ServiceAddress = ServiceTableBase + Offset;

                DebugSymbols->GetNameByOffset(ServiceAddress, (PSTR)ServiceName, _countof(ServiceName), &BytesRead, NULL);

                DebugControl->ControlledOutput(DEBUG_OUTCTL_DML,
                                               DEBUG_OUTPUT_NORMAL,
                                               (ServiceAddress >= NtosImageBase && ServiceAddress < NtosImageEnd) ? "%03lx:\t%p\t%s\n" : "%03lx:<col fg=\"changed\">\t%p\t%s</col>\n",
                                               i,
                                               ServiceAddress,
                                               ServiceName);
            }
        }

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "\n");
    }
    __finally {

        ReleaseInterfaces();
    }

    return Status;
}
