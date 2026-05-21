/*
 * PatrickvL              Copyright (c) 2018
 * wiredopposite          Copyright (c) 2026
 */

#include "rtl.hpp"
#include "rtlp.hpp"
#include "ex.hpp"
#include <string.h>


// Source: Cxbx-Reloaded
EXPORTNUM(272) VOID XBOXAPI RtlCopyString
(
	PSTRING DestinationString,
	PSTRING SourceString
)
{
	if (SourceString == nullptr) {
		DestinationString->Length = 0;
		return;
	}

	CHAR *pd = DestinationString->Buffer;
	CHAR *ps = SourceString->Buffer;
	USHORT len = SourceString->Length;
	if ((USHORT)len > DestinationString->MaximumLength) {
		len = DestinationString->MaximumLength;
	}

	DestinationString->Length = (USHORT)len;
	memcpy(pd, ps, len);
}

EXPORTNUM(273) VOID XBOXAPI RtlCopyUnicodeString
(
    PUNICODE_STRING DestinationString,
    PUNICODE_STRING SourceString
)
{
	USHORT Len;

	if (SourceString == nullptr) {
		DestinationString->Length = 0;
		return;
	}

	Len = SourceString->Length;

	if (DestinationString->MaximumLength < Len) {
		Len = DestinationString->MaximumLength;
	}

	DestinationString->Length = Len;
	memcpy(DestinationString->Buffer, SourceString->Buffer, Len);
	
	if (DestinationString->Length < DestinationString->MaximumLength) {
		DestinationString->Buffer[Len / sizeof(WCHAR)] = UNICODE_NULL;
	}
}

EXPORTNUM(274) BOOLEAN XBOXAPI RtlCreateUnicodeString
(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
)
{
	LONG Bytes = RtlpStrWCount(SourceString) * sizeof(WCHAR) + sizeof(WCHAR);
	PWSTR Buffer = static_cast<PWSTR>(ExAllocatePoolWithTag(Bytes, RtlpPoolTag));

	DestinationString->Buffer = Buffer;

	if (Buffer) {
		memmove(Buffer, SourceString, Bytes);
		DestinationString->MaximumLength = (USHORT)Bytes;
		DestinationString->Length = DestinationString->MaximumLength - sizeof(WCHAR);
		return TRUE;
	}
	
	return FALSE;
}

// Source: Cxbx-Reloaded
EXPORTNUM(289) VOID XBOXAPI RtlInitAnsiString
(
	PANSI_STRING DestinationString,
	PCSZ SourceString
)
{
	DestinationString->Buffer = const_cast<PCHAR>(SourceString);
	if (SourceString) {
		DestinationString->Length = (USHORT)strlen(DestinationString->Buffer);
		DestinationString->MaximumLength = DestinationString->Length + 1;
	}
	else {
		DestinationString->Length = DestinationString->MaximumLength = 0;
	}
}

EXPORTNUM(290) VOID XBOXAPI RtlInitUnicodeString
(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
)
{
	DestinationString->Length = 0;
	DestinationString->MaximumLength = 0;
	DestinationString->Buffer = SourceString ? const_cast<PWCHAR>(SourceString) : nullptr;

	if (DestinationString->Buffer) {
		ULONG Index = ~0u;
		WCHAR Char = 1;

		while (Index != 0 && Char != UNICODE_NULL) {
			Index--;
			Char = *SourceString++;
		}

		DestinationString->MaximumLength = ~(USHORT)Index * sizeof(WCHAR);
		DestinationString->Length = DestinationString->MaximumLength - sizeof(WCHAR);
	}
}

EXPORTNUM(361) DLLEXPORT VOID CDECL RtlSnprintf
(
    CHAR * Buffer,
    SIZE_T Size,
    const CHAR * Format,
    ...
)
{
	va_list Args;
	va_start(Args, Format);
	npf_vsnprintf(Buffer, Size, Format, Args);
	va_end(Args);
}

EXPORTNUM(362) DLLEXPORT VOID CDECL RtlSprintf
(
    CHAR * Buffer,
    const CHAR * Format,
    ...
)
{
	va_list Args;
	va_start(Args, Format);
	npf_vsnprintf(Buffer, (SIZE_T)~0U, Format, Args);
	va_end(Args);
}

EXPORTNUM(363) DLLEXPORT VOID CDECL RtlVsnprintf
(
    CHAR * Buffer,
    SIZE_T Size,
    const CHAR * Format,
    va_list Args
)
{
	npf_vsnprintf(Buffer, Size, Format, Args);
}

EXPORTNUM(364) DLLEXPORT VOID CDECL RtlVsprintf
(
    CHAR * Buffer,
    const CHAR * Format,
    va_list Args
)
{
	npf_vsnprintf(Buffer, (SIZE_T)~0U, Format, Args);
}

LONG RtlpStrWCount
(
	PCWSTR String
)
{
	PCWSTR Start = String;
	while (*String) {
		String++;
	}

	return (LONG)(String - Start);
}
