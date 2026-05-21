/*
 * wiredopposite              Copyright (c) 2026
 */

#include <assert.h>
#include "ke.hpp"
#include "ki.hpp"
#include "rtl.hpp"
#include "..\kernel.hpp"


VOID KiActivateQueueWaiter(PKQUEUE Queue)
{
    assert(KeGetCurrentIrql() == DISPATCH_LEVEL);

    Queue->CurrentCount--;

    if (Queue->CurrentCount >= Queue->MaximumCount) {
        return;
    }

    PLIST_ENTRY QueueEntry = Queue->EntryListHead.Flink;
    PLIST_ENTRY WaitEntry = Queue->Header.WaitListHead.Blink;

    if ((QueueEntry != &Queue->EntryListHead) &&
        (WaitEntry != &Queue->Header.WaitListHead)) {
        RemoveEntryList(QueueEntry);
        QueueEntry->Flink = nullptr;

        Queue->Header.SignalState--;

        PKWAIT_BLOCK WaitBlock = CONTAINING_RECORD(WaitEntry, KWAIT_BLOCK, WaitListEntry);
        PKTHREAD Thread = WaitBlock->Thread;

        KiUnwaitThread(Thread, reinterpret_cast<LONG_PTR>(QueueEntry), 0);
    }
}

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
    KIRQL OldIrql;
    PKQUEUE CurrentQueue;
    PLIST_ENTRY QueueEntry;
    KWAIT_BLOCK WaitBlock;
    LARGE_INTEGER DueTime;
    LARGE_INTEGER NewTimeout;
    PLARGE_INTEGER OriginalTimeout = Timeout;
    PKTHREAD Thread = KeGetCurrentThread();

    assert(Queue->Header.Type == QueueObject);
    assert(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    if (!Thread->WaitNext) {
        OldIrql = KeRaiseIrqlToDpcLevel();
        Thread->WaitIrql = OldIrql;
    }
    else {
        Thread->WaitNext = FALSE;
    }

    CurrentQueue = Thread->Queue;
    Thread->Queue = Queue;

    if (Queue == CurrentQueue) {
        Queue->CurrentCount--;
    }
    else {
        QueueEntry = &Thread->QueueListEntry;
        if (CurrentQueue) {
            RemoveEntryList(QueueEntry);
            KiActivateQueueWaiter(CurrentQueue);
        }

        InsertTailList(&Queue->ThreadListHead, QueueEntry);
    }

    while (TRUE) {
        QueueEntry = Queue->EntryListHead.Flink;
        if ((QueueEntry != &Queue->EntryListHead) && (Queue->CurrentCount < Queue->MaximumCount)) {
            Queue->Header.SignalState--;
            Queue->CurrentCount++;
            RemoveEntryList(QueueEntry);
            QueueEntry->Flink = nullptr;

            KiUnlockDispatcherDatabase(Thread->WaitIrql);
            return QueueEntry;
        }

        if (Thread->ApcState.KernelApcPending && (Thread->WaitIrql == PASSIVE_LEVEL)) {
            Queue->CurrentCount++;
            KiUnlockDispatcherDatabase(Thread->WaitIrql);

            OldIrql = KeRaiseIrqlToDpcLevel();
            Thread->WaitIrql = OldIrql;
            Queue->CurrentCount--;
            continue;
        }

        if ((WaitMode != KernelMode) && Thread->ApcState.UserApcPending) {
            Queue->CurrentCount++;
            KiUnlockDispatcherDatabase(Thread->WaitIrql);
            return reinterpret_cast<PLIST_ENTRY>(STATUS_USER_APC);
        }

        Thread->WaitStatus = 0;
        Thread->WaitBlockList = &WaitBlock;
        WaitBlock.WaitKey = 0;
        WaitBlock.Object = Queue;
        WaitBlock.WaitType = WaitAny;
        WaitBlock.Thread = Thread;

        if (Timeout) {
            if (Timeout->QuadPart == 0) {
                Queue->CurrentCount++;
                KiUnlockDispatcherDatabase(Thread->WaitIrql);
                return reinterpret_cast<PLIST_ENTRY>(STATUS_TIMEOUT);
            }

            WaitBlock.NextWaitBlock = &Thread->TimerWaitBlock;
            Thread->Timer.Header.WaitListHead.Flink = &Thread->TimerWaitBlock.WaitListEntry;
            Thread->Timer.Header.WaitListHead.Blink = &Thread->TimerWaitBlock.WaitListEntry;
            Thread->TimerWaitBlock.NextWaitBlock = &WaitBlock;

            if (KiInsertTimer(&Thread->Timer, *Timeout) == FALSE) {
                Queue->CurrentCount++;
                KiUnlockDispatcherDatabase(Thread->WaitIrql);
                return reinterpret_cast<PLIST_ENTRY>(STATUS_TIMEOUT);
            }

            DueTime.QuadPart = Thread->Timer.DueTime.QuadPart;
        }
        else {
            WaitBlock.NextWaitBlock = &WaitBlock;
        }

        InsertTailList(&Queue->Header.WaitListHead, &WaitBlock.WaitListEntry);
        Thread->WaitMode = WaitMode;
        Thread->Alertable = FALSE;
        Thread->WaitReason = WrQueue;
        Thread->WaitTime = KeTickCount;
        Thread->State = Waiting;
        InsertTailList(&KiWaitInListHead, &Thread->WaitListEntry);

        assert(Thread->WaitIrql <= DISPATCH_LEVEL);

        ULONG_PTR WaitResult = KiSwapThread();
        Thread->WaitReason = 0;
        if (WaitResult != STATUS_KERNEL_APC) {
            return reinterpret_cast<PLIST_ENTRY>(WaitResult);
        }

        if (Timeout) {
            Timeout = KiRecalculateTimerDueTime(OriginalTimeout, &DueTime, &NewTimeout);
        }

        OldIrql = KeRaiseIrqlToDpcLevel();
        Thread->WaitIrql = OldIrql;
        Queue->CurrentCount--;
    }
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
