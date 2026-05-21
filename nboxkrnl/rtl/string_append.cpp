/*
* PatrickvL              Copyright (c) 2016
* wiredopposite          Copyright (c) 2026
*/

#include "rtl.hpp"
#include <string.h>


// Source: Cxbx-Reloaded
EXPORTNUM(261) NTSTATUS XBOXAPI RtlAppendStringToString
(
	PSTRING Destination,
	PSTRING Source
)
{
	NTSTATUS Result = STATUS_SUCCESS;

	USHORT dstLen = Destination->Length;
	USHORT srcLen = Source->Length;
	if (srcLen > 0) {
		if ((srcLen + dstLen) > Destination->MaximumLength) {
			Result = STATUS_BUFFER_TOO_SMALL;
		}
		else {
			CHAR *dstBuf = Destination->Buffer + Destination->Length;
			CHAR *srcBuf = Source->Buffer;
			memmove(dstBuf, srcBuf, srcLen);
			Destination->Length += srcLen;
		}
	}

	return Result;
}

// Source: Cxbx-Reloaded
EXPORTNUM(262) NTSTATUS XBOXAPI RtlAppendUnicodeStringToString
(
	PUNICODE_STRING Destination,
	PUNICODE_STRING Source
)
{
	NTSTATUS Result = STATUS_SUCCESS;

	USHORT dstLen = Destination->Length;
	USHORT srcLen = Source->Length;
	if (srcLen > 0) {
		if ((srcLen + dstLen) > Destination->MaximumLength) {
			Result = STATUS_BUFFER_TOO_SMALL;
		}
		else {
			WCHAR *dstBuf = (WCHAR*)(Destination->Buffer + (Destination->Length / sizeof(WCHAR)));
			memmove(dstBuf, Source->Buffer, srcLen);
			Destination->Length += srcLen;
			if (Destination->Length < Destination->MaximumLength) {
				dstBuf[srcLen / sizeof(WCHAR)] = UNICODE_NULL;
			}
		}
	}

	return Result;
}

EXPORTNUM(263) NTSTATUS XBOXAPI RtlAppendUnicodeToString
(
    PUNICODE_STRING Destination,
    PCWSTR Source
)
{
	UNICODE_STRING SourceString;
	USHORT DstLen;
	PWSTR SrcPtr;

	if (Source) {
		RtlInitUnicodeString(&SourceString, Source);

		DstLen = Destination->Length;
		if (Destination->MaximumLength < (SourceString.Length + DstLen)) {
			return STATUS_BUFFER_TOO_SMALL;
		}

		SrcPtr = (PWSTR)(Destination->Buffer + (DstLen / sizeof(WCHAR)));
		memmove(SrcPtr, SourceString.Buffer, SourceString.Length);
		Destination->Length += SourceString.Length;

		if (Destination->Length < Destination->MaximumLength) {
			SrcPtr[SourceString.Length / sizeof(WCHAR)] = UNICODE_NULL;
		}
	}

	return STATUS_SUCCESS;
}
