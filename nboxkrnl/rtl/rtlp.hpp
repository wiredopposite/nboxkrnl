/*
 * wiredopposite          Copyright (c) 2026
 */

#pragma once

#include "..\types.hpp"

#define RtlpPoolTag 'grtS'

extern const UCHAR RtlpToUpper8[256];
extern const UCHAR RtlpToLower8[256];
extern const USHORT RtlpUpperL1[256];
extern const USHORT RtlpUpperL2[336];
extern const LONG RtlpUpperL3[1184];
extern const USHORT RtlpLowerL1[256];
extern const USHORT RtlpLowerL2[288];
extern const LONG RtlpLowerL3[1088];

WCHAR FASTCALL RtlpApply3Level(WCHAR Character, const USHORT* L1, const USHORT* L2, const LONG* L3);
LONG RtlpStrWCount(PCWSTR String);

#define RtlpUpperChar(Character) RtlpToUpper8[(UCHAR)Character]
#define RtlpLowerChar(Character) RtlpToLower8[(UCHAR)Character]
#define RtlpUpperWChar(Character) RtlpApply3Level(Character, RtlpUpperL1, RtlpUpperL2, RtlpUpperL3)
#define RtlpLowerWChar(Character) RtlpApply3Level(Character, RtlpLowerL1, RtlpLowerL2, RtlpLowerL3)
