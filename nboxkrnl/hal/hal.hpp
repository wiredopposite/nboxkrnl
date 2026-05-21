/*
 * ergo720                Copyright (c) 2023
 * wiredopposite		  Copyright (c) 2026
 */

#pragma once

#include "..\types.hpp"
#include "ke.hpp"


#define DISC_TRAY_STATE_CLOSED 	0
#define DISC_TRAY_STATE_ACTIVE 	(1u << 0)
#define DISC_TRAY_STATE_OPEN 	(1u << 4)
#define DISC_TRAY_STATE_UNK 	(1u << 5)
#define DISC_TRAY_STATE_OPENING	(3u << 4)
#define DISC_TRAY_STATE_EMPTY 	(1u << 6)
#define DISC_TRAY_STATE_CLOSING (5u << 4)
#define DISC_TRAY_STATE_UNK2 	(3u << 5)
#define DISC_TRAY_STATE_RESET 	(7u << 4)
#define DISC_TRAY_STATE_MASK 	(7u << 4)

using PHAL_SHUTDOWN_NOTIFICATION = VOID(XBOXAPI *)(
	struct HAL_SHUTDOWN_REGISTRATION *ShutdownRegistration
	);

struct HAL_SHUTDOWN_REGISTRATION {
	PHAL_SHUTDOWN_NOTIFICATION NotificationRoutine;
	LONG Priority;
	LIST_ENTRY ListEntry;
};
using PHAL_SHUTDOWN_REGISTRATION = HAL_SHUTDOWN_REGISTRATION *;

#ifdef __cplusplus
extern "C" {
#endif

EXPORTNUM(9) DLLEXPORT NTSTATUS XBOXAPI HalReadSMCTrayState
(
    PULONG TrayState,
    PULONG TrayStateChangeCount
);

EXPORTNUM(40) DLLEXPORT extern ULONG HalDiskCachePartitionCount;

EXPORTNUM(43) DLLEXPORT VOID XBOXAPI HalEnableSystemInterrupt
(
	ULONG BusInterruptLevel,
	KINTERRUPT_MODE InterruptMode
);

EXPORTNUM(44) DLLEXPORT ULONG XBOXAPI HalGetInterruptVector
(
	ULONG BusInterruptLevel,
	PKIRQL Irql
);

EXPORTNUM(45) DLLEXPORT NTSTATUS XBOXAPI HalReadSMBusValue
(
	UCHAR SlaveAddress,
	UCHAR CommandCode,
	BOOLEAN ReadWordValue,
	ULONG *DataValue
);

EXPORTNUM(46) DLLEXPORT VOID XBOXAPI HalReadWritePCISpace
(
	ULONG BusNumber,
	ULONG SlotNumber,
	ULONG RegisterNumber,
	PVOID Buffer,
	ULONG Length,
	BOOLEAN WritePCISpace
);

EXPORTNUM(47) DLLEXPORT VOID XBOXAPI HalRegisterShutdownNotification
(
	PHAL_SHUTDOWN_REGISTRATION ShutdownRegistration,
	BOOLEAN Register
);

EXPORTNUM(48) DLLEXPORT VOID FASTCALL HalRequestSoftwareInterrupt
(
	KIRQL Request
);

EXPORTNUM(50) DLLEXPORT NTSTATUS XBOXAPI HalWriteSMBusValue
(
	UCHAR SlaveAddress,
	UCHAR CommandCode,
	BOOLEAN WriteWordValue,
	ULONG DataValue
);

EXPORTNUM(356) DLLEXPORT extern ULONG HalBootSMCVideoMode;

#ifdef __cplusplus
}
#endif

inline DWORD HalCounterPerMicroseconds;

BOOLEAN HalInitSystem();
