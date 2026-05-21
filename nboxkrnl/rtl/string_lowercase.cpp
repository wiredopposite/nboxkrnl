/*
* PatrickvL              Copyright (c) 2016
* wiredopposite          Copyright (c) 2026
*/

#include "rtl.hpp"
#include "rtlp.hpp"
#include "ex.hpp"


// Source: Cxbx-Reloaded
EXPORTNUM(296) CHAR XBOXAPI RtlLowerChar
(
	CHAR Character
)
{
	BYTE CharCode = (BYTE)Character;

	if (CharCode >= 'A' && CharCode <= 'Z') {
		CharCode ^= 0x20;
	}
	// Latin alphabet (ISO 8859-1)
	else if (CharCode >= 0xc0 && CharCode <= 0xde && CharCode != 0xd7) {
		CharCode ^= 0x20;
	}

	return (CHAR)CharCode;
}

EXPORTNUM(275) WCHAR XBOXAPI RtlDowncaseUnicodeChar
(
    WCHAR SourceCharacter
)
{
	return RtlpLowerWChar(SourceCharacter);
}

EXPORTNUM(276) NTSTATUS XBOXAPI RtlDowncaseUnicodeString
(
    PUNICODE_STRING DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
)
{
	USHORT SrcLen = SourceString->Length;
	PWSTR DestBuf;
	
	if (!AllocateDestinationString) {
		if (DestinationString->MaximumLength < SrcLen) {
			return STATUS_BUFFER_OVERFLOW;
		}
	}
	else {
		DestBuf = static_cast<PWSTR>(ExAllocatePoolWithTag(SrcLen, RtlpPoolTag));
		DestinationString->MaximumLength = SrcLen;
		DestinationString->Buffer = DestBuf;

		if (!DestBuf) {
			return STATUS_NO_MEMORY;
		}
	}

	for (USHORT i = 0; i < (SrcLen / sizeof(WCHAR)); i++) {
		DestinationString->Buffer[i] = RtlpLowerWChar(SourceString->Buffer[i]);
	}

	DestinationString->Length = SrcLen;
	return STATUS_SUCCESS;
}
