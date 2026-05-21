/*
* PatrickvL              Copyright (c) 2018
* wiredopposite          Copyright (c) 2026
*/

#include "rtl.hpp"
#include "rtlp.hpp"
#include "ex.hpp"


// Source: Cxbx-Reloaded
EXPORTNUM(260) NTSTATUS XBOXAPI RtlAnsiStringToUnicodeString
(
	PUNICODE_STRING DestinationString,
	PSTRING SourceString,
	UCHAR AllocateDestinationString
)
{
	DWORD Total = (SourceString->Length + sizeof(ANSI_NULL)) * sizeof(WCHAR);

	if (Total > 0xFFFF) {
		return STATUS_INVALID_PARAMETER_2;
	}

	DestinationString->Length = (USHORT)(Total - sizeof(WCHAR));
	if (AllocateDestinationString) {
		DestinationString->MaximumLength = (USHORT)Total;
		if (!(DestinationString->Buffer = (WCHAR *)ExAllocatePoolWithTag(Total, 'grtS'))) {
			return STATUS_NO_MEMORY;
		}
	}
	else {
		if (Total > DestinationString->MaximumLength) {
			return STATUS_BUFFER_OVERFLOW;
		}
	}

	RtlMultiByteToUnicodeN((PWSTR)DestinationString->Buffer,
		(ULONG)DestinationString->Length,
		NULL,
		SourceString->Buffer,
		SourceString->Length);

	DestinationString->Buffer[DestinationString->Length / sizeof(WCHAR)] = 0;

	return STATUS_SUCCESS;
}

// Source: Cxbx-Reloaded
EXPORTNUM(267) NTSTATUS XBOXAPI RtlCharToInteger
(
	PCSZ String,
	ULONG Base,
	PULONG Value
)
{
	CHAR bMinus = 0;

	// skip leading whitespaces
	while (*String != '\0' && *String <= ' ') {
		String++;
	}

	// Check for +/-
	if (*String == '+') {
		String++;
	}
	else if (*String == '-') {
		bMinus = 1;
		String++;
	}

	// base = 0 means autobase
	if (Base == 0) {
		Base = 10;
		if (String[0] == '0') {
			if (String[1] == 'b') {
				String += 2;
				Base = 2;
			}
			else if (String[1] == 'o') {
				String += 2;
				Base = 8;
			}
			else if (String[1] == 'x') {
				String += 2;
				Base = 16;
			}
		}
	}
	else if (Base != 2 && Base != 8 && Base != 10 && Base != 16) {
		return STATUS_INVALID_PARAMETER;
	}

	if (Value == NULL) {
		return STATUS_ACCESS_VIOLATION;
	}

	ULONG RunningTotal = 0;
	while (*String != '\0') {
		CHAR chCurrent = *String;
		INT digit;

		if (chCurrent >= '0' && chCurrent <= '9') {
			digit = chCurrent - '0';
		}
		else if (chCurrent >= 'A' && chCurrent <= 'Z') {
			digit = chCurrent - 'A' + 10;
		}
		else if (chCurrent >= 'a' && chCurrent <= 'z') {
			digit = chCurrent - 'a' + 10;
		}
		else {
			digit = -1;
		}

		if (digit < 0 || digit >= (INT)Base) {
			break;
		}

		RunningTotal = RunningTotal * Base + digit;
		String++;
	}

	*Value = bMinus ? (0 - RunningTotal) : RunningTotal;

	return STATUS_SUCCESS;
}

// Source: Cxbx-Reloaded
EXPORTNUM(299) NTSTATUS XBOXAPI RtlMultiByteToUnicodeN
(
	PWSTR UnicodeString,
	ULONG MaxBytesInUnicodeString,
	PULONG BytesInUnicodeString,
	PCHAR MultiByteString,
	ULONG BytesInMultiByteString
)
{
	ULONG maxUnicodeChars = MaxBytesInUnicodeString / sizeof(WCHAR);
	ULONG numChars = (maxUnicodeChars < BytesInMultiByteString) ? maxUnicodeChars : BytesInMultiByteString;

	if (BytesInUnicodeString != NULL) {
		*BytesInUnicodeString = numChars * sizeof(WCHAR);
	}

	while (numChars) {
		*UnicodeString = (WCHAR)(*MultiByteString);

		UnicodeString++;
		MultiByteString++;
		numChars--;
	}

	return STATUS_SUCCESS;
}

