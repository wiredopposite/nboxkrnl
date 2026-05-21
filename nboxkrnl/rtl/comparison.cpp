/*
* PatrickvL              Copyright (c) 2016
* wiredopposite          Copyright (c) 2026
*/

#include "rtl.hpp"
#include "rtlp.hpp"


// Source: Cxbx-Reloaded
EXPORTNUM(268) SIZE_T XBOXAPI RtlCompareMemory
(
	PVOID Source1,
	PVOID Source2,
	SIZE_T Length
)
{
	SIZE_T Result = Length;

	PBYTE pBytes1 = (PBYTE)Source1;
	PBYTE pBytes2 = (PBYTE)Source2;
	for (DWORD i = 0; i < Length; i++) {
		if (pBytes1[i] != pBytes2[i]) {
			Result = i;
			break;
		}
	}

	return Result;
}

// Source: Cxbx-Reloaded
EXPORTNUM(269) SIZE_T XBOXAPI RtlCompareMemoryUlong
(
	PVOID Source,
	SIZE_T Length,
	ULONG Pattern
)
{
	PULONG ptr = (PULONG)Source;
	ULONG_PTR len = Length / sizeof(ULONG);

	for (ULONG_PTR i = 0; i < len; i++) {
		if (*ptr != Pattern) {
			break;
		}
		ptr++;
	}

	return (SIZE_T)((PCHAR)ptr - (PCHAR)Source);
}

// Source: Cxbx-Reloaded
EXPORTNUM(270) LONG XBOXAPI RtlCompareString
(
	PSTRING String1,
	PSTRING String2,
	BOOLEAN CaseInSensitive
)
{
	const USHORT l1 = String1->Length;
	const USHORT l2 = String2->Length;
	const USHORT maxLen = (l1 <= l2 ? l1 : l2);

	const PCHAR str1 = String1->Buffer;
	const PCHAR str2 = String2->Buffer;

	if (CaseInSensitive) {
		for (unsigned i = 0; i < maxLen; i++) {
			UCHAR char1 = RtlLowerChar(str1[i]);
			UCHAR char2 = RtlLowerChar(str2[i]);
			if (char1 != char2) {
				return char1 - char2;
			}
		}
	}
	else {
		for (unsigned i = 0; i < maxLen; i++) {
			if (str1[i] != str2[i]) {
				return str1[i] - str2[i];
			}
		}
	}

	return l1 - l2;
}

EXPORTNUM(271) LONG XBOXAPI RtlCompareUnicodeString
(
    PUNICODE_STRING String1,
    PUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
)
{
	PWCHAR Str1 = String1->Buffer, Str2 = String2->Buffer;
	LONG Len1 = String1->Length, Len2 = String2->Length;
	PWCHAR End = (PWCHAR)((PBYTE)Str1 + (Len1 < Len2 ? Len1 : Len2));

	if (!CaseInSensitive) {
		while (Str1 < End) {
			if (*Str1 != *Str2) {
				return (LONG)*Str1 - (LONG)*Str2;
			}

			Str1++;
			Str2++;
		}
	}
	else {
		WCHAR Char1, Char2;

		while (Str1 < End) {
			Char1 = RtlpUpperWChar(*Str1);
			Char2 = RtlpUpperWChar(*Str2);

			if (Char1 != Char2) {
				return (LONG)Char1 - (LONG)Char2;
			}

			Str1++;
			Str2++;
		}
	}

	return Len1 - Len2;
}

// Source: Cxbx-Reloaded
EXPORTNUM(279) BOOLEAN XBOXAPI RtlEqualString
(
	PSTRING String1,
	PSTRING String2,
	BOOLEAN CaseInSensitive
)
{
	BOOLEAN bRet = TRUE;

	USHORT l1 = String1->Length;
	USHORT l2 = String2->Length;
	if (l1 != l2) {
		return FALSE;
	}

	CHAR *p1 = String1->Buffer;
	CHAR *p2 = String2->Buffer;
	CHAR *last = p1 + l1;

	if (CaseInSensitive) {
		while (p1 < last) {
			CHAR c1 = *p1++;
			CHAR c2 = *p2++;
			if (c1 != c2) {
				c1 = RtlUpperChar(c1);
				c2 = RtlUpperChar(c2);
				if (c1 != c2) {
					return FALSE;
				}
			}
		}

		return TRUE;
	}

	while (p1 < last) {
		if (*p1++ != *p2++) {
			bRet = FALSE;
			break;
		}
	}

	return bRet;
}

EXPORTNUM(280) BOOLEAN XBOXAPI RtlEqualUnicodeString
(
    const PUNICODE_STRING String1,
    const PUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
)
{
	USHORT Len1 = String1->Length, Len2 = String2->Length;
	PWCHAR Str1 = String1->Buffer, Str2 = String2->Buffer;
	PWCHAR End = (PWCHAR)((PBYTE)Str1 + (Len1 < Len2 ? Len1 : Len2));

	if (Len1 != Len2) {
		return FALSE;
	}

	if (CaseInSensitive) {
		WCHAR Char1, Char2;
		WCHAR UpperChar1, UpperChar2;

		while (Str1 < End) {
			Char1 = *Str1++;
			Char2 = *Str2++;
			UpperChar1 = RtlpUpperWChar(Char1);
			UpperChar2 = RtlpUpperWChar(Char2);

			if ((Char1 != Char2) && (UpperChar1 != UpperChar2)) {
				return FALSE;
			}
		}
	} else {
		while (Str1 < End) {
			if (*Str1++ != *Str2++) {
				return FALSE;
			}
		}
	}

	return TRUE;
}
