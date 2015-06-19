/*++

Copyright (c) Andrey Bazhan

--*/

#include "stdafx.h"
#include "dllmain.h"


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
    HRESULT Result;
    ULONG ProcessorType;
    ULONG PlatformId;
    ULONG Major;
    ULONG Minor;
    ULONG ServicePackNumber;
    ULONG64 KeServiceDescriptorTable;
    ULONG64 KiServiceLimit;
    ULONG64 ServiceTableBase;
    ULONG Limit;
    ULONG64 Address;
    ULONG64 ServiceAddress;
    ULONG i;
    LONG Offset;
    ULONG BytesRead;
    CHAR ServiceName[MAX_PATH];

    UNREFERENCED_PARAMETER(args);

    if ((Result = QueryInterfaces(DebugClient)) != S_OK) {

        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugControl->GetActualProcessorType(&ProcessorType)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't get the processor type.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugControl->GetSystemVersion(&PlatformId, &Major, &Minor, NULL, NULL, NULL, &ServicePackNumber, NULL, NULL, NULL)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't get system version.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugSymbols->GetOffsetByName("nt!KeServiceDescriptorTable", &KeServiceDescriptorTable)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read nt!KeServiceDescriptorTable.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugSymbols->GetOffsetByName("nt!KiServiceLimit", &KiServiceLimit)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read nt!KiServiceLimit.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugDataSpaces->ReadPointersVirtual(1, KeServiceDescriptorTable, &ServiceTableBase)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read service table base.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugDataSpaces->ReadVirtual(KiServiceLimit, &Limit, sizeof(ULONG), &BytesRead)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read service table limit.\n");
        ReleaseInterfaces();
        return Result;
    }

    Address = ServiceTableBase;

    DebugControl->Output(DEBUG_OUTPUT_NORMAL, "\n");

    if (IMAGE_FILE_MACHINE_I386 == ProcessorType) {

        for (i = 0; i < Limit; i++, Address += sizeof(ULONG)) {

            ServiceName[0] = '\0';

            if ((Result = DebugDataSpaces->ReadPointersVirtual(1, Address, &ServiceAddress)) != S_OK) {

                DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read memory.\n");
                ReleaseInterfaces();
                return Result;
            }

            DebugSymbols->GetNameByOffset(ServiceAddress, (PSTR)ServiceName, _countof(ServiceName), &BytesRead, NULL);

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "%03lx:\t%p\t%s\n", i, ServiceAddress, ServiceName);
        }
    }
    else if (IMAGE_FILE_MACHINE_AMD64 == ProcessorType) {

        for (i = 0; i < Limit; i++, Address += sizeof(ULONG)) {

            ServiceName[0] = '\0';

            if ((Result = DebugDataSpaces->ReadVirtual(Address, &Offset, sizeof(Offset), &BytesRead)) != S_OK) {

                DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read memory.\n");
                ReleaseInterfaces();
                return Result;
            }

            if (Minor < 6000) {

                Offset &= ~0xF;
            }
            else {

                Offset >>= 4;
            }

            ServiceAddress = ServiceTableBase + Offset;

            DebugSymbols->GetNameByOffset(ServiceAddress, (PSTR)ServiceName, _countof(ServiceName), &BytesRead, NULL);

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "%03lx:\t%p\t%s\n", i, ServiceAddress, ServiceName);
        }
    }

    DebugControl->Output(DEBUG_OUTPUT_NORMAL, "\n");

    ReleaseInterfaces();

    return S_OK;
}
