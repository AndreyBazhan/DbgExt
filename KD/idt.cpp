/*++

Copyright (c) Andrey Bazhan

--*/

#include "stdafx.h"
#include "dllmain.h"


#define INTERRUPT_OBJECT_TYPE 22

#define GATE_TYPE_TASK        0x0500
#define GATE_TYPE_INT         0x0E00
#define GATE_TYPE_TRAP        0x0F00

typedef struct _KIDTENTRY {
   USHORT Offset;
   USHORT Selector;
   USHORT Access;
   USHORT ExtendedOffset;
} KIDTENTRY;


HRESULT
CALLBACK
idt(
    _In_ PDEBUG_CLIENT DebugClient,
    _In_opt_ PCSTR args
    )

/*++

Routine Description:

    Displays interrupt descriptor table.

Return Value:

    HRESULT

Environment:

    Kernel mode.

Note:

    Supports only the x86-based platform.

--*/

{
    HRESULT Result;
    BOOL IsAll = FALSE;
    ULONG ProcessorType;
    ULONG64 StartUnexpectedRange;
    ULONG64 EndUnexpectedRange;
    ULONG Index;
    ULONG BytesRead;
    DEBUG_VALUE idtr;
    KIDTENTRY idt[256];
    ULONG TypeId;
    ULONG64 Module;
    ULONG DispatchCode;
    ULONG InterruptListEntry;
    ULONG ServiceRoutine;
    ULONG i;

    if ((Result = QueryInterfaces(DebugClient)) != S_OK) {

        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugControl->GetActualProcessorType(&ProcessorType)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't get the processor type.\n");
        ReleaseInterfaces();
        return Result;
    }

    if (IMAGE_FILE_MACHINE_I386 != ProcessorType) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Supports only the x86-based platform.\n");
        ReleaseInterfaces();
        return S_FALSE;
    }

    if ((Result = DebugSymbols->GetOffsetByName("nt!KiStartUnexpectedRange", &StartUnexpectedRange)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read nt!KiStartUnexpectedRange.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugSymbols->GetOffsetByName("nt!KiEndUnexpectedRange", &EndUnexpectedRange)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read nt!KiEndUnexpectedRange.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugRegisters->GetIndexByName("idtr", &Index)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read index of the idtr register.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugRegisters->GetValue(Index, &idtr)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read idtr register.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugDataSpaces->ReadVirtual(DEBUG_EXTEND64(idtr.I64), &idt, sizeof(idt), &BytesRead)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read idt table.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugSymbols->GetSymbolTypeId("nt!_KINTERRUPT", &TypeId, &Module)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read TypeId of the nt!_KINTERRUPT.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugSymbols->GetFieldOffset(Module, TypeId, "DispatchCode", &DispatchCode)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read offset DispatchCode field of the nt!_KINTERRUPT.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugSymbols->GetFieldOffset(Module, TypeId, "InterruptListEntry", &InterruptListEntry)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read offset InterruptListEntry field of the nt!_KINTERRUPT.\n");
        ReleaseInterfaces();
        return Result;
    }

    if ((Result = DebugSymbols->GetFieldOffset(Module, TypeId, "ServiceRoutine", &ServiceRoutine)) != S_OK) {

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read offset ServiceRoutine field of the nt!_KINTERRUPT.\n");
        ReleaseInterfaces();
        return Result;
    }

    if (args) {

        if ((args[0] == '-' || args[0] == '/') && args[1] == 'a') {

            IsAll = TRUE;
        }
    }

    DebugControl->Output(DEBUG_OUTPUT_NORMAL, "\nDumping IDT: %p\n\n", idtr.I64);

    for (i = 0; i < sizeof(idt) / sizeof(KIDTENTRY); i++) {

        ULONG64 Offset;
        USHORT Access = 0x0FFF;

        Offset = idt[i].ExtendedOffset;
        Offset = Offset << 16;
        Offset = Offset | idt[i].Offset;

        Offset = DEBUG_EXTEND64(Offset);

        if (!IsAll && 
            (i < 0x30 || (StartUnexpectedRange <= Offset && Offset <= EndUnexpectedRange))) {

            continue;
        }

        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "%02lx:\t", i);

        Access &= idt[i].Access;

        if (Access == GATE_TYPE_TASK && idt[i].ExtendedOffset == 0) {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Task Selector = 0x%04lx\n", idt[i].Selector);
        }
        else if (Access == GATE_TYPE_INT || Access == GATE_TYPE_TRAP) {

            ULONG64 InterruptObject;
            ULONG64 Displacement;
            USHORT ObjectType;
            UCHAR Buffer[MAX_PATH];

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "%p\t", Offset);

            InterruptObject = Offset - DispatchCode;

            if (DebugDataSpaces->ReadVirtual(InterruptObject, &ObjectType, sizeof(ObjectType), &BytesRead) == S_OK) {

                if (ObjectType == INTERRUPT_OBJECT_TYPE) {

                    ULONG64 InterruptListHead;
                    ULONG64 Routine;
                    ULONG64 ForwardLink;
                    BOOL IsNextEntry = FALSE;

                    InterruptListHead = InterruptObject + InterruptListEntry;

                    do {

                        if (IsNextEntry) {

                            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "\t\t\t");
                        }

                        if (DebugDataSpaces->ReadPointersVirtual(1, InterruptObject + ServiceRoutine, &Routine) == S_OK) {

                            if (DebugSymbols->GetNameByOffset(Routine, (PSTR)Buffer, _countof(Buffer), &BytesRead, &Displacement) == S_OK) {

                                DebugControl->Output(DEBUG_OUTPUT_NORMAL, "%s", Buffer);

                                if (Displacement != 0) {

                                    DebugControl->Output(DEBUG_OUTPUT_NORMAL, "+0x%x", Displacement);
                                }
                            }
                            else {

                                DebugControl->Output(DEBUG_OUTPUT_NORMAL, "%p", Routine);
                            }
                        }
                        else {

                            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read memory.");
                        }

                        DebugControl->Output(DEBUG_OUTPUT_NORMAL, " (KINTERRUPT %p)\n", InterruptObject);

                        if (DebugDataSpaces->ReadPointersVirtual(1, InterruptObject + InterruptListEntry, &ForwardLink) != S_OK) {

                            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read memory.\n");
                            break;
                        }

                        InterruptObject = ForwardLink - InterruptListEntry;

                        IsNextEntry = TRUE;

                    } while (InterruptListHead != ForwardLink);
                }
                else {

                    if (DebugSymbols->GetNameByOffset(Offset, (PSTR)Buffer, _countof(Buffer), &BytesRead, &Displacement) == S_OK) {

                        DebugControl->Output(DEBUG_OUTPUT_NORMAL, "%s", Buffer);

                        if (Displacement != 0) {

                            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "+0x%x", Displacement);
                        }
                    }

                    DebugControl->Output(DEBUG_OUTPUT_NORMAL, "\n");
                }
            }
            else {

                DebugControl->Output(DEBUG_OUTPUT_NORMAL, "Couldn't read memory.\n");
            }
        }
        else {

            DebugControl->Output(DEBUG_OUTPUT_NORMAL, "%p\n", NULL);
        }
    }

    DebugControl->Output(DEBUG_OUTPUT_NORMAL, "\n");

    ReleaseInterfaces();

    return S_OK;
}