EXPORTNUM(300) NTSTATUS XBOXAPI RtlMultiByteToUnicodeSize
(
    PULONG BytesInUnicodeString,
    PCHAR MultiByteString,
    ULONG BytesInMultiByteString
)
{
	*BytesInUnicodeString = BytesInMultiByteString * sizeof(WCHAR);
	return STATUS_SUCCESS;
}

EXPORTNUM(308) NTSTATUS XBOXAPI RtlUnicodeStringToAnsiString
(
    PSTRING DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
)
{
	NTSTATUS Status = STATUS_SUCCESS, ConvertStatus;
	ULONG ReqMaxLen = (static_cast<ULONG>(SourceString->Length) + sizeof(WCHAR)) / sizeof(WCHAR);
	ULONG ConvertedLen;

	if (ReqMaxLen >= 0x10000) {
		return STATUS_INVALID_PARAMETER_2;
	}

	DestinationString->Length = static_cast<USHORT>(ReqMaxLen - 1);

	if (!AllocateDestinationString) {
		if (DestinationString->Length >= DestinationString->MaximumLength) {
			Status = STATUS_BUFFER_OVERFLOW;
			
			if (DestinationString->MaximumLength == 0) {
				return Status;
			}
			
			DestinationString->Length = DestinationString->MaximumLength - 1;
		}
	}
	else {
		DestinationString->MaximumLength = static_cast<USHORT>(ReqMaxLen);
		DestinationString->Buffer = static_cast<PCHAR>(ExAllocatePoolWithTag(ReqMaxLen, RtlpPoolTag));
		
		if (!DestinationString->Buffer) {
			return STATUS_NO_MEMORY;
		}
	}

	ConvertStatus = RtlUnicodeToMultiByteN(
		DestinationString->Buffer, 
		DestinationString->Length, 
		&ConvertedLen, 
		SourceString->Buffer, 
		SourceString->Length
	);

	if (!NT_SUCCESS(ConvertStatus)) {
		if (AllocateDestinationString) {
			ExFreePool(DestinationString->Buffer);
			DestinationString->Buffer = nullptr;
		}
	}
	else {
		DestinationString->Buffer[ConvertedLen] = ANSI_NULL;
		ConvertStatus = Status;
	}

	return ConvertStatus;
}

EXPORTNUM(309) NTSTATUS XBOXAPI RtlUnicodeStringToInteger
(
    PUNICODE_STRING String,
    ULONG Base,
    PULONG Value
)
{
	// TODO
	RIP_UNIMPLEMENTED();
	return STATUS_NOT_IMPLEMENTED;
}

EXPORTNUM(310) NTSTATUS XBOXAPI RtlUnicodeToMultiByteN
(
    PCHAR MultiByteString,
    ULONG MaxBytesInMultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
)
{
	ULONG NumChars = BytesInUnicodeString / sizeof(WCHAR);

	if (MaxBytesInMultiByteString <= NumChars) {
		NumChars = MaxBytesInMultiByteString;
	}

	if (BytesInMultiByteString) {
		*BytesInMultiByteString = NumChars;
	}

	while (NumChars--) {
		if (*UnicodeString < 0x100) {
			*MultiByteString = static_cast<CHAR>(static_cast<UCHAR>(*UnicodeString));
		}
		else {
			*MultiByteString = '?';
		}

		UnicodeString++;
		MultiByteString++;
	}

	return STATUS_SUCCESS;
}

EXPORTNUM(311) NTSTATUS XBOXAPI RtlUnicodeToMultiByteSize
(
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
)
{
	*BytesInMultiByteString = BytesInUnicodeString / sizeof(WCHAR);
	return STATUS_SUCCESS;
}
