/*
 * wiredopposite              Copyright (c) 2026
 */

#include "ke.hpp"
#include "ki.hpp"
#include "rtl.hpp"
#include "..\kernel.hpp"

EXPORTNUM(111) VOID XBOXAPI KeInitializeQueue
(
    PKQUEUE Queue,
    ULONG Count
)
{
    Queue->Header.SignalState = 0;
    Queue->CurrentCount = 0;

    InitializeListHead(&Queue->Header.WaitListHead);
    InitializeListHead(&Queue->EntryListHead);
    InitializeListHead(&Queue->ThreadListHead);

    Queue->Header.Type = QueueObject;
    Queue->Header.Size = sizeof(KQUEUE) / sizeof(LONG);

    if (Count != 0) {
        Queue->MaximumCount = Count;
    }
    else {
        Queue->MaximumCount = 1;
    }
}

EXPORTNUM(116) LONG XBOXAPI KeInsertHeadQueue
(
    PKQUEUE Queue,
    PLIST_ENTRY Entry
)
{
    KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
    LONG State = KiInsertQueue(Queue, Entry, TRUE);
    KiUnlockDispatcherDatabase(OldIrql);
    return State;
}

EXPORTNUM(117) LONG XBOXAPI KeInsertQueue
(
    PKQUEUE Queue,
    PLIST_ENTRY Entry
)
{
    KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
    LONG State = KiInsertQueue(Queue, Entry, FALSE);
    KiUnlockDispatcherDatabase(OldIrql);
    return State;
}

EXPORTNUM(136) PLIST_ENTRY XBOXAPI KeRemoveQueue
(
    PKQUEUE Queue,
    KPROCESSOR_MODE WaitMode,
    PLARGE_INTEGER Timeout
)
{
    // TODO
    RIP_UNIMPLEMENTED();
    return nullptr;
}

LONG FASTCALL KiInsertQueue
(
    PKQUEUE Queue,
    PLIST_ENTRY Entry,
    BOOLEAN InsertHead
)
{
    LONG OldState = Queue->Header.SignalState;
    PLIST_ENTRY WaitEntry = Queue->Header.WaitListHead.Blink;
    PKTHREAD CurrentThread = KeGetCurrentThread();

    if ((WaitEntry != &Queue->Header.WaitListHead) &&
        (Queue->CurrentCount < Queue->MaximumCount) &&
        ((CurrentThread->Queue != Queue) || (CurrentThread->WaitReason != WrQueue))) {
        RemoveEntryList(WaitEntry);

        PKWAIT_BLOCK WaitBlock = CONTAINING_RECORD(WaitEntry, KWAIT_BLOCK, WaitListEntry);
        PKTHREAD Thread = WaitBlock->Thread;

        Thread->WaitStatus = reinterpret_cast<LONG_PTR>(Entry);
        RemoveEntryList(&Thread->QueueListEntry);

        Queue->CurrentCount += 1;
        Thread->WaitReason = 0;

        if (Thread->Timer.Header.Inserted == TRUE) {
            KiRemoveTimer(&Thread->Timer);
        }

        KiReadyThread(Thread);
    }
    else {
        Queue->Header.SignalState = OldState + 1;

        if (InsertHead) {
            InsertHeadList(&Queue->EntryListHead, Entry);
        }
        else {
            InsertTailList(&Queue->EntryListHead, Entry);
        }
    }

    return OldState;
}
