/*
 * wiredopposite            Copyright (c) 2026
 */

#include "nt.hpp"
#include "ex.hpp"
#include "ob.hpp"

EXPORTNUM(186) NTSTATUS XBOXAPI NtClearEvent
(
    HANDLE EventHandle
)
{
    PKEVENT Event;
    NTSTATUS Status = ObReferenceObjectByHandle(EventHandle, &ExEventObjectType, (PVOID*)&Event);

    if (NT_SUCCESS(Status)) {
        KeClearEvent(Event);
        ObfDereferenceObject((PVOID)Event);
    }

    return Status;
}

EXPORTNUM(189) NTSTATUS XBOXAPI NtCreateEvent
(
    PHANDLE EventHandle,
    POBJECT_ATTRIBUTES ObjectAttributes,
    EVENT_TYPE EventType,
    BOOLEAN InitialState
)
{
    NTSTATUS Status;
    PKEVENT Event;

    if (EventType == NotificationEvent || EventType == SynchronizationEvent) {
        Status = ObCreateObject(&ExEventObjectType, ObjectAttributes, sizeof(KEVENT), (PVOID*)&Event);

        if (NT_SUCCESS(Status)) {
            KeInitializeEvent(Event, EventType, InitialState);
            Status = ObInsertObject(Event, ObjectAttributes, 0, EventHandle);
        }
    }
    else {
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}

EXPORTNUM(205) NTSTATUS XBOXAPI NtPulseEvent
(
    HANDLE EventHandle,
    PLONG PreviousState
)
{
    PKEVENT Event;
    NTSTATUS Status = ObReferenceObjectByHandle(EventHandle, &ExEventObjectType, (PVOID*)&Event);

    if (NT_SUCCESS(Status)) {
        LONG OldState = KePulseEvent(Event, 1, FALSE);
        ObfDereferenceObject((PVOID)Event);

        if (PreviousState) {
            *PreviousState = OldState;
        }
    }
    
    return Status;
}

EXPORTNUM(209) NTSTATUS XBOXAPI NtQueryEvent
(
    HANDLE EventHandle,
    PEVENT_BASIC_INFORMATION EventInformation
)
{
    PKEVENT Event;
    NTSTATUS Status = ObReferenceObjectByHandle(EventHandle, &ExEventObjectType, (PVOID*)&Event);

    if (NT_SUCCESS(Status)) {
        LONG SignalState = Event->Header.SignalState;
        EVENT_TYPE EventType = static_cast<EVENT_TYPE>(Event->Header.Type);

        ObfDereferenceObject((PVOID)Event);

        EventInformation->EventState = SignalState;
        EventInformation->EventType = EventType;
    }

    return Status;
}

EXPORTNUM(225) NTSTATUS XBOXAPI NtSetEvent
(
    HANDLE EventHandle,
    PLONG PreviousState
)
{
    PKEVENT Event;
    NTSTATUS Status = ObReferenceObjectByHandle(EventHandle, &ExEventObjectType, (PVOID*)&Event);

    if (NT_SUCCESS(Status)) {
        LONG OldState = KeSetEvent(Event, 1, FALSE);
        ObfDereferenceObject((PVOID)Event);

        if (PreviousState) {
            *PreviousState = OldState;
        }
    }

    return Status;
}
