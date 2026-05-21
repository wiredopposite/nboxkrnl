/*
* PatrickvL              Copyright (c) 2016
* wiredopposite          Copyright (c) 2026
*/

#include "rtl.hpp"
#include "rtlp.hpp"
#include "ex.hpp"


EXPORTNUM(313) WCHAR XBOXAPI RtlUpcaseUnicodeChar
(
    WCHAR SourceCharacter
)
{
	return RtlpUpperWChar(SourceCharacter);
}

EXPORTNUM(314) NTSTATUS XBOXAPI RtlUpcaseUnicodeString
(
    PUNICODE_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
)
{
	USHORT ReqLen = SourceString->Length;
	ULONG Count;

	if (!AllocateDestinationString) {
		if (DestinationString->MaximumLength < ReqLen) {
			return STATUS_BUFFER_OVERFLOW;
		}
	}
	else {
		DestinationString->MaximumLength = ReqLen;
		DestinationString->Buffer = static_cast<PWCHAR>(ExAllocatePoolWithTag(ReqLen, RtlpPoolTag));

		if (!DestinationString->Buffer) {
			return STATUS_NO_MEMORY;
		}
	}

	Count = ReqLen / sizeof(WCHAR);

	for (USHORT i = 0; i < Count; i++) {
		DestinationString->Buffer[i] = RtlpUpperWChar(SourceString->Buffer[i]);
	}

	DestinationString->Length = SourceString->Length;
	return STATUS_SUCCESS;
}

EXPORTNUM(315) NTSTATUS XBOXAPI RtlUpcaseUnicodeToMultiByteN
(
    PCHAR MultiByteString,
    ULONG MaxBytesInMultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
)
{
	WCHAR Tmp;
	ULONG NumChars = BytesInUnicodeString / sizeof(WCHAR);

	if (MaxBytesInMultiByteString <= NumChars) {
		NumChars = MaxBytesInMultiByteString;
	}
	if (BytesInMultiByteString) {
		*BytesInMultiByteString = NumChars;
	}

	while (NumChars--) {
		Tmp = (*UnicodeString < 0x100) ? *UnicodeString : '?';
		Tmp = RtlpUpperWChar(Tmp);
		*MultiByteString++ = (Tmp < 0x100) ? static_cast<CHAR>(static_cast<UCHAR>(Tmp)) : '?';
		UnicodeString++;
	}

	return STATUS_SUCCESS;
}

// Source: Cxbx-Reloaded
EXPORTNUM(316) CHAR XBOXAPI RtlUpperChar
(
	CHAR Character
)
{
	BYTE CharCode = (BYTE)Character;

	if (CharCode >= 'a' && CharCode <= 'z') {
		CharCode ^= 0x20;
	}
	// Latin alphabet (ISO 8859-1)
	else if (CharCode >= 0xe0 && CharCode <= 0xfe && CharCode != 0xf7) {
		CharCode ^= 0x20;
	}
	else if (CharCode == 0xFF) {
		CharCode = '?';
	}

	return CharCode;
}

// Source: Cxbx-Reloaded
EXPORTNUM(317) VOID XBOXAPI RtlUpperString
(
	PSTRING DestinationString,
	PSTRING SourceString
)
{
	CHAR *pDst = DestinationString->Buffer;
	CHAR *pSrc = SourceString->Buffer;
	ULONG length = SourceString->Length;
	if ((USHORT)length > DestinationString->MaximumLength) {
		length = DestinationString->MaximumLength;
	}

	DestinationString->Length = (USHORT)length;
	while (length > 0) {
		*pDst++ = RtlUpperChar(*pSrc++);
		length--;
	}
}
